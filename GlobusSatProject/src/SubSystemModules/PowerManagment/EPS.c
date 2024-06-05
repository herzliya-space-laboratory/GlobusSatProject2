/*
 * EPS.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include "satellite-subsystems/imepsv2_piu.h"
#include "SysI2CAddr.h"
#include "EPS.h"
/*#define WE_HAVE_EPS 1*/
int EPS_Init(){
	IMEPSV2_PIU_t stract_1;
	stract_1.i2cAddr = EPS_I2C_ADDR;
	int error = IMEPSV2_PIU_Init(&stract_1, 1);
	if(error)
		return -1;
	return 0;
}
