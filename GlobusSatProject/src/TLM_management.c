/*
 * TLM_management.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include "TLM_management.h"

#include <hcc/api_fat.h>
#include "utils.h"

FileSystemResult InitializeFS(){
	int error = fs_init();
	logError(error, "FS");
	switch(error)
	{
		case 0:
			return FS_SUCCSESS;
		case 33:
			return FS_ALLOCATION_ERROR;
		default:
			return FS_FAIL;
	}
}
