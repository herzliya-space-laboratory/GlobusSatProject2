/*
 * TRXVU.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include <hal/errors.h>
#include <string.h>
#include <hal/Drivers/I2C.h>
#include "SubSystemModules/Housekeeping/TelemetryCollector.h"

#include "SatCommandHandler.h"
#include "TRXVU.h"
#include "SysI2CAddr.h"
#include "AckHandler.h"
#include "SPL.h"


time_unix lastTimeSendingBeacon;
time_unix period;

/*
 * Initialize the TRXVU and ants.
 *
 * @return error according to <hal/errors.h>
 * */
int InitTrxvuAndAnts(){
	// Definition of I2C and TRXVU
    ISIS_VU_E_t myTRXVU[1];
    driver_error_t rv;

	//I2C addresses defined
    myTRXVU[0].rxAddr = I2C_TRXVU_RC_ADDR;
    myTRXVU[0].txAddr = I2C_TRXVU_TC_ADDR;

	//Buffer definition
    myTRXVU[0].maxSendBufferLength = SIZE_TXFRAME;
    myTRXVU[0].maxReceiveBufferLength = SIZE_RXFRAME;

	//Initialize the trxvu subsystem
	rv = ISIS_VU_E_Init(myTRXVU, 1);
	isis_vu_e__set_bitrate(0, isis_vu_e__bitrate__9600bps);

	isis_vu_e__set_tx_freq(0, 436400);
	isis_vu_e__set_rx_freq(0, 145970);
	//TODO: transponder set freq

	//Get beacon interval from FRAM
	setNewBeaconIntervalToPeriod();
	time_unix timeNow;
	logError(Time_getUnixEpoch((unsigned int*)&timeNow), "InitTrxvuAndAnts - Time_getUnixEpoch");
	if(timeNow < getTransponderEndTime())
		setTransponderOn();
#ifdef WE_HAVE_ANTS
	int retValInt = 0;
	ISIS_ANTS_t myAntennaAddress;
	myAntennaAddress.i2cAddr = ANTS_I2C_ADDR;
	int errorAnts = ISIS_ANTS_Init(myAntennaAddress, 1);

	logError(errorAnts, "Ants - ISIS_ANTS_REV2_Init")
	logError(rv, "TRXVU - ISIS_VU_E_Init");


	//Initialize the AntS system
	if(rv == 6) return erroAnts;
	return errorAnts + rv;
#else
	return logError(rv, "TRXVU - ISIS_VU_E_Init");
#endif
}

/*
 * set transponder on
 * return type=int; according to I2C_write error list
 * */
int setTransponderOn()
{
	return logError(isis_vu_e__set_tx_mode(0, trxvu_transponder_on), "setTransponderOn - isis_vu_e__set_tx_mode"); // Set transponder off
}

/*
 * set transponder off
 * return type=int; according to I2C_write error list
 * */
int setTransponderOff()
{
	return logError(isis_vu_e__set_tx_mode(0, trxvu_transponder_off), "CMD_SetOff_Transponder - isis_vu_e__set_tx_mode"); // Set transponder off
}

/*
 * Check if we pass the time of the transponder and if so get out of this state.
 * return type=int; -1 on error in getTransponderEndTime
 * 					0 on success
 * 					error according to I2C_write errors
 * */
int turnOffTransponder()
{
	time_unix timeNow;
	int error = logError(Time_getUnixEpoch((unsigned int*)&timeNow), "turnOffTransponder - Time_getUnixEpoch");
	if(error) return error;
	time_unix timeEnd = getTransponderEndTime();
	if(timeEnd == 0) return -1;
	if(timeEnd > timeNow)
		return 0;
	error = setTransponderOff();
	if(error) return error;
	time_unix new = 0;
	return setTransponderEndTime(new);
}

/*
 * Check if we pass the time of the idle and if so get out of this state.
 * return type=int; -1 on error in getIdleEndTime
 * 					0 on success
 * 					error according to SetIdleState errors
 * */
int turnOffIdle()
{
	time_unix timeNow;
	int error = logError(Time_getUnixEpoch((unsigned int*)&timeNow), "turnOffTransponder - Time_getUnixEpoch");
	if(error) return error;
	time_unix timeEnd = getIdleEndTime();
	if(timeEnd == 0) return -1;
	if(timeEnd > timeNow)
		return 0;
	return SetIdleState(isis_vu_e__onoff__off, 0);
}

/*
 * Sets Mute time end value in FRAM and check
 * @return type=int;	-1 on error
 * 						-2 on wrong set
 * 						0 on success
 */
int setMuteEndTime(time_unix endTime)
{
	time_unix timeNow;
	if(endTime > MAX_MUTE_TIME) // check we are in range and if not round it.
		endTime = MAX_MUTE_TIME;
	if(logError(Time_getUnixEpoch((unsigned int*)&timeNow), "CMD_UnMuteTRXVU - Time_getUnixEpoch"))
		return -1;
	endTime += timeNow;
	if(logError(FRAM_write((unsigned char*)&endTime, MUTE_END_TIME_ADDR, MUTE_END_TIME_SIZE), "setMuteEndTime - FRAM_write"))
		return -1;
	time_unix check = getMuteEndTime(); //TODO: need explanation for what to do if can't read
	if(check != endTime)
		return logError(-2, "setMuteEndTime - Not written what needed to be");
	return 0;
}

/*
 * Gets Mute time end value from FRAM
 * @return type=time_unix; 0 on fail
 * 						   mute end time on success
 */
time_unix getMuteEndTime()
{
	time_unix muteEndTime = 0;
	if(logError(FRAM_read((unsigned char*)&muteEndTime, MUTE_END_TIME_ADDR, MUTE_END_TIME_SIZE), "getMuteEndTime - FRAM_read"))
		return 0;
	return muteEndTime;
}

/*
 * Sets transponder end time value from FRAM
 * @param[int] name=transponderEndTime; type=time_unix; get the time we sat as the transponder end time
 * @return type=int; 	0 on success
 * 						error according to <hal/errors.h>
 */
int setTransponderEndTime(time_unix transponderEndTime)
{
	if(transponderEndTime > MAX_TRANS_TIME)
		transponderEndTime = MAX_TRANS_TIME;
	return logError(FRAM_write((unsigned char*)&transponderEndTime, TRANSPONDER_END_TIME_ADDR, TRANSPONDER_END_TIME_SIZE), "setTransponderEndTime - FRAM_write");

}



/*
 * Gets transponder end time value from FRAM
 * @return type=time_unix; 0 on fail
 * 						   Transponder end time on success
 */
time_unix getTransponderEndTime()
{
	time_unix transponderEndTime;
	if(logError(FRAM_read((unsigned char*)&transponderEndTime, TRANSPONDER_END_TIME_ADDR, TRANSPONDER_END_TIME_SIZE), "getTransponderEndTime - FRAM_read"))
		return 0;
	return transponderEndTime;
}

/*
 * Gets Idle end time value from FRAM
 * @return type=time_unix; 0 on fail
 * 						   mute end time on success
 */
time_unix getIdleEndTime()
{
	time_unix idleEndTime;
	if(logError(FRAM_read((unsigned char*)&idleEndTime, IDLE_END_TIME_ADDR, IDLE_END_TIME_SIZE), "getIdleEndTime - FRAM_read"))
		return 0;
	return idleEndTime;
}

/**
 * Sets transponder RSSI value in FRAM and check
 * @return type=int;	-1 on error
 * 						-2 on wrong set
 * 						-3 on I2C_write error
 * 						0 on success
 *
 */
int setTransponderRSSIinFRAM(short val)
{
	if(val < 0) //min according to TRXVU Transponder Mode Addendum (in drive)
		val = 0;
	else if(val > 4095) //max according to TRXVU Transponder Mode Addendum (in drive)
		val = 4095;
	unsigned char data[] = {0x52, val,0};
	if(logError(I2C_write(I2C_TRXVU_TC_ADDR, data, 3), "setTransponderRSSIinFRAM - I2C_write"))
			return -3;
	if(logError(FRAM_write((unsigned char*)&val, TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE), "setTransponderRSSIinFRAM - FRAM_write"))
		return -1;
	short check = 0;
	logError(FRAM_read((unsigned char*)&check, TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE), "setTransponderRSSIinFRAM - FRAM_read");
	if(check != val)
		return logError(-2, "setTransponderRSSIinFRAM - Not written what needed to be");
	return 0;
}

/**
 * Gets transponder RSSI value from FRAM
 * @return type=short; RSSI value from FRAM, -1 on error
 */
short getTransponderRSSIFromFRAM()
{
	short rssi_val;
	if(logError(FRAM_read((unsigned char*)&rssi_val, TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE), "getTransponderRSSIFromFRAM - FRAM_read"))
		return -1;
	return rssi_val;
}

/*
 * Sets the new beacon interval in period.
 * return type=int; error according to <hal/errors.h>
 * */
int setNewBeaconIntervalToPeriod()
{
	return logError(FRAM_read((unsigned char*)&period, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_read");
}

/*
 * @brief set the idle state of the trxvu
 * @param[in] state ON/OFF
 * @param[in] duration for how long will the satellite be in idle state, if state is OFF than this value is ignored
 * @return type=int;	0 in successful
 * 						-1 in failure
 * 						-2 FRAM read problem
 * 						-3 FRAM write problem
 * 						-4 wrong time set
 * 						-5 time problem
 * 						-6 not on or off
 */
int SetIdleState(isis_vu_e__onoff_t state, time_unix duration)
{
	if(state == isis_vu_e__onoff__on)
	{
		if(logError(isis_vu_e__set_idle_state(ISIS_TRXVU_I2C_BUS_INDEX, isis_vu_e__onoff__on), "SetIdleState - IsisTrxvu_tcSetIdlestate"))
			return -1;
		if(duration > MAX_IDLE_TIME)
			duration = MAX_IDLE_TIME;
		time_unix timeNow = 0;
		if(logError(Time_getUnixEpoch((unsigned int*)&timeNow), "CMD_UnMuteTRXVU - Time_getUnixEpoch"))
			return -5;
		duration += timeNow;
		if(logError(FRAM_write((unsigned char*)&duration, IDLE_END_TIME_ADDR, IDLE_END_TIME_SIZE), "SetIdleState - FRAM_write"))
			return -3;
		time_unix check = 0;
		if(logError(FRAM_read((unsigned char*)&check, IDLE_END_TIME_ADDR, IDLE_END_TIME_SIZE), "SetIdleState - FRAM_read"))
			return -2;
		if(check != duration)
			return logError(-4, "SetIdleState - Not written what needed to be");
	}

	if(state == isis_vu_e__onoff__off)
	{
		if(logError(isis_vu_e__set_idle_state(ISIS_TRXVU_I2C_BUS_INDEX, isis_vu_e__onoff__off), "SetIdleState - IsisTrxvu_tcSetIdlestate"))
				return -1;
		duration = 0;
		if(logError(FRAM_write((unsigned char*)&duration, IDLE_END_TIME_ADDR, IDLE_END_TIME_SIZE), "SetIdleState - FRAM_write"))
			return -3;
		time_unix check = 0;
		if(logError(FRAM_read((unsigned char*)&check, IDLE_END_TIME_ADDR, IDLE_END_TIME_SIZE), "SetIdleState - FRAM_read"))
			return -2;
		if(check != duration)
			return logError(-4, "SetIdleState - Not written what needed to be");
	}

	return -6;
}

/*
 * Gets number of packets in waiting.
 * @return type=int; -1 on error
 * 					 number of packets on success
 **/
int GetNumberOfFramesInBuffer()
{
	unsigned short frameCount;
	int err = logError(isis_vu_e__get_frame_count(0, &frameCount), "TRXVU - IsisTrxvu_rcGetFrameCount"); // Get number of packets in buffer
	if(err != E_NO_SS_ERR)
		return -1;
	return frameCount;
}

/*
 * Get commend from the buffer and divide the info according to sat_packet_t headers
 * @param[out] name=cmd; type=sat_packet_t *; Put here the info from packet according to the sat_packet_t struct.
 * @return type=CMD_ERR; return command_success on success
 * 								and error according to CMD_ERR
 * */
CMD_ERR GetOnlineCommand(sat_packet_t *cmd)
{
	unsigned char rxframebuffer[SIZE_RXFRAME] = {0};
	isis_vu_e__get_frame__from_t rx_frame = {0,0,0, rxframebuffer}; // Where the packet saved after read
	int error = logError(isis_vu_e__get_frame(0, &rx_frame), "TRXVU - IsisTrxvu_rcGetCommandFrame"); // Get packet
	isis_vu_e__remove_frame(0);
	if(error != E_NO_SS_ERR)
		return execution_error;
	error = ParseDataToCommand(rx_frame.data, cmd); // Put the info from the packet in the cmd parameter
	return error;
}

/*
 * Send Ax25 packet
 * @param name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @param name=avalFrames; type=int*; availed frames
 * @return type=int; return -1 on cmd NULL
 * 							-2 if we can't transmmit
 * 							Error code according to <hal/errors.h>
 * */
int TransmitSplPacket(sat_packet_t *packet, int *avalFrames)
{
	if(!CheckTransmitionAllowed())
		return -2;
	unsigned char avail;
	if(packet == NULL)
		return -1;
	size_t place = sizeof(packet->ID) + sizeof(packet->cmd_subtype) + sizeof(packet->cmd_type) + sizeof(packet->length) + packet->length; // Get the length of the data of the packet (including the headers we add)
	int error = logError(isis_vu_e__send_frame(0, (unsigned char *)packet, place, &avail), "TRXVU - isis_vu_e__send_frame");  // Transmit packet
	*avalFrames = (int)avail; // Get avail Frames
	return error;
}

/*
 * Send Ax25 packet with output data
 * @param name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @param name=data; type=unsigned char*; The data we want to send
 * @param name=length; type=unsigned short; length of data
 * @return type=int; return -1 on cmd NULL
 * 							-2 on fail in Assemble commend
 *	 						-3 if we can't transmmit
 * 							Error code according to <hal/errors.h>
 * */
int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data, unsigned short length)
{
	if(!CheckTransmitionAllowed())
			return -3;
	unsigned char avail;
	if(cmd == NULL)
		return -1;
	if(AssembleCommand(data, length, cmd->cmd_type, cmd->cmd_subtype, cmd->ID, cmd)) return -2; // Change the packet for send with the needed info
	size_t place = sizeof(cmd->ID) + sizeof(cmd->cmd_subtype) + sizeof(cmd->cmd_type) + sizeof(cmd->length) + cmd->length; // Get the length of the data of the packet (including the headers we add)
	return logError(isis_vu_e__send_frame(0, (unsigned char *)cmd, place, &avail), "TRXVU - isis_vu_e__send_frame"); // Transmit packet
}

/*
 * Check if we can transmit. (according to mute and EPS condition)
 * @return type=Boolean; TRUE if we can transmit
 * 						 FALSE if we can't
 */
Boolean CheckTransmitionAllowed()
{
	if(GetTxFlag()) return FALSE;
	time_unix timeNow;
	logError(Time_getUnixEpoch((unsigned int*)&timeNow), "CMD_UnMuteTRXVU - Time_getUnixEpoch");
	if(timeNow < getMuteEndTime()) // check we are after the mute end time.
		return FALSE;
	return TRUE;
}

/*
 * @brief transmits beacon according to beacon logic
 * @ return 0 if everything is fine
 * 			-1 if we not suppose to send beacon now
 * 	 	else it return error according to the stuff that didn't work
 */
int BeaconLogic()
{
	if(!(CheckExecutionTime(lastTimeSendingBeacon, period) || !CheckTransmitionAllowed())) // Check if we can transmit beacon and also if the period of time we need to wait pass
		return -1;
	sat_packet_t beacon;
	short length = sizeof(WOD_Telemetry_t);
	WOD_Telemetry_t data;
	GetCurrentWODTelemetry(&data); // Gets the telemetry of the beacon and put it in data.
	logError(AssembleCommand((unsigned char *)&data, length, trxvu_cmd_type, BEACON_SUBTYPE, CUBE_SAT_ID, &beacon), "Beacon - Assemble command"); // Create the beacon packet
	int avalFrames;
	int error = logError(TransmitSplPacket(&beacon, &avalFrames), "TRXVU - isis_vu_e__send_frame"); // Send the beacon packet
	if(error) // if error return it
		return error;
	return logError(Time_getUnixEpoch((unsigned int*)&lastTimeSendingBeacon), "TRXVU - Time_getUnixEpoch"); // Check last time we send beacon to now.

}

/*
 * Have the TRXVU logic. (Beacon send, check if have packets, read packet etc.)
 *@return type=int; return error if have and command_succsess if not
 **/
int TRX_Logic()
{
	sat_packet_t cmd;
	int error = 0;
	turnOffTransponder();
	turnOffIdle();
	BeaconLogic(); // do the beacon logic
	if(GetNumberOfFramesInBuffer() > 0) // Check if we have packets waiting
	{ // if so
		error = GetOnlineCommand(&cmd); // Get the packet and put her in the sat_packet_t struct (in the param cmd)
		if(error != command_success) // Check if have error
			return logError(error, "GetOnlineCommand");
		SendAckPacket(ACK_RECEIVE_COMM, &cmd, NULL, 0); // Send ack of receiving the packet
		ActUponCommand(&cmd); // Go to do the command
	}
	return command_success;
}
