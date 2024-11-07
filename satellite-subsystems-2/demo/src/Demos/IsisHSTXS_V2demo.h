/*
 * IsisHSTXS_V2demo.h
 *
 *  Created on: 18 oct. 2023
 *      Author: obar
 */

#ifndef ISISTXSDEMO_H_
#define ISISTXSDEMO_H_

#include <hal/boolean.h>

/***
 * Starts demo.
 * Calls Init and Menu in sequence.
 * Returns FALSE on failure to initialize.
 */
Boolean IsisHSTxSV2demoMain(void);

/***
 * Initializes the TxS subsystem driver.
 * Returns FALSE on failure.
 *
 * note:
 * Depends on an initialized I2C driver.
 * Initialize the I2C interface once before using
 * any of the subsystem library drivers
 */
Boolean IsisHSTxSV2demoInit(void);

/***
 * Loop producing an interactive
 * text menu for invoking subsystem functions
 * note:
 * Depends on an initialized TxS subsystem driver.
 */
void IsisHSTxSV2demoLoop(void);

#endif /* ISISTXSDEMO_H_ */
