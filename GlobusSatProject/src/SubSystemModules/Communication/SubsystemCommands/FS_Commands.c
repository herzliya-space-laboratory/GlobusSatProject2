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
		return SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char*)&ackError, sizeof(ackError));
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
		SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char*)&ackError, sizeof(ackError));
		return ackError;
	}
	arg.cmd = *cmd;
	memcpy((unsigned char*)&arg.dump_type, cmd->data, 1);
	memcpy((unsigned char*)&arg.t_start, cmd->data + 1, sizeof(time_unix));
	memcpy((unsigned char*)&arg.t_end, cmd->data + 5, sizeof(time_unix));

	xTaskHandle taskHandle;
	xTaskGenericCreate(TackDump, (const signed char*) "CMD_StartDump", 4096, &arg, configMAX_PRIORITIES - 2, &taskHandle, NULL, NULL);
	return 0;
}

void TackDump(void *dump)
{
	if(dump == NULL) return;
	dump_arguments_t *dump_arg = (dump_arguments_t*)dump;


	int numOfDays = (dump_arg->t_end - dump_arg->t_start) / 24 / 3600;
	Time start;
	timeU2time(dump_arg->t_start, &start);
	ReadTLMFiles(dump_arg->dump_type, start, numOfDays, dump_arg->cmd.ID);
	vTaskDelete(NULL);
}
