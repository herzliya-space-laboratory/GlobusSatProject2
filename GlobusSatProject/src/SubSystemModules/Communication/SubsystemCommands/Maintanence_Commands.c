/*
 * Maintanence_Commands.c
 *
 *  Created on: 3 Dec 2024
 *      Author: maayan
 */

#include "Maintanence_Commands.h"

/*
 * Get the time save on the sat at the sat.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error 0 on success.
 * */
int CMD_GetSatTime(sat_packet_t *cmd)
{
	time_unix timeNow;
	int error = logError(Time_getUnixEpoch((unsigned int*)&timeNow), "CMD_GetSatTime - Time_getUnixEpoch");
	if(error)
	{
		int ackError = ERROR_CANT_GET_TIME; //TODO: maybe change to char
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&timeNow, sizeof(time_unix)), "CMD_GetSatTime - TransmitDataAsSPL_Packet"); //send back the sat time
}
