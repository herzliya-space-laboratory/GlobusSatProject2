/*
 * InitSystem.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include <hal/Storage/FRAM.h>

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"

#include "InitSystem.h"
#include "utils.h"


int StartFRAM(){
	return logError(FRAM_start(), "FRAM");
}
