/*
 * AckHandler.c
 *
 *  Created on: 23 7 2024
 *      Author: maayan
 */

#include "AckHandler.h"
#include "TRXVU.h"

/*!
 * @brief transmits an ack packet as an SPL packet(using the SPL protocol)
 * @param[in] name=acksubtype; type=ack_subtype_t; type of ack to be sent according to ack_subtype_t enumeration
 * @param[in] name=cmd; type=sat_packet_t*; the command for which the ACK is a response to. CAN BE NULL!
 * @param[in] name=data; type=unsigned char*; the data we want to send with the ack. CAN BE NULL!
 * @param[in] name=length; type=unsigned short; the length of the data we want to send in bytes
 * @return errors according t <hal/errors.h>
 */
int SendAckPacket(ack_subtype_t acksubtype, sat_packet_t *cmd, unsigned char *data, unsigned short length)
{
	sat_packet_t cmd1;
	if(cmd != NULL)
		logError(AssembleCommand(data, length, ack_type, acksubtype, cmd->ID, &cmd1), "SendAckPacket - AssembleCommand"); // Assemble new cmd1 stract according to data, length and cmd
	else
		logError(AssembleCommand(data, length, ack_type, acksubtype, CUBE_SAT_ID, &cmd1), "SendAckPacket - AssembleCommand"); // Assemble new cmd1 struct according to data, length. cmd is null.
	int avalFrames;

	return logError(TransmitSplPacket(&cmd1, &avalFrames), "SendAckPacket - TransmitSplPacket"); // Send ack and return error if have else 0
}
