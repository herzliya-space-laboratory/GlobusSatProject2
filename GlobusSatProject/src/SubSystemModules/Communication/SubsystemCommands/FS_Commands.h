
#ifndef FS_COMMANDS_H_
#define FS_COMMANDS_H_
#define WAIT_TIME_SEM_DUMP	3
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "TLM_management.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Housekeeping/TelemetryCollector.h"
#include "Maintenance_Commands.h"


/*
 * Delete tlm files from sd according to tlmType, start date, end date
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and tlmType, start date, end date)
 * @return type=int; -1 cmd null
 * 					 17 wrong data length
 * 					 60 didn't delete all files
 * 					 according to SendAckPacket errors.
 * 					 0 on success
 *
 * */
int CMD_DeleteTLM(sat_packet_t *cmd); //we have


int CMD_DeleteFilesOfType(sat_packet_t *cmd); //we have

/*
 * formating the SD card.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int; return type of error according to SendAckPacket and Hard_ComponenetReset errors
 * */
int CMD_DeleteAllFiles(sat_packet_t *cmd); //we have

/*
 * Get last FS error
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int; according to SendAckPacket
 * */
int CMD_GetLastFS_Error(sat_packet_t *cmd); //we have

/*
 * Get free space on the sd in bytes <- it's not precise but it's the best we got
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int;	27 on error get from struct
 * 						according to TransmitDataAsSPL_Packet
 * */
int CMD_FreeSpace(sat_packet_t *cmd); //we have

/*
 * start dump according to days and create the task of dump
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and tlmType, start date, end date)
 * @return type=int; -1 cmd null
 * 					 17 wrong data length
 * 					 30 Already have a dump active (can't do two at the same time)
 * 					 0 on success
 *
 * */
int CMD_StartDump(sat_packet_t *cmd); //we have

/*
 * abort dump. (stops the dump)
 * @return type=int; according to xQueueSend errors
 * */
int CMD_SendDumpAbortRequest();

/*
 * the function start dump call for the task. have the dump final logic.
 * @param[in] name=dump; type=void*; have the parameters we need for the dump.
 * */
void TackDump(void *dump); //we have

/*
 * Switch between the sd cards.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and sd to switch to)
 * @return type=int;	-1 cmd null
 * 						27 wrong data length
 * 						-4 invalid sd
 * 						-2 read from FRAM
 * 						-3 write to FRAM
 * 						according to Hard_ComponenetReset
 * */
int CMD_SwitchSD_card(sat_packet_t *cmd);

/*
 * Set new period of saving telemetry according to tlmPeriod and new period
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and tlmPeriod, new period)
 * @return type=int; -1 cmd null
 * 					 17 wrong data length
 * 					 error according to SaveAndCheck
 * 					 30 invalid tlmPeriod
 * */
int CMD_SetTLMPeriodTimes(sat_packet_t *cmd); //we have

/*
 * Set default period of saving telemetry
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int;	according to FRAM_writeAndVerify
 * */
int CMD_SetTLMPeriodTimes_default(sat_packet_t *cmd); //we have - need check

/*
 * Get telemetry period times from FRAM
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int;	according to FRAM_read and TransmitDataAsSPL_Packet
 * */
int CMD_GetTLMPeriodTimes(sat_packet_t *cmd); //we have


/*int CMD_GetFileLengthByTime(sat_packet_t *cmd);*/

/*int CMD_GetTimeOfLastElementInFile(sat_packet_t *cmd);*/

/*int CMD_GetTimeOfFirstElement(sat_packet_t *cmd);*/

/*int CMD_ForceDumpAbort(sat_packet_t *cmd); */

/*int CMD_GetNumOfFilesInTimeRange(sat_packet_t *cmd);*/

//int CMD_GetNumOfFilesByType(sat_packet_t *cmd);

//int CMD_DeleteFileByTime(sat_packet_t *cmd);


#endif /* FS_COMMANDS_H_ */
