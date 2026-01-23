
#ifndef EPS_COMMANDS_H_
#define EPS_COMMANDS_H_

#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/AckErrors.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"

/*
 * Set update threshold voltages.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and the new thresholds)
* @return type=int; return type of error
* 										-1 on cmd NULL
* 										errors according to "AckErrors.h" and send ack
 * */
int CMD_UpdateThresholdVoltages(sat_packet_t *cmd);

/*
 * Get threshold voltages.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error
* 										1 on FRAM_read error, and TransmitDataAsSPL_Packet errors
 * */
int CMD_GetThresholdVoltages(sat_packet_t *cmd);

/*
 * restore default threshold voltages.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error
* 										-1 on cmd NULL
* 										errors according to "AckErrors.h" and send ack
 * */
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
* 										1 on FRAM_read error, and TransmitDataAsSPL_Packet errors
 * */
int CMD_GetAlpha(sat_packet_t *cmd);

/*
 * Get state of EPS.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error TransmitDataAsSPL_Packet
 * */
int CMD_GetState(sat_packet_t *cmd);

/*
 * Reset EPS WDT
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int; 26 can't reset, error according to SendAckPacket
 * */
int CMD_EPS_ResetWDT(sat_packet_t *cmd);

/**
 * get heater values from the config parameters.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int; return error according to isismepsv2_ivid7_piu__getconfigurationparameter or TransmitDataAsSPL_Packet
 * */
int CMD_GetHeaterVal(sat_packet_t *cmd);

/**  DO NOT USE!!!
 * set heater values from the config parameters.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and the threshold)
 * @return type=int; return error according to isismepsv2_ivid7_piu__getconfigurationparameter or TransmitDataAsSPL_Packet
 * */
int CMD_SetHeaterVal(sat_packet_t* cmd);

#endif /* EPS_COMMANDS_H_ */
