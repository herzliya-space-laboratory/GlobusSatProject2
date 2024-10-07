/*
 * EPS.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include "satellite-subsystems/imepsv2_piu.h"
#include "satellite-subsystems/IsisSolarPanelv2.h"
#include "utils.h"
#include "SysI2CAddr.h"
#include "EPS.h"

#include <hal/Drivers/SPI.h>
/*#define WE_HAVE_SP 1*/
#define WE_HAVE_EPS 1

int EPS_And_SP_Init(){
	int errorEPS = 0;
	int errorSP = 0;
#ifdef WE_HAVE_EPS
	IMEPSV2_PIU_t stract_1;
	stract_1.i2cAddr = EPS_I2C_ADDR;
	errorEPS = logError(IMEPSV2_PIU_Init(&stract_1, 1), "EPS - IMEPSV2_PIU_Init");

#endif
#ifdef WE_HAVE_SP
	errorSP = logError(IsisSolarPanelv2_initialize(slave0_spi), "Solar panels - IsisSolarPanelv2_initialize");
	if(errorSP == 0)
		errorSP = logError(IsisSolarPanelv2_sleep(), "Solar panels - sleep");
#endif
	if(errorSP || errorEPS)
			return -1;
		return 0;
}
