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
		int plusPlace = 0;
		if(memcpy(&cmd->ID, data, sizeof(cmd->ID)) == NULL)
			return null_pointer_error;
		plusPlace += sizeof(cmd->ID);

		if(memcpy(&cmd->cmd_type, data + plusPlace, sizeof(cmd->cmd_type)) == NULL)
			return null_pointer_error;
		plusPlace += sizeof(cmd->cmd_type);

		if(memcpy(&cmd->cmd_subtype, data + plusPlace, sizeof(cmd->cmd_subtype)) == NULL)
			return null_pointer_error;
		plusPlace += sizeof(cmd->cmd_subtype);

		if(memcpy(&cmd->length, data + plusPlace, sizeof(cmd->length)) == NULL)
			return null_pointer_error;
		plusPlace += sizeof(cmd->length);

		if(memcpy(&cmd->data, data + plusPlace, cmd->length) == NULL)
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
