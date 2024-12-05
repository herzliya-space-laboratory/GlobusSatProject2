/*
 * CommandDictionary.c
 *
 *  Created on: 9 7 2024
 *      Author: Maayan
 */

#include "SubsystemCommands/TRXVU_Commands.h"
#include "SubsystemCommands/EPS_Commands.h"
#include "SubsystemCommands/Maintanence_Commands.h"
#include "CommandDictionary.h"
#include <stdio.h>

/*!
 * @brief routes the data into the appropriate trxvu command according to the command sub type
 * @param[in] name=cmd; type=sat_packet_t*: command pertaining to the TRXVU system, to be executed.
 * @note if subtype not exist an unknown subtype ack is send.
 * @return errors according to <hal/errors.h> and -1 on cmd NULL
 */
int trxvu_command_router(sat_packet_t *cmd)
{
	if(cmd == NULL)
	{
		printf("cmd_is_null\r\n");
		return -1;
	}
	// Go to each subtype known and according to trxvu_subtypes_t struct and check if it's equal to the subtype in the cmd. if equal go the function of the command
	switch(cmd->cmd_subtype)
	{
		case ON_IDLE:
			return CMD_SetOn_IdleState(cmd);
		case OFF_IDLE:
			return CMD_SetOff_IdleState(cmd);
		case SET_ON_TRANSPONDER:
			return CMD_SetOn_Transponder(cmd);
		case SET_OFF_TRANSPONDER:
			return CMD_SetOff_Transponder(cmd);
		case SET_RSSI_TRANSPONDER:
			return CMD_SetRSSI_Transponder(cmd);
		case SET_RSSI_TRANSPONDER_DEFAULT:
			return CMD_SetRSSI_Transponder_DEFAULT(cmd);
		case GET_RSSI_TRANSPONDER:
			return CMD_GetRSSI_Transponder(cmd);
		case MUTE_TRXVU:
			return CMD_MuteTRXVU(cmd);
		case UNMUTE_TRXVU:
			return CMD_UnMuteTRXVU(cmd);
		case SET_BEACON_INTERVAL:
			return CMD_SetBeacon_Interval(cmd);
		case SET_BEACON_INTERVAL_DEFAULT:
			return CMD_SetBeacon_Interval_DEFAULT(cmd);
		case GET_BEACON_INTERVAL:
			return CMD_GetBeacon_Interval(cmd);
		case GET_TX_UPTIME:
			return CMD_GetTxUptime(cmd);
		case GET_RX_UPTIME:
			return CMD_GetRxUptime(cmd);
		case ANT_GET_UPTIME:
			return CMD_AntGetUptime(cmd);
		case ANT_CANCEL_DEPLOY:
			return CMD_AntCancelDeployment(cmd);
		case PING:
			return CMD_Ping(cmd);
		default:
			return logError(SendAckPacket(ACK_UNKNOWN_SUBTYPE, cmd, NULL, 0), "trxvu_command_router - SendAckPacket invalid subtype"); // Send ack that says what written in unknownSubtype_msg
	}
}


/*!
 * @brief routes the data into the appropriate eps command according to the command sub type
 * @param[in] cmd command pertaining to the EPS system, to be executed.
 * @note the type and subtype of the command are already inside cmd
 * @see sat_packet_t structure
 * @return errors according to <hal/errors.h>
 */
int eps_command_router(sat_packet_t *cmd)
{
	if(cmd == NULL)
	{
		printf("cmd_is_null\r\n");
		return -1;
	}
	// Go to each subtype known and according to trxvu_subtypes_t struct and check if it's equal to the subtype in the cmd. if equal go the function of the command
	switch(cmd->cmd_subtype)
	{
		case UPDATE_ALPHA:
			return CMD_UpdateAlpha(cmd);
		case UPDATE_ALPHA_DEFAULT:
			return CMD_RestoreDefaultAlpha(cmd);
		case GET_ALPHA:
			return CMD_GetAlpha(cmd);
		case GET_THRESHOLD:
			return CMD_GetThresholdVoltages(cmd);
		case UPDATE_THRESHOLD:
			return CMD_UpdateThresholdVoltages(cmd);
		case UPDATE_THRESHOLD_DEFAULT:
			return CMD_RestoreDefaultThresholdVoltages(cmd);
		case GET_STATE:
			return CMD_GetState(cmd);
		case RESET_EPS_WDT:
			return CMD_EPS_ResetWDT(cmd);
		default:
			return logError(SendAckPacket(ACK_UNKNOWN_SUBTYPE, cmd, NULL, 0), "eps_command_router - SendAckPacket invalid subtype"); // Send ack that says what written in unknownSubtype_msg
	}
}


/*!
 * @brief routes the data into the appropriate obc command according to the command sub type
 * @param[in] cmd command pertaining to the MANAGMENT sub routine, to be executed.
 * @note the type and subtype of the command are already inside cmd
 * @see sat_packet_t structure
 * @return errors according to <hal/errors.h>
 */
int managment_command_router(sat_packet_t *cmd)
{
	if(cmd == NULL)
	{
		printf("cmd_is_null\r\n");
		return -1;
	}
	switch(cmd->cmd_subtype)
	{
		case GET_SAT_TIME:
			return CMD_GetSatTime(cmd);
		case RESET_COMPONENT:
			return CMD_ResetComponent(cmd);
		default:
			return logError(SendAckPacket(ACK_UNKNOWN_SUBTYPE, cmd, NULL, 0), "managment_command_router - SendAckPacket invalid subtype"); // Send ack that says what written in unknownSubtype_msg
	}
}
