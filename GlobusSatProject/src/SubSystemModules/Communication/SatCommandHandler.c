/*
 * SatCommandHandler.c
 *
 *  Created on: 26 6 2024
 *      Author: Maayan
 */

#include "SatCommandHandler.h"
#include <string.h>

/*
 * Change the packet we get to sat_packet_t struct
 *
 * @param[in] name= data; type= unsigned char *; The packet we get after reading from the frame (without the headers of the Ax25 protocol).
 * @param[out] name= cmd; type= sat_packet_t *; The struct we put all the info we get from the packet.
 *
 * */
CMD_ERR ParseDataToCommand(unsigned char * data, sat_packet_t *cmd)
{
	if(data == NULL || cmd == NULL) // check none of the pointers is null
		return null_pointer_error;

	int plusPlace = 0; // get a counter for the start place of the data we read
	if(memcpy(&cmd->ID, data, sizeof(cmd->ID)) == NULL) // check the memcpy worked and put in the cmd->ID the id of the sat and the other stuff there
		return null_pointer_error;
	plusPlace += sizeof(cmd->ID); // add the size of the ID to the variable

	if(cmd->ID>>24 != CUBE_SAT_ID) // check we really are the sat it was send to.
		return invalid_sat_id;

	if(memcpy(&cmd->cmd_type, data + plusPlace, sizeof(cmd->cmd_type)) == NULL) // same just the cmd_type.
		return null_pointer_error;
	plusPlace += sizeof(cmd->cmd_type);

	if(memcpy(&cmd->cmd_subtype, data + plusPlace, sizeof(cmd->cmd_subtype)) == NULL) // same just the cmd_subtype.
		return null_pointer_error;
	plusPlace += sizeof(cmd->cmd_subtype);

	if(memcpy(&cmd->length, data + plusPlace, sizeof(cmd->length)) == NULL) // same just the length.
		return null_pointer_error;
	plusPlace += sizeof(cmd->length);

	if(memcpy(&cmd->data, data + plusPlace, cmd->length) == NULL) // same just to the data which is the length that equal to the value of the variable length.
		return null_pointer_error;


	return command_succsess;
}


CMD_ERR AssembleCommand(unsigned char *data, unsigned short data_length, char type, char subtype, unsigned int id, sat_packet_t *cmd)
{
	if(data == NULL || cmd == NULL)
		return null_pointer_error;

	cmd->ID = id;
	cmd->cmd_type = type;
	cmd->cmd_subtype = subtype;
	cmd->length = data_length;
	if(data_length > MAX_COMMAND_DATA_LENGTH)
		return index_out_of_bound;

	memcpy(&cmd->data, data, data_length);

	return command_succsess;
}
