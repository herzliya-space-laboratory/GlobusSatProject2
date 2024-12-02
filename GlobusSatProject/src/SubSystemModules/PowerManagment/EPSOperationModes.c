/*
 * EPSOperationModes.c
 *
 *  Created on: 21 Nov 2024
 *      Author: maayan
 */

#include "EPSOperationModes.h"

Boolean txOff = FALSE;
EpsState_t satState = OperationalMode;
/*!
 * @brief Executes the necessary procedure in order to initiate the system into Operational mode
 * @return	0 on success
 * 			errors according to <hal/errors.h>
 */
int EnterOperationalMode()
{
	txOff = FALSE;
	satState = OperationalMode;
	printf("entered Operational\r\n");
	return 0;
}

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Cruise mode
 * @return	0 on success
 * 			errors according to <hal/errors.h>
 */
int EnterCruiseMode()
{
	txOff = FALSE;
	satState = CruiseMode;
	printf("entered Cruise\r\n");
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

/*!
 * returns the current system state according to the EpsState_t enumeration
 * @return system state according to EpsState_t
 */
EpsState_t GetSystemState()
{
	return satState;
}
