#ifndef TAUSAT2_PDHUDEMO_H_
#define TAUSAT2_PDHUDEMO_H_

#include <hal/boolean.h>

/***
 * Starts demo.
 * Calls Init and Menu in sequence.
 * Returns FALSE on failure to initialize.
 */
Boolean TAUSAT2PdhuDemoMain(void);

/***
 * Initializes the TxS subsystem driver.
 * Returns FALSE on failure.
 *
 * note:
 * Depends on an initialized I2C driver.
 * Initialize the I2C interface once before using
 * any of the subsystem library drivers
 */
Boolean TAUSAT2PdhuDemoInit(void);

/***
 * Loop producing an interactive
 * text menu for invoking subsystem functions
 */
void TAUSAT2PdhuDemoLoop(void);

#endif /* TAUSAT2_PDHUDEMO_H_ */
