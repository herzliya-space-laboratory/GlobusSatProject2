/*
 * Payload_Commands.c
 *
 *  Created on: 2 Jan 2025
 *      Author: maayan
 */
//int USE_PAYLOAD = FALSE;

#include "Payload_Commands.h"
#include <hal/supervisor.h>
#ifdef USE_PAYLOAD
/*
 * turn on the flag of kill payload and turn payload off
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int;	2 on write_to_fram problem
 * 						error according to SendAckPacket
 * */
int CMD_TurnOnKillPayloadFlag(sat_packet_t *cmd)
{
	Boolean true = TRUE;
	if(logError(FRAM_writeAndVerify((unsigned char*)&true, PAYLOAD_IS_DEAD_ADDR, PAYLOAD_IS_DEAD_SIZE), "CMD_TurnOnKillPayloadFlag - FRAM_writeAndVerify"))
	{
		unsigned char ackError = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	logError(payloadTurnOff(), "CMD_TurnOnKillPayloadFlag - payloadTurnOff");
	return SendAckPacket(ACK_KILL_PAYLOAD, cmd, NULL, 0);

}

/*
 * turn off the flag of kill payload and turn payload on
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int;	2 on write_to_fram problem
 * 						error according to SendAckPacket
 * */
int CMD_TurnOffKillPayloadFlag(sat_packet_t *cmd)
{
	Boolean false = FALSE;
	if(logError(FRAM_writeAndVerify((unsigned char*)&false, PAYLOAD_IS_DEAD_ADDR, PAYLOAD_IS_DEAD_SIZE), "CMD_TurnOffKillPayloadFlag - FRAM_writeAndVerify"))
	{
		unsigned char ackError = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG, cmd, &ackError, sizeof(ackError));
		return ackError;
	}
	payloadTurnOn();
	return SendAckPacket(ACK_TURN_ON_PAYLOAD, cmd, NULL, 0);
}
#endif
