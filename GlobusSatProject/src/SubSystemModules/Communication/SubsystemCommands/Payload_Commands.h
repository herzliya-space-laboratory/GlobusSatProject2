/*
 * Payload_Commands.h
 *
 *  Created on: 2 Jan 2025
 *      Author: maayan
 */

#ifndef PAYLOAD_COMMANDS_H_
#define PAYLOAD_COMMANDS_H_

#include "SubSystemModules/Payload/payload_drivers.h"
#include "SubSystemModules/Communication/AckHandler.h"

int CMD_TurnOnKillPayloadFlag(sat_packet_t *cmd);

int CMD_TurnOffKillPayloadFlag(sat_packet_t *cmd);



#endif /* PAYLOAD_COMMANDS_H_ */
