/*
 * TLM_management.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include "TLM_management.h"

#include <hcc/api_fat.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <hcc/api_mdriver.h>
#include <hcc/api_hcc_mem.h>


#include "utils.h"

FileSystemResult InitializeFS(){
	int flag = 0;
	// in FS init we don't want to use a log file !
		// Initialize the memory for the FS
	int err = hcc_mem_init();
	if (err != E_NO_SS_ERR){
		logError(err, "FS - hcc_mem_init");
		flag = 1;
	}
	// Initialize the FS
	err = fs_init();
	if (err != E_NO_SS_ERR){
		logError(err, "FS - fs_init");
		flag = 1;
	}

	fs_start();

	// Tell the OS (freeRTOS) about our FS
	err = f_enterFS();
	if (err != E_NO_SS_ERR){
		logError(err, "FS - f_enterFS");
		flag = 1;
	}

	// Initialize the volume of SD card 0 (A)
	err = f_initvolume( 0, atmel_mcipdc_initfunc, 0 );
	if (err != E_NO_SS_ERR){
	// error init SD 0 so de-itnit and init SD 1
	//printf("f_initvolume primary error:%d\n",err);
		hcc_mem_init();
		fs_init();
		f_enterFS();
		err = f_initvolume( 0, atmel_mcipdc_initfunc, 1 );
		if (err != E_NO_SS_ERR){
				//printf("f_initvolume secondary error:%d\n",err);
		}
	}
	if(flag)
		return FS_FAIL;

	return FS_SUCCSESS;
}
