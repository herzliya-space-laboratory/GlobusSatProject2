#include "EPS_Commands.h"
#include <hal/errors.h>

#include <Time.h>
#include "string.h"
#include "SubsystemModules/PowerManagment/EPS.h"
#include "SubsystemModules/PowerManagment/EPSOperationModes.h"
#include "GlobalStandards.h"
#include "utils.h"
#include "SubsystemModules/Communication/AckErrors.h"
#include "SubsystemModules/Communication/TRXVU.h"
Time TimeSinceLastChangeReset;

int CMD_UpdateThresholdVoltages(sat_packet_t *cmd) {
	if (cmd == NULL){
	return E_INPUT_POINTER_NULL;

	}
	if (cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	if(GetcurrentMode() != AutmaticMode) {
		int errmsg = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char *)&errmsg, sizeof(errmsg));
		return E_Manual_Override;
	}

	EpsThreshVolt_t threshHolds;
	unsigned int offset = 0; //offset in reading the offset value.
	memcpy(&threshHolds.fields.Vup_operational, cmd->data + offset, sizeof(voltage_t));
	offset += sizeof(voltage_t);

	memcpy(&threshHolds.fields.Vup_cruise, cmd->data + offset, sizeof(voltage_t));
	offset += sizeof(voltage_t);

	memcpy(&threshHolds.fields.Vdown_operational, cmd->data + offset, sizeof(voltage_t));
	offset += sizeof(voltage_t);

	memcpy(&threshHolds.fields.Vdown_cruise, cmd->data + offset, sizeof(voltage_t));

	SendAckPacket(ACK_UPDATE_EPS_ALPHA, cmd, NULL, 0);
	int error = SetEPSThreshold(&threshHolds);
	return error;

}

int CMD_UpdateSmoothingFactor(sat_packet_t *cmd) {
//Cmd check
	if (cmd == NULL){
		return E_INPUT_POINTER_NULL;
	}

	if (cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	if(GetcurrentMode() != AutmaticMode) {
		int errmsg = ERROR_CANT_DO;
		SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char *)&errmsg, sizeof(errmsg));
		return E_Manual_Override;
	}
//copy alpha
	int newalpha = 0;
	memcpy(&newalpha, cmd->data, cmd->length);
	//#todo del later

	float convalpha = (float)(newalpha / 100);
	int error = UpdateAlpha(convalpha);
//update alpha
	if (error == E_PARAM_OUTOFBOUNDS) {
		 int errmsg = ERROR_OUT_OF_BOUND;
		 SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char *)&errmsg , sizeof(errmsg));
	}
	if(error != 0) {
		 int errmsg = ERROR_UNKOWN;
		SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char *)&errmsg, sizeof(errmsg));
	}
	SendAckPacket(ACK_UPDATE_THRESHOLD, cmd, NULL, 0);
	return error;

}

int CMD_RestoreDefaultThresholdVoltages() {
	return RestoreDefaultThresholdVoltages();
}

int CMD_RestoreDefaultAlpha() {
	return RestoreDefaultAlpha();
}

int CMD_EPSSetMode(sat_packet_t *cmd) {
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	if (cmd->data == NULL) return E_INPUT_POINTER_NULL;
	char State = 0;
	EpsState_t State_t = OperationalMode;
	char mode = 0;
	memcpy(&mode, cmd->data, sizeof(char));
	if(mode == 0) {
		EnterAutomaticMode();
		return 0;
	}
	memcpy(&State, cmd->data+sizeof(char), sizeof(char));
	switch(State) {
	case(0): {
		State_t = OperationalMode;
		break;
		}
	case(1): {
		State_t = CruiseMode;
		break;
	}
	case(2): {
		State_t = PowerSafeMode;
		break;
	}
	default:
		logError(1, "No valid State");
		int errmsg = ERROR_OUT_OF_BOUND;
		SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char *)&errmsg, sizeof(errmsg));
		break;
	}
	SendAckPacket(ACK_UPDATE_EPS_MODE, cmd, NULL, 0);
	int error = EnterManualMode(State_t);
	return error;
}

int CMD_GetSmoothingFactor(sat_packet_t *cmd) {
	if (cmd == NULL){
		return E_INPUT_POINTER_NULL;
	}
	float alpha = 0;
	GetAlpha(&alpha);
	int transalpha = (int)(alpha * 100);

	TransmitDataAsSPL_Packet(cmd, (unsigned char *)&transalpha, sizeof(int));
	return 0;
}

int CMD_GetThresholdVoltages(sat_packet_t *cmd) {
	if (cmd == NULL) {
		return E_INPUT_POINTER_NULL;
	}
	EpsThreshVolt_t Threshold;
	unsigned short size = sizeof(Threshold);
	GetEPSThreshold(&Threshold);
	TransmitDataAsSPL_Packet(cmd, (unsigned char *)&Threshold, size);
	return 0;
}

int CMD_GetCurrentMode(sat_packet_t *cmd) {
	if (cmd == NULL) {
		return E_INPUT_POINTER_NULL;

	}
	if (cmd->data == NULL) {
		return E_INPUT_POINTER_NULL;
	}
	EpsMode_t mode = GetcurrentMode();
	EpsState_t state = GetSystemState();
	if (mode == ManualMode) {
		unsigned char data = 0;
		memcpy(&data, &mode, 1);
		memcpy(&data + 1, &state, 1);
		TransmitDataAsSPL_Packet(cmd, (unsigned char *)&data, 2);
	}	else {
		TransmitDataAsSPL_Packet(cmd, (unsigned char *)&mode, 2);
	}
	return 0;
}

int CMD_GET_STATE_CHANGES(sat_packet_t *cmd) {
	int error = 0;
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	error = logError(FRAM_read((unsigned char *)&TimeSinceLastChangeReset, EPS_LAST_STATE_CHANGE_ADDR, EPS_LAST_STATE_CHANGE_SIZE), "CMD_GET_STATE_CHANGES, FRAM_read");
	unsigned char data[sizeof(int)+sizeof(int)+sizeof(Time)] ;
	memcpy(data, (unsigned char *)&CHANGES_OPERATIONAL, sizeof(int));
	memcpy(data+sizeof(int), (unsigned char *)&CHANGES_POWERSAFE, sizeof(int));
	memcpy(data+sizeof(int)*2, (unsigned char *)&TimeSinceLastChangeReset, sizeof(Time));
	TransmitDataAsSPL_Packet(cmd, (unsigned char *)&data, sizeof(int)+sizeof(int)+sizeof(Time));
	if(error!=0) {
		int errmsg = ERROR_READ_FROM_FRAM;
		 SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char *)&errmsg, sizeof(errmsg));
	}
	return error;
}
