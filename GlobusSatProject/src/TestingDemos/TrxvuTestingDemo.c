/*
 * TrxvuTestingDemo.c
 *
 *  Created on: 24 7 2024
 *      Author: maayan
 */

#include "TrxvuTestingDemo.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"

void AssembleCommandNullCmd_test()
{
	unsigned char data[] = "hello";
	int error = logError(AssembleCommand(data, sizeof(data), 0x00, 0x00, CUBE_SAT_ID, NULL), "AssembleCommand - test 1");
	if(error == command_succsess)
	{
		printf("something is wrong - test 1\n\r");
	}
}

void AssembleCommandNullDataAndLengthZero_test()
{
	sat_packet_t test;
	int error = logError(AssembleCommand(NULL, 0, 0x00, 0x00, CUBE_SAT_ID, &test), "AssembleCommand - test 2");
	if(error != command_succsess)
	{
		printf("something is wrong - test 2\n\r");
	}
}

void AssembleCommandNullDataAndLengthBiggerThenZero_test()
{
	sat_packet_t test;
	int error = logError(AssembleCommand(NULL, 3, 0x00, 0x00, CUBE_SAT_ID, &test), "AssembleCommand - test 3");
	if(error == command_succsess)
	{
		printf("something is wrong - test 3\n\r");
	}
}

void AssembleCommandHaveDataAndLengthBiggerThenZero_test()
{
	sat_packet_t test;
	unsigned char data[] = "hello";
	int error = logError(AssembleCommand(data, sizeof(data), 0x00, 0x00, CUBE_SAT_ID, &test), "AssembleCommand - test 4");
	if(error != command_succsess)
	{
		printf("something is wrong - test 4\n\r");
	}
}

void AssembleCommandHaveDataAndLengthBiggerThenMaxLength_test()
{
	sat_packet_t test;
	unsigned char data[] = "hello";
	int error = logError(AssembleCommand(data, MAX_COMMAND_DATA_LENGTH + 1, 0x00, 0x00, CUBE_SAT_ID, &test), "AssembleCommand - test 5");
	if(error == command_succsess)
	{
		printf("something is wrong - test 5\n\r");
	}
}

void ParseDataToCommandWorkNormal()
{
	unsigned char pac[] = {0x00, 0x00, 0x00, 0x13, 0x00, 0x01, 0x01, 0x00, 0x15};
	sat_packet_t cmd;
	int error = logError(ParseDataToCommand(pac, &cmd), "ParseDataToCommand - test 6");
	if(error != command_succsess)
	{
		printf("something is wrong - test 6\n\r");
	}
}

void ParseDataToCommandWrongSatId()
{
	unsigned char pac[] = {0x00, 0x00, 0x00, 0x15, 0x00, 0x01, 0x01, 0x00, 0x15};
	sat_packet_t cmd;
	int error = logError(ParseDataToCommand(pac, &cmd), "ParseDataToCommand - test 7");
	if(error == command_succsess)
	{
		printf("something is wrong - test 7\n\r");
	}
}

void ParseDataToCommandDontHaveLength()
{
	unsigned char pac[] = {0x00, 0x00, 0x00, 0x13, 0x00, 0x01};
	sat_packet_t cmd;
	int error = logError(ParseDataToCommand(pac, &cmd), "ParseDataToCommand - test 8");
	if(error == command_succsess)
	{
		printf("something is wrong - test 8\n\r");
	}
}

void ParseDataToCommandLengthEqualZero()
{
	unsigned char pac[] = {0x00, 0x00, 0x00, 0x13, 0x00, 0x01, 0x00, 0x00};
	sat_packet_t cmd;
	int error = logError(ParseDataToCommand(pac, &cmd), "ParseDataToCommand - test 9");
	if(error != command_succsess)
	{
		printf("something is wrong - test 9\n\r");
	}
}

void MainTrxvuTestBench()
{
	AssembleCommandNullCmd_test();
	AssembleCommandNullDataAndLengthZero_test();
	AssembleCommandNullDataAndLengthBiggerThenZero_test();
	AssembleCommandHaveDataAndLengthBiggerThenZero_test();
	AssembleCommandHaveDataAndLengthBiggerThenMaxLength_test();
	ParseDataToCommandWorkNormal();
	ParseDataToCommandWrongSatId();
	ParseDataToCommandDontHaveLength();
	ParseDataToCommandLengthEqualZero();
}
