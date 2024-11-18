/*
 * isismepsv2_ivid5_demo.h
 *
 *  Created on: oct 2023
 *      Author: obar
 */

#ifndef ISIS_EPS_DEMO_H_
#define ISIS_EPS_DEMO_H_

#include <hal/boolean.h>


/***
 * Starts demo.
 * Calls Init and Menu in sequence.
 * Returns FALSE on failure to initialize.
 */
Boolean isismepsv2_ivid5_piu___demo__main(void);

/***
 * Initializes the isis_epsS subsystem driver.
 * Returns FALSE on failure.
 *
 * note:
 * Depends on an initialized I2C driver.
 * Initialize the I2C interface once before using
 * any of the subsystem library drivers
 */
Boolean isismepsv2_ivid5_piu__demo__init(void);

/***
 * Loop producing an interactive
 * text menu for invoking subsystem functions
 * note:
 * Depends on an initialized isis_eps subsystem driver.
 */
void isismepsv2_ivid5_piu__demo__loop(void);

/***
 * (obsolete) Legacy function to start interactive session
 * Always returns TRUE
 *
 * Note:
 * Use isis_eps__demo__main instead.
 */
Boolean isismepsv2_ivid5_piu__test(void);

#endif /* ISIS_EPS_DEMO_H_ */
