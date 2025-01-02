/*
 * FS_Commands.c
 *
 *  Created on: 26 Dec 2024
 *      Author: maayan
 */

#include "FS_Commands.h"
#include "String.h"
dump_arguments_t arg;

/*
 * formating the SD card.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int; return type of error according to SendAckPacket and Hard_ComponenetReset errors
 * */
int CMD_DeleteAllFiles(sat_packet_t *cmd)
{
	if(Delete_allTMFilesFromSD())
	{
		unsigned char ackError = ERROR_CANT_DO;
		return SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
	}
	SendAckPacket(ACK_DELETE_TLM, cmd, NULL, 0);
	return Hard_ComponenetReset();
}

/*
 * start dump according to days and create the task of dump
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and tlmType, start date, end date)
 * @return type=int; -1 cmd null
 * 					 17 wrong data length
 * 					 30 Already have a dump active (can't do two at the same time)
 * 					 0 on success
 *
 * */
int CMD_StartDump(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	unsigned char ackError = 0;
	if(cmd->length != 9)
	{
		ackError = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	//get the param to give to dump task
	arg.cmd = *cmd;
	memcpy((unsigned char*)&arg.dump_type, cmd->data, 1);
	memcpy((unsigned char*)&arg.t_start, cmd->data + 1, sizeof(time_unix));
	memcpy((unsigned char*)&arg.t_end, cmd->data + 5, sizeof(time_unix));

	//check we not already in dump
	if(xSemaphoreTake(semaphorDump, SECONDS_TO_TICKS(WAIT_TIME_SEM_DUMP)) == pdFALSE)
	{
		ackError = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	xTaskHandle taskHandle;
	xTaskGenericCreate(TackDump, (const signed char*) "CMD_StartDump", 4096, &arg, configMAX_PRIORITIES - 2, &taskHandle, NULL, NULL); //create dump task
	return 0;
}

/*
 * abort dump. (stops the dump)
 * @return type=int; according to xQueueSend errors
 * */
int CMD_SendDumpAbortRequest()
{
	Boolean true = TRUE;
	return xQueueSend(queueAbortDump, &true, (portTickType) 10); //enter to the queue true for the dump to stop
}

/*
 * the function start dump call for the task. have the dump final logic.
 * @param[in] name=dump; type=void*; have the parameters we need for the dump.
 * */
void TackDump(void *dump)
{
	if(dump == NULL) return;
	dump_arguments_t *dump_arg = (dump_arguments_t*)dump;

	f_enterFS();
	int numOfDays = (dump_arg->t_end - dump_arg->t_start) / 24 / 3600; //get num of days
	Time start;
	timeU2time(dump_arg->t_start, &start); //change from time_unix to Time
	SendAckPacket(ACK_DUMP_START, &dump_arg->cmd, NULL, 0);
	ReadTLMFiles(dump_arg->dump_type, start, numOfDays, dump_arg->cmd.ID); //sends the dump packets
	SendAckPacket(ACK_DUMP_FINISHED, &dump_arg->cmd, NULL, 0);
	f_releaseFS();
	xSemaphoreGive(semaphorDump); //release the dump semaphor
	vTaskDelete(NULL); //Delete the task
}

/*
 * Delete tlm files from sd according to tlmType, start date, end date
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and tlmType, start date, end date)
 * @return type=int; -1 cmd null
 * 					 17 wrong data length
 * 					 60 didn't delete all files
 * 					 according to SendAckPacket errors.
 * 					 0 on success
 *
 * */
int CMD_DeleteTLM(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	unsigned char ackError = 0;
	if(cmd->length != 9)
	{
		ackError = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	tlm_type_t type;
	time_unix start;
	time_unix end;
	memcpy((unsigned char*)&type, cmd->data, 1);
	memcpy((unsigned char*)&start, cmd->data + 1, sizeof(time_unix));
	memcpy((unsigned char*)&end, cmd->data + 5, sizeof(time_unix));
	Time start_t;
	timeU2time(start, &start_t);
	int numOfDays = (end - start) / 24 / 3600;
	int error = logError(DeleteTLMFiles(type, start_t, numOfDays), "CMD_DeleteFilesOfType - DeleteTLMFiles");
	if(error != numOfDays + 1)
	{
		ackError = ERROR_COULDNT_DELETE_ALL;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	return SendAckPacket(ACK_DELETE_TLM, cmd, NULL, 0);
}

/*
 * Get last FS error
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int; according to SendAckPacket
 * */
int CMD_GetLastFS_Error(sat_packet_t *cmd)
{
	int FS_error = f_getlasterror();
	return SendAckPacket(ACK_FS_LAST_ERROR, cmd, (unsigned char*)&FS_error, sizeof(FS_error));
}

/*
 * Get free space on the sd in bytes <- it's not precise but it's the best we got
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int;	27 on error get from struct
 * 						according to TransmitDataAsSPL_Packet
 * */
int CMD_FreeSpace(sat_packet_t *cmd)
{
	F_SPACE space; //FS struct
	int error = logError(f_getfreespace(f_getdrive(), &space), "CMD_FreeSpace - f_getfreespace"); //gets the variables to the struct
	if(error)
	{
		unsigned char ackError = ERROR_GET_FROM_STRUCT;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&space.free, sizeof(space.free)), "CMD_FreeSpace - TransmitDataAsSPL_Packet");
}

/*
 * Help function to CMD_SetTLMPeriodTimes. sets the new one according to the params we got
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @param[in] name=addr; type=unsigned int; get the place in FRAM we need to write to.
 * @param[in] name=saveTime; type=unsigned int; the new period time we want to save.
 * @return type=int;	1 on read from FRAM
 * 						2 on write to FRAM
 * 						3 on written wrong in FRAM
 * 						0 on success
 * */
int SaveAndCheck(unsigned int addr, unsigned int saveTime, sat_packet_t* cmd)
{
	unsigned char ackError = 0;
	int error = logError(FRAM_writeAndVerify((unsigned char*)&saveTime, addr, sizeof(saveTime)), "CMD_SetTLMPeriodTimes - FRAM_writeAndVerify");
	if(error)
	{
		ackError = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	unsigned int check = 0;
	error = logError(FRAM_read((unsigned char*)&check, addr, sizeof(saveTime)), "SaveAndCheck - FRAM_read");
	if(error)
	{
		ackError = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	if(check != saveTime)
	{
		ackError = ERROR_WRITTEN_IN_FRAM_WRONG;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	InitSavePeriodTimes();
	return 0;
}

/*
 * Set new period of saving telemetry according to tlmPeriod and new period
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and tlmPeriod, new period)
 * @return type=int; -1 cmd null
 * 					 17 wrong data length
 * 					 error according to SaveAndCheck
 * 					 30 invalid tlmPeriod
 * */
int CMD_SetTLMPeriodTimes(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	unsigned char ackError = 0;
	if(cmd->length != 5)
	{
		ackError = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	char type;
	unsigned int saveTime;
	memcpy(&type, cmd->data, 1);
	memcpy(&saveTime, cmd->data + 1, sizeof(int));
	int error;
	switch(type)
	{
		case 0: //eps
			error = SaveAndCheck(EPS_SAVE_TLM_PERIOD_ADDR, saveTime, cmd);
			if(error)
				return error;
			break;
		case 1: //trx
			error = SaveAndCheck(TRXVU_SAVE_TLM_PERIOD_ADDR, saveTime, cmd);
			if(error)
				return error;
			break;
		case 2: //ants
			error = SaveAndCheck(ANT_SAVE_TLM_PERIOD_ADDR, saveTime, cmd);
			if(error)
				return error;
			break;
		case 3: //sp
			error = SaveAndCheck(SOLAR_SAVE_TLM_PERIOD_ADDR, saveTime, cmd);
			if(error)
				return error;
			break;
		case 4: //wod
			error = SaveAndCheck(WOD_SAVE_TLM_PERIOD_ADDR, saveTime, cmd);
			if(error)
				return error;
			break;
		case 5: //rad
			error = SaveAndCheck(RADFET_SAVE_TLM_PERIOD_ADDR, saveTime, cmd);
			if(error)
				return error;
			break;
		case 6: //seu_sel
			error = SaveAndCheck(SEU_SEL_SAVE_TLM_PERIOD_ADDR, saveTime, cmd);
			if(error)
				return error;
			break;
		default:
			ackError = ERROR_CANT_DO;
			SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
			return ackError;
	}
	SendAckPacket(ACK_SET_NEW_TLM_PERIOD, cmd, NULL, 0);
	return 0;

}

/*
 * Set default period of saving telemetry
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int;	according to FRAM_writeAndVerify
 * */
int CMD_SetTLMPeriodTimes_default(sat_packet_t *cmd)
{
	int arrPeriod[7] = {DEFAULT_EPS_SAVE_TLM_TIME, DEFAULT_TRXVU_SAVE_TLM_TIME, DEFAULT_ANT_SAVE_TLM_TIME, DEFAULT_SOLAR_SAVE_TLM_TIME, DEFAULT_WOD_SAVE_TLM_TIME, DEFAULT_RADFET_SAVE_TLM_TIME, DEFAULT_SEU_SEL_SAVE_TLM_TIME};
	int error = logError(FRAM_writeAndVerify((unsigned char*)arrPeriod, TLM_SAVE_PERIOD_START_ADDR, sizeof(arrPeriod)), "CMD_SetTLMPeriodTimes_default - FRAM_writeAndVerify");
	if(error)
	{
		unsigned char ackError = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return error;
	}
	InitSavePeriodTimes();
	SendAckPacket(ACK_SET_NEW_TLM_PERIOD, cmd, NULL, 0);
	return 0;
}

/*
 * Get telemetry period times from FRAM
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int;	according to FRAM_read and TransmitDataAsSPL_Packet
 * */
int CMD_GetTLMPeriodTimes(sat_packet_t *cmd)
{
	int error = 0;
	int arrPeriod[7] = {0};
	error = logError(FRAM_read((unsigned char*)arrPeriod, TLM_SAVE_PERIOD_START_ADDR, sizeof(arrPeriod)), "CMD_GetTLMPeriodTimes - FRAM_read");
	if(error)
	{
		unsigned char ackError = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return error;
	}
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)arrPeriod, sizeof(arrPeriod)), "CMD_GetTLMPeriodTimes - TransmitDataAsSPL_Packet");
}

/*
 * Switch between the sd cards.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and sd to switch to)
 * @return type=int;	-1 cmd null
 * 						27 wrong data length
 * 						-4 invalid sd
 * 						-2 read from FRAM
 * 						-3 write to FRAM
 * 						according to Hard_ComponenetReset
 * */
int CMD_SwitchSD_card(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	unsigned char ackError = 0;
	if(cmd->length != 1)
	{
		ackError = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	uint8_t newSD = cmd->data[0];
	uint8_t oldSD;
	if(newSD != 1 && newSD != 0)
	{
		unsigned char ackError = ERROR_NOT_VALID_SD;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return -4;
	}
	if(logError(FRAM_read((unsigned char*)&oldSD, SD_CARD_USED_ADDR, SD_CARD_USED_SIZE), "CMD_SwitchSD_card - FRAM_read"))
	{
		unsigned char ackError = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return -2;
	}
	if(oldSD == newSD)
	{
		unsigned char ackError = ERROR_SAME_SD_WE_USE;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return 0;
	}
	if(logError(FRAM_writeAndVerify((unsigned char*)&newSD, SD_CARD_USED_ADDR, SD_CARD_USED_SIZE), "CMD_SwitchSD_card - FRAM_writeAndVerify"))
	{
		unsigned char ackError = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return -3;
	}
	SendAckPacket(ACK_SWITCHING_SD_CARD, cmd, NULL, 0);
	return Hard_ComponenetReset();
}


int CMD_DeleteFilesOfType(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	unsigned char ackError = 0;
	if(cmd->length != 1)
	{
		ackError = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	uint8_t type;
	memcpy(&type, cmd->data, 1);
	if(type < 0 || type > 10)
	{
		ackError = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	char endFile[3] = {0};
	int structNotNeeded;
	GetTlmTypeInfo(type, endFile, &structNotNeeded);
	F_FIND find;
	char fileFind[5] = {0};
	sprintf(fileFind, "*.%s", endFile);
	f_findfirst(fileFind, &find);
	do
	{
		f_delete(find.filename);
	}
	while(!f_findnext(&find));
	return 0;
}
