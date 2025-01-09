
#ifndef MAINTENANCE_COMMANDS_H_
#define MAINTENANCE_COMMANDS_H_

#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include <hal/Timing/Time.h>
#include <hal/supervisor.h>

#define MIN_GROUND_WDT (3*24*3600) //3 days

typedef enum __attribute__ ((__packed__)) reset_type_t
{
	reset_software, //from eps
	reset_hardware, //from eps
	reset_tx_hard,
	reset_rx_hard,
	reset_ants,
	reset_payload,
	reset_payload_hard,
	reset_fram,
	reset_filesystem

}reset_type_t;

/*int CMD_GenericI2C(sat_packet_t *cmd);*/

/*
 * Set the time on the OBC
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add and the new time)
 * @return type=int; 	-1 on cmd Null
 * 						-3 on wrong length
 * 						1 if cant set new time
 * 						type of error according to <hal/error.h>
 * 					 	0 on success.
 * */
int CMD_UpdateSatTime(sat_packet_t *cmd);

/*
 * Get the time save on the sat at the sat.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error 0 on success.
 * */
int CMD_GetSatTime(sat_packet_t *cmd);

/*
 * Get the time the OBC was active since the last reset
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error according to <hal/error.h> or TransmitDataAsSPL_Packet errors
 * 					 0 on success.
 * */
int CMD_GetSatUptime(sat_packet_t *cmd);


/*help functions of CMD_ResetComponent*/
int HardPayload_ComponenetReset();

int Payload_ComponenetReset();

int HardTX_ComponenetReset();

int HardRX_ComponenetReset();

int Soft_ComponenetReset();

int Hard_ComponenetReset();

int Ants_ComponenetReset();

int FRAM_ComponenetReset();

int FS_ComponenetReset();

/*!
 * @brief 	starts a reset according to 'reset_type_t'
 * 			The function will raise the reset flag and send an ACK before starting the reset.
 * @return 	0 on success
 * 			Error code according to <hal/errors.h>
 */
int CMD_ResetComponent(sat_packet_t *cmd);



int CMD_SetGsWdtKickTime(sat_packet_t* cmd);

int CMD_GetGsWdtKickTime(sat_packet_t* cmd);

int CMD_SetFirstActiveFlags();

#endif /* MAINTENANCE_COMMANDS_H_ */
