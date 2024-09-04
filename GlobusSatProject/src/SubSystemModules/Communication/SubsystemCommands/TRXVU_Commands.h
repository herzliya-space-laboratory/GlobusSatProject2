
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

/*
* The command sets the transponder off.
* @param[in and out] name=cmd; type=sat_packet_t; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1.
* */
int CMD_SetOff_Transponder(sat_packet_t *cmd);

int CMD_SetRSSI_Transponder(sat_packet_t *cmd);

/*
 * Change the end time of mute to unmute the transmiter.
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error.
 * */
int CMD_UnMuteTRXVU(sat_packet_t *cmd);

/*
* The command get the beacon interval from the FRAM and send it back.
* @param[in and out] name=cmd; type=sat_packet_t; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; return type of error and if the parameter (cmd) is NULL return -1.
* */
int CMD_GetBeacon_Interval(sat_packet_t *cmd);

/*
* The command change the beacon interval in the FRAM and check it change correctly. also check the new interval is between the max and min interval
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the new_interval and the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1.
* */
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
