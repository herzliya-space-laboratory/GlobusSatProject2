/*
 *
 * @file	FRAM_FlightParameters.h
 * @brief	ordering all flight parameters(saved on the FRAM)  addresses and sizes
 * @note 	sizes are measured in chars = 1 byte. e.g size = 4, meaning 4 bytes(=int)
 */

#ifndef FRAM_FLIGHTPARAMETERS_H_
#define FRAM_FLIGHTPARAMETERS_H_

#include <hal/Storage/FRAM.h>


// <Satellite Management>
#define DEPLOYMENT_TIME_ADDR			0x05		//<! time at which the satellites starts deployment
#define DEPLOYMENT_TIME_SIZE			4			//<! size of parameter in bytes

#define SECONDS_SINCE_DEPLOY_ADDR		0x09		//<! counts how many seconds has past since wakup for use in deployment.
#define SECONDS_SINCE_DEPLOY_SIZE		4			//<! size of the parameter in bytes

#define SD_CARD_USED_ADDR				0x15		//<! save which SD we are using (0/1)
#define SD_CARD_USED_SIZE				1			//<! size of the parameter in bytes

#define DEFAULT_NO_COMM_WDT_KICK_TIME  (7*24*60*60)	//<! number of seconds in 7 days
#define NO_COMM_WDT_KICK_TIME_ADDR  	0x24		///< number of seconds of no communications before GS WDT kick
#define NO_COMM_WDT_KICK_TIME_SIZE		4			//<! size of the parameter in bytes

#define STOP_REDEPOLOY_FLAG_ADDR		0x30		//<! stop calling ant deploy every 30 min flag
#define STOP_REDEPOLOY_FLAG_SIZE		4			//<! length in bytes of

#define MUTE_END_TIME_ADDR				0x36		//<! mute end time
#define MUTE_END_TIME_SIZE				4			//<! length in bytes of

#define FIRST_ACTIVATION_FLAG_ADDR		0x42		//<! is this the first activation after launch flag
#define FIRST_ACTIVATION_FLAG_SIZE		4			//<! length in bytes of FIRST_ACTIVATION_FLAG

#define TRANSPONDER_END_TIME_ADDR		0x48		//<! mute end time
#define TRANSPONDER_END_TIME_SIZE		4			//<! length in bytes of

#define TRANSPONDER_RSSI_ADDR			0x52		//<! mute end time
#define TRANSPONDER_RSSI_SIZE			2			//<! length in bytes of

#define MOST_UPDATED_SAT_TIME_ADDR		0x54		//<! this parameters saves the sat time to be read after resets
#define MOST_UPDATED_SAT_TIME_SIZE		4			//<! size of the parameter in bytes

#define NUMBER_OF_RESETS_ADDR			0x60		//<! counts how many restarts did the satellite endure
#define NUMBER_OF_RESETS_SIZE			4			//<! size of the parameter in bytes

#define IDLE_END_TIME_ADDR				0x70		//<! mute end time
#define IDLE_END_TIME_SIZE				4			//<! length in bytes of

#define RESET_CMD_FLAG_ADDR				0x105		//<! the flag is raised whenever a restart is commissioned
#define RESET_CMD_FLAG_SIZE				4			//<! size of the parameter in bytes

#define NUM_OF_CHANGES_IN_MODE_ADDR		0x110		//<! changes between cruise to operational
#define NUM_OF_CHANGES_IN_MODE_SIZE		4			//<! size of the parameter in bytes

#define LAST_TRY_TO_DEPLOY_TIME_ADDR	0x115		//<! save time_unix of the last time we tried to deploy ants
#define LAST_TRY_TO_DEPLOY_TIME_SIZE	4			//<! size of the parameter in bytes

#define TLM_SAVE_PERIOD_START_ADDR		0x130		//<! start of the save periods in the FRAM

#define DEFAULT_EPS_SAVE_TLM_TIME		10			//<! save EPS TLM every 5 seconds
#define EPS_SAVE_TLM_PERIOD_ADDR		0x130		//<! address where the save tlm period will be

#define DEFAULT_TRXVU_SAVE_TLM_TIME		10			//<! save TRXVU TLM every 5 seconds
#define TRXVU_SAVE_TLM_PERIOD_ADDR		0x134		//<! address where the save tlm period will be

#define DEFAULT_ANT_SAVE_TLM_TIME		10			//<! save antenna TLM every 5 seconds
#define ANT_SAVE_TLM_PERIOD_ADDR		0x138		//<! address where the save tlm period will be

#define DEFAULT_SOLAR_SAVE_TLM_TIME		10			//<! save solar panel TLM every 5 seconds
#define SOLAR_SAVE_TLM_PERIOD_ADDR		0x13C		//<! address where the save tlm period will be

#define DEFAULT_WOD_SAVE_TLM_TIME		10			//<! save WOD TLM every 5 seconds
#define WOD_SAVE_TLM_PERIOD_ADDR		0x140		//<! address where the save tlm period will be

#define DEFAULT_RADFET_SAVE_TLM_TIME	15*60		//<! save RADFET TLM every 15 minutes
#define RADFET_SAVE_TLM_PERIOD_ADDR		0x144		//<! address where the save tlm period will be

#define DEFAULT_SEU_SEL_SAVE_TLM_TIME	20			//<! save sel and seu TLM every 20 seconds
#define SEU_SEL_SAVE_TLM_PERIOD_ADDR	0x148		//<! address where the save tlm period will be


#define NUMBER_OF_CMD_RESETS_ADDR		0x160		//<! counts how many restarts did the satellite endure from ground command
#define NUMBER_OF_CMD_RESETS_SIZE		4			//<! size of the parameter in bytes

#define DELAYED_CMD_FRAME_COUNT_ADDR	0x200		//<! Address where amount of frames in delayed buffer is held
#define DELAYED_CMD_FRAME_COUNT_SIZE	1			//<! number of bytes describing the frame count of delayed buffer

#define LAST_RADFET_READ_START			0x210

#define LAST_RADFET_ONE_VALUE_ADDR		0x210		//<! the first value of the fram need to keep after get telemetry.
#define LAST_RADFET_ONE_VALUE_SIZE		4			//<! size of the parameter in bytes

#define LAST_RADFET_TWO_VALUE_ADDR		0x214		//<! the second value of the fram need to keep after get telemetry.
#define LAST_RADFET_TWO_VALUE_SIZE		4			//<! size of the parameter in bytes

#define LAST_RADFET_TEMP_ADDR			0x218		//<! last temp of RADFET need to keep after get telemetry.
#define LAST_RADFET_TEMP_SIZE			8			//<! size of the parameter in bytes

#define	TIME_LAST_RADFET_READ_ADDR		0x220		//<! last time save tlm of RADFET
#define	TIME_LAST_RADFET_READ_SIZE		4			//<! size of the parameter in bytes

#define HAD_RESET_IN_A_MINUTE_ADDR		0x230		//<! the flag that check if we had a reset in less then a minute
#define HAD_RESET_IN_A_MINUTE_SIZE		4			//<! size of the parameter in bytes (Boolean)

#define PAYLOAD_IS_DEAD_ADDR			0x240		//<! the flag that tales us if we need to kill the payload or if he is already dead
#define PAYLOAD_IS_DEAD_SIZE			4			//<! size of the parameter in bytes (Boolean)

#define TRY_TO_DEPLOY_ADDR				0x250 		//zero will be stop trys to deploy ants
#define TRY_TO_DEPLOY_SIZE				4			//<! size of the parameter in bytes

#define TRANS_ABORT_FLAG_ADDR			0x500		//<! transmission abort request flag
#define TRANS_ABORT_FLAG_SIZE			1			//<! size of mute flag in bytes

#define EPS_ALPHA_FILTER_VALUE_ADDR 	0x550			//<! filtering value in the LPF formula
#define EPS_ALPHA_FILTER_VALUE_SIZE 	sizeof(float)	//<! size of double (alpha)

#define EPS_THRESH_VOLTAGES_ADDR		0x666		//<! starting address for eps threshold voltages array
#define EPS_THRESH_VOLTAGES_SIZE 		(NUMBER_OF_THRESHOLD_VOLTAGES * sizeof(voltage_t)) //<! number of bytes in eps threshold voltages array

#define BEACON_INTERVAL_TIME_ADDR 		0x4590		//<! address of value of the delay between 2 beacons
#define BEACON_INTERVAL_TIME_SIZE 		4			//<! size of parameter in bytes

#define LAST_COMM_TIME_ADDR 			0x9485		//<! saves the last unix time at which communication has occured
#define LAST_COMM_TIME_SIZE				4			//<! size of last communication time in bytes

#define DEFAULT_BEACON_INTERVAL_TIME 	20			//<! how many seconds between two beacons [sec]
#define MAX_BEACON_INTERVAL				60			// beacon every 1 minute
#define MIN_BEACON_INTERVAL				5			// beacon every 5 seconds

#endif /* FRAM_FLIGHTPARAMETERS_H_ */
