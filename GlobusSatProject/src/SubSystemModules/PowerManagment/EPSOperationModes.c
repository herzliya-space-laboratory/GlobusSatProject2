/*
 * EPSOperationModes.c
 *
 *  Created on: 21 Nov 2024
 *      Author: maayan
 */

#include "EPSOperationModes.h"
#include "utils.h"
#include "hal/Drivers/I2C.h"

Boolean txOff = FALSE;
Boolean payloadOff = FALSE;
EpsState_t satState = OperationalMode;
/*!
 * @brief Executes the necessary procedure in order to initiate the system into Operational mode
 * @return	0 on success
 * 			errors according to <hal/errors.h>
 */
int EnterOperationalMode()
{
	printf("entered Operational\r\n");
	if(satState == OperationalMode) return 0;

	logError(payloadTurnOn(), "EnterOperationalMode - payloadTurnOn");
	int countChange = 0;
	logError(FRAM_read((unsigned char*)&countChange, NUM_OF_CHANGES_IN_MODE_ADDR, NUM_OF_CHANGES_IN_MODE_SIZE), "EnterOperationalMode - FRAM_read");
	countChange += 1;
	logError(FRAM_writeAndVerify((unsigned char*)&countChange, NUM_OF_CHANGES_IN_MODE_ADDR, NUM_OF_CHANGES_IN_MODE_SIZE), "EnterOperationalMode - FRAM_writeAndVerify");
	txOff = FALSE;
	payloadOff = FALSE;
	satState = OperationalMode;

	return 0;
}

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Cruise mode
 * @return	0 on success
 * 			errors according to <hal/errors.h>
 */
int EnterCruiseMode()
{
	printf("entered Cruise\r\n");
	if(satState == OperationalMode)
	{
		logError(payloadTurnOff(), "EnterCruiseMode - payloadTurnOff");
		vTaskDelay(5);
	}
	txOff = FALSE;
	payloadOff = TRUE;
	satState = CruiseMode;
	return 0;
}

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Power Safe mode
 * @return	0 on success
 * 			errors according to <hal/errors.h>
 */
int EnterPowerSafeMode()
{
	txOff = TRUE;
	satState = PowerSafeMode;
	printf("entered power safe mode\r\n");
	return 0;
}

/*
 * Get if the Tx flag is on or off for us to put in the CheckAllowed in the TRXVU.c code.
 * */
Boolean GetTxFlag()
{
	return txOff;
}

/*
 * Get if the Payload flag is on or off for us to put in the CheckAllowed in the telemetry collector
 * */
Boolean GetPayloadFlag()
{
	return payloadOff;
}

/*!
 * returns the current system state according to the EpsState_t enumeration
 * @return system state according to EpsState_t
 */
EpsState_t GetSystemState()
{
	return satState;
}
