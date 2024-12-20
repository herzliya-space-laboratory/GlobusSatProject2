/*
 * TLM_management.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include "TLM_management.h"

#include <hcc/api_fat.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <hcc/api_mdriver.h>
#include <hcc/api_hcc_mem.h>
#include "SubSystemModules/Housekeeping/TelemetryCollector.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Payload/payload_drivers.h"
#include <string.h>

#include "utils.h"

FileSystemResult InitializeFS(){
	int flag = 0;
	// in FS init we don't want to use a log file !
		// Initialize the memory for the FS
	int err = hcc_mem_init();
	if (err != E_NO_SS_ERR){
		printf("FS - hcc_mem_init Error - %d ", err);
		flag = 1;
	}
	// Initialize the FS
	err = fs_init();
	if (err != E_NO_SS_ERR){
		printf("FS - fs_init Error - %d ", err);
		flag = 1;
	}

	fs_start();

	// Tell the OS (freeRTOS) about our FS
	err = f_enterFS();
	if (err != E_NO_SS_ERR){
		printf("FS - f_enterFS Error - %d ", err);
		flag = 1;
	}

	// Initialize the volume of SD card 0 (A)
	err = f_initvolume( 0, atmel_mcipdc_initfunc, 0 );
	if (err != E_NO_SS_ERR){
	// error init SD 0 so de-itnit and init SD 1
	//printf("f_initvolume primary error:%d\n",err);
		logError(hcc_mem_init(), "FS - hcc_mem_init - 2");
		logError(fs_init(), "FS - fs_init - 2");
		logError(f_enterFS(), "FS - fs_enterFS - 2");
		err = f_initvolume( 0, atmel_mcipdc_initfunc, 1 );
		if (err != E_NO_SS_ERR){
				//printf("f_initvolume secondary error:%d\n",err);
		}
	}
	if(flag)
		return FS_FAIL;

	return FS_SUCCSESS;
}

void calculateFileName(Time curr_date,char* file_name, char* endFileName, int days2Add)
{
	/* initialize */
	struct tm t = { .tm_year = curr_date.year + 100, .tm_mon = curr_date.month - 1, .tm_mday = curr_date.date }; //TODO: ask for explanation
	/* modify */
	t.tm_mday += days2Add;
	mktime(&t);

	char file_buff[7];

	strftime(file_buff, sizeof file_buff, "%y%0m%0d", &t);

	sprintf(file_name, "%s.%s" ,file_buff, endFileName);
}

void getTlmTypeInfo(tlm_type_t tlmType, char* endFileName, int* structSize)
{
	switch(tlmType)
	{
		case tlm_tx:
		{
			memcpy(endFileName, END_FILE_NAME_TX, sizeof(END_FILE_NAME_TX));
			*structSize = sizeof(ISIStrxvuTxTelemetry);
			break;
		}
		case tlm_rx:
		{
			memcpy(endFileName, END_FILE_NAME_RX, sizeof(END_FILE_NAME_RX));
			*structSize = sizeof(ISIStrxvuRxTelemetry);
			break;
		}
		case tlm_antenna:
		{
			memcpy(endFileName,END_FILE_NAME_ANTENNA,sizeof(END_FILE_NAME_ANTENNA));
			*structSize = sizeof(ISISantsTelemetry);
			break;
		}
		case tlm_eps_raw_mb_NOT_USED:
		{
			memcpy(endFileName,END_FILENAME_EPS_RAW_MB_TLM,sizeof(END_FILENAME_EPS_RAW_MB_TLM));
			*structSize = sizeof(imepsv2_piu__gethousekeepingraw__from_t);
			break;

		}
		case tlm_eps_raw_cdb_NOT_USED:
		{
			memcpy(endFileName,END_FILENAME_EPS_RAW_CDB_TLM,sizeof(END_FILENAME_EPS_RAW_CDB_TLM));
			*structSize = sizeof(imepsv2_piu__gethousekeepingrawincdb__from_t);
			break;
		}
		case tlm_eps:
		{
			memcpy(endFileName,END_FILENAME_EPS_TLM,sizeof(END_FILENAME_EPS_TLM));
			*structSize = sizeof(imepsv2_piu__gethousekeepingeng__from_t);
			break;
		}
		case tlm_eps_eng_cdb_NOT_USED:
		{
			memcpy(endFileName,END_FILENAME_EPS_ENG_CDB_TLM,sizeof(END_FILENAME_EPS_ENG_CDB_TLM));
			*structSize = sizeof(imepsv2_piu__gethousekeepingrunningavg__from_t);
			break;
		}
		case tlm_eps_eng_mb_NOT_USED:
		{
			memcpy(endFileName,END_FILENAME_EPS_ENG_CDB_TLM,sizeof(END_FILENAME_EPS_ENG_CDB_TLM));
			*structSize = sizeof(imepsv2_piu__gethousekeepingengrunningavgincdb__from_t);
			break;
		}
		case tlm_wod:
		{
			memcpy(endFileName,END_FILENAME_WOD_TLM,sizeof(END_FILENAME_WOD_TLM));
			*structSize = sizeof(WOD_Telemetry_t);
			break;
		}
		case tlm_solar:
		{
			memcpy(endFileName,END_FILENAME_SOLAR_PANELS_TLM,sizeof(END_FILENAME_SOLAR_PANELS_TLM));
			*structSize = sizeof(solar_tlm_t);
			break;
		}
		case tlm_log:
		{
			memcpy(endFileName,END_FILENAME_LOGS,sizeof(END_FILENAME_LOGS));
			*structSize = sizeof(logData_t);
			break;
		}
		case tlm_radfet:
		{
			memcpy(endFileName,END_FILENAME_RADFET_TLM,sizeof(END_FILENAME_RADFET_TLM));
			*structSize = sizeof(PayloadEnvironmentData);
			break;
		}
		case tlm_sel:
		{
			memcpy(endFileName,END_FILENAME_SEL_TLM,sizeof(END_FILENAME_SEL_TLM));
			*structSize = sizeof(int);
			break;
		}
		case tlm_seu:
		{
			memcpy(endFileName,END_FILENAME_SEU_TLM,sizeof(END_FILENAME_SEU_TLM));
			*structSize = sizeof(int);
			break;
		}
		default:
			break;
	}
}

int write2File(void* data, tlm_type_t tlmType)
{
	time_unix timeNow;
	if(Time_getUnixEpoch((unsigned int*)&timeNow)) return -2;

	Time currDate;
	if(Time_get(&currDate)) return -3;
	char endFile[3];
	int structSize;
	getTlmTypeInfo(tlmType, endFile, &structSize);

	char fileName[MAX_FILE_NAME_SIZE] = {0};
	calculateFileName(currDate , fileName, endFile, 0);
	F_FILE *fp =  f_open(fileName, "a");
	if(!fp)
	{
		printf("hi, we have an error in opening the file");
		return -1;
	}
	f_write(&timeNow , sizeof(timeNow) ,1, fp );
	f_write(data , structSize , 1, fp );

	/* close the file*/
	f_flush(fp);
	f_close (fp);
	return 0;
}
