/*
 * Payload_Commands.c
 *
 *  Created on: 2 Jan 2025
 *      Author: maayan
 */

#include "Payload_Commands.h"
#include <hal/supervisor.h>

int CMD_TurnOnKillPayloadFlag(sat_packet_t *cmd)
{
	Boolean true = TRUE;
	if(logError(FRAM_writeAndVerify((unsigned char*)&true, PAYLOAD_IS_DEAD_ADDR, PAYLOAD_IS_DEAD_SIZE), "CMD_TurnOnKillPayloadFlag - FRAM_writeAndVerify"))
	payloadTurnOff();
	return SendAckPacket(ACK_KILL_PAYLOAD, cmd, NULL, 0);

}

int CMD_TurnOffKillPayloadFlag(sat_packet_t *cmd)
{
	Boolean false = FALSE;
	if(logError(FRAM_writeAndVerify((unsigned char*)&false, PAYLOAD_IS_DEAD_ADDR, PAYLOAD_IS_DEAD_SIZE), "CMD_TurnOffKillPayloadFlag - FRAM_writeAndVerify"))
	payloadTurnOn();
	return SendAckPacket(ACK_TURN_ON_PAYLOAD, cmd, NULL, 0);
}
