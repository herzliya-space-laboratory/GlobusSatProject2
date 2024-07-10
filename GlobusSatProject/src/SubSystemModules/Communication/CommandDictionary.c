/*
 * CommandDictionary.c
 *
 *  Created on: 9 7 2024
 *      Author: Maayan
 */


#include "CommandDictionary.h"
#include <stdio.h>
#include "TRXVU.h"

int trxvu_command_router(sat_packet_t *cmd)
{
	if(cmd == NULL)
	{
		printf("cmd_is_null\r\n");
		return -1;
	}
	return TransmitDataAsSPL_Packet(cmd, (unsigned char *)"hello world", sizeof("hello world"));
}
