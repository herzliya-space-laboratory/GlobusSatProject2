
#ifndef FS_COMMANDS_H_
#define FS_COMMANDS_H_
#define WAIT_TIME_SEM_DUMP	3
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "TLM_management.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "Maintenance_Commands.h"


/**
 * delete files from SD by passing tlmtype, fromtime, totime
 */
int CMD_DeleteTLM(sat_packet_t *cmd);

int CMD_DeleteFileByTime(sat_packet_t *cmd);

int CMD_DeleteFilesOfType(sat_packet_t *cmd);

int CMD_DeleteAllFiles(sat_packet_t *cmd);

int CMD_GetNumOfFilesInTimeRange(sat_packet_t *cmd);

int CMD_GetNumOfFilesByType(sat_packet_t *cmd);

int CMD_GetLastFS_Error(sat_packet_t *cmd);

int CMD_FreeSpace(sat_packet_t *cmd);

int CMD_GetFileLengthByTime(sat_packet_t *cmd);

int CMD_GetTimeOfLastElementInFile(sat_packet_t *cmd);

int CMD_GetTimeOfFirstElement(sat_packet_t *cmd);

int CMD_StartDump(sat_packet_t *cmd);

int CMD_SendDumpAbortRequest(sat_packet_t *cmd);

int CMD_ForceDumpAbort(sat_packet_t *cmd);

void TackDump(void *dump);

#endif /* FS_COMMANDS_H_ */
