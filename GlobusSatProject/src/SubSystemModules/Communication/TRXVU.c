/*
 * TRXVU.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include <hal/errors.h>
#include <string.h>

#include "SubSystemModules/Housekeeping/TelemetryCollector.h"
#include "satellite-subsystems/IsisTRXVU.h"
#include "satellite-subsystems/IsisAntS.h"

#include "SatCommandHandler.h"
#include "TRXVU.h"
#include "SysI2CAddr.h"
#include "AckHandler.h"
#include "SPL.h"

/*#define WE_HAVE_ANTS 0*/

time_unix lastTimeSendingBeacon;
time_unix period;

/*
 * Initialize the TRXVU and ants.
 *
 * @return error according to <hal/errors.h>
 * */
int InitTrxvuAndAnts(){
	// Definition of I2C and TRXVU
	ISIStrxvuI2CAddress myTRXVUAddress[1];
	ISIStrxvuFrameLengths myTRXVUBuffers[1];
	ISIStrxvuBitrate myTRXVUBitrates[1];
    int rv;

	//I2C addresses defined
	myTRXVUAddress[0].addressVu_rc = I2C_TRXVU_RC_ADDR;
	myTRXVUAddress[0].addressVu_tc = I2C_TRXVU_TC_ADDR;
	//Buffer definition
	myTRXVUBuffers[0].maxAX25frameLengthTX = SIZE_TXFRAME;
	myTRXVUBuffers[0].maxAX25frameLengthRX = SIZE_RXFRAME;
	//Bitrate definition
	myTRXVUBitrates[0] = trxvu_bitrate_9600;
	//Initialize the trxvu subsystem
	rv = IsisTrxvu_initialize(myTRXVUAddress, myTRXVUBuffers, myTRXVUBitrates, 1);
	//Get beacon interval from FRAM
	logError(FRAM_read((unsigned char*)&period, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_read");
#ifdef WE_HAVE_ANTS
	int retValInt = 0;
	ISISantsI2Caddress myAntennaAddress[2];
	myAntennaAddress[0].addressSideA = ANTS_I2C_SIDE_A_ADDR;
	myAntennaAddress[0].addressSideB = ANTS_I2C_SIDE_B_ADDR;
	int errorAnts = IsisAntS_initialize(myAntennaAddress, 1);

	logError(errorAnts, "Ants - IsisAntS_initialize")
	logError(rv, "TRXVU - IsisTrxvu_initialize");


	//Initialize the AntS system
	return errorAnts + rv;
#else
	return logError(rv, "TRXVU - IsisTrxvu_initialize");
#endif
}

/*
 * Gets number of packets in waiting.
 * @return type=int; -1 on error
 * 					 number of packets on success
 **/
int GetNumberOfFramesInBuffer()
{
	unsigned short frameCount;
	int err = logError(IsisTrxvu_rcGetFrameCount(0, &frameCount), "TRXVU - IsisTrxvu_rcGetFrameCount"); // Get number of packets in buffer
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
	ISIStrxvuRxFrame rx_frame = {0,0,0, rxframebuffer}; // Where the packet saved after read
	int error = logError(IsisTrxvu_rcGetCommandFrame(0, &rx_frame), "TRXVU - IsisTrxvu_rcGetCommandFrame"); // Get packet
	if(error != E_NO_SS_ERR)
		return execution_error;
	error = ParseDataToCommand(rx_frame.rx_framedata, cmd); // Put the info from the packet in the cmd parameter
	return error;
}

/*
 * Send Ax25 packet
 * @param name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @param name=avalFrames; type=int*; availed frames
 * @return type=int; return -1 on cmd NULL
 * 							Error code according to <hal/errors.h>
 * */
int TransmitSplPacket(sat_packet_t *packet, int *avalFrames)
{
	unsigned char avail;
	if(packet == NULL)
		return -1;
	int place = sizeof(packet->ID) + sizeof(packet->cmd_subtype) + sizeof(packet->cmd_type) + sizeof(packet->length) + packet->length; // Get the length of the data of the packet (including the headers we add)
	int error = logError(IsisTrxvu_tcSendAX25DefClSign(0, (unsigned char *)packet, place, &avail), "TRXVU - IsisTrxvu_tcSendAX25DefClSign");  // Transmit packet
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
 * 							Error code according to <hal/errors.h>
 * */
int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data, unsigned short length)
{
	unsigned char avail;
	if(cmd == NULL)
		return -1;
	if(AssembleCommand(data, length, cmd->cmd_type, cmd->cmd_subtype, cmd->ID, cmd)) return -2; // Change the packet for send with the needed info
	int place = sizeof(cmd->ID) + sizeof(cmd->cmd_subtype) + sizeof(cmd->cmd_type) + sizeof(cmd->length) + cmd->length; // Get the length of the data of the packet (including the headers we add)
	return logError(IsisTrxvu_tcSendAX25DefClSign(0, (unsigned char *)cmd, place, &avail), "TRXVU - IsisTrxvu_tcSendAX25DefClSign"); // Transmit packet
}

Boolean CheckTransmitionAllowed()
{
	//TODO: all the function
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
	if(!(CheckExecutionTime(lastTimeSendingBeacon, period) && CheckTransmitionAllowed())) // Check if we can transmit beacon and also if the period of time we need to wait pass
		return -1;
	sat_packet_t beacon;
	short length = sizeof(WOD_Telemetry_t);
	WOD_Telemetry_t data;
	GetCurrentWODTelemetry(&data); // Gets the telemetry of the beacon and put it in data.
	logError(AssembleCommand((unsigned char *)&data, length, trxvu_cmd_type, BEACON_SUBTYPE, CUBE_SAT_ID, &beacon), "Beacon - Assemble command"); // Create the beacon packet
	int avalFrames;
	int error = logError(TransmitSplPacket(&beacon, &avalFrames), "TRXVU - IsisTrxvu_tcSendAX25DefClSign"); // Send the beacon packet
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
