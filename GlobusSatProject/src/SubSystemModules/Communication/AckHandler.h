
#ifndef ACKHANDLER_H_
#define ACKHANDLER_H_
#include "SubSystemModules/Communication/AckErrors.h"
#include <hal/errors.h>
#include "SatCommandHandler.h"
#include "SPL.h"
#include "utils.h"
#include <freertos/FreeRTOS.h>
#include "AckErrors.h"


#define SendAnonymosAck(acksubtype) SendAckPacket(acksubtype,NULL,NULL,0) //<! sends an ACK without command ID nor data


/*!
 * @brief transmits an ack packet as an SPL packet(using the SPL protocol)
 * @param[in] name=acksubtype; type=ack_subtype_t; type of ack to be sent according to ack_subtype_t enumeration
 * @param[in] name=cmd; type=sat_packet_t*; the command for which the ACK is a response to. CAN BE NULL!
 * @param[in] name=data; type=unsigned char*; the data we want to send with the ack. CAN BE NULL!
 * @param[in] name=length; type=unsigned short; the length of the data we want to send in bytes
 * @return errors according t <hal/errors.h>
 */
int SendAckPacket(ack_subtype_t acksubtype, sat_packet_t *cmd, unsigned char *data, unsigned short length);


#endif /* ACKHANDLER_H_ */
