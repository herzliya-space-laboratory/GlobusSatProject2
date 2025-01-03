
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
	ACK_PAYLOAD_RESET = 	0x71,			// before going to payload reset
	ACK_TX_HARD_RESET =		0x73,			// before going to trxvu reset
	ACK_RX_HARD_RESET =		0x74,			// before going to trxvu reset
	ACK_ANTS_RESET = 		0x75,			// before reseting antennas
	ACK_EPS_RESET = 		0x80,			// before going to eps reset
	ACK_SOFT_RESET = 		0x81,			// before going to reset
	ACK_HARD_RESET = 		0x82,			// before starting hardware reset
	ACK_FRAM_RESET =		0x83,

	ACK_UPDATE_TIME = 0x12,					// after updating time

	ACK_MEMORY_DELETE = 0x84,				// when memory delete is completed success
	ACK_SET_NEW_TLM_PERIOD = 0x85,

	ACK_UPDATE_BEACON_BIT_RATE = 0x13,
	ACK_UPDATE_BEACON_INTERVAL = 0x14,

	ACK_UPDATE_RSSI_VALUE = 0x18,
	ACK_UPDATE_EPS_VOLTAGES = 0x16,
	ACK_UPDATE_EPS_ALPHA = 0x17,
	ACK_EPS_RESET_WDT = 0x15,

	ACK_MUTE = 0x8D,
	ACK_UNMUTE = 0x8E,
	ACK_ALLOW_TRANSPONDER = 0x8F,
	ACK_TRANSPONDER_OFF = 0x9F,
	ACK_IDLE_ON = 0xA1,
	ACK_IDLE_OFF = 0xA2,

	ACK_DUMP_START = 0x90,
	ACK_DUMP_FINISHED = 0x92,

	ACK_GENERIC_I2C_CMD = 0x93,
	ACK_ARM_DISARM = 0x94,					//after changing arm state of the ants
	ACK_REDEPLOY = 0x95,
	ACK_RESET_DELAYED_CMD = 0x9E,			//maybe we dont need
	ACK_ANT_CANCEL_DEPLOY = 0xA3,

	ACK_FINISH_FIRST_ACTIVE = 0x35,

	ACK_SWITCHING_SD_CARD = 0x30,

	ACK_DELETE_TLM = 0xC0,					// after deleting TLM file(s)
	ACK_FS_LAST_ERROR = 0xC1,
	ACK_PING = 0xAA,

	ACK_SET_GWDT = 0xA5,

	ACK_KILL_PAYLOAD = 0x50,
	ACK_TURN_ON_PAYLOAD = 0x51,

	ACK_CHANGE_HEATER_THRESHOLDS = 0x55,

	ACK_UNKNOWN_SUBTYPE = 0xBB,				//when the given subtype is unknown
	ACK_UNKNOWN_TYPE = 0xCC,				//when the given type is unknown
	ACK_ERROR_MSG = 0XFF 					// send this ACK when error has occurred
}ack_subtype_t;


typedef enum __attribute__ ((__packed__)) trxvu_subtypes_t
{
	BEACON_SUBTYPE =	        	0x01,	//0b00000001 =
	MUTE_TRXVU = 		        	0x02,	//0b00010001 *
	UNMUTE_TRXVU = 		        	0x03,	//0b10001000 *
	ON_IDLE = 		               	0x09,   // *
	OFF_IDLE =                      0x0A,
	GET_BEACON_INTERVAL =         	0x06,	//0b00100011 =
	SET_BEACON_INTERVAL =         	0x04,	//0b00100100 *
	SET_BEACON_INTERVAL_DEFAULT =   0x05,	//0b00100100 *
	GET_TX_UPTIME = 	        	0x07,	//0b01100110 = //long int
	GET_RX_UPTIME = 	        	0x08,	//0b01101000 = same func as the TX uptime //long int
	SET_RSSI_TRANSPONDER =          0x0D,   //0b10100101
	GET_RSSI_TRANSPONDER =          0x14,   //0b10100101
	SET_RSSI_TRANSPONDER_DEFAULT =  0x0E,   //0b10100101
	SET_ON_TRANSPONDER =            0x0B,   //0b10100110
	SET_OFF_TRANSPONDER =           0x0C,   //0b10100110
	ANT_GET_UPTIME =	        	0x10,	//0b10110011 = //long int
	ANT_CANCEL_DEPLOY =         	0x11,	//0b10110111 *//delete before fllght
	PING = 				        	0x13,
	OFF_ANTS_DEPLOY = 				0x15

}trxvu_subtypes_t;


typedef enum __attribute__ ((__packed__)) eps_subtypes_t
{
	RESET_EPS_WDT	 	       	= 0x01,
	UPDATE_ALPHA 		       	= 0x02,
	UPDATE_ALPHA_DEFAULT       	= 0x03,
	GET_ALPHA	    	       	= 0x04,
	UPDATE_THRESHOLD	       	= 0x05,
	UPDATE_THRESHOLD_DEFAULT   	= 0x06,
	GET_THRESHOLD	           	= 0x07,
	GET_HEATER_VALUES   	   	= 0x0C,
	SET_HEATER_VALUES			= 0x0D,
	GET_STATE				   	= 0x0E,

}eps_subtypes_t;


typedef enum __attribute__ ((__packed__)) filesystem_subtypes_t
{
	DELETE_ALL_FILES		= 0x01,
	DELETE_TLM				= 0xA1,
	DELETE_TLM_BY_TYPE		= 0xB1,
	GET_LAST_FS_ERROR		= 0xBB,
	SET_TLM_PERIOD			= 0xCC,
	SET_DEFAULT_TLM_PERIOD	= 0xCE,
	GET_TLM_PERIOD			= 0xCD,
	START_DUMP_BY_DAYS		= 0x0E,
	GET_FREE_SPACE			= 0x0B,
	SWITCH_SD_CARD			= 0x0A,
	ABORT_DUMP				= 0x1E
}filesystem_subtypes_t;


typedef enum __attribute__ ((__packed__)) management_subtypes_t
{
/*
	SOFT_RESET_SUBTYPE = 		0xAA,		//0b10101010
	HARD_RESET_SUBTYPE = 		0x99,		//0b10101010
	TRXVU_SOFT_RESET_SUBTYPE =	0xC3,		//0b11000011
	TRXVU_HARD_RESET_SUBTYPE = 	0x3C,		//0b00111100
	EPS_RESET_SUBTYPE =			0xBB,		//0b10111011
	FS_RESET_SUBTYPE =			0xCC,		//0b11001100
*/
	GET_PERIOD_GROUND_WDT = 	0x01,
	SET_PERIOD_GROUND_WDT = 	0x02,
	UPDATE_SAT_TIME =           0xDD,       //0b‭11011101‬
	GENERIC_I2C_CMD =           0xDF,       //0b‭11011111‬
	RESET_COMPONENT =           0xDE,       //0b‭11011110
	GET_SAT_UPTIME =    		0xE3,
	GET_SAT_TIME =				0xE4
}management_subtypes_t;
//-----------------


typedef enum __attribute__ ((__packed__)) payload_subtypes_t
{
	TURN_ON_KILL_PAYLOAD = 	0x11,
	TURN_OFF_KILL_PAYLOAD = 0x20

}payload_subtypes_t;
#endif /* SPL_H_ */
