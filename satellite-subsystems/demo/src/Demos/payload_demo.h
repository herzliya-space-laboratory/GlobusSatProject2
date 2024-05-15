/*
 * payload_demo.h
 *
 *  Created on: 15 במאי 2024
 *      Author: maayan
 */

#ifndef PAYLOAD_DEMO_H_
#define PAYLOAD_DEMO_H_

#include <hal/boolean.h>
#include <hcc/api_hcc_mem.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>

Boolean PayloadDemoMain(void);

void PayloadDemoLoop(void);

Boolean PayloadTest(void);

#endif /* PAYLOAD_DEMO_H_ */
