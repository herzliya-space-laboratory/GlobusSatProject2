
#ifndef TRXVUTESTINGDEMO_H_
#define TRXVUTESTINGDEMO_H_

#include <hal/Utility/util.h>

#include "GlobalStandards.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"

Boolean SelectAndExecuteTrxvu();

void MainTrxvuTestBench();

void AssembleCommandNullCmd_test();

void AssembleCommandNullDataAndLengthZero_test();

void AssembleCommandNullDataAndLengthBiggerThenZero_test();

void AssembleCommandHaveDataAndLengthBiggerThenZero_test();

void AssembleCommandHaveDataAndLengthBiggerThenMaxLength_test();

void ParseDataToCommandWorkNormal();

void ParseDataToCommandWrongSatId();

void ParseDataToCommandDontHaveLength();

void ParseDataToCommandLengthEqualZero();

#endif /* TRXVUTESTINGDEMO_H_ */
