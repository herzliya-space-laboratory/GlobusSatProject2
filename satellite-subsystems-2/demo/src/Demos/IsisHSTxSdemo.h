/*
 * IsisHSTxSdemo.h
 *
 *  Created on: 4 mrt. 2015
 *      Author: malv
 */

#ifndef ISISHSTXSDEMO_H_
#define ISISHSTXSDEMO_H_

#include <hal/boolean.h>

/***
 * Starts demo.
 * Calls Init and Menu in sequence.
 * Returns FALSE on failure to initialize.
 */
Boolean IsisHSTxSdemoMain(void);

/***
 * Initializes the TxS subsystem driver.
 * Returns FALSE on failure.
 *
 * note:
 * Depends on an initialized I2C driver.
 * Initialize the I2C interface once before using
 * any of the subsystem library drivers
 */
Boolean IsisHSTxSdemoInit(void);

/***
 * Loop producing an interactive
 * text menu for invoking subsystem functions
 * note:
 * Depends on an initialized TxS subsystem driver.
 */
void IsisHSTxSdemoLoop(void);

/***
 * (obsolete) Legacy function to start interactive session
 * Always returns TRUE
 *
 * Note:
 * Use IsisTxSdemoMain instead.
 */
Boolean HSTxStest(void);

#endif /* ISISTXSDEMO_H_ */
