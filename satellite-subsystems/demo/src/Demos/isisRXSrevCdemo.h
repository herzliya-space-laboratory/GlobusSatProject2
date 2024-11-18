/*
 * isisRXSrevCdemo.h
 *
 *  Created on: 23 Nov 2023
 *      Author: obar
 */

#ifndef ISISRXSREVCDEMO_H_
#define ISISRXSREVCDEMO_H_

#include <hal/boolean.h>

/***
 * Starts demo.
 * Calls Init and Menu in sequence.
 * Returns FALSE on failure to initialize.
 */
Boolean IsisRXSrevCdemoMain(void);

/***
 * Initializes the TRXVU subsystem driver.
 * Returns FALSE on failure.
 *
 * note:
 * Depends on an initialized I2C driver.
 * Initialize the I2C interface once before using
 * any of the subsystem library drivers
 */
Boolean IsisRXSrevCdemoInit(void);

/***
 * Loop producing an interactive
 * text menu for invoking subsystem functions
 * note:
 * Depends on an initialized TRXVU subsystem driver.
 */
void IsisRXSrevCdemoLoop(void);

/***
 * (obsolete) Legacy function to start interactive session
 * Always returns TRUE
 *
 * Note:
 * Use IsisTRXVUdemoMain instead.
 */
Boolean RXSrevCtest(void);

#endif /* ISISRXSREVCDEMO_H_ */
