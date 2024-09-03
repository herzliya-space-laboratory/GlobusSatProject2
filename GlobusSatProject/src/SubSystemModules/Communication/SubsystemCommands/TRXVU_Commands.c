/*
 * TRXVU_Commands.c
 *
 *  Created on: 3 9 2024
 *      Author: maayan
 */

#include "TRXVU_Commands.h"

int CMD_GetBeacon_Interval(sat_packet_t *cmd)
{
	time_unix period;
	int error;
	error = logError(FRAM_read((unsigned char*)&period, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_read");
	if(error)
	{
		unsigned char error_msg[] = "CMD_GetBeacon_Interval - Can't read from FRAM";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg));
		return error;
	}
	if(cmd == NULL)
		return -1;
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&period, BEACON_INTERVAL_TIME_SIZE), "CMD_GetBeacon_Interval - TransmitDataAsSPL_Packet");
}

int CMD_SetBeacon_Interval(sat_packet_t *cmd)
{
	if(cmd == NULL)
		return -1;
	time_unix new_interval;
	int error;
	memcpy(&new_interval, &cmd->data, cmd->length);
	if(new_interval >= MAX_BEACON_INTERVAL)
		new_interval = MAX_BEACON_INTERVAL;
	else if(new_interval <= MIN_BEACON_INTERVAL)
		new_interval = MIN_BEACON_INTERVAL;
	error = logError(FRAM_write((unsigned char*)&new_interval, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_write");
	if(error)
	{
		unsigned char error_msg[] = "CMD_SetBeacon_Interval - Can't write to FRAM";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg));
		return error;
	}
	time_unix check;
	error = logError(FRAM_read((unsigned char*)&check, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_read");
	if(error)
	{
		unsigned char error_msg[] = "CMD_SetBeacon_Interval - Can't read from FRAM";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg));
		return error;
	}
	if(check != new_interval)
	{
		unsigned char error_msg[] = "CMD_SetBeacon_Interval - didn't write the right number in FRAM";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg));
		return error;
	}

	return logError(SendAckPacket(ACK_UPDATE_BEACON_INTERVAL , cmd, (unsigned char*)&new_interval, sizeof(new_interval)), "CMD_SetBeacon_Interval - SendAckPacket");
}


int CMD_SetOff_Transponder(sat_packet_t *cmd)
{
	if(cmd == NULL)
		return -1;
	unsigned char data[] = {0x38, 0x01};
	int error = logError(I2C_write(I2C_TRXVU_TC_ADDR, data, 2), "CMD_SetOff_Transponder - I2C_write");
	if(error)
	{
		unsigned char error_msg[] = "CMD_SetOff_Transponder - can't turn off transponder. Probably a fault in I2C write";
		SendAckPacket(ACK_ERROR_MSG , cmd, error_msg, sizeof(error_msg));
		return error;
	}
	return logError(SendAckPacket(ACK_TRANSPONDER_OFF , cmd, (unsigned char*)"Transponder off", sizeof("Transponder off")), "CMD_SetOff_Transponder - SendAckPacket");

}
