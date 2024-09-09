
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
	SET_RSSI_TRANSPONDER_DEFAULT =  0x0E,   //0b10100101
	SET_ON_TRANSPONDER =            0x0B,   //0b10100110
	SET_OFF_TRANSPONDER =           0x0C,   //0b10100110
	ANT_GET_ARM_STATUS =        	0x0F,	//0b10110010 =
	ANT_GET_UPTIME =	        	0x10,	//0b10110011 = //long int
	ANT_CANCEL_DEPLOY =         	0x11,	//0b10110111 *//delete before fllght
	ANT_DEPLOY = 		        	0x12,	//0b10110111 *//delete before fllght
	PING = 				        	0x13,

}trxvu_subtypes_t;


typedef enum __attribute__ ((__packed__)) eps_subtypes_t
{
	RESET_EPS_WDT	 	       = 0x01,
	UPDATE_ALPHA 		       = 0x02,
    UPDATE_ALPHA_DEFAULT       = 0x03,
    GET_ALPHA	    	       = 0x04,
    UPDATE_THRESHOLD	       = 0x05,
    UPDATE_THRESHOLD_DEFAULT   = 0x06,
    GET_THRESHOLD	           = 0x07,
    SET_MODE     	           = 0x08,
    GET_MODE    	           = 0x09,
    GET_STATE_CHANGES_OVERTIME = 0x0A,
    GET_HEATER_VALUES   	   = 0x0B,
    SET_HEATER_VALUES          = 0x0C,





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
