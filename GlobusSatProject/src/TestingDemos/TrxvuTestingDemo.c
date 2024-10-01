/*
 * TrxvuTestingDemo.c
 *
 *  Created on: 24 7 2024
 *      Author: maayan
 */

#include "TrxvuTestingDemo.h"

void AssembleCommandNullCmd_test()
{
	unsigned char data[] = "hello";
	int error = logError(AssembleCommand(data, sizeof(data), 0x00, 0x00, CUBE_SAT_ID, NULL), "AssembleCommand - test 1");
	if(error == command_success)
	{
		printf("something is wrong - test 1\n\r");
	}
}

void AssembleCommandNullDataAndLengthZero_test()
{
	sat_packet_t test;
	int error = logError(AssembleCommand(NULL, 0, 0x00, 0x00, CUBE_SAT_ID, &test), "AssembleCommand - test 2");
	if(error != command_success)
	{
		printf("something is wrong - test 2\n\r");
	}
}

void AssembleCommandNullDataAndLengthBiggerThenZero_test()
{
	sat_packet_t test;
	int error = logError(AssembleCommand(NULL, 3, 0x00, 0x00, CUBE_SAT_ID, &test), "AssembleCommand - test 3");
	if(error == command_success)
	{
		printf("something is wrong - test 3\n\r");
	}
}

void AssembleCommandHaveDataAndLengthBiggerThenZero_test()
{
	sat_packet_t test;
	unsigned char data[] = "hello";
	int error = logError(AssembleCommand(data, sizeof(data), 0x00, 0x00, CUBE_SAT_ID, &test), "AssembleCommand - test 4");
	if(error != command_success)
	{
		printf("something is wrong - test 4\n\r");
	}
}

void AssembleCommandHaveDataAndLengthBiggerThenMaxLength_test()
{
	sat_packet_t test;
	unsigned char data[] = "hello";
	int error = logError(AssembleCommand(data, MAX_COMMAND_DATA_LENGTH + 1, 0x00, 0x00, CUBE_SAT_ID, &test), "AssembleCommand - test 5");
	if(error == command_success)
	{
		printf("something is wrong - test 5\n\r");
	}
}

void ParseDataToCommandWorkNormal()
{
	unsigned char pac[] = {0x00, 0x00, 0x00, 0x13, 0x00, 0x01, 0x01, 0x00, 0x15};
	sat_packet_t cmd;
	int error = logError(ParseDataToCommand(pac, &cmd), "ParseDataToCommand - test 6");
	if(error != command_success)
	{
		printf("something is wrong - test 6\n\r");
	}
}

void ParseDataToCommandWrongSatId()
{
	unsigned char pac[] = {0x00, 0x00, 0x00, 0x15, 0x00, 0x01, 0x01, 0x00, 0x15};
	sat_packet_t cmd;
	int error = logError(ParseDataToCommand(pac, &cmd), "ParseDataToCommand - test 7");
	if(error == command_success)
	{
		printf("something is wrong - test 7\n\r");
	}
}

void ParseDataToCommandDontHaveLength()
{
	unsigned char pac[] = {0x00, 0x00, 0x00, 0x13, 0x00, 0x01};
	sat_packet_t cmd;
	int error = logError(ParseDataToCommand(pac, &cmd), "ParseDataToCommand - test 8");
	if(error == command_success)
	{
		printf("something is wrong - test 8\n\r");
	}
}

void ParseDataToCommandLengthEqualZero()
{
	unsigned char pac[] = {0x00, 0x00, 0x00, 0x13, 0x00, 0x01, 0x00, 0x00};
	sat_packet_t cmd;
	int error = logError(ParseDataToCommand(pac, &cmd), "ParseDataToCommand - test 9");
	if(error != command_success)
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

Boolean SelectAndExecuteTrxvu()
{
	int selection = 0;
	printf( "\n\r Select a test to perform: \n\r");
	printf("\t0) Go back to menu\n\r");
	printf("\t1) AssembleCommand null cmd test\n\r");
	printf("\t2) AssembleCommand null data and length zero test\n\r");
	printf("\t3) AssembleCommand null data and length bigger then zero test\n\r");
	printf("\t4) AssembleCommand have data and length bigger then zero test\n\r");
	printf("\t5) AssembleCommand have data and length bigger then max length test\n\r");
	printf("\t6) ParseDataToCommand work test\n\r");
	printf("\t7) ParseDataToCommend wrong sat id\n\r");
	printf("\t8) ParseDataToCommand don't have length\n\r");
	printf("\t9) ParseDataToCommand length equal to zero\n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 9) == 0); //you have to write a number between the two numbers include or else it ask you to enter a number between the two.

	switch(selection)
	{
		case 0:
			return FALSE;
		case 1:
			AssembleCommandNullCmd_test();
			break;
		case 2:
			AssembleCommandNullDataAndLengthZero_test();
			break;
		case 3:
			AssembleCommandNullDataAndLengthBiggerThenZero_test();
			break;
		case 4:
			AssembleCommandHaveDataAndLengthBiggerThenZero_test();
			break;
		case 5:
			AssembleCommandHaveDataAndLengthBiggerThenMaxLength_test();
			break;
		case 6:
			ParseDataToCommandWorkNormal();
			break;
		case 7:
			ParseDataToCommandWrongSatId();
			break;
		case 8:
			ParseDataToCommandDontHaveLength();
			break;
		case 9:
			ParseDataToCommandLengthEqualZero();
			break;
		default:
			break;
	}
	return TRUE;
}

void IsisTRXVUTestingLoop(void)
{
	Boolean offerMoreTests = TRUE;
	while(offerMoreTests)
	{
		offerMoreTests = SelectAndExecuteTrxvu();
	}
}
