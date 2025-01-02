/*
 * Payload_Commands.c
 *
 *  Created on: 2 Jan 2025
 *      Author: maayan
 */

#include "Payload_Commands.h"

int CMD_TurnOnKillPayloadFlag(sat_packet_t *cmd)
{
	uint8_t one = 1;
	if(logError(FRAM_writeAndVerify((unsigned char*)&one, PAYLOAD_IS_DEAD_ADDR, PAYLOAD_IS_DEAD_SIZE), "CMD_TurnOnKillPayloadFlag - FRAM_writeAndVerify"))
	payloadOff();
	return SendAckPacket(ACK_KILL_PAYLOAD, cmd, NULL, 0);

}

int CMD_TurnOffKillPayloadFlag(sat_packet_t *cmd)
{
	int zero = 0;
	if(logError(FRAM_writeAndVerify((unsigned char*)&zero, PAYLOAD_IS_DEAD_ADDR, PAYLOAD_IS_DEAD_SIZE), "CMD_TurnOffKillPayloadFlag - FRAM_writeAndVerify"))
	payloadOn();
	return SendAckPacket(ACK_TURN_ON_PAYLOAD, cmd, NULL, 0);
}
