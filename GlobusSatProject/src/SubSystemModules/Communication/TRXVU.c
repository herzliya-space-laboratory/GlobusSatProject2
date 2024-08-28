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

time_unix lastTimeSendingBeacon = 0;
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
	unsigned char beaconIntervalTime[BEACON_INTERVAL_TIME_SIZE];
	logError(FRAM_read(beaconIntervalTime, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "InitTrxvu - FRAM_read");
	period = (time_unix)beaconIntervalTime;
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

int GetNumberOfFramesInBuffer()
{
	unsigned short frameCount;
	int err = logError(IsisTrxvu_rcGetFrameCount(0, &frameCount), "TRXVU - IsisTrxvu_rcGetFrameCount");
	if(err != E_NO_SS_ERR)
		return -1;
	return frameCount;
}

CMD_ERR GetOnlineCommand(sat_packet_t *cmd)
{
	unsigned char rxframebuffer[SIZE_RXFRAME] = {0};
	ISIStrxvuRxFrame rx_frame = {0,0,0, rxframebuffer};
	int error = logError(IsisTrxvu_rcGetCommandFrame(0, &rx_frame), "TRXVU - IsisTrxvu_rcGetCommandFrame");
	if(error != E_NO_SS_ERR)
		return execution_error;
	error = ParseDataToCommand(rx_frame.rx_framedata, cmd);
	return error;
}

int TransmitSplPacket(sat_packet_t *packet, int *avalFrames)
{
	unsigned char avail;
	if(packet == NULL)
		return -1;
	int place = sizeof(packet->ID) + sizeof(packet->cmd_subtype) + sizeof(packet->cmd_type) + sizeof(packet->length) + packet->length;
	int error = logError(IsisTrxvu_tcSendAX25DefClSign(0, (unsigned char *)packet, place, &avail), "TRXVU - IsisTrxvu_tcSendAX25DefClSign");
	*avalFrames = (int)avail;
	return error;
}

int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data, unsigned short length)
{
	unsigned char avail;
	if(cmd == NULL)
		return -1;
	if(AssembleCommand(data, length, cmd->cmd_type, cmd->cmd_subtype, cmd->ID, cmd)) return -2;
	int place = sizeof(cmd->ID) + sizeof(cmd->cmd_subtype) + sizeof(cmd->cmd_type) + sizeof(cmd->length) + cmd->length;
	return logError(IsisTrxvu_tcSendAX25DefClSign(0, (unsigned char *)cmd, place, &avail), "TRXVU - IsisTrxvu_tcSendAX25DefClSign");
}

Boolean CheckTransmitionAllowed()
{
	//TODO: all the function
	return TRUE;
}

int BeaconLogic()
{
	if(CheckExecutionTime(lastTimeSendingBeacon, period) || CheckTransmitionAllowed())
		return -1;
	sat_packet_t beacon;
	short length = sizeof(WOD_Telemetry_t);
	unsigned char data[] = {0, 0, 0, 0};
	//TODO: get data from the sat
	logError(AssembleCommand(data, length, trxvu_cmd_type, BEACON_SUBTYPE, CUBE_SAT_ID, &beacon), "Beacon - Assemble command");
	int avalFrames;
	int error = logError(TransmitSplPacket(&beacon, &avalFrames), "TRXVU - IsisTrxvu_tcSendAX25DefClSign");
	if(error)
		return error;
	return logError(Time_getUnixEpoch((unsigned int*)&lastTimeSendingBeacon), "TRXVU - Time_getUnixEpoch");

}

int TRX_Logic()
{
	sat_packet_t cmd;
	int error = 0;
	BeaconLogic();

	//TODO: send beacon every 20 second and update the past time
	if(GetNumberOfFramesInBuffer() > 0)
	{
		error = GetOnlineCommand(&cmd);
		if(error != command_succsess)
			return logError(error, "GetOnlineCommand");
		SendAckPacket(ACK_RECEIVE_COMM, &cmd, NULL, 0);
		ActUponCommand(&cmd);
	}
	return 0;
}
