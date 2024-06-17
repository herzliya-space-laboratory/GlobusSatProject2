/*
 * InitSystem.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include <hal/Storage/FRAM.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>
#include <hal/Utility/util.h>

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "TLM_management.h"

#include "GlobalStandards.h"
#include "InitSystem.h"
#include "utils.h"

#define I2CBusSpeed_Hz 100000
#define I2CTransferTimeout 10
#define TIME_SYNCINTERVAL  60

int StartFRAM(){
	return logError(FRAM_start(), "FRAM");
}

//TODO: void WriteDefaultValuesToFRAM() {} // need to use FRAM_writeAndVerify or FRAM_write

int StartI2C(){
	return logError(I2C_start(I2CBusSpeed_Hz, I2CTransferTimeout), "I2C");
}

int StartSPI(){
	return logError(SPI_start(bus1_spi, slave1_spi), "SPI");
}

int StartTIME(){
	const Time time = UNIX_DATE_JAN_D1_Y2000;
	return logError(Time_start(&time, TIME_SYNCINTERVAL), "Time");
}

int InitSubsystems(){
	StartI2C();

	StartSPI();

	StartFRAM();

	StartTIME();

	InitTrxvuAndAnts();

	InitializeFS();

#ifdef WE_HAVE_EPS
	EPS_And_SP_Init();
#endif
	printf("Did init\r\n");
	return 0;
}
