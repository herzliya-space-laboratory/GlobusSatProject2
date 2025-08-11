/*
 * IsisGomEPSdemo.c
 *
 *  Created on: 11 nov. 2013
 *      Author: malv
 */

#include "GomEPSdemo.h"
#include "common.h"

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

#include <satellite-subsystems/GomEPS.h>

#include <hal/errors.h>
#include <hal/Timing/Time.h>

/*
 * prints all the parameters in the structure of gom_eps_hk_basic_t.
 * */
static Boolean EPS_TelemetryHKbasic(void)
{
	gom_eps_hk_basic_t myEpsTelemetry_hk_basic;

	printf("\r\nEPS Telemetry HK basic \r\n\n");
	print_error(GomEpsGetHkData_basic(0, &myEpsTelemetry_hk_basic));

	printf("Number of EPS reboots = %d \r\n", myEpsTelemetry_hk_basic.fields.counter_boot);

	printf("Temperature sensors. TEMP1 = %d \r\n", myEpsTelemetry_hk_basic.fields.temp[0]);
	printf("Temperature sensors. TEMP2 = %d \r\n", myEpsTelemetry_hk_basic.fields.temp[1]);
	printf("Temperature sensors. TEMP3 = %d \r\n", myEpsTelemetry_hk_basic.fields.temp[2]);
	printf("Temperature sensors. TEMP4 = %d \r\n", myEpsTelemetry_hk_basic.fields.temp[3]);
	printf("Temperature sensors. BATT0 = %d \r\n", myEpsTelemetry_hk_basic.fields.temp[4]);
	printf("Temperature sensors. BATT1 = %d \r\n", myEpsTelemetry_hk_basic.fields.temp[5]);

	printf("Cause of last EPS reset = %d\r\n", myEpsTelemetry_hk_basic.fields.bootcause);
	printf("Battery Mode = %d\r\n", myEpsTelemetry_hk_basic.fields.battmode);
	printf("PPT tracker Mode = %d\r\n", myEpsTelemetry_hk_basic.fields.pptmode);
	printf(" \r\n");

	return TRUE;
}

/*
 * prints all the parameters in the structure of gom_eps_hk_wdt_t.
 * */
static Boolean EPS_TelemetryHKwdt(void)
{
	gom_eps_hk_wdt_t myEpsTelemetry_hk_wdt;

	printf("\r\nEPS Telemetry HK wdt \r\n\n");
	print_error(GomEpsGetHkData_wdt(0, &myEpsTelemetry_hk_wdt));

	printf("Time left on I2C WDT = %d \r\n", myEpsTelemetry_hk_wdt.fields.wdt_i2c_time_left);
	printf("Time left on I2C WDT GND = %d \r\n", myEpsTelemetry_hk_wdt.fields. wdt_gnd_time_left);
	printf("Time left on I2C WDT CSP ping 1= %d \r\n", myEpsTelemetry_hk_wdt.fields.wdt_csp_pings_left[0]);
	printf("Time left on I2C WDT CSP ping 2= %d \r\n", myEpsTelemetry_hk_wdt.fields.wdt_csp_pings_left[1]);

	printf("Number of WDT I2C reboots = %d \r\n", myEpsTelemetry_hk_wdt.fields.counter_wdt_i2c);
	printf("Number of WDT GND reboots = %d \r\n", myEpsTelemetry_hk_wdt.fields.counter_wdt_gnd);
	printf("Number of WDT CSP reboots ping 1= %d \r\n", myEpsTelemetry_hk_wdt.fields.counter_wdt_csp[0]);
	printf("Number of WDT CSP reboots ping 2= %d \r\n", myEpsTelemetry_hk_wdt.fields.counter_wdt_csp[1]);
	printf(" \r\n");

	return TRUE;
}

/*
 * prints all the parameters in the structure of gom_eps_hk_out_t.
 * */
static Boolean EPS_TelemetryHKout(void)
{
	gom_eps_hk_out_t myEpsTelemetry_hk_out;

	printf("\r\nEPS Telemetry HK out \r\n\n");
	print_error(GomEpsGetHkData_out(0, &myEpsTelemetry_hk_out));

	printf("Current outputs current 1 = %d mA\r\n", myEpsTelemetry_hk_out.fields.curout[0]);
	printf("Current outputs current 2 = %d mA\r\n", myEpsTelemetry_hk_out.fields.curout[1]);
	printf("Current outputs current 3 = %d mA\r\n", myEpsTelemetry_hk_out.fields.curout[2]);
	printf("Current outputs current 4 = %d mA\r\n", myEpsTelemetry_hk_out.fields.curout[3]);
	printf("Current outputs current 5 = %d mA\r\n", myEpsTelemetry_hk_out.fields.curout[4]);
	printf("Current outputs current 6 = %d mA\r\n", myEpsTelemetry_hk_out.fields.curout[5]);

	printf("Output Status 1 = %d \r\n", myEpsTelemetry_hk_out.fields.output[0]);
	printf("Output Status 2 = %d \r\n", myEpsTelemetry_hk_out.fields.output[1]);
	printf("Output Status 3 = %d \r\n", myEpsTelemetry_hk_out.fields.output[2]);
	printf("Output Status 4 = %d \r\n", myEpsTelemetry_hk_out.fields.output[3]);
	printf("Output Status 5 = %d \r\n", myEpsTelemetry_hk_out.fields.output[4]);
	printf("Output Status 6 = %d \r\n", myEpsTelemetry_hk_out.fields.output[5]);
	printf("Output Status 7 = %d \r\n", myEpsTelemetry_hk_out.fields.output[6]);
	printf("Output Status 8 = %d \r\n", myEpsTelemetry_hk_out.fields.output[7]);

	printf("Output Time until Power is on 1 = %d \r\n", myEpsTelemetry_hk_out.fields.output_on_delta[0]);
	printf("Output Time until Power is on 2 = %d \r\n", myEpsTelemetry_hk_out.fields.output_on_delta[1]);
	printf("Output Time until Power is on 3 = %d \r\n", myEpsTelemetry_hk_out.fields.output_on_delta[2]);
	printf("Output Time until Power is on 4 = %d \r\n", myEpsTelemetry_hk_out.fields.output_on_delta[3]);
	printf("Output Time until Power is on 5 = %d \r\n", myEpsTelemetry_hk_out.fields.output_on_delta[4]);
	printf("Output Time until Power is on 6 = %d \r\n", myEpsTelemetry_hk_out.fields.output_on_delta[5]);
	printf("Output Time until Power is on 7 = %d \r\n", myEpsTelemetry_hk_out.fields.output_on_delta[6]);
	printf("Output Time until Power is on 8 = %d \r\n", myEpsTelemetry_hk_out.fields.output_on_delta[7]);

	printf("Output Time until Power is off 1 = %d \r\n", myEpsTelemetry_hk_out.fields.output_off_delta[0]);
	printf("Output Time until Power is off 2 = %d \r\n", myEpsTelemetry_hk_out.fields.output_off_delta[1]);
	printf("Output Time until Power is off 3 = %d \r\n", myEpsTelemetry_hk_out.fields.output_off_delta[2]);
	printf("Output Time until Power is off 4 = %d \r\n", myEpsTelemetry_hk_out.fields.output_off_delta[3]);
	printf("Output Time until Power is off 5 = %d \r\n", myEpsTelemetry_hk_out.fields.output_off_delta[4]);
	printf("Output Time until Power is off 6 = %d \r\n", myEpsTelemetry_hk_out.fields.output_off_delta[5]);
	printf("Output Time until Power is off 7 = %d \r\n", myEpsTelemetry_hk_out.fields.output_off_delta[6]);
	printf("Output Time until Power is off 8 = %d \r\n", myEpsTelemetry_hk_out.fields.output_off_delta[7]);

	printf("Number of latch-ups 1 = %d \r\n", myEpsTelemetry_hk_out.fields.latchup[0]);
	printf("Number of latch-ups 2 = %d \r\n", myEpsTelemetry_hk_out.fields.latchup[1]);
	printf("Number of latch-ups 3 = %d \r\n", myEpsTelemetry_hk_out.fields.latchup[2]);
	printf("Number of latch-ups 4 = %d \r\n", myEpsTelemetry_hk_out.fields.latchup[3]);
	printf("Number of latch-ups 5 = %d \r\n", myEpsTelemetry_hk_out.fields.latchup[4]);
	printf("Number of latch-ups 6 = %d \r\n", myEpsTelemetry_hk_out.fields.latchup[5]);
	printf(" \r\n");

	return TRUE;
}

/**
 * Prints out all the parameters of the struct gom_eps_hk_vi_t.
 */
static Boolean EPS_TelemetryHKvi(void)
{
	gom_eps_hk_vi_t myEpsTelemetry_hk_vi;

	printf("\r\nEPS Telemetry HK vi \r\n\n");
	print_error(GomEpsGetHkData_vi(0, &myEpsTelemetry_hk_vi));

	printf("Voltage of boost converters PV1 = %d mV\r\n", myEpsTelemetry_hk_vi.fields.vboost[0]);
	printf("Voltage of boost converters PV2 = %d mV\r\n", myEpsTelemetry_hk_vi.fields.vboost[1]);
	printf("Voltage of boost converters PV3 = %d mV\r\n", myEpsTelemetry_hk_vi.fields.vboost[2]);

	printf("Voltage of the battery = %d mV\r\n", myEpsTelemetry_hk_vi.fields.vbatt);

	printf("Current inputs current 1 = %d mA\r\n", myEpsTelemetry_hk_vi.fields.curin[0]);
	printf("Current inputs current 2 = %d mA\r\n", myEpsTelemetry_hk_vi.fields.curin[1]);
	printf("Current inputs current 3 = %d mA\r\n", myEpsTelemetry_hk_vi.fields.curin[2]);

	printf("Current from boost converters = %d mA\r\n", myEpsTelemetry_hk_vi.fields.cursun);
	printf("Current out of the battery = %d mA\r\n", myEpsTelemetry_hk_vi.fields.cursys);
	printf(" \r\n");

	return TRUE;
}
/*
 * prints all the parameters in the structure of gom_eps_hk_t.
 * */
static Boolean EPS_TelemetryHKGeneral(void)
{
	gom_eps_hk_t myEpsTelemetry_hk;

	printf("\r\nEPS Telemetry HK General \r\n\n");
	print_error(GomEpsGetHkData_general(0, &myEpsTelemetry_hk));

	printf("Voltage of boost converters PV1 = %d mV\r\n", myEpsTelemetry_hk.fields.vboost[0]);
	printf("Voltage of boost converters PV2 = %d mV\r\n", myEpsTelemetry_hk.fields.vboost[1]);
	printf("Voltage of boost converters PV3 = %d mV\r\n", myEpsTelemetry_hk.fields.vboost[2]);

	printf("Voltage of the battery = %d mV\r\n", myEpsTelemetry_hk.fields.vbatt);

	printf("Current inputs current 1 = %d mA\r\n", myEpsTelemetry_hk.fields.curin[0]);
	printf("Current inputs current 2 = %d mA\r\n", myEpsTelemetry_hk.fields.curin[1]);
	printf("Current inputs current 3 = %d mA\r\n", myEpsTelemetry_hk.fields.curin[2]);

	printf("Current from boost converters = %d mA\r\n", myEpsTelemetry_hk.fields.cursun);
	printf("Current out of the battery = %d mA\r\n", myEpsTelemetry_hk.fields.cursys);

	printf("Current outputs current 1 = %d mA\r\n", myEpsTelemetry_hk.fields.curout[0]);
	printf("Current outputs current 2 = %d mA\r\n", myEpsTelemetry_hk.fields.curout[1]);
	printf("Current outputs current 3 = %d mA\r\n", myEpsTelemetry_hk.fields.curout[2]);
	printf("Current outputs current 4 = %d mA\r\n", myEpsTelemetry_hk.fields.curout[3]);
	printf("Current outputs current 5 = %d mA\r\n", myEpsTelemetry_hk.fields.curout[4]);
	printf("Current outputs current 6 = %d mA\r\n", myEpsTelemetry_hk.fields.curout[5]);

	printf("Output Status 1 = %d \r\n", myEpsTelemetry_hk.fields.output[0]);
	printf("Output Status 2 = %d \r\n", myEpsTelemetry_hk.fields.output[1]);
	printf("Output Status 3 = %d \r\n", myEpsTelemetry_hk.fields.output[2]);
	printf("Output Status 4 = %d \r\n", myEpsTelemetry_hk.fields.output[3]);
	printf("Output Status 5 = %d \r\n", myEpsTelemetry_hk.fields.output[4]);
	printf("Output Status 6 = %d \r\n", myEpsTelemetry_hk.fields.output[5]);
	printf("Output Status 7 = %d \r\n", myEpsTelemetry_hk.fields.output[6]);
	printf("Output Status 8 = %d \r\n", myEpsTelemetry_hk.fields.output[7]);

	printf("Output Time until Power is on 1 = %d \r\n", myEpsTelemetry_hk.fields.output_on_delta[0]);
	printf("Output Time until Power is on 2 = %d \r\n", myEpsTelemetry_hk.fields.output_on_delta[1]);
	printf("Output Time until Power is on 3 = %d \r\n", myEpsTelemetry_hk.fields.output_on_delta[2]);
	printf("Output Time until Power is on 4 = %d \r\n", myEpsTelemetry_hk.fields.output_on_delta[3]);
	printf("Output Time until Power is on 5 = %d \r\n", myEpsTelemetry_hk.fields.output_on_delta[4]);
	printf("Output Time until Power is on 6 = %d \r\n", myEpsTelemetry_hk.fields.output_on_delta[5]);
	printf("Output Time until Power is on 7 = %d \r\n", myEpsTelemetry_hk.fields.output_on_delta[6]);
	printf("Output Time until Power is on 8 = %d \r\n", myEpsTelemetry_hk.fields.output_on_delta[7]);

	printf("Output Time until Power is off 1 = %d \r\n", myEpsTelemetry_hk.fields.output_off_delta[0]);
	printf("Output Time until Power is off 2 = %d \r\n", myEpsTelemetry_hk.fields.output_off_delta[1]);
	printf("Output Time until Power is off 3 = %d \r\n", myEpsTelemetry_hk.fields.output_off_delta[2]);
	printf("Output Time until Power is off 4 = %d \r\n", myEpsTelemetry_hk.fields.output_off_delta[3]);
	printf("Output Time until Power is off 5 = %d \r\n", myEpsTelemetry_hk.fields.output_off_delta[4]);
	printf("Output Time until Power is off 6 = %d \r\n", myEpsTelemetry_hk.fields.output_off_delta[5]);
	printf("Output Time until Power is off 7 = %d \r\n", myEpsTelemetry_hk.fields.output_off_delta[6]);
	printf("Output Time until Power is off 8 = %d \r\n", myEpsTelemetry_hk.fields.output_off_delta[7]);

	printf("Number of latch-ups 1 = %d \r\n", myEpsTelemetry_hk.fields.latchup[0]);
	printf("Number of latch-ups 2 = %d \r\n", myEpsTelemetry_hk.fields.latchup[1]);
	printf("Number of latch-ups 3 = %d \r\n", myEpsTelemetry_hk.fields.latchup[2]);
	printf("Number of latch-ups 4 = %d \r\n", myEpsTelemetry_hk.fields.latchup[3]);
	printf("Number of latch-ups 5 = %d \r\n", myEpsTelemetry_hk.fields.latchup[4]);
	printf("Number of latch-ups 6 = %d \r\n", myEpsTelemetry_hk.fields.latchup[5]);

	printf("Time left on I2C WDT = %d \r\n", myEpsTelemetry_hk.fields.wdt_i2c_time_left);
	printf("Time left on WDT GND = %d \r\n", myEpsTelemetry_hk.fields. wdt_gnd_time_left);
	printf("Time left on I2C WDT CSP ping 1 = %d \r\n", myEpsTelemetry_hk.fields.wdt_csp_pings_left[0]);
	printf("Time left on I2C WDT CSP ping 2 = %d \r\n", myEpsTelemetry_hk.fields.wdt_csp_pings_left[1]);

	printf("Number of I2C WD reboots = %d\r\n", myEpsTelemetry_hk.fields.counter_wdt_i2c);
	printf("Number of WDT GND reboots = %d\r\n", myEpsTelemetry_hk.fields.counter_wdt_gnd);
	printf("Number of WDT CSP ping 1 reboots = %d\r\n", myEpsTelemetry_hk.fields.counter_wdt_csp[0]);
	printf("Number of WDT CSP ping 2 reboots = %d\r\n", myEpsTelemetry_hk.fields.counter_wdt_csp[0]);
	printf("Number of EPS reboots = %d\r\n", myEpsTelemetry_hk.fields.counter_boot);

	printf("Temperature sensors. TEMP1 = %d \r\n", myEpsTelemetry_hk.fields.temp[0]);
	printf("Temperature sensors. TEMP2 = %d \r\n", myEpsTelemetry_hk.fields.temp[1]);
	printf("Temperature sensors. TEMP3 = %d \r\n", myEpsTelemetry_hk.fields.temp[2]);
	printf("Temperature sensors. TEMP4 = %d \r\n", myEpsTelemetry_hk.fields.temp[3]);
	printf("Temperature sensors. BATT0 = %d \r\n", myEpsTelemetry_hk.fields.temp[4]);
	printf("Temperature sensors. BATT1 = %d \r\n", myEpsTelemetry_hk.fields.temp[5]);

	printf("Cause of last EPS reset = %d\r\n", myEpsTelemetry_hk.fields.bootcause);
	printf("Battery Mode = %d\r\n", myEpsTelemetry_hk.fields.battmode);
	printf("PPT tracker Mode = %d\r\n", myEpsTelemetry_hk.fields.pptmode);
	printf(" \r\n");

	return TRUE;
}

/**
 * Get the status of the voltage on the EPS according to the voltage thresholds that we set in the PDR of Tavel2
 * */
static Boolean EPS_Status(void)
{
	return TRUE;
}

/*
 * prints all the parameters in the structure of gom_eps_hkparam_t.
 * */
static Boolean EPS_TelemetryHKParam(void)
{
	gom_eps_hkparam_t myEpsTelemetry_param;

	printf("\r\nEPS Telemetry HK Param \r\n\n");
	print_error(GomEpsGetHkData_param(0, &myEpsTelemetry_param));
	printf("Photovoltaic in Voltage 3 = %d mV \r\n", myEpsTelemetry_param.fields.photoVoltaic3);
	printf("Photovoltaic in Voltage 2 = %d mV \r\n", myEpsTelemetry_param.fields.photoVoltaic2);
	printf("Photovoltaic in Voltage 1 = %d mV \r\n", myEpsTelemetry_param.fields.photoVoltaic1);

	printf("Photocurrent = %d mA \r\n", myEpsTelemetry_param.fields.pc);
	printf("Battery Voltage = %d mV \r\n", myEpsTelemetry_param.fields.bv);
	printf("System current = %d mA \r\n", myEpsTelemetry_param.fields.sc);

	printf("Temperature Converter 1 = %d C \r\n", myEpsTelemetry_param.fields.tempConverter1);
	printf("Temperature Converter 2 = %d C \r\n", myEpsTelemetry_param.fields.tempConverter2);
	printf("Temperature Converter 3 = %d C \r\n", myEpsTelemetry_param.fields.tempConverter3);
	printf("Temperature Battery = %d C \r\n", myEpsTelemetry_param.fields.tempBattery);

	printf("Battery Temp = %d C \r\n", myEpsTelemetry_param.fields.batteryBoardTemp[0]);
	printf("Battery Temp = %d C \r\n", myEpsTelemetry_param.fields.batteryBoardTemp[1]);

	printf("Latchup at 3.3V3 = %d \r\n", myEpsTelemetry_param.fields.latchUp3v3Channel3);
	printf("Latchup at 3.3V2 = %d \r\n", myEpsTelemetry_param.fields.latchUp3v3Channel2);
	printf("Latchup at 3.3V1 = %d \r\n", myEpsTelemetry_param.fields.latchUp3v3Channel1);
	printf("Latchup at 5V3 = %d \r\n", myEpsTelemetry_param.fields.latchUp5vChannel3);
	printf("Latchup at 5V2 = %d \r\n", myEpsTelemetry_param.fields.latchUp5vChannel2);
	printf("Latchup at 5V1 = %d \r\n", myEpsTelemetry_param.fields.latchUp5vChannel1);

	printf("Cause of EPS Reset = %d \r\n", myEpsTelemetry_param.fields.reset);

	printf("Number of EPS Reboot = %d \r\n", myEpsTelemetry_param.fields.bootCount);
	printf("Number of Software Errors = %d \r\n", myEpsTelemetry_param.fields.swErrors);

	printf("PPT Mode = %d \r\n", myEpsTelemetry_param.fields.pptMode);

	printf("Status 5V Channel 1 = %d \r\n", myEpsTelemetry_param.fields.channelStatus.fields.channel5V_1);
	printf("Status 5V Channel 2 = %d \r\n", myEpsTelemetry_param.fields.channelStatus.fields.channel5V_2);
	printf("Status 5V Channel 3 = %d \r\n", myEpsTelemetry_param.fields.channelStatus.fields.channel5V_3);
	printf("Status 3.3V Channel 1 = %d \r\n", myEpsTelemetry_param.fields.channelStatus.fields.channel3V3_1);
	printf("Status 3.3V Channel 2 = %d \r\n", myEpsTelemetry_param.fields.channelStatus.fields.channel3V3_2);
	printf("Status 3.3V Channel 3 = %d \r\n", myEpsTelemetry_param.fields.channelStatus.fields.channel3V3_3);
	printf("Status QuadBatt switch = %d \r\n", myEpsTelemetry_param.fields.channelStatus.fields.quadbatSwitch);
	printf("Status QuadBatt heater = %d \r\n", myEpsTelemetry_param.fields.channelStatus.fields.quadbatHeater);

	printf(" \r\n");

	return TRUE;
}

/** Prints the config of the GomEPS
 * The config is in short of configuration and represents the gomEPS configuration, and is set by the struct eps_config_t which you can find at GomEPS.h
 */
static Boolean EPS_ConfigGet(void)
{
	eps_config_t config_data;
	print_error(GomEpsConfigGet(0, &config_data));

	printf("%u -config_commandReplay\r\n", config_data.fields.commandReply);

	printf("%u -config_ppt_mode\r\n",config_data.fields.ppt_mode);

	printf("%u -config_battheater_mode\r\n",config_data.fields.battheater_mode);

	printf("%u -config_battheater_low\r\n",config_data.fields.battheater_low);

	printf("%u -config_battheater_high\r\n",config_data.fields.battheater_high);


	int i;

	for(i = 0; i<=7; i++)
	{
		printf("%u",config_data.fields.output_normal_value[i]);
		printf("-config_output_normal_value[%d]\r\n", i);
	}

	for(i = 0; i<=7; i++)
	{
		printf("%u",config_data.fields.output_safe_value[i]);
		printf("-config_output_safe_value[%d]\r\n", i);
	}

	for(i = 0; i<=7; i++)
	{
		printf("%u",config_data.fields.output_initial_on_delay[i]);
		printf("-output_initial_on_delay[%d]\r\n", i);
	}

	for(i = 0; i<=7; i++)
	{
		printf("%u",config_data.fields.output_initial_off_delay[i]);
		printf("-output_initial_off_delay[%d]\r\n", i);
	}

	for(i = 0; i<=2; i++)
	{
		printf("%u",config_data.fields.vboost[i]);
		printf("-vboost[%d]\r\n", i);
	}

	return TRUE;
}

/**
 * Prints the config of the GomEPS. most of the configs are about the batts and voltage thresholds.
 * The config is in short of configuration and represents the gomEPS configuration, and is set by the struct eps_config2_t which you can find at GomEPS.h
 */
static Boolean EPS_Config2Get(void)
{
	eps_config2_t config_data;
	print_error(GomEpsConfig2Get(0, &config_data));

	printf("%u -config_commandReply\r\n", config_data.fields.commandReply);

	printf("%u -config_batt_max_voltage\r\n", config_data.fields.batt_maxvoltage);

	printf("%u -config_batt_safe_voltage\r\n", config_data.fields.batt_safevoltage);

	printf("%u -config_batt_critical_voltage\r\n", config_data.fields.batt_criticalvoltage);

	printf("%u -config_batt_normal_voltage\r\n", config_data.fields.batt_normalvoltage);


	return TRUE;
}

/**
 * Turns on a selected channel.
 */
static Boolean EPS_SetOutputOn(void)
{
    int selection;

    printf( "Select channel to enable: 0, 1, 2, 3, 4, 5. Type 6 to cancel \n\r");
    while(UTIL_DbguGetIntegerMinMax(&selection, 0, 6) == 0); //get selected channel to turn on.

    if(selection != 6)
    {
        print_error(GomEpsSetSingleOutput(0, selection, gomeps_channel_on, 0));
    }

    return TRUE;
}

/**
 * Turns off a selected channel.
 */
static Boolean EPS_SetOutputOff(void)
{
    int selection;

    printf( "Select channel to disable: 0, 1, 2, 3, 4, 5. Type 6 to cancel \n\r");
    while(UTIL_DbguGetIntegerMinMax(&selection, 0, 6) == 0); //get selected channel to turn off.

    if(selection != 6)
    {
        print_error(GomEpsSetSingleOutput(0, selection, gomeps_channel_off, 0));
    }

    return TRUE;
}

/**
 * Reboots the EPS through a chosen mode, can either soft reboot or hard reboot.
 */
static Boolean EPS_Reboot(void)
{
    int selection;

    printf( "Select eps reboot mode: 0 = soft, 1 = hard. Type 2 to cancel \n\r");
    while(UTIL_DbguGetIntegerMinMax(&selection, 0, 2) == 0);

    switch(selection)
    {
    case 0:
    	(void)GomEpsSoftReset(0);
    	break;
    case 1:
    	(void)GomEpsHardReset(0);
    	break;
    }

    return TRUE;
}

/**
 * Function turns on a stoppable loop that shows battery related telemetry.
 */
static Boolean EPS_TelemetryHKGeneral_BatteryLoop(void)
{
	int i = 0;
    printf("Shows battery related telemetry once per minute\n\r");
    printf("Press any key to stop\n\r");
    while(!DBGU_IsRxReady())
    {
    	gom_eps_hk_t data;
    	int rv;

    	if(i++ % 60 == 0)						// every 60 steps we'll do stuff
    	{
			rv = GomEpsGetHkData_general(0, &data);
			if(rv)
			{
				printf("ERROR: GomEpsGetHkData_general rv=%d! \n\r", rv);
			}
			else
			{
				#define GOMEPS_BAT_CHARGE_CONVERTER_IDX	2

				printf("boost[PV%d] - V: %dmV, C: %dmA, T: %ddegC | batt - V: %dmV, C[cursun]: %dmA, C[cursys]: %dmA, T: %ddegC, mode: %s \n\r",
						GOMEPS_BAT_CHARGE_CONVERTER_IDX+1, data.fields.vboost[GOMEPS_BAT_CHARGE_CONVERTER_IDX], data.fields.curin[GOMEPS_BAT_CHARGE_CONVERTER_IDX], data.fields.temp[GOMEPS_BAT_CHARGE_CONVERTER_IDX],
						data.fields.vbatt, data.fields.cursun, data.fields.cursys, data.fields.temp[3], data.fields.battmode==1?"undervolt!":data.fields.battmode==2?"overvolt!":"normal");
			}

			vTaskDelay(1000/portTICK_RATE_MS); // wait a second
    	}
    }

    // Empty queue
    DBGU_GetChar();

    return TRUE;
}

/**
 * The function sends a ping and prints it if the ping is valid
 */

static Boolean EPS_PingTest(void)
{
	// TODO check how long the ping takes

	unsigned char ping_byte_out;
	GomEpsPing(0, 'A',&ping_byte_out);
	printf("%c",ping_byte_out);




    return TRUE;
}

/**
 * Prints Heater Mode,bp4 Heater Mode,Onboard Heater Mode.
 * The bp4 heater is the heater built on the batteries
 * The Onboard Heater is the heater built on the Arduino Motherboard.
 */
static Boolean EPS_GetHeaterMode(void)
{

	gom_eps_heater_status_t my_heater_status_eps;
	print_error(GomEpsGetHeaterMode(0,&my_heater_status_eps));
	printf("bp4 - %d\r\n",my_heater_status_eps.fields.bp4_heatermode);
	printf("onboard - %d\r\n",my_heater_status_eps.fields.onboard_heatermode);

    return TRUE;
}

/**
 * Print the counters then reset it and print it again, to check if it really resets the counters.
 */
static Boolean Eps_ResetCounters(void)
{
	gom_eps_hk_wdt_t data_out;
	print_error(GomEpsGetHkData_wdt(0,&data_out));
	printf("%u -counter_wdt_i2c\r\n",data_out.fields.counter_wdt_i2c);
	printf("%u -counter_wdt_gnd\r\n",data_out.fields.counter_wdt_gnd);
	printf("%u -counter_wdt_csp[0]\r\n",data_out.fields.counter_wdt_csp[0]);
	printf("%u -counter_wdt_csp[1]\r\n",data_out.fields.counter_wdt_csp[1]);


	GomEpsResetCounters(0);
	printf("\r\n it reset \r\n");

	print_error(GomEpsGetHkData_wdt(0,&data_out));
	printf("%u -counter_wdt_i2c\r\n",data_out.fields.counter_wdt_i2c);
	printf("%u -counter_wdt_gnd\r\n",data_out.fields.counter_wdt_gnd);
	printf("%u -counter_wdt_csp[0]\r\n",data_out.fields.counter_wdt_csp[0]);
	printf("%u -counter_wdt_csp[1]\r\n",data_out.fields.counter_wdt_csp[1]);



	return TRUE;
}

/**
 * Print just the gnd then resets it and prints it again, to check if it actually resets the gnd.
 * */
static Boolean Eps_ResetWDT(void)
{
	gom_eps_hk_wdt_t data_out;
	print_error(GomEpsGetHkData_wdt(0,&data_out));
	printf("%u -wdt_gnd_time_left\r\n",data_out.fields.wdt_gnd_time_left);

	GomEpsResetWDT(0);
	printf("\r\n it resets \r\n");

	print_error(GomEpsGetHkData_wdt(0,&data_out));
	printf("%u -wdt_gnd_time_left\r\n",data_out.fields.wdt_gnd_time_left);

	return TRUE;
}
/**
Asks the user which test he wants or if he wants to exit the test loop.
 * all the functions returns TRUE while the exit is FALSE.
 * @return type= Boolean; offerMoreTest that get to an infinite loop and the loop ends if the function return FALSE.
*/
static Boolean selectAndExecuteGomEPSDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) EPS Telemetry HK Param \n\r");
	printf("\t 2) EPS Telemetry HK General \n\r");
	printf("\t 3) EPS Telemetry HK General - Battery Loop \n\r");
	printf("\t 4) EPS Telemetry HK VI  \n\r");
	printf("\t 5) EPS Telemetry HK OUT  \n\r");
	printf("\t 6) EPS Telemetry HK WDT \n\r");
	printf("\t 7) EPS Telemetry HK Basic \n\r");
	printf("\t 8) EPS Enable channel \n\r");
	printf("\t 9) EPS Disable channel \n\r");
	printf("\t 10) EPS Reboot \n\r");
	printf("\t 11) EPS status \n\r");
	printf("\t 12) EPS Ping \n\r");
	printf("\t 13) EPS Get Heater Mode \n\r");
	printf("\t 14) Reset the GOMSpace EPS counters \n\r");
	printf("\t 15) print to check if it actually resets the gnd \n\r");
	printf("\t 16) Get Config Parameters\n\r");
	printf("\t 17) Get Config2 Parameters\n\r");


	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 17) == 0);//you have to write a number between the two numbers include or else it ask you to enter a number between the two.

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = EPS_TelemetryHKParam();
		break;
	case 2:
		offerMoreTests = EPS_TelemetryHKGeneral();
		break;
	case 3:
		offerMoreTests = EPS_TelemetryHKGeneral_BatteryLoop();
		break;
	case 4:
		offerMoreTests = EPS_TelemetryHKvi();
		break;
	case 5:
		offerMoreTests = EPS_TelemetryHKout();
		break;
	case 6:
		offerMoreTests = EPS_TelemetryHKwdt();
		break;
	case 7:
		offerMoreTests = EPS_TelemetryHKbasic();
		break;
    case 8:
        offerMoreTests = EPS_SetOutputOn();
        break;
    case 9:
        offerMoreTests = EPS_SetOutputOff();
        break;
    case 10:
    	offerMoreTests = EPS_Reboot();
    	break;
    case 11:
    	offerMoreTests = EPS_Status();
    	break;
    case 12:
    	offerMoreTests = EPS_PingTest();
    	break;
    case 13:
    	offerMoreTests = EPS_GetHeaterMode();
    	break;
    case 14:
      	offerMoreTests = Eps_ResetCounters();
      	break;
    case 15:
    	offerMoreTests = Eps_ResetWDT();
    	break;
    case 16:
    	offerMoreTests = EPS_ConfigGet();
       	break;
    case 17:
    	offerMoreTests = EPS_Config2Get();
    	break;

	default:
		break;

	}

	return offerMoreTests;
}

Boolean GomEPSdemoInit(void)
{
    unsigned char i2c_address = 0x02;
    int rv;

	rv = GomEpsInitialize(&i2c_address, 1);
	if(rv != E_NO_SS_ERR && rv != E_IS_INITIALIZED)
	{
		// we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
		// hanging the code
		TRACE_ERROR("\n\r GomEpsInitialize() failed; err=%d! Exiting ... \n\r", rv);
		return FALSE;
	}

	return TRUE;
}

void GomEPSdemoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteGomEPSDemoTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
}

Boolean GomEPSdemoMain(void)
{
	if(GomEPSdemoInit())									// initialize of I2C and GomEPS subsystem drivers succeeded?
	{
		GomEPSdemoLoop();									// show the main GomEPS demo interface and wait for user input
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

Boolean GomEPStest(void)
{
	GomEPSdemoMain();
	return TRUE;
}

