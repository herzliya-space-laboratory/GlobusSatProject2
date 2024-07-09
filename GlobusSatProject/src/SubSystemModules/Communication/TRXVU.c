/*
 * TRXVU.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include <hal/errors.h>

#include "satellite-subsystems/IsisTRXVU.h"
#include "satellite-subsystems/IsisAntS.h"

#include "SatCommandHandler.h"
#include "TRXVU.h"
#include "SysI2CAddr.h"

/*#define WE_HAVE_ANTS 0*/


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

int TRX_Logic()
{
	sat_packet_t cmd;
	unsigned char rxframebuffer[SIZE_RXFRAME] = {0};
	ISIStrxvuRxFrame rx_frame = {0,0,0, rxframebuffer};
	int error = 0;
	if(GetNumberOfFramesInBuffer() > 0)
	{
		error = logError(IsisTrxvu_rcGetCommandFrame(0, &rx_frame), "TRXVU - IsisTrxvu_rcGetCommandFrame");
		if(error != E_NO_SS_ERR)
			return error;
		error = ParseDataToCommand(rx_frame.rx_framedata, &cmd);
		if(error != command_succsess)
			return error;
		ActUponCommand(&cmd);
	}
	return 0;
}
