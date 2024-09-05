/*
 * TRXVU_Commands.c
 *
 *  Created on: 3 9 2024
 *      Author: maayan
 */

#include "TRXVU_Commands.h"

/*
* The command get the beacon interval from the FRAM and send it back.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; return type of error and if the parameter (cmd) is NULL return -1.
* */
int CMD_GetBeacon_Interval(sat_packet_t *cmd)
{
	time_unix period;
	int error;
	error = logError(FRAM_read((unsigned char*)&period, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_read"); // Read the beacon interval from FRAM
	if(error)
	{
		unsigned char error_msg[] = "CMD_GetBeacon_Interval - Can't read from FRAM";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (couldn't read from FRAM)
		return error;
	}
	if(cmd == NULL)
		return -1;
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&period, BEACON_INTERVAL_TIME_SIZE), "CMD_GetBeacon_Interval - TransmitDataAsSPL_Packet"); // Send back the beacon interval
}

/*
* The command change the beacon interval in the FRAM and check it change correctly. also check the new interval is between the max and min interval
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the new_interval and the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1, -2 if length isn't in size.
* */
int CMD_SetBeacon_Interval(sat_packet_t *cmd)
{
	if(cmd == NULL)
		return -1;
	if(cmd->length != 4)
	{
		unsigned char error_msg[] = "CMD_SetBeacon_Interval - the length isn't in size";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (wrong length)
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
		unsigned char error_msg[] = "CMD_SetBeacon_Interval - Can't write to FRAM";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (couldn't write to FRAM)
		return error;
	}
	time_unix check;
	error = logError(FRAM_read((unsigned char*)&check, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_read"); // Read from FRAM in the place we wrote to for check
	if(error)
	{
		unsigned char error_msg[] = "CMD_SetBeacon_Interval - Can't read from FRAM";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (couldn't read from FRAM)
		return error;
	}
	setNewBeaconIntervalToPeriod();
	if(check != new_interval) // Check if what we wrote and what have been written is the same
	{
		unsigned char error_msg[] = "CMD_SetBeacon_Interval - didn't write the right number in FRAM. To check what is the number that written use the command CMD_GetBeacon_Interval";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (written the wrong number)
		return error;
	}

	return logError(SendAckPacket(ACK_UPDATE_BEACON_INTERVAL , cmd, (unsigned char*)&new_interval, sizeof(new_interval)), "CMD_SetBeacon_Interval - SendAckPacket"); // // Send ack with the new_interval with subtype of ACK_UPDATE_BEACON_INTERVAL
}

/*
* The command sets the transponder off.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; return type of error.
* */
int CMD_SetOff_Transponder(sat_packet_t *cmd)
{
	unsigned char data[] = {0x38, trxvu_transponder_off}; // 0x38 - number of commend to change the transmitter mode.
	int error = logError(I2C_write(I2C_TRXVU_TC_ADDR, data, 2), "CMD_SetOff_Transponder - I2C_write"); // Set transponder off
	if(error)
	{
		unsigned char error_msg[] = "CMD_SetOff_Transponder - can't turn off transponder. Probably a fault in I2C write";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (couldn't turn off)
		return error;
	}
	return logError(SendAckPacket(ACK_TRANSPONDER_OFF , cmd, (unsigned char*)"Transponder off", sizeof("Transponder off")), "CMD_SetOff_Transponder - SendAckPacket"); // Send ack of success in turn off transponder
}

/*
 * Change the end time of mute to unmute the transmiter.
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error.
 * */
int CMD_UnMuteTRXVU(sat_packet_t *cmd)
{
	time_unix timeNow;
	int error = logError(Time_getUnixEpoch((unsigned int*)&timeNow), "CMD_UnMuteTRXVU - Time_getUnixEpoch"); //get time now
	if(error)
	{
		unsigned char error_msg[] = "CMD_UnMuteTRXVU - can't get the time";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (couldn't get time)
		return error;
	}
	error = setMuteEndTime(timeNow); // set new end time to time now
	if(error == -2)
	{
		unsigned char error_msg[] = "CMD_UnMuteTRXVU - written the wrong number in FRAM";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (written wrong number)
		return error;
	}
	else if(error)
	{
		unsigned char error_msg[] = "CMD_UnMuteTRXVU - can't set new end time";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (couldn't set new end time)
		return error;
	}
	return logError(SendAckPacket(ACK_UNMUTE , cmd, NULL, 0), "CMD_UnMuteTRXVU - SendAckPacket"); // Send ack of success in unmuting the transmitter
}

/*
* The command change the rssi value in the FRAM and check it change correctly. also check the new rssi value is between the 0 and 4095
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the new rssi val and the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1 and on error in setTransponderRSSIinFRAM, -2 if length isn't in size.
 * */
int CMD_SetRSSI_Transponder(sat_packet_t *cmd)
{
	if(cmd == NULL)
		return -1;
	short new_rssi_val = -1;
	if(cmd->length != 2)
	{
		unsigned char error_msg[] = "CMD_SetRSSI_Transponder - the length isn't in size";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (wrong length)
		return -2;
	}
	memcpy(&new_rssi_val, &cmd->data, cmd->length);
	int error = setTransponderRSSIinFRAM(new_rssi_val);
	if(error == -2)
	{
		unsigned char error_msg[] = "CMD_SetRSSI_Transponder - written the wrong number in FRAM";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (couldn't set new rssi)
		return error;
	}
	else if(error)
	{
		unsigned char error_msg[] = "CMD_SetRSSI_Transponder - can't set new rssi";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (couldn't set new rssi)
		return error;
	}
	return logError(SendAckPacket(ACK_UPDATE_RSSI_VALUE , cmd, (unsigned char*)&new_rssi_val, sizeof(new_rssi_val)), "CMD_SetRSSI_Transponder - SendAckPacket"); // Send ack of success in change rssi and to what
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

/*
 * Set transmitter to mute for the time written in the data in cmd
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the mute duration and the headers we add)
 * @return type=int; -1 on cmd NULL
 * 					 -3 on incorrect length
 * 					 errors according to setMuteEndTime
 * */
int CMD_MuteTRXVU(sat_packet_t *cmd)
{
	if(cmd == NULL)
		return -1;
	if(cmd->length != 4)
	{
		unsigned char error_msg[] = "CMD_MuteTRXVU - the length isn't in size";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (wrong length)
		return -3;
	}
	time_unix muteEndTime;
	memcpy(&muteEndTime, cmd->data, cmd->length);
	if(muteEndTime > MAX_MUTE_TIME)
		muteEndTime = MAX_MUTE_TIME;
	logError(SendAckPacket(ACK_MUTE , cmd, (unsigned char*)&muteEndTime, sizeof(muteEndTime)), "CMD_MuteTRXVU - SendAckPacket"); // Send ack of success in change rssi and to what
	int error = setMuteEndTime(muteEndTime);
	if(error == -2)
	{
		unsigned char error_msg[] = "CMD_MuteTRXVU - written the wrong number in FRAM";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (couldn't set new rssi)
		return error;
	}
	else if(error)
	{
		unsigned char error_msg[] = "CMD_MuteTRXVU - can't set end time";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg)); // Send ack error that says what written in error_msg (couldn't set new rssi)
		return error;
	}
	return 0;
}

