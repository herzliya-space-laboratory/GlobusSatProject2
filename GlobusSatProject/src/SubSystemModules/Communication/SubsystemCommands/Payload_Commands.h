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

/*
 * turn on the flag of kill payload and turn payload off
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int;	2 on write_to_fram problem
 * 						error according to SendAckPacket
 * */
int CMD_TurnOnKillPayloadFlag(sat_packet_t *cmd);

/*
 * turn off the flag of kill payload and turn payload on
 * @param[in] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (the headers we add)
 * @return type=int;	2 on write_to_fram problem
 * 						error according to SendAckPacket
 * */
int CMD_TurnOffKillPayloadFlag(sat_packet_t *cmd);



#endif /* PAYLOAD_COMMANDS_H_ */
