
#ifndef SATCOMMANDS_H_
#define SATCOMMANDS_H_

#include "GlobalStandards.h"

#define MAX_COMMAND_DATA_LENGTH 200 ///< maximum AX25 data field available for downlink
#define IMG_CHUNK_SIZE 50 //190 // leave room for chunk number and more

#define FRAM_MAX_ADDRESS 539216400

//<! how many command can be saved in the buffer
#define MAX_NUM_OF_DELAYED_CMD (100)
#define CUBE_SAT_ID 	14
#define ALL_SAT_ID 		10


typedef enum __attribute__ ((__packed__)) CMD_ERR{
	command_success = 0,				///< a successful operation. no errors
	command_found = 0,					///< a command was found
	no_command_found ,					///< no commands were found in command buffers
	index_out_of_bound,					///< index out of bound error
	null_pointer_error,					///< input parameter pointer is null
	execution_error, 					///< an execution error has occured
	invalid_sat_id 					///<
}CMD_ERR;

typedef struct __attribute__ ((__packed__)) sat_packet_t
{
	unsigned int ID;
	char cmd_type;
	char cmd_subtype;
	unsigned short length;
	unsigned char data[MAX_COMMAND_DATA_LENGTH];

}sat_packet_t;

/*
 * Change the packet we get to sat_packet_t struct
 *
 * @param[in] name= data; type= unsigned char *; The packet we get after reading from the frame (without the headers of the Ax25 protocol).
 * @param[out] name= cmd; type= sat_packet_t *; The struct we put all the info we get from the packet.
 * @return error according to CMD_ERR
 *
 * */
CMD_ERR ParseDataToCommand(unsigned char * data, sat_packet_t *cmd);

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
CMD_ERR AssembleCommand(unsigned char *data, unsigned short data_length, char type, char subtype, unsigned int id, sat_packet_t *cmd);

/*
 * According to the type in the cmd struct goes to the right type and there do the commend.
 *
 * @param[out] name= cmd; type= sat_packet_t *; The packet as sat_packet_t struct.
 * @note if type not exist an unknown tpye ack is send.
 * @return error according to <hal/errors.h>
 *
 * */
int ActUponCommand(sat_packet_t *cmd);

#endif /* SATCOMMANDS_H_ */
