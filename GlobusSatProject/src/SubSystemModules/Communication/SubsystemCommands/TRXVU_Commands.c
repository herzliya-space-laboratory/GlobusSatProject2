/*
 * TRXVU_Commands.c
 *
 *  Created on: 3 9 2024
 *      Author: maayan
 */

#include "TRXVU_Commands.h"

/**
 * set trxvu idle on with time end
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the idle duration and the headers we add)
 * @return type=int; -6 on cmd null
 * 					 other according to SetIdleState error list.
 * 					 or SendAckPacket
 */
int CMD_SetOn_IdleState(sat_packet_t *cmd)
{
	unsigned char error_ack;
	if(cmd == NULL)
		return -6;
	if(cmd->length != 4)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -3;
	}
	time_unix duration;
	memcpy(&duration, cmd->data, cmd->length);
	if(duration > MAX_IDLE_TIME) duration = MAX_IDLE_TIME;
	int error = SetIdleState(trxvu_idle_state_on, duration);
	switch(error)
	{
		case -1:
		{
			error_ack = ERROR_CANT_DO;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error;
		}
		case -2:
		{
			error_ack = ERROR_READ_FROM_FRAM;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error;
		}
		case -3:
		{
			error_ack = ERROR_WRITE_TO_FRAM;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error;
		}
		case -4:
		{
			error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error;
		}
		case -5:
		{
			error_ack = ERROR_CANT_GET_TIME;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error;
		}
		default:
		{
			SendAckPacket(ACK_ERROR_MSG , cmd, NULL, 0);
			return error;
		}
	}
	return logError(SendAckPacket(ACK_IDLE_ON , cmd, (unsigned char*)&duration, sizeof(duration)), "CMD_SetOn_dleState - SendAckPacket");
}

/* set trxvu idle off
* @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; according to SetIdleState error list.
* 					 or SendAckPacket if we don't have error in the SetIdleState
*/
int CMD_SetOff_IdleState(sat_packet_t *cmd)
{
	unsigned char error_ack;
	int error = SetIdleState(trxvu_idle_state_off, 0);
	switch(error)
	{
		case -1:
		{
			error_ack = ERROR_CANT_DO;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error;
		}
		case -2:
		{
			error_ack = ERROR_READ_FROM_FRAM;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error;
		}
		case -3:
		{
			error_ack = ERROR_WRITE_TO_FRAM;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error;
		}
		case -4:
		{
			error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error;
		}
		default:
		{
			SendAckPacket(ACK_ERROR_MSG , cmd, NULL, 0);
			return error;
		}
	}
	return logError(SendAckPacket(ACK_IDLE_OFF , cmd, NULL, 0), "CMD_SetOff_dleState - SendAckPacket");

}

/*
 * Set transmitter to transponder state for the time written in the data in cmd
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the mute duration and the headers we add)
 * @return type=int; -1 on cmd NULL
 * 					 -2 on written wrong number to FRAM
 * 					 -3 on incorrect length
 * 					 -4 can't read from FRAM
 * 					 -5 can't write to FRAM
 * 					 errors according to I2C_write
 * */
int CMD_SetOn_Transponder(sat_packet_t *cmd)
{
	unsigned char error_ack;
	if(cmd == NULL)
		return -1;
	if(cmd->length != 4)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -3;
	}
	time_unix duration;
	memcpy(&duration, cmd->data, cmd->length);
	if(duration > MAX_TRANS_TIME)
		duration = MAX_TRANS_TIME;
	int timeNow;
	logError(Time_getUnixEpoch((unsigned int*)&timeNow), "CMD_SetOn_Transponder - Time_getUnixEpoch");
	duration += timeNow;
	if(logError(FRAM_write((unsigned char*)&duration, TRANSPONDER_END_TIME_ADDR, TRANSPONDER_END_TIME_SIZE), "CMD_SetOn_Transponder - FRAM_write"))
	{
		error_ack = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -5;
	}
	time_unix check = getTransponderEndTime();
	if(!check)
	{
		error_ack = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -4;
	}
	if(check != duration)
	{
		error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -2;
	}
	logError(SendAckPacket(ACK_ALLOW_TRANSPONDER , cmd, (unsigned char*)&duration, sizeof(duration)), "CMD_SetOn_Transponder - SendAckPacket"); // Send ack of success in turn on transponder and to how much time
	int error = setTransponderOn(); // Set transponder on
	if(error)
	{
		error_ack = ERROR_WRITE_TO_I2C;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return 0;

}

/*
* The command sets the transponder off.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; return type of error.
* */
int CMD_SetOff_Transponder(sat_packet_t *cmd)
{
	unsigned char error_ack;
	unsigned char data[] = {0x38, trxvu_transponder_off}; // 0x38 - number of commend to change the transmitter mode.
	int error = logError(I2C_write(I2C_TRXVU_TC_ADDR, data, 2), "CMD_SetOff_Transponder - I2C_write"); // Set transponder off
	if(error)
	{
		error_ack = ERROR_WRITE_TO_I2C;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	time_unix timeNow = 1;
	error = logError(FRAM_write((unsigned char*)&timeNow, TRANSPONDER_END_TIME_ADDR, TRANSPONDER_END_TIME_SIZE), "CMD_SetOn_Transponder - FRAM_write");
	if(error)
	{
		int error_ack = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack));	// Send ack error according to "AckErrors.h"
		return error;
	}
	time_unix check = getTransponderEndTime();
	if(!check)
	{
		error_ack = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -4;
	}
	if(check != timeNow)
	{
		error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack));	// Send ack error according to "AckErrors.h"
		return -2;
	}
	return logError(SendAckPacket(ACK_TRANSPONDER_OFF, cmd, NULL, 0), "CMD_SetOff_Transponder - SendAckPacket"); // Send ack of success in turn off transponder
}

/*
* The command change the rssi value in the FRAM and check it change correctly. also check the new rssi value is between the 0 and 4095
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the new rssi val and the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1 and on error in setTransponderRSSIinFRAM, -2 if length isn't in size.
 * */
int CMD_SetRSSI_Transponder(sat_packet_t *cmd)
{
	unsigned char error_ack;
	if(cmd == NULL)
		return -1;
	short new_rssi_val = -1;
	if(cmd->length != 2)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -2;
	}
	memcpy(&new_rssi_val, &cmd->data, cmd->length);
	int error = setTransponderRSSIinFRAM(new_rssi_val);
	if(error == -2)
	{
		error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	else if(error)
	{
		error_ack = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return logError(SendAckPacket(ACK_UPDATE_RSSI_VALUE , cmd, (unsigned char*)&new_rssi_val, sizeof(new_rssi_val)), "CMD_SetRSSI_Transponder - SendAckPacket"); // Send ack of success in change rssi and to what
}

/*
* The command change the rssi value in the FRAM to default and check it change correctly.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information and set the data to default rssi val (the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1 and on error in setTransponderRSSIinFRAM, -2 if length isn't in size.
 * */
int CMD_SetRSSI_Transponder_DEFAULT(sat_packet_t *cmd)
{
	cmd->length = 2;
	short new_rssi = DEFAULT_RSSI_VALUE;
	memcpy(cmd->data, (unsigned char*)&new_rssi, cmd->length);
	return CMD_SetRSSI_Transponder(cmd);
}

/*
* The command get the rssi value from the FRAM and send it back.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; return type of error 0 on success and if the parameter (cmd) is NULL return -2, if cant read from fram return -1.
* */
int CMD_GetRSSI_Transponder(sat_packet_t *cmd)
{
	if(cmd == NULL)
		return -2;
	short rssi_val = getTransponderRSSIFromFRAM();
	if(rssi_val == -1)
	{
		unsigned char error_ack = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -1;
	}
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&rssi_val, TRANSPONDER_RSSI_SIZE), "CMD_GetRSSI_Transponder - TransmitDataAsSPL_Packet"); // Send back the beacon interval
}

/*
 * Set transmitter to mute for the time written in the data in cmd
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the mute duration and the headers we add)
 * @return type=int; -1 on cmd NULL
 * 					 -3 on incorrect length
 * 					 errors according to setMuteEndTime
 * */
int CMD_MuteTRXVU(sat_packet_t *cmd)
{
	unsigned char error_ack;
	if(cmd == NULL)
		return -1;
	if(cmd->length != 4)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -3;
	}
	time_unix muteEndTime = 0;
	memcpy(&muteEndTime, cmd->data, cmd->length);
	if(muteEndTime > MAX_MUTE_TIME)
		muteEndTime = MAX_MUTE_TIME;
	logError(SendAckPacket(ACK_MUTE , cmd, (unsigned char*)&muteEndTime, sizeof(muteEndTime)), "CMD_MuteTRXVU - SendAckPacket"); // Send ack of success at mute
	int error = setMuteEndTime(muteEndTime);
	setTransponderOff();
	if(error == -2)
	{
		error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack));	// Send ack error according to "AckErrors.h"
		return error;
	}
	else if(error)
	{
		error_ack = ERROR_CANT_SET_END_TIME;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return 0;
}

/*
 * Change the end time of mute to unmute the transmiter.
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error.
 * */
int CMD_UnMuteTRXVU(sat_packet_t *cmd)
{
	unsigned char error_ack;
	int error = setMuteEndTime(0); // set new end time to time now
	if(error == -2)
	{
		error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	else if(error)
	{
		error_ack = ERROR_CANT_SET_END_TIME;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error that says what written in error_msg (couldn't set new end time)
		return error;
	}
	return logError(SendAckPacket(ACK_UNMUTE , cmd, NULL, 0), "CMD_UnMuteTRXVU - SendAckPacket"); // Send ack of success in unmuting the transmitter
}

/*
* The command change the beacon interval in the FRAM and check it change correctly. also check the new interval is between the max and min interval
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the new_interval and the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1, -2 if length isn't in size.
* */
int CMD_SetBeacon_Interval(sat_packet_t *cmd)
{
	unsigned char error_ack;
	if(cmd == NULL)
		return -1;
	if(cmd->length != 4)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -2;
	}
	time_unix new_interval;
	int error;
	memcpy(&new_interval, &cmd->data, cmd->length);
	if(new_interval >= MAX_BEACON_INTERVAL) // Check if new_interval is bigger then max and if it is compares him to max interval
		new_interval = MAX_BEACON_INTERVAL;
	else if(new_interval <= MIN_BEACON_INTERVAL) // Same only to min interval
		new_interval = MIN_BEACON_INTERVAL;
	error = logError(FRAM_write((unsigned char*)&new_interval, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_write"); // Write the new interval to FRAM
	if(error)
	{
		error_ack = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	time_unix check;
	error = logError(FRAM_read((unsigned char*)&check, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_read"); // Read from FRAM in the place we wrote to for check
	if(error)
	{
		error_ack = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	if(check != new_interval) // Check if what we wrote and what have been written is the same
	{
		error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	setNewBeaconIntervalToPeriod();

	return logError(SendAckPacket(ACK_UPDATE_BEACON_INTERVAL , cmd, (unsigned char*)new_interval, sizeof(new_interval)), "CMD_SetBeacon_Interval - SendAckPacket"); // // Send ack with the new_interval with subtype of ACK_UPDATE_BEACON_INTERVAL
}

/*
* The command change the beacon interval in the FRAM to default interval
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error.
* */
int CMD_SetBeacon_Interval_DEFAULT(sat_packet_t *cmd)
{
	cmd->length = 4;
	int beacon_interval = DEFAULT_BEACON_INTERVAL_TIME;
	memcpy(cmd->data, (unsigned char*)&beacon_interval, cmd->length);
	return CMD_SetBeacon_Interval(cmd);
}

/*
* The command get the beacon interval from the FRAM and send it back.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; return type of error and if the parameter (cmd) is NULL return -1.
* */
int CMD_GetBeacon_Interval(sat_packet_t *cmd)
{
	time_unix period;
	int error = logError(FRAM_read((unsigned char*)&period, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_read"); // Read the beacon interval from FRAM
	if(error)
	{
		unsigned char error_ack = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	if(cmd == NULL)
		return -1;
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&period, BEACON_INTERVAL_TIME_SIZE), "CMD_GetBeacon_Interval - TransmitDataAsSPL_Packet"); // Send back the beacon interval
}

/*
 * Get transmitter uptime and send to ground
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error according to <hal/errors.h>
 * */
int CMD_GetTxUptime(sat_packet_t *cmd)
{
	unsigned int uptime;
	int error = IsisTrxvu_tcGetUptime(ISIS_TRXVU_I2C_BUS_INDEX, &uptime);
	if(error)
	{
		unsigned char error_ack = ERROR_GET_UPTIME;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&uptime, sizeof(uptime)), "CMD_GetTxUptime - TransmitDataAsSPL_Packet");
}

/*
 * Get receiver uptime and send to ground
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error according to <hal/errors.h>
 * */
int CMD_GetRxUptime(sat_packet_t *cmd)
{
	unsigned int uptime;
	int error = IsisTrxvu_rcGetUptime(ISIS_TRXVU_I2C_BUS_INDEX, &uptime);
	if(error)
	{
		unsigned char error_ack = ERROR_GET_UPTIME;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&uptime, sizeof(uptime)), "CMD_GetTxUptime - TransmitDataAsSPL_Packet");
}

/*
 * Helper function to get side of ants.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the ant side and the headers we add)
 * @param[out] name=side; type=char*; here we left the side for further use.
 * */
int GetAntSide(sat_packet_t *cmd, char *side)
{
	unsigned char error_ack;
	if(cmd == NULL) return -1;
	if(cmd->length != 1)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -2;
	}
	*side = cmd->data[0];
	return 0;
}

/*
 * Gets Ant uptime according to side.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the ant side and the headers we add)
 * @return type=int; return type of error according to this
 * 																-1 cmd NULL
 * 																-2 if length isn't in size.
 * 																-3 got wrong side (not exist one)
 * 																else from <hal/errors.h>
 * */
int CMD_AntGetUptime(sat_packet_t *cmd)
{
	unsigned char error_ack;
	if(cmd == NULL) return -1;
	int error = 0;
	char side;
	error = GetAntSide(cmd, &side);
	if(error) return error;
	unsigned int uptime;
	if(side == 'A')
		error = IsisAntS_getUptime(0, isisants_sideA, &uptime);
	else if(side == 'B')
		error = IsisAntS_getUptime(0, isisants_sideB, &uptime);
	else
	{
		error_ack = ERROR_SIDE_ANTS_NOT_A_OR_B;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -3;
	}
	if(error)
	{
		error_ack = ERROR_GET_UPTIME;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&uptime, sizeof(uptime)), "CMD_AntGetUptime - TransmitDataAsSPL_Packet");
}

/*
 * cancel deployment according to side.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the ant side and the headers we add)
 * @return type=int; return type of error according to this
 * 																-1 cmd NULL
 * 																-2 if length isn't in size.
 * 																-3 got wrong side (not exist one)
 * 																else from <hal/errors.h>
 * */
int CMD_AntCancelDeployment(sat_packet_t *cmd)
{
	unsigned char error_ack;
	if(cmd == NULL) return -1;
	int error;
	char side;
	error = GetAntSide(cmd, &side);
	if(error) return error;
	if(side == 'A')
		error = IsisAntS_cancelDeployment(0, isisants_sideA);
	else if(side == 'B')
		error = IsisAntS_cancelDeployment(0, isisants_sideB);
	else
	{
		error_ack = ERROR_SIDE_ANTS_NOT_A_OR_B;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return -3;
	}
	if(error)
	{
		error_ack = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return logError(SendAckPacket(ACK_ANT_CANCEL_DEPLOY , cmd, NULL, 0), "CMD_AntCancelDeployment - SendAckPacket");
}

/*
* Send ack ping
* @param[in] name=cmd; type=sat_packet_t*; Not needed can be NULL
* @return type=int; according to errors <hal/errors.h>
* */
int CMD_Ping(sat_packet_t *cmd)
{
	unsigned char pong_msg[] = "pong";
	return SendAckPacket(ACK_PING , cmd, pong_msg, sizeof(pong_msg)); // send ack ping (says pong)
}
