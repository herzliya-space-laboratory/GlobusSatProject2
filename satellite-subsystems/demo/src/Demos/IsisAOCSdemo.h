#ifndef ISISAOCSDEMO_H_
#define ISISAOCSDEMO_H_

#include <hal/boolean.h>

/***
 * Starts demo.
 * Calls Init and Menu in sequence.
 * Returns FALSE on failure to initialize.
 */
Boolean isis_aocs_demo_main(void);

/***
 * Initializes the ISISPACE AOCS subsystem driver.
 * Returns FALSE on failure.
 *
 * note:
 * Depends on an initialized I2C driver.
 * Initialize the I2C interface once before using
 * any of the subsystem library drivers
 */
Boolean isis_aocs_demo_Init(void);

/***
 * Loop producing an interactive
 * text menu for invoking subsystem functions
 * note:
 * Depends on an initialized ISISPACE AOCS subsystem driver.
 */
void isis_aocs_demo_Loop(void);

/***
 * (obsolete) Legacy function to start interactive session
 * Always returns TRUE
 *
 * Note:
 * Use cspaceADCSdemoMain instead.
 */
Boolean isis_aocs_demo(void);

#endif /* ISISAOCSDEMO_H_ */
