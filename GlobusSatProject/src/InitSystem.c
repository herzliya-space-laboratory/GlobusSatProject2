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
#include <hal/supervisor.h>

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
	return logError(FRAM_start(), "FRAM - FRAM_start");
}

//TODO: void WriteDefaultValuesToFRAM() {} // need to use FRAM_writeAndVerify or FRAM_write

int StartI2C(){
	return logError(I2C_start(I2CBusSpeed_Hz, I2CTransferTimeout), "I2C - I2C_start");
}

int StartSPI(){
	return logError(SPI_start(bus1_spi, slave1_spi), "SPI - SPI_start");
}

int StartTIME(){
	const Time time = UNIX_DATE_JAN_D1_Y2000;
	return logError(Time_start(&time, TIME_SYNCINTERVAL), "Time - Time_start");
}

int InitSupervisor()
{
	uint8_t po = SUPERVISOR_SPI_INDEX;
	int error = Supervisor_start(&po, 0);
	return logError(error, "Supervisor - Supervisor_start");
}

int InitSubsystems(){
	StartI2C();

	StartSPI();

	StartFRAM();

	StartTIME();

	InitializeFS();

	EPS_And_SP_Init();

	InitTrxvuAndAnts();

	printf("Did init\r\n");
	return 0;
}
