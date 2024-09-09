#ifndef TRXVU_H_
#define TRXVU_H_

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include "FRAM_FlightParameters.h"
#include "GlobalStandards.h"
#include "AckHandler.h"
#include "SatCommandHandler.h"
#include "utils.h"
#include <hal/Timing/Time.h>
#include "SubSystemModules/Maintenance/Maintenance.h"


#define MAX_MUTE_TIME 		(60*60*24*3) 	///< max mute duration will be 3 days = 60*60*24*3 [sec]
#define MAX_IDLE_TIME 		(2400) 	///< max IDLE duration will be 40 minutes = 120 *20 [sec]

#define SIZE_RXFRAME	200
#define SIZE_TXFRAME	235
#define BEACON_SPL_ID	0x02FFFFFF

#define MIN_TRXVU_BUFF 5
#define DEFAULT_RSSI_VALUE 10

typedef struct __attribute__ ((__packed__))
{
	sat_packet_t cmd;
	unsigned char dump_type;
	time_unix t_start;
	time_unix t_end; // if passed 0 we use the readTLMFiles function. Otherwise, we use the time range function
	int resulotion;
} dump_arguments_t;

typedef enum __attribute__ ((__packed__)) _ISIStrxvuTransponderMode
{
    trxvu_transponder_off = 0x01,
    trxvu_transponder_on = 0x02
} ISIStrxvutransponderMode;


int setMuteEndTime(time_unix endTime);

time_unix getMuteEndTime();

int setNewBeaconIntervalToPeriod();

time_unix getTransponderEndTime();

/**
 * Sets transponder RSSI value in FRAM
 */
int setTransponderRSSIinFRAM(short val);

/**
 * Gets transponder RSSI value from FRAM
 */
short getTransponderRSSIFromFRAM();

/*!
 * @breif Initializes data filed for transmission - semaphores, parameters from the FRAM
 * @return
 */
void InitTxModule();

/*!
 * @brief initializes the TRXVU subsystem
 * @return	0 on successful init
 * 			errors according to <hal/errors.h>
 */
int InitTrxvuAndAnts();

void checkTransponderFinish();

int CMD_SetBeaconInterval(sat_packet_t *cmd);

/*!
 * @brief The TRXVU logic according to the sub-system flowchart
 * @return	command_succsess on success
 * 			errors according to CMD_ERR enumeration
 * @see "SatCommandHandler.h"
 */
int TRX_Logic();

/**
 * sets the transponder's RSSI value
 */
int SetRSSITransponder(short rssiValue);

/*
 * Check if we pass the time of the transponder and if so get out of this state.
 * return type=int; -1 time end not smaller then time now
 * 					0 on success
 * 					error according to <hal/errors.h>
 * */
int turnOffTransponder();

/*
 * set transponder on
 * return type=int; according to I2C_write error list
 * */
int setTransponderOn();

Boolean CheckDumpAbort();

/*!
 * @brief checks if transmission is possible on grounds of low voltage and TX mute
 * @return	TRUE if transmission is allowed
 * 			FALSE if transmission is denied
 */
Boolean CheckTransmitionAllowed();

/*!
 * @brief 	Transmits a packet according to the SPL protocol
 * @param[in] packet packet to be transmitted
 * @param[out] avalFrames Number of the available slots in the transmission buffer of the VU_TC after the frame has been added. Set NULL to skip available slot count read-back.
 * @return    Error code according to <hal/errors.h>
 */
int TransmitSplPacket(sat_packet_t *packet, int *avalFrames);

/*!
 * @brief sends an abort message via a freeRTOS queue.
 */
void SendDumpAbortRequest();

/*!
 * @brief Closes a dump task if one is executing, using vTaskDelete.
 * @note Can be used to forcibly abort the task
 */
void AbortDump(sat_packet_t *cmd);

void FinishDump(sat_packet_t *cmd,unsigned char *buffer, ack_subtype_t acktype,
		unsigned char *err, unsigned int size) ;

/*!
 * @brief transmits beacon according to beacon logic
 * @ return 0 if everything is fine
 * 			-1 if we not suppose to send beacon now
 * 	 	else it return error according to the stuff that didn't work
 */
int BeaconLogic();

/*
 * @brief set the idle state of the trxvu
 * @param[in] state ON/OFF
 * @param[in] duration for how long will the satellite be in idle state, if state is OFF than this value is ignored
 * @return	0 in successful
 * 			-1 in failure
 */
int SetIdleState(ISIStrxvuIdleState state, time_unix duration);


/*!
 * @brief checks if the Trxvu mute time has terminated
 * @return	TRUE if the termination time has arrived
 * 			FALSE else
 */
/*Boolean CheckForMuteEnd();*/

/*!
 * @brief returns number of online frames are in the TRX frame buffer
 * @return	#number number of packets available
 * 			-1 in case of failure
 */
int GetNumberOfFramesInBuffer();

/*!
 * @brief returns an online(immediate) command to be executed if there is one in the command buffer
 * @param[out] cmd pointer to parsed command from online TRXVU frame buffer
 * @note cmd is set
 * @return	errors according to CMD_ERR
 */
CMD_ERR GetOnlineCommand(sat_packet_t *cmd);


/*!
 * @brief transmits data as SPL packet
 * @param[in] cmd the given command.
 * @param[in] data the out data.
 * @param[in] length number of bytes in 'data' fields.
 * @return errors according to <hal/errors.h>
 */
int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data, unsigned short length);

#endif
