
#ifndef EPS_COMMANDS_H_
#define EPS_COMMANDS_H_

#include "SubSystemModules/Communication/SatCommandHandler.h"

int CMD_UpdateThresholdVoltages(sat_packet_t *cmd);

int CMD_GetThresholdVoltages(sat_packet_t *cmd);

int CMD_UpdateAlpha(sat_packet_t *cmd);

int CMD_RestoreDefaultAlpha(sat_packet_t *cmd);

int CMD_RestoreDefaultThresholdVoltages(sat_packet_t *cmd);

int CMD_GetAlpha(sat_packet_t *cmd);

int CMD_GetMode(sat_packet_t *cmd);

int CMD_SetMode(sat_packet_t *cmd);

int CMD_GetState(sat_packet_t *cmd);

int CMD_ChangHeaterTemp(sat_packet_t *cmd);

//int CMD_EPS_NOP(sat_packet_t *cmd);

int CMD_EPS_ResetWDT(sat_packet_t *cmd);

//int CMD_GetEpsParemeter(sat_packet_t *cmd);

//int CMD_SetEpsParemeter(sat_packet_t *cmd);

//int CMD_ResetParameter(sat_packet_t *cmd);

//int CMD_ResetConfig(sat_packet_t *cmd);

//int CMD_LoadConfig(sat_packet_t *cmd);

//int CMD_SaveConfig(sat_packet_t *cmd);

//int CMD_SolarPanelWake(sat_packet_t *cmd);

//int CMD_SolarPanelSleep(sat_packet_t *cmd);

//int CMD_GetSolarPanelState(sat_packet_t *cmd);

int CMD_GetheaterValues(sat_packet_t *cmd);

int CMD_SetheaterValues(sat_packet_t *cmd);

int CMD_GetstateChangesOverTime(sat_packet_t *cmd);

#endif /* EPS_COMMANDS_H_ */
