/*
 * InitSystem.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include "InitSystem.h"
#include "utils.h"
#include <hal/Storage/FRAM.h>

int StartFRAM(){
	return logError(FRAM_start(), "FRAM");
}
