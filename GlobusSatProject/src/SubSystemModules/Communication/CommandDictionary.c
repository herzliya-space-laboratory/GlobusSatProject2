/*
 * CommandDictionary.c
 *
 *  Created on: 9 7 2024
 *      Author: Maayan
 */

#include "SubsystemCommands/TRXVU_Commands.h"
#include "CommandDictionary.h"
#include <stdio.h>

int trxvu_command_router(sat_packet_t *cmd)
{
	if(cmd == NULL)
	{
		printf("cmd_is_null\r\n");
		return -1;
	}
	switch(cmd->cmd_subtype)
	{
		case GET_BEACON_INTERVAL:
			return CMD_GetBeacon_Interval(cmd);
		case SET_BEACON_INTERVAL:
			return CMD_SetBeacon_Interval(cmd);
		case SET_OFF_TRANSPONDER:
			return CMD_SetOff_Transponder(cmd);
		default:
		{
			unsigned char unknownSubtype_msg[] = "TRXVU - unknown subtype";
			return logError(SendAckPacket(ACK_UNKNOWN_SUBTYPE, cmd, unknownSubtype_msg, sizeof(unknownSubtype_msg)), "trxvu_command_router - SendAckPacket invalid subtype");
		}
	}
}
