/*
 * utils.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include <stdio.h>
#include <stdbool.h>
#define RANGE(number, min, max) (number >= min && number <= max)
int logError(int error ,char* msg)
{
	if(error != 0)
	{
		printf("%s - ERROR %d\r\n", msg, error);
	}
	return error;
}
//Boolean Range(int number, int min, int max) return {number >= min && number <= max}
