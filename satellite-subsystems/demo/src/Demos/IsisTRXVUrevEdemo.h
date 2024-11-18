/*
 * IsisTRXVUEdemo.h
 *
 *  Created on: 6 feb. 2015
 *      Author: malv
 */

#ifndef ISISTRXVUEDEMO_H_
#define ISISTRXVUEDEMO_H_

#include <hal/boolean.h>

/***
 * Starts demo.
 * Calls Init and Menu in sequence.
 * Returns FALSE on failure to initialize.
 */
Boolean IsisTRXVUrevEdemoMain(void);

/***
 * Initializes the TRXVU subsystem driver.
 * Returns FALSE on failure.
 *
 * note:
 * Depends on an initialized I2C driver.
 * Initialize the I2C interface once before using
 * any of the subsystem library drivers
 */
Boolean IsisTRXVUrevEdemoInit(void);

/***
 * Loop producing an interactive
 * text menu for invoking subsystem functions
 * note:
 * Depends on an initialized TRXVU subsystem driver.
 */
void IsisTRXVUrevEdemoLoop(void);

/***
 * (obsolete) Legacy function to start interactive session
 * Always returns TRUE
 *
 * Note:
 * Use IsisTRXVUdemoMain instead.
 */
Boolean TRXVUrevEtest(void);

#endif /* ISISTRXVUEDEMO_H_ */
