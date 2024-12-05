/*
 * Maintanence_Commands.c
 *
 *  Created on: 3 Dec 2024
 *      Author: maayan
 */

#include "Maintanence_Commands.h"
#include <string.h>

/*
 * Get the time save on the sat at the sat.
 * @param[in and out] name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @return type=int; return type of error 0 on success.
 * */
int CMD_GetSatTime(sat_packet_t *cmd)
{
	time_unix timeNow;
	int error = logError(Time_getUnixEpoch((unsigned int*)&timeNow), "CMD_GetSatTime - Time_getUnixEpoch");
	if(error)
	{
		unsigned char ackError = ERROR_CANT_GET_TIME;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
		return error;
	}
	return logError(TransmitDataAsSPL_Packet(cmd, (unsigned char*)&timeNow, sizeof(time_unix)), "CMD_GetSatTime - TransmitDataAsSPL_Packet"); //send back the sat time
}

/*help functions of CMD_ResetComponent*/
int HardTX_ComponenetReset()
{
	logError(SendAckPacket(ACK_TX_HARD_RESET, NULL, NULL, 0), "SoftTX_ComponenetReset - SendAckPacket");
	return logError(IsisTrxvu_componentHardReset(0, trxvu_tc), "SoftTX_ComponenetReset - IsisTrxvu_componentSoftReset");
}

int HardRX_ComponenetReset()
{
	logError(SendAckPacket(ACK_RX_HARD_RESET, NULL, NULL, 0), "SoftRX_ComponenetReset - SendAckPacket");
	return logError(IsisTrxvu_componentHardReset(0, trxvu_rc), "SoftRX_ComponenetReset - IsisTrxvu_componentSoftReset");
}

int Soft_ComponenetReset()
{
	int one = 1;
	logError(SendAckPacket(ACK_SOFT_RESET, NULL, NULL, 0), "SoftRX_ComponenetReset - SendAckPacket");
	logError(FRAM_writeAndVerify((unsigned char*)&one, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "Hard_ComponenetReset - cmd reset flag");
	restart();
	return 0;
}

int Ants_ComponenetReset()
{
	logError(SendAckPacket(ACK_ANTS_RESET, NULL, NULL, 0), "SoftRX_ComponenetReset - SendAckPacket");
	int err1 = logError(IsisAntS_reset(0, isisants_sideA), "Ants_ComponenetReset - IsisAntS_reset - a");
	int err2 = logError(IsisAntS_reset(0, isisants_sideB), "Ants_ComponenetReset - IsisAntS_reset - b");
	return err1 + err2;
}

int Hard_ComponenetReset()
{
	imepsv2_piu__replyheader_t replyheader;
	int one = 1;
	logError(FRAM_writeAndVerify((unsigned char*)&one, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "Hard_ComponenetReset - cmd reset flag");
	logError(SendAckPacket(ACK_HARD_RESET, NULL, NULL, 0), "SoftRX_ComponenetReset - SendAckPacket");
	return logError(imepsv2_piu__reset(0, &replyheader), "Hard_ComponenetReset - imepsv2_piu__reset");
}

int FS_ComponenetReset()
{
	return 0; //TODO later
}


/*!
 * @brief 	starts a reset according to 'reset_type_t'
 * 			The function will raise the reset flag and send an ACK before starting the reset.
 * @return 	0 on success
 * 			Error code according to <hal/errors.h>
 */
int CMD_ResetComponent(sat_packet_t *cmd)
{
	unsigned char ackError = 0;
	if(cmd->length != 1)
	{
		ackError = ERROR_WRONG_LENGTH_DATA;
		SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
		return ackError;
	}
	short type;
	memcpy(&type, cmd->data, cmd->length);
	switch(type)
	{
		case reset_software:
		{
			return Soft_ComponenetReset();
		}
		case reset_hardware:
		{
			if(Hard_ComponenetReset())
			{
				ackError = ERROR_CANT_RESET;
				SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
				return ackError;
			}
			return 0;
		}
		case reset_tx_hard:
		{
			if(HardTX_ComponenetReset())
			{
				ackError = ERROR_CANT_RESET;
				SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
				return ackError;
			}
			return 0;
		}
		case reset_rx_hard:
		{
			if(HardRX_ComponenetReset())
			{
				ackError = ERROR_CANT_RESET;
				SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
				return ackError;
			}
			return 0;
		}
		case reset_ants:
		{
			if(Ants_ComponenetReset())
			{
				ackError = ERROR_CANT_RESET;
				SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
				return ackError;
			}
			return 0;
		}
		case reset_filesystem:
		{
			return 0;
		}
		default:
		{
			ackError = ERROR_CANT_DO;
			SendAckPacket(ACK_ERROR_MSG , cmd, (unsigned char*)&ackError, sizeof(ackError)); // Send ack error according to "AckErrors.h"
			return ackError;
		}
	}
}
