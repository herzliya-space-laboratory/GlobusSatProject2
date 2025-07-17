/*
 * isisAOCSdemo.c
 *
 *  Created on: Oct. 2023
 */

#include "IsisAOCSdemo.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>

#include <hal/Utility/util.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/LED.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <satellite-subsystems/isis_aocs.h>
#include <satellite-subsystems/isis_aocs_types.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct __attribute__ ((__packed__))
{
    uint8_t imtq : 1;
    uint8_t scg : 1;
    uint8_t rw : 1;
    uint8_t gnss : 1;
    uint8_t str : 1;
    uint8_t thr : 1;
    uint8_t mtm : 1;
}subsystem_mode_t;

typedef enum subsystems_enum
{
    SUBSYSTEM_IMTQ  = 1,
    SUBSYSTEM_SCG  = 2,
    SUBSYSTEM_RW  = 3,
    SUBSYSTEM_GNSS = 4,
    SUBSYSTEM_STR  = 5,
    SUBSYSTEM_THR  = 6,
    SUBSYSTEM_MTM_EXT = 7,
}subsystems_e;


static subsystem_mode_t current_mode = {0};
static 	isis_aocs__tlm_entry_last_pps_sample_forced_t get_latest_entry = {.fields = {.reference = isis_aocs__tlm_entry_reference__relative, .number = 0}};

subsystems_e subsystem_select(void)
{
    int selection = 0;
    printf( "\n\r Select subsystem: \n\r");
    printf("\t 1) iMTQ \n\r");
    printf("\t 2) Self-calibrating Gyroscope \n\r");
    printf("\t 3) Reaction wheels \n\r");
    printf("\t 4) GNSS \n\r");
    printf("\t 5) Startracker \n\r");
    printf("\t 6) Thruster \n\r");
    printf("\t 7) MTM \n\r");

    while(UTIL_DbguGetIntegerMinMax(&selection, 1, 7) == 0);
    return selection;
}
static Boolean IsisAOCS_SetAOCSMode(void)
{
    int selection = 0;
    isis_aocs__generic_error_code_t set_mode_err;
    isis_aocs__set_operating_mode__to_t set_mode_to = {{0}};

    printf( "\n\r Select AOCS mode: \n\r");
    printf("\t 0) IDLE \n\r");
    printf("\t 1 - 15) MISSION SPECIFIC MODES \n\r");

    while(UTIL_DbguGetIntegerMinMax(&selection, 0, 15) == 0) {}

	set_mode_to.fields.mode = selection;
	isis_aocs__set_operating_mode(0, &set_mode_to, &set_mode_err);

    printf("AOCS set operating mode returned: %d \n\r", set_mode_err);
    return TRUE;
}
static Boolean IsisAOCS_GetAOCSTelemetry(void)
{
	int i;
   // isis_aocs__retrieve_aocs_tlm__from_t aocs_tlm = {{0}};
    //int retVal = isis_aocs__retrieve_aocs_tlm(0, 0, &aocs_tlm);
	int retVal = 0;

    isis_aocs__retrieve_current_aocs_tlm__from_t aocs_tlm ={{0}};

    retVal = isis_aocs__retrieve_current_aocs_tlm(0, get_latest_entry, &aocs_tlm);
    if (retVal != isis_aocs__generic_error_code__none)
        printf("AOCS Retrieve Telemetry returned error: %d \n\r", retVal);
    else
    {
        printf("AOCS response status: \n\r");
        printf("\t Error Code: %u ; Entry: %ld ; Timestamp: %lu ; Frame counter: %lu. \n\r",
                aocs_tlm.fields.response_status.fields.error_code,
                aocs_tlm.fields.response_status.fields.entry,
				(long unsigned int)aocs_tlm.fields.response_status.fields.timestamp,
                aocs_tlm.fields.response_status.fields.frame_counter);
        printf("AOCS system status: \n\r");
        printf("\t Subsystem error Code: %d ; Operating mode: %u \n\r",
                aocs_tlm.fields.status.fields.subsystem_error_code,
                aocs_tlm.fields.status.fields.operating_mode);
        printf("\t Subsystems: \n\r");
        printf("\t\t adc_power:  %u; \t adc_state:  %u \n\r",
                aocs_tlm.fields.status.fields.subsystems.fields.power_state.fields.adc,
                aocs_tlm.fields.status.fields.subsystems.fields.adc_state);
        printf("\t\t imtq_power: %u; \t imtq_state: %u \n\r",
                aocs_tlm.fields.status.fields.subsystems.fields.power_state.fields.imtq,
                aocs_tlm.fields.status.fields.subsystems.fields.imtq_state);
        printf("\t\t scg_power:  %u; \t scg_state:  %u \n\r",
                aocs_tlm.fields.status.fields.subsystems.fields.power_state.fields.scg,
                aocs_tlm.fields.status.fields.subsystems.fields.scg_state);
        printf("\t\t rw_power:   %u; \t rw_state:   %u \n\r",
                aocs_tlm.fields.status.fields.subsystems.fields.power_state.fields.rw,
                aocs_tlm.fields.status.fields.subsystems.fields.rw_state);
        printf("\t\t gnss_power: %u; \t gnss_state: %u \n\r",
                aocs_tlm.fields.status.fields.subsystems.fields.power_state.fields.gnss,
                aocs_tlm.fields.status.fields.subsystems.fields.gnss_state);
        printf("\t AOCS status: %d ; Fault count: %d ; Last fault ID: %d ; Last fault time: %lu ; Fatal Error: %d \n\r",
                aocs_tlm.fields.status.fields.aocs_status,
                aocs_tlm.fields.status.fields.fault_count,
                aocs_tlm.fields.status.fields.last_fault_id,
                (long unsigned int)aocs_tlm.fields.status.fields.last_fault_timestamp,
                aocs_tlm.fields.status.fields.fatal_error);

        printf("AOCS Measurements: \n\r");
        printf("\t time_source : %u \n\r", aocs_tlm.fields.measurements.fields.time_source);
        printf("\t unix_time:    %lu \n\r", (long unsigned int)aocs_tlm.fields.measurements.fields.unix_time);
        printf("\t Measurements: \n\r");
        printf("\t\t MTM temp: %i \n\r", aocs_tlm.fields.measurements.fields.measurements.fields.MTM_TEMP);
        for (i = 0; i < 3; i++)
        {
            printf("\t\t MTM_INT[%u]: %f [uT]\n\r", i, (aocs_tlm.fields.measurements.fields.measurements.fields.MTM_INT[i] * 0.001));
            printf("\t\t MTM_EXT[%u]: %f [uT]\n\r", i, (aocs_tlm.fields.measurements.fields.measurements.fields.MTM_EXT[i] * 0.001));
            printf("\t\t SCG_RAW[%u]: %f [deg/s]\n\r", i, (aocs_tlm.fields.measurements.fields.measurements.fields.SCG_RAW[i] * 57.29577951308232));
            printf("\t\t SCG_FILT[%u]: %f [deg/s]\n\r", i, (aocs_tlm.fields.measurements.fields.measurements.fields.SCG_FILT[i] * 57.29577951308232));

        }
        for (i = 0; i < 4; i++)
        {
            printf("\t\t FSS_1[%u]: %u \n\r", i, aocs_tlm.fields.measurements.fields.measurements.fields.FSS_1[i]);
            printf("\t\t FSS_2[%u]: %u \n\r", i, aocs_tlm.fields.measurements.fields.measurements.fields.FSS_2[i]);
            printf("\t\t FSS_3[%u]: %u \n\r", i, aocs_tlm.fields.measurements.fields.measurements.fields.FSS_3[i]);
        }
        for (i = 0; i < 8; i++)
        {
            printf("\t\t PD[%u]: %u \n\r", i, aocs_tlm.fields.measurements.fields.measurements.fields.PD[i]);
        }
        for (i = 0; i < 3; i++)
        {
            printf("\t\t position[%u]: %f [km]\n\r", i, (aocs_tlm.fields.measurements.fields.measurements.fields.GNSS.fields.position[i] * 0.001));
            printf("\t\t velocity[%u]: %f [km/s]\n\r", i, (aocs_tlm.fields.measurements.fields.measurements.fields.GNSS.fields.velocity[i] * 0.001));
        }
        printf("\t Processed sensor data: \n\r");
        for (i = 0; i < 3; i++)
        {
            printf("\t\t MTM_INT[%u]: %f [uT]\n\r", i, (aocs_tlm.fields.processed_sensor_data.fields.MTM_INT[i] * 1000000));
            printf("\t\t MTM_EXT[%u]: %f [uT]\n\r", i, (aocs_tlm.fields.processed_sensor_data.fields.MTM_EXT[i] * 1000000));
            printf("\t\t CSS[%u]: %f \n\r", i, aocs_tlm.fields.processed_sensor_data.fields.CSS[i]);
            printf("\t\t SCG_RAW[%u]: %f [deg/s]\n\r", i, (aocs_tlm.fields.processed_sensor_data.fields.SCG_RAW[i] * 57.29577951308232));
            printf("\t\t SCG_FILT[%u]: %f [deg/s]\n\r", i, (aocs_tlm.fields.processed_sensor_data.fields.SCG_FILT[i] * 57.29577951308232));
            printf("\t\t FSS_1[%u]: %f \n\r", i, aocs_tlm.fields.processed_sensor_data.fields.FSS_1[i]);
            printf("\t\t FSS_2[%u]: %f \n\r", i, aocs_tlm.fields.processed_sensor_data.fields.FSS_2[i]);
            printf("\t\t FSS_3[%u]: %f \n\r", i, aocs_tlm.fields.processed_sensor_data.fields.FSS_3[i]);
        }

    }
    return TRUE;
}
static Boolean IsisAOCS_SetSubsystemMode(void)
{
    subsystems_e selection = subsystem_select();

    isis_aocs__generic_error_code_t set_mode_err;
    isis_aocs__set_subsystem_mode__to_t subsystem_mode_to;
    switch(selection)
    {
        case SUBSYSTEM_IMTQ:
            subsystem_mode_to.fields.mode = !current_mode.imtq;
            current_mode.imtq = !current_mode.imtq;
            subsystem_mode_to.fields.subsystem = isis_aocs__subsys__imtq;
            isis_aocs__set_subsystem_mode(0, &subsystem_mode_to, &set_mode_err);
            break;
        case SUBSYSTEM_SCG:
            subsystem_mode_to.fields.mode = !current_mode.scg;
            current_mode.scg = !current_mode.scg;
            subsystem_mode_to.fields.subsystem = isis_aocs__subsys__scg;
            isis_aocs__set_subsystem_mode(0, &subsystem_mode_to, &set_mode_err);
            break;
        case SUBSYSTEM_RW:
            subsystem_mode_to.fields.mode = !current_mode.rw;
            current_mode.rw = !current_mode.rw;
            subsystem_mode_to.fields.subsystem = isis_aocs__subsys__rw;
            isis_aocs__set_subsystem_mode(0, &subsystem_mode_to, &set_mode_err);
            break;
        case SUBSYSTEM_GNSS:
            subsystem_mode_to.fields.mode = !current_mode.gnss;
            current_mode.gnss = !current_mode.gnss;
            subsystem_mode_to.fields.subsystem = isis_aocs__subsys__gnss;
            isis_aocs__set_subsystem_mode(0, &subsystem_mode_to, &set_mode_err);
            break;
        case SUBSYSTEM_STR:
            subsystem_mode_to.fields.mode = !current_mode.str;
            current_mode.str = !current_mode.str;
            subsystem_mode_to.fields.subsystem = isis_aocs__subsys__str;
            isis_aocs__set_subsystem_mode(0, &subsystem_mode_to, &set_mode_err);
            break;
        case SUBSYSTEM_THR:
            subsystem_mode_to.fields.mode = !current_mode.thr;
            current_mode.thr = !current_mode.thr;
            subsystem_mode_to.fields.subsystem = isis_aocs__subsys__thr;
            isis_aocs__set_subsystem_mode(0, &subsystem_mode_to, &set_mode_err);
            break;
        case SUBSYSTEM_MTM_EXT:
            subsystem_mode_to.fields.mode = !current_mode.mtm;
            current_mode.thr = !current_mode.mtm;
            subsystem_mode_to.fields.subsystem = isis_aocs__subsys__mtm_ext;
            isis_aocs__set_subsystem_mode(0, &subsystem_mode_to, &set_mode_err);
            break;
        default:
            break;
    }
    printf("AOCS set operating mode returned: %d \n\r", set_mode_err);
    return TRUE;
}

void print_tlm_header(isis_aocs__generic_error_code_t error_code, int32_t entry, uint32_t timestamp, isis_aocs__power_hk_t* power)
{
    printf("Error Code: %u ; Entry: %ld ; Timestamp: %lu \n\r",
            error_code, entry, timestamp);

    printf("System Power [current, voltage]: \n\r");

    printf("PG: %d, %d\n\r", power->fields.system_pg.fields.current, power->fields.system_pg.fields.voltage);
    printf("In: %d, %d\n\r", power->fields.system_in.fields.current, power->fields.system_in.fields.voltage);
    printf("5v: %d, %d\n\r", power->fields.system_5v.fields.current, power->fields.system_5v.fields.voltage);
    printf("3v3: %d, %d\n\r", power->fields.system_3v3.fields.current, power->fields.system_3v3.fields.voltage);
}

void get_imtq_tlm(void)
{
	int i;

	isis_aocs__retrieve_current_subsystem_tlm__from_t response = {{0}};
    isis_aocs__retrieve_current_subsystem_tlm(0, &response);

    print_tlm_header(response.fields.response_status.fields.error_code, response.fields.response_status.fields.entry,
    					response.fields.response_status.fields.timestamp, &response.fields.tlm.fields.power);

    printf("iMTQ Telemetry: \n\r");
    printf("\t Mode: %u \n\r", response.fields.tlm.fields.imtq.fields.mode);
    printf("\t Error: %u \n\r", response.fields.tlm.fields.imtq.fields.error);
    printf("\t Config status: %u \n\r", response.fields.tlm.fields.imtq.fields.config_status);
    printf("\t Uptime: %lu \n\r", response.fields.tlm.fields.imtq.fields.uptime);
    printf("\t V_D: %f V \n\r", ((float)response.fields.tlm.fields.imtq.fields.V_D * 0.001));
    printf("\t V_A: %f V \n\r", ((float)response.fields.tlm.fields.imtq.fields.V_A * 0.001));
    printf("\t I_D: %f mA \n\r", ((float)response.fields.tlm.fields.imtq.fields.I_D * 0.1));
    printf("\t I_A: %f mA \n\r", ((float)response.fields.tlm.fields.imtq.fields.I_A * 0.1));
    for (i = 0; i < 3; i++)
    {
        printf("\t I_C[%u]: %f mA \n\r", i, ((float)response.fields.tlm.fields.imtq.fields.I_C[i] * 0.1));
        printf("\t T_C{%u}: %i deg. C \n\r", i,response.fields.tlm.fields.imtq.fields.T_C[i]);
    }
    printf("\t T_MCU: %i deg. C \n\r", response.fields.tlm.fields.imtq.fields.T_MCU);
}

void get_scg_tlm(void)
{
	isis_aocs__retrieve_current_subsystem_tlm__from_t response = {{0}};
    isis_aocs__retrieve_current_subsystem_tlm(0, &response);

    print_tlm_header(response.fields.response_status.fields.error_code, response.fields.response_status.fields.entry,
    					response.fields.response_status.fields.timestamp, &response.fields.tlm.fields.power);

    printf("SCG Telemetry: \n\r");
    printf("\t Uptime: %lu seconds\n\r", response.fields.tlm.fields.scg.fields.system_uptime);
    printf("\t mcu_temp: %i deg. C\n\r", response.fields.tlm.fields.scg.fields.mcu_temp);
    printf("\t gyro1_temp: %i deg. C\n\r", 25 - response.fields.tlm.fields.scg.fields.gyro1_temp);
    printf("\t gyro2_temp: %i deg. C\n\r", 25 - response.fields.tlm.fields.scg.fields.gyro2_temp);
    printf("\t gyro3_temp: %i deg. C\n\r", 25 - response.fields.tlm.fields.scg.fields.gyro3_temp);
    printf("\t current: %u mA \n\r", response.fields.tlm.fields.scg.fields.current);
    printf("\t voltage: %f V \n\r", ((float)response.fields.tlm.fields.scg.fields.voltage * 0.001));
    printf("\t SCG system error: \n\r");
    printf("\t \t spi1_error: %u \n\r", response.fields.tlm.fields.scg.fields.bitmap1.fields.spi1_error);
    printf("\t \t spi2_error: %u \n\r", response.fields.tlm.fields.scg.fields.bitmap1.fields.spi2_error);
    printf("\t \t i2c2_error: %u \n\r", response.fields.tlm.fields.scg.fields.bitmap1.fields.i2c2_error);
    printf("\t \t usart1_error: %u \n\r", response.fields.tlm.fields.scg.fields.bitmap1.fields.usart1_error);
    printf("\t \t can1_error: %u \n\r", response.fields.tlm.fields.scg.fields.bitmap1.fields.can1_error);
    printf("\t \t gyro_ovr_error: %u \n\r", response.fields.tlm.fields.scg.fields.bitmap1.fields.gyro_ovr_error);
    printf("\t \t flash_error: %u \n\r", response.fields.tlm.fields.scg.fields.bitmap1.fields.flash_error);
    printf("\t \t ugakf_init_error: %u \n\r", response.fields.tlm.fields.scg.fields.bitmap1.fields.ugakf_init_error);
    printf("\t \t ugakf_update_error: %u \n\r", response.fields.tlm.fields.scg.fields.bitmap1.fields.ugakf_update_error);
    printf("\t \t sram2_parity_error: %u \n\r", response.fields.tlm.fields.scg.fields.bitmap1.fields.sram2_parity_error);
}

void get_rw_tlm(void)
{
	isis_aocs__retrieve_current_aocs_tlm__from_t response;
	isis_aocs__retrieve_current_aocs_tlm(0, get_latest_entry, &response);

	isis_aocs__retrieve_current_subsystem_tlm__from_t response_tlm = {{0}};
    isis_aocs__retrieve_current_subsystem_tlm(0, &response_tlm);

    print_tlm_header(response_tlm.fields.response_status.fields.error_code, response_tlm.fields.response_status.fields.entry,
    					response_tlm.fields.response_status.fields.timestamp, &response_tlm.fields.tlm.fields.power);

    printf("Reaction wheel 1 telemetry: \n\r");
	printf("\t\t Speed: %f rpm\n\r", (response.fields.measurements.fields.measurements.fields.RW[0] * 9.549296585513721));
	printf("\t\t Current: %i mA\n\r", response_tlm.fields.tlm.fields.power.fields.rw0.fields.current);

    printf("Reaction wheel 2 telemetry: \n\r");
	printf("\t\t Speed: %f rpm\n\r", (response.fields.measurements.fields.measurements.fields.RW[1] * 9.549296585513721));
	printf("\t\t Current: %i mA\n\r", response_tlm.fields.tlm.fields.power.fields.rw1.fields.current);

    printf("Reaction wheel 3 telemetry: \n\r");
	printf("\t\t Speed: %f rpm\n\r", (response.fields.measurements.fields.measurements.fields.RW[2] * 9.549296585513721));
	printf("\t\t Current: %i mA\n\r", response_tlm.fields.tlm.fields.power.fields.rw2.fields.current);

    printf("Reaction wheel 4 telemetry: \n\r");
	printf("\t\t Speed: %f rpm\n\r", (response.fields.measurements.fields.measurements.fields.RW[3] * 9.549296585513721));
	printf("\t\t Current: %i mA\n\r", response_tlm.fields.tlm.fields.power.fields.rw3.fields.current);

}
void get_gnss_tlm(void)
{
	isis_aocs__retrieve_current_subsystem_tlm__from_t response = {{0}};
    isis_aocs__retrieve_current_subsystem_tlm(0, &response);

    print_tlm_header(response.fields.response_status.fields.error_code, response.fields.response_status.fields.entry,
    					response.fields.response_status.fields.timestamp, &response.fields.tlm.fields.power);
    printf("GNSS  telemetry: \n\r");
    printf("\t Novatel status: \n\r");
    printf("\t\t PPS pin status: %u\n\r", response.fields.tlm.fields.gnss.fields.novatel_status.fields.pos_valid_pin_status);
    printf("\t\t Time since last message: %lu s\n\r", (long unsigned int)response.fields.tlm.fields.gnss.fields.novatel_status.fields.time_lastmsg);
    printf("\t Hardware monitor: \n\r");
    printf("\t\t temperature: %u deg. C\n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.temperature);
    printf("\t\t temperature_2: %u deg. C\n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.temperature_2);
    printf("\t\t antenna_current: %u mA\n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.antenna_current);
    printf("\t\t antenna_voltage: %f V\n\r", ((float)response.fields.tlm.fields.gnss.fields.hwmonitor.fields.antenna_voltage * 0.001));
    printf("\t\t core_voltage: %f V\n\r", ((float)response.fields.tlm.fields.gnss.fields.hwmonitor.fields.core_voltage * 0.001));
    printf("\t\t supply_voltage: %f V\n\r", ((float)response.fields.tlm.fields.gnss.fields.hwmonitor.fields.supply_voltage * 0.001));
    printf("\t\t voltage_1v8: %f V\n\r", ((float)response.fields.tlm.fields.gnss.fields.hwmonitor.fields.voltage_1v8 * 0.001));
    printf("\t\t peripheral_voltage: %f V\n\r", ((float)response.fields.tlm.fields.gnss.fields.hwmonitor.fields.peripheral_voltage * 0.001));
    printf("\t\t temperature_status: %u \n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.temperature_status);
    printf("\t\t temperature_2_status: %u \n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.temperature_2_status);
    printf("\t\t antenna_current_status: %u \n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.antenna_current_status);
    printf("\t\t antenna_voltage_status: %u \n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.antenna_voltage_status);
    printf("\t\t core_voltage_status: %u \n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.core_voltage_status);
    printf("\t\t supply_voltage_status: %u \n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.supply_voltage_status);
    printf("\t\t voltage_1v8_status: %u \n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.voltage_1v8_status);
    printf("\t\t peripheral_voltage_status: %u \n\r", response.fields.tlm.fields.gnss.fields.hwmonitor.fields.peripheral_voltage_status);
    printf("\t Time status: \n\r");
    printf("\t\t pps_status: %u\n\r", response.fields.tlm.fields.gnss.fields.time_status.fields.pps_status);
    printf("\t\t time_status: %u\n\r", response.fields.tlm.fields.gnss.fields.time_status.fields.time_status);
    printf("\t Solution status: \n\r");
    printf("\t\t solstatus: %u\n\r", response.fields.tlm.fields.gnss.fields.solution_status.fields.solstatus);
    printf("\t\t sats_tracked: %u\n\r", response.fields.tlm.fields.gnss.fields.solution_status.fields.sats_tracked);
    printf("\t\t sats_used: %u\n\r", response.fields.tlm.fields.gnss.fields.solution_status.fields.sats_used);

}

void get_str_tlm(void)
{
	isis_aocs__retrieve_current_subsystem_tlm__from_t response = {{0}};
	isis_aocs__retrieve_current_subsystem_tlm(0, &response);

    print_tlm_header(response.fields.response_status.fields.error_code, response.fields.response_status.fields.entry,
    					response.fields.response_status.fields.timestamp, &response.fields.tlm.fields.power);
    printf("Star Tracker  telemetry: \n\r");
    printf("\t Star Tracker timestamp: %f \n\r", response.fields.tlm.fields.str.fields.MainStatus.fields.StatusTimestamp);
    printf("\t Star Tracker mode: %c \n\r", response.fields.tlm.fields.str.fields.MainStatus.fields.STRMode);
    printf("\t Star Tracker OH1 temp: %f deg. C\n\r", ((float)response.fields.tlm.fields.str.fields.MainStatus.fields.OH1Temp * 0.01));
    printf("\t Star Tracker OH2 temp: %f deg. C\n\r", ((float)response.fields.tlm.fields.str.fields.MainStatus.fields.OH2Temp * 0.01));
    printf("\t Star Tracker uptime: %u s\n\r", (unsigned int)response.fields.tlm.fields.str.fields.MainStatus.fields.Uptime);
    printf("\t Star Tracker pps age: %f s\n\r", ((float)response.fields.tlm.fields.str.fields.PpsAge * 0.001));
}

void get_thr_tlm(void)
{
	isis_aocs__retrieve_current_subsystem_tlm__from_t response = {{0}};
    isis_aocs__retrieve_current_subsystem_tlm(0, &response);

    print_tlm_header(response.fields.response_status.fields.error_code, response.fields.response_status.fields.entry,
    					response.fields.response_status.fields.timestamp, &response.fields.tlm.fields.power);
    printf("Thruster  telemetry: \n\r");
    printf("\t Thruster telemetry is a blob which is defined per mission, as such no telemetry will be parsed here but defined in a provided ICD.\n\r");
    printf("\t Thruster timestamp: %lu \n\r", (long unsigned int)response.fields.tlm.fields.thr.fields.data.fields.timestamp);
}

void get_mtm_tlm(void)
{
    isis_aocs__retrieve_current_aocs_tlm__from_t aocs_tlm ={{0}};
    int i = 0;
    driver_error_t retVal = driver_error_none;

    retVal = isis_aocs__retrieve_current_aocs_tlm(0, get_latest_entry, &aocs_tlm);

    if (retVal != driver_error_none)
    {
        printf("AOCS Retrieve Telemetry returned error: %d \n\r", retVal);
    }
    else
    {
        printf("AOCS response status: \n\r");
        printf("\t Error Code: %u ; Entry: %ld ; Timestamp: %lu ; Frame counter: %lu. \n\r",
                aocs_tlm.fields.response_status.fields.error_code,
                aocs_tlm.fields.response_status.fields.entry,
				(long unsigned int)aocs_tlm.fields.response_status.fields.timestamp,
                aocs_tlm.fields.response_status.fields.frame_counter);

        printf("\r\nMTM  telemetry: \n\r");
        printf("\t\t MTM temp: %i \n\r", aocs_tlm.fields.measurements.fields.measurements.fields.MTM_TEMP);

        for (i = 0; i < 3; i++)
        {
            printf("\t\t MTM_INT[%u]: %f [uT]\n\r", i, ((float)aocs_tlm.fields.measurements.fields.measurements.fields.MTM_INT[i] * 0.001));
            printf("\t\t MTM_EXT[%u]: %f [uT]\n\r", i, ((float)aocs_tlm.fields.measurements.fields.measurements.fields.MTM_EXT[i] * 0.001));
        }

        printf("\t Processed sensor data: \n\r");
        for (i = 0; i < 3; i++)
        {
            printf("\t\t MTM_INT[%u]: %f [uT]\n\r", i, ((float)aocs_tlm.fields.processed_sensor_data.fields.MTM_INT[i] * 1000000));
            printf("\t\t MTM_EXT[%u]: %f [uT]\n\r", i, ((float)aocs_tlm.fields.processed_sensor_data.fields.MTM_EXT[i] * 1000000));
        }
    }

}

static Boolean IsisAOCS_GetSubsystemTelemetry(void)
{
    subsystems_e selection = subsystem_select();

    switch(selection)
    {
        case SUBSYSTEM_IMTQ:
            get_imtq_tlm();
            break;
        case SUBSYSTEM_SCG:
            get_scg_tlm();
            break;
        case SUBSYSTEM_RW:
            get_rw_tlm();
            break;
        case SUBSYSTEM_GNSS:
            get_gnss_tlm();
            break;
        case SUBSYSTEM_STR:
        	get_str_tlm();
            break;
        case SUBSYSTEM_THR:
        	get_thr_tlm();
            break;
        case SUBSYSTEM_MTM_EXT:
        	get_mtm_tlm();
            break;
        default:
            break;
    }
    return TRUE;
}
static Boolean selectAndExecute_IsisAOCSDemo(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 1) Set AOCS mode \n\r");
    printf("\t 2) Retrieve AOCS telemetry \n\r");
    printf("\t 3) Toggle subsystem mode \n\r");
    printf("\t 4) Retrieve subsystem telemetry \n\r");
    printf("\t 5) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 5) == 0);

	switch(selection)
	{
		case 1:
			offerMoreTests = IsisAOCS_SetAOCSMode();
			break;
		case 2:
			offerMoreTests = IsisAOCS_GetAOCSTelemetry();
			break;
		case 3:
			offerMoreTests = IsisAOCS_SetSubsystemMode();
			break;
		case 4:
		    offerMoreTests = IsisAOCS_GetSubsystemTelemetry();
		    break;
        case 5:
            offerMoreTests = FALSE;
            break;
		default:
			break;
	}

	return offerMoreTests;
}

Boolean isis_aocs_demo_Init(void)
{
    int rv;

    rv = ISIS_AOCS_Init(&(ISIS_AOCS_t){.i2cAddr = 0x66}, 1);
    if(rv != E_NO_SS_ERR && rv != E_IS_INITIALIZED && rv != driver_error_reinit)
    {
        // we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
        // hanging the code
        TRACE_ERROR("\n\r ISIS_AOCS_Init() failed; err=%d! Exiting ... \n\r", rv);
        return FALSE;
    }

    return TRUE;
}

void isis_aocs_demo_Loop(void)
{
    Boolean offerMoreTests = FALSE;

    while(1)
    {
        offerMoreTests = selectAndExecute_IsisAOCSDemo(); // show the demo command line interface and handle commands

        if(offerMoreTests == FALSE)  // was exit/back
        {
            break;
        }
    }
}

Boolean isis_aocs_demo_main(void)
{
    if(isis_aocs_demo_Init())
    {
        isis_aocs_demo_Loop();
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

Boolean isis_aocs_demo(void)
{
    return isis_aocs_demo_main();
}
