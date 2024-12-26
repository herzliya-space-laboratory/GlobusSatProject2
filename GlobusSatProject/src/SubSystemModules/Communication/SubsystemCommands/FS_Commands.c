/*
 * FS_Commands.c
 *
 *  Created on: 26 Dec 2024
 *      Author: maayan
 */

#include "FS_Commands.h"

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
