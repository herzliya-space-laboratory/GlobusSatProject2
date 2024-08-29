
#ifndef TRXVU_COMMANDS_H_
#define TRXVU_COMMANDS_H_

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"

// wait time for taking semaphores
#define WAIT_TIME_SEM_TX	3
#define MAX_TRANS_TIME  	259200    // max time of transponder - 72 hours


#define ANTENNA_DEPLOYMENT_TIMEOUT 10 //<! in seconds
/**
 * Active the mute flag
All comunication operations should check about the flag state beafore any transmission (Flag On)
 * max mute duration will be 3 days
 */

int CMD_MuteTRXVU(sat_packet_t *cmd);

/**
 * set on idel state for time duration
 * max idel duration will be 40 minutes
 */
int CMD_SetOn_IdleState(sat_packet_t *cmd);

/*set off idel state for time duration
 * max idel duration will be 40 minutes
 */
int CMD_SetOff_IdleState(sat_packet_t *cmd);

/*
 * Turn on TRXVU Transponder
 */
int CMD_SetOn_Transponder(sat_packet_t *cmd);

/*
 * Turn off TRXVU Transponder
 */
int CMD_SetOff_Transponder(sat_packet_t *cmd);

/*
 * RSSI value
 */
int CMD_SetRSSI_Transponder(sat_packet_t *cmd);

/*
 * turn off the mute flag
 */
int CMD_UnMuteTRXVU(sat_packet_t *cmd);

/*
 * Get the beacon cycle time in FRAM
 */
int CMD_GetBeacon_Interval(sat_packet_t *cmd);

/*
 * Set a new beacon cycle time in FRAM
 * The default value should be 20 seconds
no smaller than 5 sec(if you do so it will round to 5)
And not bigger than 60(if you do so it will round to 60)
 */
int CMD_SetBeacon_Interval(sat_packet_t *cmd);

/*
 * Get for how long the TX was on
 */
int CMD_GetTxUptime(sat_packet_t *cmd);

/*
 * Get for how long the RX was on
 */
int CMD_GetRxUptime(sat_packet_t *cmd);

/*
 * ANT get arm status
 */
int CMD_AntGetArmStatus(sat_packet_t *cmd);

/*
 * For how long the ANT is on for side A or side B
 */
int CMD_AntGetUptime(sat_packet_t *cmd);

/*
 * cancel ANT deploy(delete before fllght)
 */
int CMD_AntCancelDeployment(sat_packet_t *cmd);

/*
 * deploy the ANT (delete before fllght and be careful with that)
 */
int CMD_AntDeploy(sat_packet_t *cmd);

/*
 * we send ping and we get pong
 */
int CMD_Ping(sat_packet_t *cmd);


//int CMD_StopReDeployment(sat_packet_t *cmd);

#endif
/*COMMANDS_H_ */
