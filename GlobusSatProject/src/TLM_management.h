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
 typedef struct imageInfo
 {
 unsigned short imageID;
 unsigned short numberChunks;
 char imageType;
 } imageInfo_t;

 typedef struct imageData
 {
 unsigned short chunkID;
 char data[IMG_CHUNK_SIZE];
 } imageData_t;

 */

static Boolean stopDump = FALSE;

/*int CMD_getInfoImage(sat_packet_t *cmd);

 int CMD_getDataImage(sat_packet_t *cmd);*/

/**
 * write telematry data to file
 */
int Write2File(void* data, tlm_type_t tlmType);

int DeleteTLMFiles(tlm_type_t tlmType, Time date, int numOfDays);

/*
 * delete a single file from the SD
 */
int DeleteTLMFile(tlm_type_t tlmType, Time date, int days2Add);

void CalculateFileName(Time date, char* file_name, char* endFileName,
		int days2Add);
/*
 * Read telematry file and send it to ground station over RF
 */
int ReadTLMFile(tlm_type_t tlmType, Time date, int days2Add, int cmd_id/*,
		int resolution*/);
/*
 *
 */
int ReadTLMFiles(tlm_type_t tlmType, Time startDate, int numOfDays, int cmd_id/*, int resolution*/);

int ReadTLMFileTimeRange(tlm_type_t tlmType, time_t from_time, time_t to_time,
		int cmd_id/*, int resolution*/);

/*
 * doing a formating to the sd we are on.
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
void DeInitializeFS(int sd_card);

void PrintTLM(void* element, tlm_type_t tlmType);

char* GetName();
void print_file(char* c_file_name);
int FS_test();
void test_i();
#endif /* TM_MANAGMENT_H_ */

