#ifndef TELEMETRYCOLLECTOR_H_
#define TELEMETRYCOLLECTOR_H_

#include "GlobalStandards.h"
#include "TelemetryFiles.h"
#include "TLM_management.h"
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include <hal/supervisor.h>
#include <hcc/api_fat.h>
#include <hal/Storage/FRAM.h>
#include "TLM_management.h"

#include "utils.h"
#define NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS 7

typedef union __attribute__ ((__packed__)) _PeriodTimes
{
    unsigned char raw[sizeof(int)*7];
    struct __attribute__ ((__packed__))
    {
        unsigned int eps;
        unsigned int trxvu;
        unsigned int ants;
        unsigned int solar_panels;
        unsigned int wod;
        unsigned int radfet;
        unsigned int seu_sel;
    } fields;
} PeriodTimes;


typedef struct __attribute__ ((__packed__)) WOD_Telemetry_t
{
	voltage_t voltBattery;					///< the current voltage on the battery [mV]
	voltage_t volt_5V;				///< the current voltage on the 5V bus [mV]
	voltage_t volt_3V3;				///< the current voltage on the 3V3 bus [mV]
	power_t charging_power;			///< the current charging power [mW]
	power_t consumed_power;			///< the power consumed by the satellite [mW]
	current_t electric_current;		///< the up-to-date electric current of the battery [mA]
	current_t current_3V3;			///< the up-to-date 3.3 Volt bus current of the battery [mA]
	current_t current_5V;			///< the up-to-date 5 Volt bus current of the battery [mA]
	temp_t mcu_temp; 				/*!< Measured temperature provided by a sensor internal to the MCU [degC]*/
	temp_t bat_temp; 				/*!< 2 cell battery pack: not used 4 cell battery pack: Battery pack temperature on the front of the battery pack. [degC] */
	int32_t solar_panels[NUMBER_OF_SOLAR_PANELS]; // temp of each solar panel
	time_unix sat_time;				///< current Unix time of the satellites clock [sec]
	unsigned int free_memory;		///< number of bytes free in the satellites SD [byte]
	unsigned int corrupt_bytes;		///< number of currpted bytes in the memory	[bytes]
	unsigned int total_memory;		///< number of bytes in the memory	[bytes]
	unsigned int used_bytes;		///< number of used bytes in the memory	[bytes]
	int lastFS_error;				///< last error of the file system
	unsigned int number_of_resets;	///< counts the number of resets the satellite has gone through [#]
	time_unix sat_uptime;			///< Sat uptime
	unsigned int num_of_cmd_resets;///< counts the number of resets the satellite has gone through due to ground station command [#]
	int sel_counter;
	int seu_counter;
	int sensor_one_radfet;
	int sensor_two_radfet;
	double radfet_temp;
	time_unix last_radfet_read_time;
	voltage_t power_payload;
	uint8_t payload_flag;

} WOD_Telemetry_t;


typedef struct solar_tlm { int32_t values[ISIS_SOLAR_PANEL_COUNT]; } solar_tlm_t;

typedef struct
{
	int count;
	int sat_resets_count;
	int changes_in_mode;
} payloadSEL_data;

/**
 * get all tlm save time periods from FRAM
 */
void InitSavePeriodTimes();

/*!
 * @brief saves all telemetries into the appropriate TLM files
 */
void TelemetryCollectorLogic();

/*!
 *  @brief saves current EPS telemetry into file
 */
void TelemetrySaveEPS();

/*!
 *  @brief saves current TRXVU telemetry into file
 */
void TelemetrySaveTx();

/*!
 *  @brief saves current TRXVU telemetry into file
 */
void TelemetrySaveRx();


/*!
 *  @brief saves current Antenna telemetry into file
 */
void TelemetrySaveAnt0();

/*!
 *  @brief saves current Antenna telemetry into file
 */
void TelemetrySaveAnt1();

/*!
 *  @brief saves current solar panel telemetry (temparture of each panel) into file
 */
void TelemetrySaveSolarPanels();

void TelemetrySavePayloadRADFET();

void TelemetrySavePayloadEvents();

/*!
 *  @brief saves current WOD telemetry into file
 */
void TelemetrySaveWOD();

/*!
 * @brief Gets all necessary telemetry and arranges it into a WOD structure
 * @param[out] output WOD telemetry. If an error occurred while getting TLM the fields will be -1
 * @return 0 	//error according to <hal/errors.h>
 */
int GetCurrentWODTelemetry(WOD_Telemetry_t *wod);

void GetSEL_telemetry(PayloadEventData eventsData, payloadSEL_data *selData);

void TelemetrySavePayloadSEL(PayloadEventData eventsData, time_unix time);


#endif /* TELEMETRYCOLLECTOR_H_ */
