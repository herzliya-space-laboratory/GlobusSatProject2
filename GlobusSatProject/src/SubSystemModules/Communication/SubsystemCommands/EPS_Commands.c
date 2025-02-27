/*
 * EPS_Commands.c
 *
 *  Created on: 21 Nov 2024
 *      Author: maayan
 */

#include "EPS_Commands.h"
#include <string.h>

/*
 * Set new alpha value.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and the new alpha val)
* @return type=int; return type of error
* 										-1 on cmd NULL
* 										errors according to "AckErrors.h"
 * */
int CMD_UpdateAlpha(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	unsigned char error_ack = 0;
	if(cmd->length != 4)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	}
	float alpha;
	memcpy(&alpha, cmd->data, cmd->length);
	int error = UpdateAlpha(alpha);
	switch(error)
	{
	 case -1:
	 {
		error_ack = ERROR_WRITE_TO_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	 }
	 case -2:
	 {
		error_ack = ERROR_INVALID_ALPHA;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	 }
	 case -4:
	 {
		error_ack = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	 }
	 case -5:
	 {
		error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	 }
	 default:
		 break;
	}
	return SendAckPacket(ACK_UPDATE_EPS_ALPHA , cmd, NULL, 0);
}

/*
 * Set default alpha value.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error
* 										-1 on cmd NULL
* 										errors according to "AckErrors.h"
 * */
int CMD_RestoreDefaultAlpha(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	cmd->length = 4;
	float alpha = DEFAULT_ALPHA_VALUE;
	memcpy(cmd->data, (unsigned char*)&alpha, cmd->length);
	return CMD_UpdateAlpha(cmd);
}

/*
 * Get alpha value.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error
* 										1 on FRAM_read error, and TransmitDataAsSPL_Packet errors
 * */
int CMD_GetAlpha(sat_packet_t *cmd)
{
	float alpha;
	int error = GetAlpha(&alpha);
	if(error == -2)
	{
		unsigned char error_ack = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	}
	return SendAckPacket(ACK_GET_ALPHA, cmd, (unsigned char*)&alpha, EPS_ALPHA_FILTER_VALUE_SIZE); // Send back the alpha value
}

/*
 * Get threshold voltages.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error
* 										1 on FRAM_read error, and TransmitDataAsSPL_Packet errors
 * */
int CMD_GetThresholdVoltages(sat_packet_t *cmd)
{
	EpsThreshVolt_t threshold;
	int error = GetThresholdVoltages(&threshold);
	if(error == -2)
	{
		unsigned char error_ack = ERROR_READ_FROM_FRAM;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	}
	return SendAckPacket(ACK_GET_EPS_THRESHOLDS, cmd, (unsigned char*)threshold.raw, EPS_THRESH_VOLTAGES_SIZE); // Send back the threshold voltages

}

/*
 * Set update threshold voltages.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and the new thresholds)
* @return type=int; return type of error
* 										-1 on cmd NULL
* 										errors according to "AckErrors.h" and send ack
 * */
int CMD_UpdateThresholdVoltages(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	unsigned char error_ack = 0;
	if(cmd->length != 8)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	}
	EpsThreshVolt_t threshold;
	memcpy(threshold.raw, &(cmd->data), cmd->length);
	int error = UpdateThresholdVoltages(threshold);
	switch(error)
	{
		case -1:
		{
			error_ack = ERROR_WRITE_TO_FRAM;
			SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error_ack;
		}
		case -2:
		{
			error_ack = ERROR_INVALID_TRESHOLD;
			SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error_ack;
		}
		case -3:
		{
			error_ack = ERROR_READ_FROM_FRAM;
			SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error_ack;
		}
		case -4:
		{
			error_ack = ERROR_WRITTEN_IN_FRAM_WRONG;
			SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
			return error_ack;
		}
		default:
			break;
	}
	return SendAckPacket(ACK_UPDATE_EPS_VOLTAGES , cmd, NULL, 0);
}

/*
 * restore default threshold voltages.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error
* 										-1 on cmd NULL
* 										errors according to "AckErrors.h" and send ack
 * */
int CMD_RestoreDefaultThresholdVoltages(sat_packet_t *cmd)
{
	if(cmd == NULL) return -1;
	cmd->length = 8;
	voltage_t defaultThershold[NUMBER_OF_THRESHOLD_VOLTAGES] = DEFAULT_EPS_THRESHOLD_VOLTAGES;
	EpsThreshVolt_t threshold;
	for(int i = 0; i < NUMBER_OF_THRESHOLD_VOLTAGES; i++)
		threshold.raw[i] = defaultThershold[i];

	memcpy(cmd->data, (unsigned char*)&threshold, cmd->length);
	return CMD_UpdateThresholdVoltages(cmd);
}

/*
 * Get state of EPS.
* @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
* @return type=int; return type of error TransmitDataAsSPL_Packet
 * */
int CMD_GetState(sat_packet_t *cmd)
{
	EpsState_t state = GetSystemState();
	return SendAckPacket(ACK_GET_STATE, cmd, (unsigned char*)&state, sizeof(state)); // Send back the state of the eps
}

/*
 * Reset EPS WDT
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int; 26 can't reset, error according to SendAckPacket
 * */
int CMD_EPS_ResetWDT(sat_packet_t *cmd)
{
	isismepsv2_ivid7_piu__replyheader_t response;


	int error = isismepsv2_ivid7_piu__resetwatchdog(0, &response);
	if(error)
	{
		unsigned char error_ack = ERROR_CANT_RESET_WDT;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	}
	return SendAckPacket(ACK_EPS_RESET_WDT , cmd, NULL, 0);
}

/**
 * get heater values from the config parameters.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int; return error according to isismepsv2_ivid7_piu__getconfigurationparameter or TransmitDataAsSPL_Packet
 * */
int CMD_GetHeaterVal(sat_packet_t* cmd)
{
	isismepsv2_ivid7_piu__getconfigurationparameter__from_t response;
	//threshold low (when to start)
	int err = logError(isismepsv2_ivid7_piu__getconfigurationparameter(0, 0x3000, &response), "CMD_GetHeaterVal - isismepsv2_ivid7_piu__getconfigurationparameter"); ///gets the data structure of the parameter to response according to the id of the parameter
	if(err)
	{
		unsigned char ackErr = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG , cmd, &ackErr, sizeof(ackErr)); // Send ack error according to "AckErrors.h"
		return err;
	}
	//threshold high (when to end)
	isismepsv2_ivid7_piu__getconfigurationparameter__from_t response2;
	err = logError(isismepsv2_ivid7_piu__getconfigurationparameter(0, 0x3003, &response2), "CMD_GetHeaterVal - isismepsv2_ivid7_piu__getconfigurationparameter"); ///gets the data structure of the parameter to response according to the id of the parameter
	if(err)
	{
		unsigned char ackErr = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG , cmd, &ackErr, sizeof(ackErr)); // Send ack error according to "AckErrors.h"
		return err;
	}

	short respones[2] = {0, 0};
	memcpy((unsigned char*)&respones[0], response.fields.par_val, 2);
	memcpy((unsigned char*)&respones[1], response2.fields.par_val, 2);
	float respones_float[2] = {(respones[0] * 0.01), (respones[1] * 0.01)};
	return SendAckPacket(ACK_GET_HEATER_THRESHOLD, cmd, (unsigned char*)respones_float, sizeof(respones_float)); // Send back the alpha value
}

/**
 * set heater values from the config parameters. DO NOT USE!!!
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add and the threshold)
 * @return type=int; return error according to isismepsv2_ivid7_piu__getconfigurationparameter or TransmitDataAsSPL_Packet
 * */
int CMD_SetHeaterVal(sat_packet_t* cmd)
{
	if(cmd == NULL) return -1;
	unsigned char error_ack = 0;
	if(cmd->length != 8)
	{
		error_ack = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, &error_ack, sizeof(error_ack)); // Send ack error according to "AckErrors.h"
		return error_ack;
	}
	float forUsToSwitchToShort;
	memcpy((unsigned char*)&forUsToSwitchToShort, cmd->data, sizeof(float));
	int16_t temp = (short)(forUsToSwitchToShort / 0.01);
	isismepsv2_ivid7_piu__setconfigurationparameter__from_t response;
	isismepsv2_ivid7_piu__setconfigurationparameter__to_t send;
	//threshold low
	send.fields.par_id = 0x3000;
	memcpy(send.fields.par_val, (unsigned char*)&temp, sizeof(temp));
	int err = logError(isismepsv2_ivid7_piu__setconfigurationparameter(0, &send, &response), "CMD_SetHeaterVal - isismepsv2_ivid7_piu__setconfigurationparameter"); ///gets the data structure of the parameter to response according to the id of the parameter
	if(err)
	{
		unsigned char ackErr = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG , cmd, &ackErr, sizeof(ackErr)); // Send ack error according to "AckErrors.h"
		return err;
	}
	//threshold high
	send.fields.par_id = 0x3003;
	memcpy((unsigned char*)&forUsToSwitchToShort, cmd->data + sizeof(float), sizeof(float));
	temp = (short)(forUsToSwitchToShort / 0.01);
	memcpy(send.fields.par_val, (unsigned char*)&temp, sizeof(temp));
	err = logError(isismepsv2_ivid7_piu__setconfigurationparameter(0, &send, &response), "CMD_SetHeaterVal - isismepsv2_ivid7_piu__setconfigurationparameter"); ///gets the data structure of the parameter to response according to the id of the parameter
	if(err)
	{
		unsigned char ackErr = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG , cmd, &ackErr, sizeof(ackErr)); // Send ack error according to "AckErrors.h"
		return err;
	}
	return SendAckPacket(ACK_CHANGE_HEATER_THRESHOLDS, cmd, NULL, 0);
}

