/*
 * FS_Commands.c
 *
 *  Created on: 26 Dec 2024
 *      Author: maayan
 */

#include "FS_Commands.h"
#include "String.h"
dump_arguments_t arg;

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
	arg.cmd = *cmd;
	memcpy((unsigned char*)&arg.dump_type, cmd->data, 1);
	memcpy((unsigned char*)&arg.t_start, cmd->data + 1, sizeof(time_unix));
	memcpy((unsigned char*)&arg.t_end, cmd->data + 5, sizeof(time_unix));

	if(xSemaphoreTake(semaphorDump, SECONDS_TO_TICKS(WAIT_TIME_SEM_DUMP)) == pdFALSE)
	{
		ackError = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	xTaskHandle taskHandle;
	xTaskGenericCreate(TackDump, (const signed char*) "CMD_StartDump", 4096, &arg, configMAX_PRIORITIES - 2, &taskHandle, NULL, NULL);
	return 0;
}

void TackDump(void *dump)
{
	if(dump == NULL) return;
	dump_arguments_t *dump_arg = (dump_arguments_t*)dump;

	f_enterFS();
	int numOfDays = (dump_arg->t_end - dump_arg->t_start) / 24 / 3600;
	Time start;
	timeU2time(dump_arg->t_start, &start);
	SendAckPacket(ACK_DUMP_START, &dump_arg->cmd, NULL, 0);
	ReadTLMFiles(dump_arg->dump_type, start, numOfDays, dump_arg->cmd.ID);
	SendAckPacket(ACK_DUMP_FINISHED, &dump_arg->cmd, NULL, 0);
	f_releaseFS();
	xSemaphoreGive(semaphorDump);
	vTaskDelete(NULL);
}

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


int CMD_GetLastFS_Error(sat_packet_t *cmd)
{
	int FS_error = f_getlasterror();
	return SendAckPacket(ACK_FS_LAST_ERROR, cmd, (unsigned char*)&FS_error, sizeof(FS_error));
}


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
	return 0;
}

/**
 * set a new periodTime
 */
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

//TODO: switch sd card. for that I need to create DeInitializeFS (TODO)
