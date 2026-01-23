#ifndef TRXVU_H_
#define TRXVU_H_

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <satellite-subsystems/isis_vu_e.h>
#include "satellite-subsystems/isis_ants.h"

#include "FRAM_FlightParameters.h"
#include "GlobalStandards.h"
#include "AckHandler.h"
#include "SatCommandHandler.h"
#include "utils.h"
#include <hal/Timing/Time.h>
#include "SubSystemModules/Maintenance/Maintenance.h"

/*#define WE_HAVE_ANTS 0*/

#define MAX_MUTE_TIME 		(60*60*24*7) 	///< max mute duration will be 7 days = 60*60*24*3 [sec]
#define MAX_IDLE_TIME 		(2400) 	///< max IDLE duration will be 40 minutes = 120 *20 [sec]
#define MAX_TRANS_TIME  	(60*60*24*14)    // max time of transponder - 14 days

#define SIZE_RXFRAME	200
#define SIZE_TXFRAME	235
#define BEACON_SPL_ID	0x02FFFFFF

#define MIN_TRXVU_BUFF 5
#define DEFAULT_RSSI_VALUE 10

xSemaphoreHandle semaphorDump;
xQueueHandle queueAbortDump;

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

/*
 * Gets Idle end time value from FRAM
 * @return type=time_unix; 0 on fail
 * 						   idle end time on success
 */
time_unix getIdleEndTime();

/*
 * Sets Mute time end value in FRAM and check
 * @return type=int;	-1 on error
 * 						-2 on wrong set
 * 						0 on success
 */
int setMuteEndTime(time_unix endTime);

/*
 * Gets Mute time end value from FRAM
 * @return type=time_unix; 0 on fail
 * 						   mute end time on success
 */
time_unix getMuteEndTime();

/*
 * Sets the new beacon interval in period.
 * return type=int; error according to <hal/errors.h>
 * */
int setNewBeaconIntervalToPeriod();

/*
 * Gets transponder end time value from FRAM
 * @return type=time_unix; 0 on fail
 * 						   Transponder end time on success
 */
time_unix getTransponderEndTime();

/**
 * Sets transponder RSSI value in FRAM and check
 * @return type=int;	-1 on error
 * 						-2 on wrong set
 * 						-3 on I2C_write error
 * 						0 on success
 *
 */
int setTransponderRSSIinFRAM(short val);

/**
 * Gets transponder RSSI value from FRAM
 * @return type=short; RSSI value from FRAM, -1 on error
 */
short getTransponderRSSIFromFRAM();

/*!
 * @breif Initializes data filed for transmission - semaphores, parameters from the FRAM
 * @return
 */
void InitTxModule();

/*
 * set in FRAM that we need to stop try to deploy
 * */
void SetNeedToStopAntDeploy();

/*
 * Initialize the TRXVU and ants.
 *
 * @return error according to <hal/errors.h>
 * */
int InitTrxvuAndAnts();

/*
 * set in the TRXVU his config param. (freq, bitrate and more)
 * */
void SetTRXVU_config_param();

/*
 * Have the TRXVU logic. (Beacon send, check if have packets, read packet etc.)
 *@return type=int; return error if have and command_succsess if not
 **/
int TRX_Logic();

/*
 * Check if we pass the time of the transponder and if so get out of this state.
 * return type=int; -1 time end not smaller then time now
 * 					0 on success
 * 					error according to <hal/errors.h>
 * */
int turnOffTransponder();

/*
 * Check if we pass the time of the idle and if so get out of this state.
 * return type=int; -1 on error in getIdleEndTime
 * 					0 on success
 * 					error according to SetIdleState errors
 * */
int turnOffIdle();

/*
 * set transponder on
 * return type=int; according to I2C_write error list
 * */
int setTransponderOn();

/*
 * set transponder off
 * return type=int; according to I2C_write error list
 * */
int setTransponderOff();

/*
 * Sets transponder end time value from FRAM
 * @return type=time_unix; 0 on success
 * 						   error according to <hal/errors.h>
 */
int setTransponderEndTime(time_unix transponderEndTime);

/*
 * check if we are needed to abort the dump
 * @return type=Boolean; FALSE - not needed. TRUE - abort
 * */
Boolean CheckDumpAbort();

/*
 * Check if we can transmit. (according to mute and EPS condition)
 * @return type=Boolean; TRUE if we can transmit
 * 						 FALSE if we can't
 */
Boolean CheckTransmitionAllowed();

/*
 * Send Ax25 packet
 * @param name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @param name=avalFrames; type=int*; availed frames
 * @return type=int; return -1 on cmd NULL
 * 							-2 if we can't transmmit
 * 							Error code according to <hal/errors.h>
 * */
int TransmitSplPacket(sat_packet_t *packet, int *avalFrames);

/*
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
 * 			-2 FRAM read problem
 * 			-3 FRAM write problem
 * 			-4 wrong time set
 * 			-5 time problem
 * 			-6 not on or off
 */
int SetIdleState(isis_vu_e__onoff_t state, time_unix duration);


/*
 * Gets number of packets in waiting.
 * @return type=int; -1 on error
 * 					 number of packets on success
 **/
int GetNumberOfFramesInBuffer();

/*
 * Get commend from the buffer and divide the info according to sat_packet_t headers
 * @param[out] name=cmd; type=sat_packet_t *; Put here the info from packet according to the sat_packet_t struct.
 * @return type=CMD_ERR; return command_success on success
 * 								and error according to CMD_ERR
 * */
CMD_ERR GetOnlineCommand(sat_packet_t *cmd);


/*
 * Send Ax25 packet with output data
 * @param name=cmd; type=sat_packet_t*; The packet the sat got and use to find all the required information (like the headers we add)
 * @param name=data; type=unsigned char*; The data we want to send
 * @param name=length; type=unsigned short; length of data
 * @return type=int; return -1 on cmd NULL
 * 							-2 on fail in Assemble commend
 *	 						-3 if we can't transmmit
 * 							Error code according to <hal/errors.h>
 * */
int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data, unsigned short length);

void setBeaconIntervalNOT_FROM_FRAM(unsigned int interval);

#endif
