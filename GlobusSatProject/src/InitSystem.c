/*
 * InitSystem.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include <hal/Storage/FRAM.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"

#include "InitSystem.h"
#include "utils.h"

#define I2CBusSpeed_Hz 100000
#define I2CTransferTimeout 10

int StartFRAM(){
	return logError(FRAM_start(), "FRAM");
}

//TODO: void WriteDefaultValuesToFRAM()

int StartI2C(){
	return logError(I2C_start(I2CBusSpeed_Hz, I2CTransferTimeout), "I2C");
}

//TODO: need to be filled right.
int StartSPI(){
	return logError(SPI_start(1, 1), "SPI");
}
