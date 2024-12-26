/*
 * SatCommandHandler.c
 *
 *  Created on: 26 6 2024
 *      Author: Maayan
 */

#include <hal/errors.h>

#include "SatCommandHandler.h"
#include "CommandDictionary.h"
#include "AckHandler.h"

#include "SPL.h"
#include "utils.h"

#include <string.h>

/*
 * Change the packet we get to sat_packet_t struct
 *
 * @param[in] name= data; type= unsigned char *; The packet we get after reading from the frame (without the headers of the Ax25 protocol).
 * @param[out] name= cmd; type= sat_packet_t *; The struct we put all the info we get from the packet.
 * @return error according to CMD_ERR
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

	if(cmd->ID>>24 != CUBE_SAT_ID || cmd->ID>>24 != ALL_SAT_ID) // check we really are the sat it was send to.
		return logError(invalid_sat_id, "invalid_sat_id");

	if(memcpy(&cmd->cmd_type, data + plusPlace, sizeof(cmd->cmd_type)) == NULL) // same just the cmd_type.
		return null_pointer_error;
	plusPlace += sizeof(cmd->cmd_type);

	if(memcpy(&cmd->cmd_subtype, data + plusPlace, sizeof(cmd->cmd_subtype)) == NULL) // same just the cmd_subtype.
		return null_pointer_error;
	plusPlace += sizeof(cmd->cmd_subtype);

	if(memcpy(&cmd->length, data + plusPlace, sizeof(cmd->length)) == NULL) // same just the length.
		return null_pointer_error;
	plusPlace += sizeof(cmd->length);

	if(cmd->length != 0){
		if(memcpy(&cmd->data, data + plusPlace, cmd->length) == NULL) // same just to the data which is the length that equal to the value of the variable length.
			return null_pointer_error;
	}

	return command_success;
}

/*
 * Create a sat_packet_t struct that in the end the sat send to us
 *
 * @param[in] name= data; type= unsigned char *; The data part in the struct
 * @param[in] name= data_length; type= unsigned short; The length of the data that going to in the struct. Also it will be in the length part in the struct
 * @param[in] name= type; type= char; The cmd_type part in the struct. Says which subsystem its from.
 * @param[in] name= subtype; type= char; The cmd_subtype part in the struct. Says which commend of the subsystem its from.
 * @param[in] name= id; type= unsigned int;  The ID part in the struct. Have the id of the sat and some other things.
 * @param[out] name= cmd; type= sat_packet_t *; The struct we put all the info we get from the params.
 * @return error according to CMD_ERR
 *
 * */
CMD_ERR AssembleCommand(unsigned char *data, unsigned short data_length, char type, char subtype, unsigned int id, sat_packet_t *cmd)
{
	if(cmd == NULL)
		return null_pointer_error;

	cmd->ID = id;
	cmd->cmd_type = type;
	cmd->cmd_subtype = subtype;
	cmd->length = data_length;
	if(data_length != 0)
	{
		if(data == NULL)
			return null_pointer_error;
		else
		{
			if(data_length > MAX_COMMAND_DATA_LENGTH) // Check the data_length is not larger then the max length available
				return index_out_of_bound;

			memcpy(&cmd->data, data, data_length);
		}
	}

	return command_success;
}

/*
 * According to the type in the cmd struct goes to the right type and there do the commend.
 *
 * @param[out] name= cmd; type= sat_packet_t *; The packet as sat_packet_t struct.
 * @note if type not exist an unknown tpye ack is send.
 * @return error according to <hal/errors.h>
 *
 * */
int ActUponCommand(sat_packet_t *cmd)
{
	int error = 0;
	if(cmd == NULL)
		return null_pointer_error;
	// Go to each type known and according to spl_command_type struct and check if it's equal to the type in the cmd. if equal go the right router
	switch(cmd->cmd_type)
	{
		case trxvu_cmd_type:
			return logError(trxvu_command_router(cmd), "Command Dictionary - trxvu_command_router");
		case eps_cmd_type:
			error = logError(eps_command_router(cmd), "Command Dictionary - eps_command_router");
			return error;
		case filesystem_cmd_type:
/*			error = logError(filesystem_command_router(cmd), "Command Dictionary - filesystem_command_router");*/
			return error;
		case managment_cmd_type:
			error = logError(managment_command_router(cmd), "Command Dictionary - managment_command_router");
			return error;
		case payload_cmd_type:
/*			error = logError(payload_command_router(cmd), "Command Dictionary - telemetry_command_router");*/
			return error;
		default:
		{
			return logError(SendAckPacket(ACK_UNKNOWN_TYPE, cmd, NULL, 0), "ActUponCommand - SendAckPacket invalid type"); // Send ack that says what written in unknownType_msg
		}
	}
}
