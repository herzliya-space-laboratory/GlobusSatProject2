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

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"

#include "GlobalStandards.h"
#include "InitSystem.h"
#include "utils.h"

#define I2CBusSpeed_Hz 100000
#define I2CTransferTimeout 10

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

//TODO: need to be filled right.
int StartTIME(){
	const Time time = UNIX_DATE_JAN_D1_Y2000;
	return logError(Time_start(time, I2CTransferTimeout), "Time");
}

int InitSubsystems(){
	if(StartFRAM())
		return -1;
	else if(StartI2C())
		return -1;
	else if(StartSPI())
		return -1;
	else if(StartTime())
		return -1;
	return 0;
}
