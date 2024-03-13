/*
 * isis_OBC_demo.h
 *
 *  Created on: 20 בדצמ 2023
 *      Author: maaya
 */

#ifndef ISIS_OBC_DEMO_H_
#define ISIS_OBC_DEMO_H_
//#define USE_EPS_ISIS TRUE

#include <hal/boolean.h>
#include <hcc/api_hcc_mem.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>

Boolean IsisOBCdemoMain(void);

void IsisOBCdemoLoop(void);

Boolean OBCtest(void);


#endif /* ISIS_OBC_DEMO_H_ */
