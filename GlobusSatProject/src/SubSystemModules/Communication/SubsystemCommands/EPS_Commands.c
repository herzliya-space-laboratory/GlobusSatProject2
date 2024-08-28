#include "EPS_Commands.h"
#include <hal/errors.h>

#include <Time.h>
#include "string.h"
#include "SubsystemModules/PowerManagment/EPS.h"
#include "SubsystemModules/PowerManagment/EPSOperationModes.h"
#include "GlobalStandards.h"
#include "utils.h"
#include "SubsystemModules/Communication/TRXVU.h"
Time TimeSinceLastChangeReset;

int CMD_UpdateThresholdVoltages(sat_packet_t *cmd) {
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	if (cmd->data == NULL) return E_INPUT_POINTER_NULL;
	if(GetcurrentMode() != AutmaticMode) {
		return E_Manual_Override;
	}
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	EpsThreshVolt_t threshHolds;
	memcpy(&threshHolds.fields.Vup_operational, cmd->data, sizeof(voltage_t));
	memcpy(&threshHolds.fields.Vup_cruise, cmd->data + sizeof(voltage_t), sizeof(voltage_t));
	memcpy(&threshHolds.fields.Vdown_operational, cmd->data + sizeof(voltage_t) * 2, sizeof(voltage_t));
	memcpy(&threshHolds.fields.Vdown_cruise, cmd->data + sizeof(voltage_t) * 3, sizeof(voltage_t));
	int error = SetEPSThreshold(&threshHolds);
	return error;

}
int CMD_UpdateSmoothingFactor(sat_packet_t *cmd) {
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	if (cmd->data == NULL) return E_INPUT_POINTER_NULL;
	if(GetcurrentMode() != AutmaticMode) {
		return E_Manual_Override;
	}
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	float newalpha;
	memcpy(&newalpha, cmd->data, sizeof(float));
	int error = UpdateAlpha(newalpha);
	return error;
}
int CMD_RestoreDefaultThresholdVoltages() {
	int error = RestoreDefaultThresholdVoltages();
	return error;
}
int CMD_RestoreDefaultAlpha() {
	int error = RestoreDefaultAlpha();
	return error;
}
int CMD_EPSSetMode(sat_packet_t *cmd) {
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	if (cmd->data == NULL) return E_INPUT_POINTER_NULL;
	char State;
	EpsState_t State_t;
	char mode;
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
		LogError(1, "No valid State");
		break;
	}
	int error = EnterManualMode(State_t);
	return error;
}
int CMD_GetSmoothingFactor(sat_packet_t *cmd) {
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	float alpha;
	unsigned short size = sizeof(alpha);
	GetAlpha(&alpha);
	TransmitDataAsSPL_Packet((sat_packet_t *)&cmd, (unsigned char *)&alpha, size);
	return 0;
}
int CMD_GetThresholdVoltages(sat_packet_t *cmd) {
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	EpsThreshVolt_t Threshold;
	unsigned short size = sizeof(Threshold);
	GetEPSThreshold(&Threshold);
	TransmitDataAsSPL_Packet((sat_packet_t *)&cmd, (unsigned char *)&Threshold, size);
	return 0;
}
int CMD_GetCurrentMode(sat_packet_t *cmd) {
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	EpsMode_t mode = GetSystemMode();
	EpsState_t state = GetSystemMode();
	if (mode == ManualMode) {
		unsigned char data;
		memcpy(&data, &mode, 1);
		memcpy(&data + 1, &state, 1);
		TransmitDataAsSPL_Packet((sat_packet_t *)&cmd, (unsigned char *)&data, 2);
	}
	else {
		TransmitDataAsSPL_Packet((sat_packet_t *)&cmd, (unsigned char *)&mode, 2);
	}
	return 0;
}
int CMD_GET_STATE_CHANGES(sat_packet_t *cmd) {
	int error = 0;
	if (cmd == NULL) return E_INPUT_POINTER_NULL;
	error = LogError(FRAM_read((unsigned char *)&TimeSinceLastChangeReset, EPS_LAST_STATE_CHANGE_ADDR, EPS_LAST_STATE_CHANGE_SIZE), "CMD_GET_STATE_CHANGES, FRAM_read");
	unsigned char data[sizeof(int)+sizeof(int)+sizeof(Time)] ;
	memcpy(data, (unsigned char *)&CHANGES_OPERATIONAL, sizeof(int));
	memcpy(data+sizeof(int), (unsigned char *)&CHANGES_POWERSAFE, sizeof(int));
	memcpy(data+sizeof(int)*2, (unsigned char *)&TimeSinceLastChangeReset, sizeof(Time));
	TransmitDataAsSPL_Packet((sat_packet_t *)&cmd, (unsigned char *)&data, sizeof(int)+sizeof(int)+sizeof(Time));

	return error;
}
