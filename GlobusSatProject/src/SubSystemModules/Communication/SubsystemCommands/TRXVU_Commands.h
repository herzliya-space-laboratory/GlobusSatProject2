
#ifndef TRXVU_COMMANDS_H_
#define TRXVU_COMMANDS_H_

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"

#include <hal/Drivers/I2C.h>
#include <string.h>

// wait time for taking semaphores
#define WAIT_TIME_SEM_DUMP	3
#define WAIT_TIME_SEM_TX	3
#define MAX_TRANS_TIME  	259200    // max time of transponder - 72 hours


#define ANTENNA_DEPLOYMENT_TIMEOUT 10 //<! in seconds


int CMD_MuteTRXVU(sat_packet_t *cmd);
/**
 * set trxvu idle state
 * cmd data should include 0x01 (On) or 0x00 (Off). if sent ON than also pass the duration (4 bytes int)
 */
int CMD_SetOn_dleState(sat_packet_t *cmd);

int CMD_SetOff_dleState(sat_packet_t *cmd);

int CMD_SetOn_Transponder(sat_packet_t *cmd);

int CMD_SetOff_Transponder(sat_packet_t *cmd);

int CMD_SetRSSI_Transponder(sat_packet_t *cmd);

int CMD_UnMuteTRXVU(sat_packet_t *cmd);

int CMD_GetBeacon_Interval(sat_packet_t *cmd);

int CMD_SetBeacon_Interval(sat_packet_t *cmd);


//int CMD_Trasmit_Beacon(sat_packet_t *cmd);


int CMD_GetTxUptime(sat_packet_t *cmd);

int CMD_GetRxUptime(sat_packet_t *cmd);

//int CMD_DeleteDelayedCmdByID(sat_packet_t *cmd);

//int CMD_DeleteAllDelayedBuffer(sat_packet_t *cmd);

int CMD_AntGetArmStatus(sat_packet_t *cmd);

int CMD_AntGetUptime(sat_packet_t *cmd);

int CMD_AntCancelDeployment(sat_packet_t *cmd);

int CMD_AntDeploy(sat_packet_t *cmd);

int CMD_Ping(sat_packet_t *cmd);


//int CMD_StopReDeployment(sat_packet_t *cmd);

#endif
/*COMMANDS_H_ */
