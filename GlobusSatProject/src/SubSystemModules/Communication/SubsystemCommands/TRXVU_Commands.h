
#ifndef TRXVU_COMMANDS_H_
#define TRXVU_COMMANDS_H_

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/AckErrors.h"
#include <hal/Drivers/I2C.h>
#include <string.h>

// wait time for taking semaphores
#define WAIT_TIME_SEM_DUMP	3
#define WAIT_TIME_SEM_TX	3


#define ANTENNA_DEPLOYMENT_TIMEOUT 10 //<! in seconds

/**
 * set trxvu idle on with time end
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the idle duration and the headers we add)
 * @return type=int; -6 on cmd null
 * 					 other according to SetIdleState error list.
 * 					 or SendAckPacket
 */
int CMD_SetOn_IdleState(sat_packet_t *cmd);

/* set trxvu idle off
* @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; according to SetIdleState error list.
* 					 or SendAckPacket if we don't have error in the SetIdleState
*/
int CMD_SetOff_IdleState(sat_packet_t *cmd);

/*
 * Set transmitter to transponder state for the time written in the data in cmd
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the mute duration and the headers we add)
 * @return type=int; -1 on cmd NULL
 * 					 -2 on written wrong number to FRAM
 * 					 -3 on incorrect length
 * 					 -4 can't read from FRAM
 * 					 -5 can't write to FRAM
 * 					 errors according to I2C_write
 * */
int CMD_SetOn_Transponder(sat_packet_t *cmd);

/*
* The command sets the transponder off.
* @param[in and out] name=cmd; type=sat_packet_t; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1.
* */
int CMD_SetOff_Transponder(sat_packet_t *cmd);

/*
* The command change the rssi value in the FRAM and check it change correctly. also check the new rssi value is between the 0 and 4095
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the new rssi val and the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1 and on error in setTransponderRSSIinFRAM, -2 if length isn't in size.
 * */
int CMD_SetRSSI_Transponder(sat_packet_t *cmd);

/*
* The command change the rssi value in the FRAM to default and check it change correctly.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information and set the data to default rssi val (the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1 and on error in setTransponderRSSIinFRAM, -2 if length isn't in size.
 * */
int CMD_SetRSSI_Transponder_DEFAULT(sat_packet_t *cmd);

/*
* The command get the rssi value from the FRAM and send it back.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; return type of error 0 on success and if the parameter (cmd) is NULL return -2, if cant read from fram return -1.
* */
int CMD_GetRSSI_Transponder(sat_packet_t *cmd);

/*
 * The command change the end time of mute to unmute the transmiter.
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error.
 * */
int CMD_UnMuteTRXVU(sat_packet_t *cmd);

/*
 * Set transmitter to mute for the time written in the data in cmd
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the mute duration and the headers we add)
 * @return type=int; -1 on cmd NULL
 * 					 -3 on incorrect length
 * 					 errors according to setMuteEndTime
 * */
int CMD_MuteTRXVU(sat_packet_t *cmd);

/*
* The command change the beacon interval in the FRAM to default interval
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error.
* */
int CMD_SetBeacon_Interval_DEFAULT(sat_packet_t *cmd);

/*
* The command change the beacon interval in the FRAM and check it change correctly. also check the new interval is between the max and min interval
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the new_interval and the headers we add)
* @return type=int; return type of error and if the parameter is NULL return -1.
* */
int CMD_SetBeacon_Interval(sat_packet_t *cmd);

/*
* The command get the beacon interval from the FRAM and send it back.
* @param[in and out] name=cmd; type=sat_packet_t; The packet the sat got and use to find all the required information (like the headers we add)
* @return type=int; return type of error and if the parameter (cmd) is NULL return -1.
* */
int CMD_GetBeacon_Interval(sat_packet_t *cmd);

/*
 * Get transmitter uptime and send to ground
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error according to <hal/errors.h>
 * */
int CMD_GetTxUptime(sat_packet_t *cmd);

/*
 * Get receiver uptime and send to ground
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error according to <hal/errors.h>
 * */
int CMD_GetRxUptime(sat_packet_t *cmd);

//int CMD_DeleteAllDelayedBuffer(sat_packet_t *cmd);

int CMD_AntGetArmStatus(sat_packet_t *cmd);

/*
 * Gets Ant uptime according to side.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the ant side and the headers we add)
 * @return type=int; return type of error according to this
 * 																-1 cmd NULL
 * 																-2 if length isn't in size.
 * 																-3 got wrong side (not exist one)
 * 																else from <hal/errors.h>
 * */
int CMD_AntGetUptime(sat_packet_t *cmd);

/*
 * cancel deployment according to side.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the ant side and the headers we add)
 * @return type=int; return type of error according to this
 * 																-1 cmd NULL
 * 																-2 if length isn't in size.
 * 																-3 got wrong side (not exist one)
 * 																else from <hal/errors.h>
 * */
int CMD_AntCancelDeployment(sat_packet_t *cmd);

/*
 * Send ack ping
* @param[in] name=cmd; type=sat_packet_t*; Not needed can be NULL
* @return type=int; according to errors <hal/errors.h>
 * */
int CMD_Ping(sat_packet_t *cmd);

/*
 * Helper function to get side of ants.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the ant side and the headers we add)
 * @param[out] name=side; type=char*; here we left the side for further use.
 * */
int GetAntSide(sat_packet_t *cmd, char *side);

#endif
/*COMMANDS_H_ */
