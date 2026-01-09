/*
 * TM_managment.h
 *
 *  Created on: Apr 8, 2019
 *      Author: Hoopoe3n
 */

#ifndef TM_MANAGMENT_H_
#define TM_MANAGMENT_H_

#include <hal/Boolean.h>
#include <GlobalStandards.h>
#include "SubSystemModules/Housekeeping/TelemetryFiles.h"
#include "SubSystemModules/Housekeeping/TelemetryCollector.h"
#include <hal/Timing/Time.h>
#include <time.h>
#include "SubSystemModules/Communication/SatCommandHandler.h"

#define MAX_FILE_NAME_SIZE 11
#define NUM_ELEMENTS_READ_AT_ONCE 1000

#define FS_FILE_ENDING	"TLM"
#define FS_FILE_ENDING_SIZE	3

#ifndef FSFRAM
#define FSFRAM 0x20000
#endif

typedef enum {
	FS_SUCCSESS,
	FS_DUPLICATED,
	FS_LOCKED,
	FS_TOO_LONG_NAME,
	FS_BUFFER_OVERFLOW,
	FS_NOT_EXIST,
	FS_ALLOCATION_ERROR,
	FS_FRAM_FAIL,
	FS_FAT_API_FAIL,
	FS_FAIL
} FileSystemResult;


/*
 * write to a file and create a file if not exist the file we need to write to
 * @param[in] name=data; type=void*; which data we want to save (can be any type).
 * @param[in] name=days2Add; type=int; get size of the struct we want to keep in bytes
 * @return type=int; 	-2 on get sat time problem (as seconds)
 * 						-3 on get sat time as Time struct
 * 						-1 on problem with opening the file
 * */
int Write2File(void* data, tlm_type_t tlmType);

/*
 * Delete files from specific start time and type
 * @param[in] name=tlmType; type=tlm_type_t; which type of telemtry we want to delete.
 * @param[in] name=date; type=Time; which time we want to start delete from
 * @param[in] name=numOfDays; type=int; how many days we want to delete from the date start
 * @return type=int; how many days were successfully deleted
 * */
int DeleteTLMFiles(tlm_type_t tlmType, Time date, int numOfDays);

/*
 * Delete a file from specific time and type
 * @param[in] name=tlmType; type=tlm_type_t; which subsystem we are want to delete a file from
 * @param[in] name=date; type=Time; which time we want to delete
 * @param[in] name=days2Add; type=int; for the counter of days to add a number of days for the last struct
 * @return type=int; return error according to "https://drive.google.com/file/d/0B7WEDvdtTWV6NXlmOWpPV20yelk/view?usp=sharing&resourcekey=0-ltj73kO4Iv1PaHq2sZopEw" page 142
 * */
int DeleteTLMFile(tlm_type_t tlmType, Time date, int days2Add);

/*
 * Get important info about the file (for save, read and delete).
 * @param[in] name=tlmType; type=tlm_type_t; which type of telemtry we want.
 * @param[out] name=endFileName; type=char*; get the ".something" to the specific type
 * @param[out] name=structSize; type=int*; get size of the struct we want to keep in bytes
 * */
void GetTlmTypeInfo(tlm_type_t tlmType, char* endFileName, int* structSize);

/*
 * Get the final file name.
 * @param[in] name=date; type=Time; which time we want to save/delete/read the data.
 * @param[out] name=file_name; type=char*; final file name will be written to here
 * @param[in] name=endFileName; type=char*; the ".something" to the specific type we want to find the name
 * @param[in] name=days2Add; type=int; get size of the struct we want to keep in bytes
 * */
void CalculateFileName(Time date, char* file_name, char* endFileName, int days2Add);

/*
 * Read telematry file and send it to ground station over RF
 *	@param[in] name=tlmType; type=tlm_type_t; type of data we save.
 *	@param[in] name=startDate; type=Time; time to start from the reading/the dump.
 *	@param[in] name=days2Add; type=int; day to add for startDate to read the next days from there.
 *	@param[in] name=resolution; type=int; resolution is how many times we need to not send until we send
 *	@return type=int; num of elements sent
 */
int ReadTLMFile(tlm_type_t tlmType, Time date, int days2Add, int cmd_id, int resolution);

/*
 *	call function read according to name of dates and start and resolution
 *	@param[in] name=tlmType; type=tlm_type_t; type of data we save.
 *	@param[in] name=startDate; type=Time; time to start from the reading.
 *	@param[in] name=numOfDays; type=int; how many days we wand to read => how many files form the same type from start time
 *	@param[in] name=resolution; type=int; resolution in seconds of the tlm of the same day we want to read
 *	@return type=int; num of elements read
 */
int ReadTLMFiles(tlm_type_t tlmType, Time startDate, int numOfDays, int cmd_id, int resolution);

/*
 * Get save tlm period time according to type to save.
 * @param[in] name=type; type=tlm_type_t; have which tlm we try to save/send
 * @return type=int; period time for type and 0 for log and unknown type
 * */
int GetPeriodTimeAccordingToTlmType(tlm_type_t type);


/*
 * Formating the SD we are on and then call to init
 * return type=int; return -2 if we are having an error in f_getdrive and -1 on formating error
 * */
int Delete_allTMFilesFromSD();

/*!
 * Initializes the file system.
 * @note call once for boot and after DeInitializeFS.
 * @return FS_FAIL if Initializing the FS failed,
 * FS_ALLOCATION_ERROR on malloc error,
 * FS_SUCCSESS on success.
 */
FileSystemResult InitializeFS();

/*!
 * DeInitializes the file system.
 */
void DeInitializeFS();

#endif /* TM_MANAGMENT_H_ */

