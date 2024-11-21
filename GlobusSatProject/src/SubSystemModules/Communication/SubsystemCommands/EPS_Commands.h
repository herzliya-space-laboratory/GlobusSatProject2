
#ifndef EPS_COMMANDS_H_
#define EPS_COMMANDS_H_

#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/AckErrors.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"

int CMD_UpdateThresholdVoltages(sat_packet_t *cmd);

int CMD_GetThresholdVoltages(sat_packet_t *cmd);

int CMD_RestoreDefaultThresholdVoltages(sat_packet_t *cmd);

/*
 * Set new alpha value.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and the new alpha val)
* @return type=int; return type of error
* 										-1 on cmd NULL
* 										errors according to "AckErrors.h"
 * */
int CMD_UpdateAlpha(sat_packet_t *cmd);

/*
 * Set default alpha value.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error
* 										-1 on cmd NULL
* 										errors according to "AckErrors.h"
 * */
int CMD_RestoreDefaultAlpha(sat_packet_t *cmd);

/*
 * Get alpha value.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error
* 										-2 on FRAM_read error, and TransmitDataAsSPL_Packet errors
 * */
int CMD_GetAlpha(sat_packet_t *cmd);

//int CMD_GetMode(sat_packet_t *cmd);
//
//int CMD_SetMode(sat_packet_t *cmd);

int CMD_GetState(sat_packet_t *cmd);

int CMD_ChangHeaterTemp(sat_packet_t *cmd);

int CMD_EPS_ResetWDT(sat_packet_t *cmd);

int CMD_GetheaterValues(sat_packet_t *cmd);

//int CMD_SetheaterValues(sat_packet_t *cmd); //????????????

int CMD_GetstateChangesOverTime(sat_packet_t *cmd);

#endif /* EPS_COMMANDS_H_ */
