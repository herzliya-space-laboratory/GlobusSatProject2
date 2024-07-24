
#ifndef SPL_H_
#define SPL_H_

typedef enum __attribute__ ((__packed__)) spl_command_type_t {
	trxvu_cmd_type,
	eps_cmd_type,
	filesystem_cmd_type, //2
	managment_cmd_type,
	payload_cmd_type,
	ack_type,// 5
	dump_type
}spl_command_type;



typedef enum __attribute__ ((__packed__)) ack_subtype_t
{
	ACK_RECEIVE_COMM = 		0x00,			// when receive any packet
	ACK_COMD_EXEC	 = 		0x01,			// when executed any packet / SPL command

	ACK_RESET_WAKEUP = 		0x7F,			// after waking up from reset

	ACK_FS_RESET = 			0x70,			// before going to filesystem reset
	ACK_TRXVU_SOFT_RESET = 	0x71,			// before going to trxvu reset
	ACK_TRXVU_HARD_RESET =	0x72,			// before going to trxvu reset
	ACK_ANTS_RESET = 		0x73,			// before reseting antennas
	ACK_EPS_RESET = 		0x80,			// before going to eps reset
	ACK_SOFT_RESET = 		0x81,			// before going to reset
	ACK_HARD_RESET = 		0x82,			// before starting hardware reset

	ACK_MEMORY_DELETE = 0x83,				// when memory delete is completed success

	ACK_UPDATE_TIME = 0x12,					// after updating time
	ACK_UPDATE_BEACON_BIT_RATE = 0x13,
	ACK_UPDATE_BEACON_TIME_DELAY = 0x14,
	ACK_UPDATE_EPS_VOLTAGES = 0x15,
	ACK_UPDATE_EPS_HEATER_VALUES = 0x16,
	ACK_UPDATE_EPS_ALPHA = 0x17,

	ACK_MUTE = 0x8D,
	ACK_UNMUTE = 0x8E,
	ACK_ALLOW_TRANSPONDER = 0x8F,

	ACK_DUMP_START = 0x90,
	ACK_DUMP_ABORT = 0x91,
	ACK_DUMP_FINISHED = 0x92,

	ACK_GENERIC_I2C_CMD = 0x93,
	ACK_ARM_DISARM = 0x94,					//after changing arm state of the ants
	ACK_REDEPLOY = 0x95,
	ACK_RESET_DELAYED_CMD = 0x9E,			//maybe we dont need
	ACK_FRAM_RESET = 0xA0,

	ACK_DELETE_TLM = 0xC0,					// after deleting TLM file(s)

	ACK_PING = 0xAA,
	ACK_UNKNOWN_SUBTYPE = 0xBB,				//when the given subtype is unknown
	ACK_ERROR_MSG = 0XFF 					// send this ACK when error has occurred
}ack_subtype_t;


typedef enum __attribute__ ((__packed__)) trxvu_subtypes_t
{
	BEACON_SUBTYPE =		0x01,	//0b00000001 =
	MUTE_TRXVU = 			0x11,	//0b00010001 *
	UNMUTE_TRXVU = 			0x88,	//0b10001000 *
	TRXVU_IDLE = 			0x87,   // *
	DUMP_DAYS =				0x69,	//0b01101001 +
	DUMP_TIME_RANGE =		0x70,   // +
	ABORT_DUMP_SUBTYPE = 	0x22,	//0b00100010 *
	GET_BAUD_RATE = 		0x13,	//0b00010011 =
	GET_BEACON_INTERVAL = 	0x23,	//0b00100011 =
	SET_BEACON_INTERVAL = 	0x24,	//0b00100100 *
	TRANSMIT_BEACON		= 	0x25,
	SET_BAUD_RATE = 		0x15,	//0b00010101 *
	SET_BEACON_CYCLE_TIME =	0x25,	//0b00100101 //its not in the cmd dictionary
	GET_TX_UPTIME = 		0x66,	//0b01100110 = //long int
	GET_RX_UPTIME = 		0x68,	//0b01101000 = same func as the TX uptime //long int
	GET_NUM_OF_ONLINE_CMD = 0xA7,	//0b10100111 =
	SET_RSSI_TRANSPONDER =  0xA5,   //0b10100101
	SET_TRANSPONDER =       0xA6,   //0b10100110
	ANT_SET_ARM_STATUS = 	0xB0,	//0b10110000 *
	ANT_GET_ARM_STATUS = 	0xB2,	//0b10110010 =
	ANT_GET_UPTIME =		0xB3,	//0b10110011 = //long int
	ANT_CANCEL_DEPLOY = 	0xB7,	//0b10110111 *
	ANT_DEPLOY = 			0xB9,	//0b10110111 *
	ANT_STOP_REDEPLOY = 	0xC0,
	PING = 					0xC2,
	FORCE_ABORT_DUMP_SUBTYPE = 0x33,//0b00110011 *
	DELETE_DUMP_TASK = 0x44			//0b00100010 //its not in the cmd dictionary

}trxvu_subtypes_t;


typedef enum __attribute__ ((__packed__)) eps_subtypes_t
{
	UPDATE_ALPHA 		= 0x01,
    GET_HEATER_VALUES 	= 0x02,
    SET_HEATER_VALUES 	= 0x03,
    RESET_EPS_WDT	 	= 0x04
}eps_subtypes_t;


typedef enum __attribute__ ((__packed__)) filesystem_subtypes_t
{
	DELETE_FILE 		= 0xAA,		//0b10101010
	DELETE_ALL_FILES	= 0xAB,		//0b10101010
	GET_LAST_FS_ERROR	= 0xBB,
	SET_TLM_PERIOD		= 0xCC,
	GET_TLM_PERIOD		= 0xCD,
	GET_IMAGE_INFO		= 0xE1,
	GET_IMAGE_DATA		= 0xE2

}filesystem_subtypes_t;


typedef enum __attribute__ ((__packed__)) management_subtypes_t
{
	SOFT_RESET_SUBTYPE = 		0xAA,		//0b10101010
	HARD_RESET_SUBTYPE = 		0x99,		//0b10101010
	TRXVU_SOFT_RESET_SUBTYPE =	0xC3,		//0b11000011
	TRXVU_HARD_RESET_SUBTYPE = 	0x3C,		//0b00111100
	EPS_RESET_SUBTYPE =			0xBB,		//0b10111011
	FS_RESET_SUBTYPE =			0xCC,		//0b11001100
	UPDATE_SAT_TIME =           0xDD,       //0b‭11011101‬
	GENERIC_I2C_CMD =           0xDF,       //0b‭11011111‬
	RESET_COMPONENT =           0xDE,       //0b‭11011110
	FRAM_WRITE_AND_TRANSMIT =   0xE0,       //0b11100000
	FRAM_READ_AND_TRANSMIT =    0xE1,
	FRAM_RESTART =    			0xE2,
	GET_SAT_UPTIME =    		0xE3
}management_subtypes_t;
//-----------------


typedef enum __attribute__ ((__packed__)) payload_subtypes_t
{
	TURN_ON_PAYLOAD = 	0x11,
	TURN_OFF_PAYLOAD = 	0x20

}payload_subtypes_t;
#endif /* SPL_H_ */
