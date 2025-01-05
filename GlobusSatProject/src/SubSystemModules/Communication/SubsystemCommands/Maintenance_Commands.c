/*
 * Maintenance_Commands.c
 *
 *  Created on: 3 Dec 2024
 *      Author: maayan
 */

#include "Maintenance_Commands.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include <string.h>

/*
 * Get the time save on the sat at the sat.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error 0 on success.
 * */
int CMD_GetSatTime(sat_packet_t *cmd)
{
	time_unix timeNow;
	int error = logError(Time_getUnixEpoch((unsigned int*)&timeNow), "CMD_GetSatTime - Time_getUnixEpoch");
	if(error)
	{
		unsigned char ackError = ERROR_CANT_GET_TIME;
		SendAckPacket(ACK_ERROR_MSG , cmd, &ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return SendAckPacket(ACK_GET_SAT_TIME, cmd, (unsigned char*)&timeNow, sizeof(time_unix)); //send back the sat time
}

/*help functions of CMD_ResetComponent*/
int Payload_ComponenetReset()
{
	SendAckPacket(ACK_PAYLOAD_SOFT_RESET, NULL, NULL, 0);
	return logError(payloadSoftReset(), "Payload_ComponenetReset - payloadSoftReset");
}

int HardPayload_ComponenetReset()
{
	SendAckPacket(ACK_PAYLOAD_HARD_RESET, NULL, NULL, 0);
	payloadTurnOff();
	return payloadTurnOn();
}

int HardTX_ComponenetReset()
{
	SendAckPacket(ACK_TX_HARD_RESET, NULL, NULL, 0);
	logError(isis_vu_e__reset_hw_tx(0), "HardTX_ComponenetReset - isis_vu_e__reset_hw_tx");
	return InitTrxvuAndAnts();
}

int HardRX_ComponenetReset()
{
	SendAckPacket(ACK_RX_HARD_RESET, NULL, NULL, 0);
	logError(isis_vu_e__reset_hw_rx(0), "HardRX_ComponenetReset - isis_vu_e__reset_hw_rx");
	return InitTrxvuAndAnts();
}

int Soft_ComponenetReset()
{
	Boolean true = TRUE; Boolean false = FALSE;
	SendAckPacket(ACK_SOFT_RESET, NULL, NULL, 0);
	logError(FRAM_writeAndVerify((unsigned char*)&true, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "Soft_ComponenetReset - cmd reset flag");
	logError(FRAM_writeAndVerify((unsigned char*)&false, HAD_RESET_IN_A_MINUTE_ADDR, HAD_RESET_IN_A_MINUTE_SIZE), "Soft_ComponenetReset - FRAM_writeAndVerify");
	restart();
	vTaskDelay(20);
	return 0;
}

int Ants_ComponenetReset()
{
	SendAckPacket(ACK_ANTS_RESET, NULL, NULL, 0);
	int err = logError(isis_ants__reset(0), "Ants_ComponenetReset - isis_ants__reset - 0");
	err += logError(isis_ants__reset(1), "Ants_ComponenetReset - isis_ants__reset - 1");
	return err;
}

int Hard_ComponenetReset()
{
	isismepsv2_ivid7_piu__replyheader_t replyheader;
	Boolean true = TRUE; Boolean false = FALSE;
	logError(FRAM_writeAndVerify((unsigned char*)&true, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "Hard_ComponenetReset - cmd reset flag");
	logError(FRAM_writeAndVerify((unsigned char*)&false, HAD_RESET_IN_A_MINUTE_ADDR, HAD_RESET_IN_A_MINUTE_SIZE), "Hard_ComponenetReset - FRAM_writeAndVerify");
	SendAckPacket(ACK_HARD_RESET, NULL, NULL, 0);
	return logError(isismepsv2_ivid7_piu__reset(0, &replyheader), "Hard_ComponenetReset - isismepsv2_ivid7_piu__reset");
}

int FS_ComponenetReset()
{
	SendAckPacket(ACK_FS_RESET, NULL, NULL, 0);
	DeInitializeFS();
	return InitializeFS();
}

int FRAM_ComponenetReset()
{
	SendAckPacket(ACK_FRAM_RESET, NULL, NULL, 0);
	FRAM_stop();
	return logError(FRAM_start(), "FRAM_ComponenetReset - FRAM_start");
}

int SendErrorCantReset(sat_packet_t *cmd)
{
	unsigned char ackError = ERROR_CANT_RESET;
	SendAckPacket(ACK_ERROR_MSG , cmd, &ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
	return ackError;
}


/*!
 * @brief 	starts a reset according to 'reset_type_t'
 * 			The function will raise the reset flag and send an ACK before starting the reset.
 * @return 	0 on success
 * 			Error code according to <hal/errors.h>
 */
int CMD_ResetComponent(sat_packet_t *cmd)
{
	unsigned char ackError = 0;
	if(cmd->length != 1)
	{
		ackError = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, &ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
		return ackError;
	}
	uint8_t type;
	memcpy(&type, cmd->data, cmd->length);
	MostCurrentTimeToFRAM();
	switch(type)
	{
		case reset_software:
			return Soft_ComponenetReset();
		case reset_hardware:
		{
			if(Hard_ComponenetReset())
				return SendErrorCantReset(cmd);
			vTaskDelay(20);
			return 0;
		}
		case reset_tx_hard:
		{
			if(HardTX_ComponenetReset())
				return SendErrorCantReset(cmd);
			return 0;
		}
		case reset_rx_hard:
		{
			if(HardRX_ComponenetReset())
				return SendErrorCantReset(cmd);
			return 0;
		}
		case reset_ants:
		{
			if(Ants_ComponenetReset())
				return SendErrorCantReset(cmd);
			return 0;
		}
		case reset_payload:
		{
			if(Payload_ComponenetReset())
				return SendErrorCantReset(cmd);
			return 0;
		}
		case reset_payload_hard:
		{
			if(HardPayload_ComponenetReset())
				return SendErrorCantReset(cmd);
			return 0;
		}
		case reset_fram:
		{
			if(FRAM_ComponenetReset())
				return SendErrorCantReset(cmd);
			return 0;
		}
		case reset_filesystem:
		{
			if(FS_ComponenetReset())
				return SendErrorCantReset(cmd);
			return 0;
		}
		default:
		{
			ackError = ERROR_CANT_DO;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
			return ackError;
		}
	}
}

/*
 * Get the time the OBC was active since the last reset
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error according to <hal/error.h> or TransmitDataAsSPL_Packet errors
 * 					 0 on success.
 * */
int CMD_GetSatUptime(sat_packet_t *cmd)
{
	supervisor_housekeeping_t supervisorHK; //create a variable that is the struct we need from supervisor
	int error = logError(Supervisor_getHousekeeping(&supervisorHK, SUPERVISOR_SPI_INDEX), "CMD_GetSatUptime - Supervisor_getHousekeeping"); //gets the variables to the struct and also check error.
	if(error)
	{
		unsigned char ack_error = ERROR_GET_FROM_STRUCT;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&ack_error, sizeof(ack_error)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return SendAckPacket(ACK_GET_SAT_UPTIME, cmd, (unsigned char*)&supervisorHK.fields.iobcUptime, sizeof(supervisorHK.fields.iobcUptime)); //send back the sat time
}


/*
 * Set the time on the OBC
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add and the new time)
 * @return type=int; 	-1 on cmd Null
 * 						-3 on wrong length
 * 						1 if cant set new time
 * 						type of error according to <hal/error.h>
 * 					 	0 on success.
 * */
int CMD_UpdateSatTime(sat_packet_t *cmd)
{
	//can sat at min on 1.1.2000 00:00:00 start at 1.1.1970 so the min time in seconds is 946,728,000 or in hex 386D EC40 and we need it as opposite so 40 EC 6D 38
	if(cmd == NULL) return -1;
	unsigned char error_ack = 0;
	if(cmd->length != 4)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -3;
	}
	time_unix passSatTime;
	logError(Time_getUnixEpoch((unsigned int*)&passSatTime), "CMD_UpdateSatTime - Time_getUnixEpoch");
	time_unix newSatTime;
	memcpy((unsigned char*)&newSatTime, cmd->data, cmd->length);
	int error = logError(Time_setUnixEpoch(newSatTime), "CMD_UpdateSatTime - Time_setUnixEpoch");
	if(error)
	{
		error_ack = ERROR_COULDNT_UPDATE_SAT_TIME;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return 1;
	}
	MostCurrentTimeToFRAM();
	KickGroundCommWDT();
	if(newSatTime < passSatTime)
		Delete_allTMFilesFromSD();
	return SendAckPacket(ACK_UPDATE_TIME , cmd, (unsigned char*)&newSatTime, sizeof(newSatTime));

}

int CMD_SetGsWdtKickTime(sat_packet_t* cmd)
{
	if(cmd == NULL) return -1;
	unsigned char error_ack = 0;
	if(cmd->length != 4)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -2;
	}
	unsigned int newGroundWDT;
	memcpy((unsigned char*)&newGroundWDT, cmd->data, cmd->length);
	if(newGroundWDT < MIN_GROUND_WDT)
	{
		error_ack = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -3;
	}
	if(logError(FRAM_writeAndVerify((unsigned char*)&newGroundWDT, NO_COMM_WDT_KICK_TIME_ADDR, NO_COMM_WDT_KICK_TIME_SIZE), "CMD_SetGsWdtKickTime - FRAM_writeAndVerify"))
	{
		error_ack = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -4;
	}
	return SendAckPacket(ACK_SET_GWDT, cmd, NULL, 0);
}

int CMD_GetGsWdtKickTime(sat_packet_t* cmd)
{
	unsigned int groundWDT = 0;
	if(logError(FRAM_read((unsigned char*)&groundWDT, NO_COMM_WDT_KICK_TIME_ADDR, NO_COMM_WDT_KICK_TIME_SIZE), "CMD_GetGsWdtKickTime - FRAM_read"))
	{
		unsigned char ackError = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, &ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
		return -1;
	}
	return SendAckPacket(ACK_GET_GROUND_WDT, cmd, (unsigned char*)&groundWDT, sizeof(groundWDT));

}
