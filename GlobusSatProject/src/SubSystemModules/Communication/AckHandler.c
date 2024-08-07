/*
 * AckHandler.c
 *
 *  Created on: 23 7 2024
 *      Author: maayan
 */

#include "AckHandler.h"
#include "TRXVU.h"

int SendAckPacket(ack_subtype_t acksubtype, sat_packet_t *cmd, unsigned char *data, unsigned short length)
{
	sat_packet_t cmd1;
	if(cmd != NULL)
		logError(AssembleCommand(data, length, ack_type, acksubtype, cmd->ID, &cmd1), "Packets - Assemble command");
	else
		logError(AssembleCommand(data, length, ack_type, acksubtype, CUBE_SAT_ID, &cmd1), "Packets - Assemble command");
	int avalFrames;

	return logError(TransmitSplPacket(&cmd1, &avalFrames), "TRXVU - IsisTrxvu_tcSendAX25DefClSign");
}
