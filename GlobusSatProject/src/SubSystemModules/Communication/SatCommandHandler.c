/*
 * SatCommandHandler.c
 *
 *  Created on: 26 6 2024
 *      Author: Maayan
 */

#include "SatCommandHandler.h"
#include <string.h>


CMD_ERR ParseDataToCommand(unsigned char * data, sat_packet_t *cmd)
{
	if(data != NULL && cmd != NULL)
	{
		if(memcpy(&cmd->ID, data, 4) == NULL)
			return null_pointer_error;

		if(memcpy(&cmd->cmd_type, data + 4, 1) == NULL)
			return null_pointer_error;

		if(memcpy(&cmd->cmd_subtype, data + 5, 1) == NULL)
			return null_pointer_error;

		if(memcpy(&cmd->length, data + 6, 2) == NULL)
			return null_pointer_error;

		if(memcpy(cmd->data, data + 8, MAX_COMMAND_DATA_LENGTH) == NULL)
			return null_pointer_error;

		return command_succsess;
	}
	return null_pointer_error;
}


