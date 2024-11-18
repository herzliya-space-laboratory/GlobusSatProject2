
#ifndef EPS_COMMANDS_H_
#define EPS_COMMANDS_H_

#include "SubSystemModules/Communication/SatCommandHandler.h"
/*
 * #todo

 * CMD_GetThresholdVoltages
 * CMD_GetSmoothingFactor
 * CMD_GetCurrentMode
 * CMD_GetSolarPanelState
 * CMD_SolarPanelSleep //Isn't this super dangerous????
 * CMD_SolarPanelWake
 *
 */


/*
 * 00 00 13 01 03 00 reset alpha to default
 * 00 00 13 01 02 01 25 update alpha
 * 00 00 13 01 04 00 get alpha
 *
 * 00 00 13 01 07 00 get threshold
 * 00 00 13 01 05 08 1D 4C 1B 58 1C E8 1A F4 UPDATE THRESHOLD
 * 00 00 13 01 06 00 reset threshold
 *
 * 00 00 13 01 08 01  set state automatic
 * 00 00 13 01 08 02 00 set mode operational
 * 00 00 13 01 08 02 01 set mode cruise
 * 00 00 13 01 08 02 02 set mode powersafe
 * 00 00 13 01 09 00 get mode
 * 00 00 13 01 0A 00 GET_STATE_CHANGES_OVERTIME
 */



/*
 * @brief. updates the thresholds
 * @param 4 thresholds
 * @return error according to the <hal.error>
 */
int CMD_UpdateThresholdVoltages(sat_packet_t *cmd);


/*
 * @brief sends the threshols back to earth.
 * @param the cmd
 * @return error according to the <hal.error>
 */
int CMD_GetThresholdVoltages(sat_packet_t *cmd);

/*
 * @brief updates alpha
 * @param the cmd (data = the alpha value)
 * @return error according to the <hal.error>
 */
int CMD_UpdateSmoothingFactor(sat_packet_t *cmd);

/*
 * @brief restores alpha to the orignial value (0.25)
 * @return error according to the <hal.error>
 */
int CMD_RestoreDefaultAlpha();

/*
 * @brief records and sends the changes across a 24 hour period (since last refresh). sends the 2 changes. from high to mid (plus other way around) and mid to low (plus other way around)
*  @return error according to the <hal.error>
 */
int CMD_GET_STATE_CHANGES();

/*
 *@brief return the thresholds value to their original size
* @return error according to the <hal.error>
 */
int CMD_RestoreDefaultThresholdVoltages();

/*
 * @brief. sends the smoothing factor back to earth.
 * @return error according to the <hal.error>
 */
int CMD_GetSmoothingFactor();

/*
 * @brief. sets the mode. according to the dicteonary
 * @param read dict
 * @return error according to the <hal.error>
 */
int CMD_EPSSetMode(sat_packet_t *cmd);

/*
 * @brief sends back to earth all of the data about the mode
 * @return error according to the <hal.error>
 */
int CMD_GetCurrentMode(sat_packet_t *cmd);

int CMD_EPS_NOP(sat_packet_t *cmd);

int CMD_EPS_ResetWDT(sat_packet_t *cmd);

int CMD_EPS_SetChannels(sat_packet_t *cmd);

int CMD_SetChannels3V3_On(sat_packet_t *cmd);

int CMD_SetChannels3V3_Off(sat_packet_t *cmd);

int CMD_SetChannels5V_On(sat_packet_t *cmd);

int CMD_SetChannels5V_Off(sat_packet_t *cmd);

int CMD_GetEpsParemeter(sat_packet_t *cmd);

int CMD_SetEpsParemeter(sat_packet_t *cmd);

int CMD_ResetParameter(sat_packet_t *cmd);

int CMD_ResetConfig(sat_packet_t *cmd);

int CMD_LoadConfig(sat_packet_t *cmd);

int CMD_SaveConfig(sat_packet_t *cmd);

int CMD_SolarPanelWake(sat_packet_t *cmd);

int CMD_SolarPanelSleep(sat_packet_t *cmd);

int CMD_GetSolarPanelState(sat_packet_t *cmd);

#endif /* EPS_COMMANDS_H_ */
