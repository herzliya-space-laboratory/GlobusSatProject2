/*
 * utils.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include <stdio.h>

int logError(int error ,char* msg)
{
	if(error != 0)
	{
		printf("%s - ERROR %d\r\n", msg, error);
	}
	return error;
}
