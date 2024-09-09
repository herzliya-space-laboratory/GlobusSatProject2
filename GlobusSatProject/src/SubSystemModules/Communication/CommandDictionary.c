/*
 * CommandDictionary.c
 *
 *  Created on: 9 7 2024
 *      Author: Maayan
 */


#include "CommandDictionary.h"
#include <stdio.h>
#include "TRXVU.h"
#include "SubsystemCommands/EPS_Commands.h"
#include "utils.h"
int trxvu_command_router(sat_packet_t *cmd)
{
	if(cmd == NULL)
	{
		printf("cmd_is_null\r\n");
		return -1;
	}
	return TransmitDataAsSPL_Packet(cmd, (unsigned char *)"hello world", sizeof("hello world"));
}

int eps_command_router(sat_packet_t *cmd)
{
	if(cmd == NULL)
	{
		logError(-1, "cmd_is_null\r\n");
		return -1;
	}
	int error = 0;
	switch (cmd->cmd_subtype) {
		case (UPDATE_ALPHA): {
			 error = CMD_UpdateSmoothingFactor(cmd);
			break;
		}
		case (GET_ALPHA): {
				error = CMD_GetSmoothingFactor();
				break;
		}
		case (UPDATE_ALPHA_DEFAULT): {
			error = CMD_RestoreDefaultAlpha();
			break;
		}
		case (UPDATE_THRESHOLD): {
			error = CMD_UpdateThresholdVoltages(cmd);
			break;
		}
		case (GET_THRESHOLD): {
			error = CMD_GetThresholdVoltages(cmd);
			break;
		}
		case (UPDATE_THRESHOLD_DEFAULT): {
			error = CMD_RestoreDefaultAlpha();
			break;
		}
		case (SET_MODE): {
			error = CMD_EPSSetMode(cmd);
			break;
		}
		case (GET_MODE): {
			error = CMD_GetCurrentMode(cmd);
			break;
		}
		case (GET_STATE_CHANGES_OVERTIME): {
			error = CMD_GET_STATE_CHANGES(cmd);
			break;
		}
	}
	return error;
}
