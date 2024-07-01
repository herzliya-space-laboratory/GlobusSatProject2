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

		if(memcpy(&cmd->data, data + 8, cmd->length) == NULL)
			return null_pointer_error;

		return command_succsess;
	}
	return null_pointer_error;
}


CMD_ERR AssembleCommand(unsigned char *data, unsigned short data_length, char type, char subtype, unsigned int id, sat_packet_t *cmd)
{
	if(data != NULL && cmd != NULL)
	{
		cmd->ID = id;
		cmd->cmd_type = type;
		cmd->cmd_subtype = subtype;
		cmd->length = data_length;
		if(data_length <= MAX_COMMAND_DATA_LENGTH)
		{
			memcpy(&cmd->data, data, data_length);
		}
		else
			return index_out_of_bound;

		return command_succsess;
	}
	return null_pointer_error;
}
