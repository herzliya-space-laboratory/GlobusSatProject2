
#ifndef MAINTENANCETESTINGDEMO_H_
#define MAINTENANCETESTINGDEMO_H_

#include "GlobalStandards.h"
#include <hal/Utility/util.h>
#include "SubSystemModules/Maintenance/Maintenance.h"

Boolean CheckExecutionTimeFiveTimes();

void CheckIsFSCorrupted();

void MainMaintenanceTestBench();

Boolean SelectAndExecuteMaintenance();

void IsisMaintenanceTestingLoop();


#endif /* MAINTENANCETESTINGDEMO_H_ */
