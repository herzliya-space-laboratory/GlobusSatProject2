#include "EPS_Commands.h"
#include <hal/errors.h>


#include "string.h"
#include "SubsystemModules/PowerManagment/EPS.h"
#include "SubsystemModules/PowerManagment/EPSOperationModes.h"
#include "GlobalStandards.h"
#include "utils.h"
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
int CMD_RestoreDefaultThresholdVoltages(sat_packet_t *cmd) {
	int error = RestoreDefaultThresholdVoltages();
	return error;
}
int CMD_RestoreDefaultAlpha(sat_packet_t *cmd) {
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
