/*
 * EPS_Commands.c
 *
 *  Created on: 21 Nov 2024
 *      Author: maayan
 */

#include "EPS_Commands.h"
#include <string.h>

/*
 * Set new alpha value.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and the new alpha val)
* @return type=int; return type of error
* 										-1 on cmd NULL
* 										errors according to "AckErrors.h"
 * */
int CMD_UpdateAlpha(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	int error_ack = 0;
	if(cmd->length != 4)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	}
	float alpha;
	memcpy(&alpha, cmd->data, cmd->length);
	int error = UpdateAlpha(alpha);
	switch(error)
	{
	 case -1:
	 {
		error_ack = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	 }
	 case -2:
	 {
		error_ack = INVALID_ALPHA;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	 }
	 case -4:
	 {
		error_ack = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	 }
	 case -5:
	 {
		error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	 }
	 default:
		 break;
	}
	SendAckPacket(ACK_UPDATE_EPS_ALPHA , cmd, NULL, 0);
	return 0;
}

/*
 * Set default alpha value.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error
* 										-1 on cmd NULL
* 										errors according to "AckErrors.h"
 * */
int CMD_RestoreDefaultAlpha(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	cmd->length = 4;
	int alpha = DEFAULT_ALPHA_VALUE;
	memcpy(cmd->data, (unsigned char*)&alpha, cmd->length);
	return CMD_UpdateAlpha(cmd);
}

/*
 * Get alpha value.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error
* 										-2 on FRAM_read error, and TransmitDataAsSPL_Packet errors
 * */
int CMD_GetAlpha(sat_packet_t *cmd)
{
	float alpha;
	int error = GetAlpha(&alpha);
	if(error == -2)
	{
		int error_ack = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	}
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&alpha, EPS_ALPHA_FILTER_VALUE_SIZE), "CMD_GetSmoothingFactor - TransmitDataAsSPL_Packet"); // Send back the beacon interval
}
