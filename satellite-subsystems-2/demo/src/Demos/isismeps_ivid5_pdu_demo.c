/*
 * isismepsv2_ivid5_pdu_demo.c
 *
 *  Created on: oct. 2023
 *      Author: obar
 *
 * *Note* IVID5 demo only, for IVID7 only minor differences exist.
 * 			IVID5 interface should work with IVID7 hardware.
 */

#include <Demos/isismeps_ivid5_pdu_demo.h>
#include "input.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <at91/utility/trace.h>

#include <hal/Drivers/I2C.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Utility/util.h>
#include <hal/boolean.h>

#include <satellite-subsystems/isismepsv2_ivid5_pdu.h>
#include <satellite-subsystems/isismepsv2_ivid5_pcu.h>
#include <satellite-subsystems/isismepsv2_ivid5_pbu.h>

#define I2CADDR_IMEPS_DU	0x20
#define I2CADDR_IMEPS_BU	0x28
#define I2CADDR_IMEPS_CU	0x2C

#define WDT_KICK_PERIOD 60000 //Set for 60 seconds

ISISMEPSV2_IVID5_PDU_t pdu_subsystem[1]; // One instance to be initialised.
ISISMEPSV2_IVID5_PCU_t pcu_subsystem[1]; // One instance to be initialised.
ISISMEPSV2_IVID5_PBU_t pbu_subsystem[1]; // One instance to be initialised.

static uint8_t _index;
static xTaskHandle WDTTaskHandle = NULL;

static void _print_pdu_respone(isismepsv2_ivid5_pdu__replyheader_t* replyheader)
{
	printf("System Type Identifier: %u \n\r", replyheader->fields.stid);
	printf("Interface Version Identifier: %u \n\r", replyheader->fields.ivid);
	printf("Response Code: %u \n\r", replyheader->fields.rc);
	printf("Board Identifier: %u \n\r", replyheader->fields.bid);
	printf("Command error: %u \n\r", replyheader->fields.cmderr);
	printf("New flag: %u \n\r", replyheader->fields.new_flag);
	printf("\n\r");
}

static Boolean _reset( void )
{
	isismepsv2_ivid5_pdu__replyheader_t response;

	printf("\nReset command sent to PDU, no reply available.\n\n\r");

	int error = isismepsv2_ivid5_pdu__reset(_index, &response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pdu__reset(...) return error (%d)!\n\r",error);
		return FALSE;
	}
	
	return TRUE;
}

static Boolean _cancel( void )
{
    isismepsv2_ivid5_pdu__replyheader_t response;

	printf("\n\rNote: switches off any output bus channels that have been switched on after the system powered up up.\n\r");


	int error = isismepsv2_ivid5_pdu__cancel(_index,&response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pdu__cancel(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_pdu cancel operation response: \n\n\r");
	_print_pdu_respone(&response);
	
	return TRUE;
}

static Boolean _watchdog( void )
{
    isismepsv2_ivid5_pdu__replyheader_t response;

	printf("\n\rNote: resets the watchdog timer, keeping the system from performing a reset.\n\r");

	int error = isismepsv2_ivid5_pdu__resetwatchdog(_index,&response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pdu__watchdog(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps watchdog kick response: \n\n\r");
	_print_pdu_respone(&response);
	
	return TRUE;
}

static Boolean _outputbuschannelon( void )
{
	isismepsv2_ivid5_pdu__replyheader_t response;

	printf("\n\rNote: turn ON a single output bus channel using the bus channel index. Index 0 represents channel 0 (OBC0), index 1 represents channel 1 (OBC1), etc.\n\r");

	isismepsv2_ivid5_pdu__eps_channel_t obc_idx = INPUT_GetINT8("Single Output Bus Channel Index: ");

	int error = isismepsv2_ivid5_pdu__outputbuschannelon(_index, obc_idx, &response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pdu__outputbuschannelon(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps output bus channel on response: \n\n\r");
	_print_pdu_respone(&response);
	
	return TRUE;
}

static Boolean _outputbuschanneloff( void )
{
	isismepsv2_ivid5_pdu__replyheader_t response;

	printf("\n\rNote: turn OFF a single output bus channel using the bus channel index. Index 0 represents channel 0 (OBC0), index 1 represents channel 1 (OBC1), etc.\n\r");

	isismepsv2_ivid5_pdu__eps_channel_t obc_idx = INPUT_GetINT8("Single Output Bus Channel Index: ");

	int error = isismepsv2_ivid5_pdu__outputbuschanneloff(_index, obc_idx, &response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pdu__outputbuschanneloff(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps output bus channel off response: \n\n\r");
	_print_pdu_respone(&(response));
	
	return TRUE;
}

static Boolean _switchtonominal( void )
{
    isismepsv2_ivid5_pdu__replyheader_t response;

	printf("\n\rNote: move system to nominal mode. This provides full control of all output buses. The system automatically enters nominal mode after startup mode or when the PDU system is in safety mode or emergency low power mode and the PDU rail voltage exceeds their respective high threshold set in the configuration parameter system.\n\r");

	int error = isismepsv2_ivid5_pdu__switchtonominal(_index,&response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pdu__switchtonominal(...) return error (%d)!\n\r",error);
		return FALSE;
	}
		
	printf("\nisis_eps switch to nominal response: \n\n\r");
	_print_pdu_respone(&response);
	
	return TRUE;
}

static Boolean _switchtosafety( void )
{
    isismepsv2_ivid5_pdu__replyheader_t response;

	int error = isismepsv2_ivid5_pdu__switchtosafety(_index,&response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pdu__switchtosafety(...) return error (%d)!\n\r",error);
		return FALSE;
	}
		
	printf("\nisis_eps switch to safety response: \n\n\r");
	_print_pdu_respone(&response);
	
	return TRUE;
}

static Boolean _getsystemstatus( void )
{
	const char* mode_string[] = {"STARTUP", "NOMINAL", "SAFETY", "EMLOPO", "UNKNOWN"};
	const char* reset_string[] = {"POWER-ON", "WATCHDOG", "COMMAND", "CONTROL", "EMLOPO", "UNKNOWN"};

	uint8_t string_index;

	isismepsv2_ivid5_pdu__getsystemstatus__from_t response;

	printf("\n\rNote: returns system status information\n\r");

	int error = isismepsv2_ivid5_pdu__getsystemstatus(_index,&response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pdu__getsystemstatus(...) return error (%d)!\n\r",error);
		return FALSE;
	}
		
	printf("\nisis_eps get system status response: \n\n\r");
	_print_pdu_respone(&(response.fields.reply_header));

	string_index = response.fields.mode;
	if(response.fields.mode > 4)
	{
		string_index = 4;
	}
	printf("Current mode: %u [%s]\n\r", response.fields.mode, mode_string[string_index]);

	printf("Configuration status: %u \n\r", response.fields.conf);

	string_index = response.fields.reset_cause;
	if(response.fields.reset_cause > 5)
	{
		string_index = 5;
	}
	printf("Reset cause: %u [%s]\n\r", response.fields.reset_cause, reset_string[string_index]);
	printf("Uptime since last reset: %lu seconds\n\r", response.fields.uptime);
	printf("Error : %u \n\r", response.fields.error);
	printf("Number of Power-On reset occurrences: %u \n\r", response.fields.rc_cnt_pwron);
	printf("Number of Watchdog Timer reset occurrences: %u \n\r", response.fields.rc_cnt_wdg);
	printf("Number of Commanded reset occurrences: %u \n\r", response.fields.rc_cnt_cmd);
	printf("Number of EPS Controller reset occurrences: %u \n\r", response.fields.rc_cnt_pweron_mcu);
	printf("Number of EMLOPO Mode reset occurrences: %u \n\r", response.fields.rc_cnt_emlopo);
	printf("Time elapsed since previous command: %u \n\r", response.fields.prevcmd_elapsed);
	printf("\n\r");

	return TRUE;
}

static Boolean _gethousekeepingraw_pdu( void )
{
	isismepsv2_ivid5_pdu__gethousekeepingdataraw__from_t response;

	int error = isismepsv2_ivid5_pdu__gethousekeepingdataraw(_index,&response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pdu__gethousekeepingraw(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps get housekeeping data raw response: \n\n\r");
	_print_pdu_respone(&(response.fields.reply_header));

	printf("(UINT8) reserved : %u \n\r", response.fields.reserved);
	printf("(INT16) volt_brdsup : %d \n\r", response.fields.volt_brdsup);
	printf("(INT16) temp : %d \n\r", response.fields.temp);
	printf("(INT16) vip_input.fields.volt : %d \n\r", response.fields.vip_input.fields.volt);
	printf("(INT16) vip_input.fields.current : %d \n\r", response.fields.vip_input.fields.current);
	printf("(INT16) vip_input.fields.power : %d \n\r", response.fields.vip_input.fields.power);
	printf("(UINT16) stat_ob_on : %u \n\r", response.fields.stat_ob_on);
	printf("(UINT16) stat_ob_ocf : %u \n\r", response.fields.stat_ob_ocf);
	printf("(INT16) vip_obc00.fields.volt : %d \n\r", response.fields.vip_obc00.fields.volt);
	printf("(INT16) vip_obc00.fields.current : %d \n\r", response.fields.vip_obc00.fields.current);
	printf("(INT16) vip_obc00.fields.power : %d \n\r", response.fields.vip_obc00.fields.power);
	printf("(INT16) vip_obc01.fields.volt : %d \n\r", response.fields.vip_obc01.fields.volt);
	printf("(INT16) vip_obc01.fields.current : %d \n\r", response.fields.vip_obc01.fields.current);
	printf("(INT16) vip_obc01.fields.power : %d \n\r", response.fields.vip_obc01.fields.power);
	printf("(INT16) vip_obc02.fields.volt : %d \n\r", response.fields.vip_obc02.fields.volt);
	printf("(INT16) vip_obc02.fields.current : %d \n\r", response.fields.vip_obc02.fields.current);
	printf("(INT16) vip_obc02.fields.power : %d \n\r", response.fields.vip_obc02.fields.power);
	printf("(INT16) vip_obc03.fields.volt : %d \n\r", response.fields.vip_obc03.fields.volt);
	printf("(INT16) vip_obc03.fields.current : %d \n\r", response.fields.vip_obc03.fields.current);
	printf("(INT16) vip_obc03.fields.power : %d \n\r", response.fields.vip_obc03.fields.power);
	printf("(INT16) vip_obc04.fields.volt : %d \n\r", response.fields.vip_obc04.fields.volt);
	printf("(INT16) vip_obc04.fields.current : %d \n\r", response.fields.vip_obc04.fields.current);
	printf("(INT16) vip_obc04.fields.power : %d \n\r", response.fields.vip_obc04.fields.power);
	printf("(INT16) vip_obc05.fields.volt : %d \n\r", response.fields.vip_obc05.fields.volt);
	printf("(INT16) vip_obc05.fields.current : %d \n\r", response.fields.vip_obc05.fields.current);
	printf("(INT16) vip_obc05.fields.power : %d \n\r", response.fields.vip_obc05.fields.power);
	printf("(INT16) vip_obc06.fields.volt : %d \n\r", response.fields.vip_obc06.fields.volt);
	printf("(INT16) vip_obc06.fields.current : %d \n\r", response.fields.vip_obc06.fields.current);
	printf("(INT16) vip_obc06.fields.power : %d \n\r", response.fields.vip_obc06.fields.power);
	printf("(INT16) vip_obc07.fields.volt : %d \n\r", response.fields.vip_obc07.fields.volt);
	printf("(INT16) vip_obc07.fields.current : %d \n\r", response.fields.vip_obc07.fields.current);
	printf("(INT16) vip_obc07.fields.power : %d \n\r", response.fields.vip_obc07.fields.power);
	printf("(INT16) vip_obc08.fields.volt : %d \n\r", response.fields.vip_obc08.fields.volt);
	printf("(INT16) vip_obc08.fields.current : %d \n\r", response.fields.vip_obc08.fields.current);
	printf("(INT16) vip_obc08.fields.power : %d \n\r", response.fields.vip_obc08.fields.power);
	printf("(INT16) vip_obc09.fields.volt : %d \n\r", response.fields.vip_obc09.fields.volt);
	printf("(INT16) vip_obc09.fields.current : %d \n\r", response.fields.vip_obc09.fields.current);
	printf("(INT16) vip_obc09.fields.power : %d \n\r", response.fields.vip_obc09.fields.power);
	printf("(INT16) vip_obc10.fields.volt : %d \n\r", response.fields.vip_obc10.fields.volt);
	printf("(INT16) vip_obc10.fields.current : %d \n\r", response.fields.vip_obc10.fields.current);
	printf("(INT16) vip_obc10.fields.power : %d \n\r", response.fields.vip_obc10.fields.power);
	printf("(INT16) vip_obc11.fields.volt : %d \n\r", response.fields.vip_obc11.fields.volt);
	printf("(INT16) vip_obc11.fields.current : %d \n\r", response.fields.vip_obc11.fields.current);
	printf("(INT16) vip_obc11.fields.power : %d \n\r", response.fields.vip_obc11.fields.power);
	printf("(INT16) vip_obc12.fields.volt : %d \n\r", response.fields.vip_obc12.fields.volt);
	printf("(INT16) vip_obc12.fields.current : %d \n\r", response.fields.vip_obc12.fields.current);
	printf("(INT16) vip_obc12.fields.power : %d \n\r", response.fields.vip_obc12.fields.power);
	printf("(INT16) vip_obc13.fields.volt : %d \n\r", response.fields.vip_obc13.fields.volt);
	printf("(INT16) vip_obc13.fields.current : %d \n\r", response.fields.vip_obc13.fields.current);
	printf("(INT16) vip_obc13.fields.power : %d \n\r", response.fields.vip_obc13.fields.power);
	printf("(INT16) vip_obc14.fields.volt : %d \n\r", response.fields.vip_obc14.fields.volt);
	printf("(INT16) vip_obc14.fields.current : %d \n\r", response.fields.vip_obc14.fields.current);
	printf("(INT16) vip_obc14.fields.power : %d \n\r", response.fields.vip_obc14.fields.power);
	printf("(INT16) vip_obc15.fields.volt : %d \n\r", response.fields.vip_obc15.fields.volt);
	printf("(INT16) vip_obc15.fields.current : %d \n\r", response.fields.vip_obc15.fields.current);
	printf("(INT16) vip_obc15.fields.power : %d \n\r", response.fields.vip_obc15.fields.power);

	printf("\n\r");
	
	return TRUE;
}

static Boolean _gethousekeepingeng_pdu( void )
{
	isismepsv2_ivid5_pdu__gethousekeepingdataeng__from_t response;

	int error = isismepsv2_ivid5_pdu__gethousekeepingdataeng(_index, &response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pdu__gethousekeepingeng(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps get housekeeping data engineering response: \n\n\r");
	_print_pdu_respone(&(response.fields.reply_header));

	printf("Internal EPS board voltage: %u mV\n\r", response.fields.volt_brdsup);
	printf("MCU temperature: %.2f deg. C\n\r", ((double)response.fields.temp) * 0.01);
	printf("EPS input voltage: %d mV \n\r", response.fields.vip_input.fields.volt);
	printf("EPS input current: %d mA \n\r", response.fields.vip_input.fields.current);
	printf("EPS power consumption: %d mW \n\r", response.fields.vip_input.fields.power * 10);

	printf("\n\rOutput bus channel status: 0x%04x \n\r", response.fields.stat_ob_on);
	printf("Output bus channel over-current status: 0x%04x \n\r", response.fields.stat_ob_ocf);

	printf("\n\rAll output channels:\n\r");
	printf("Channel\t\tVoltage [mV]\tCurrent [mA]\tPower [mW]\n\r");

	printf("[VD0] V_BAT\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc00.fields.volt, response.fields.vip_obc00.fields.current, response.fields.vip_obc00.fields.power * 10);
	printf("[VD1] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc01.fields.volt, response.fields.vip_obc01.fields.current, response.fields.vip_obc01.fields.power * 10);
	printf("[VD2] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc02.fields.volt, response.fields.vip_obc02.fields.current, response.fields.vip_obc02.fields.power * 10);
	printf("[VD3] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc03.fields.volt, response.fields.vip_obc03.fields.current, response.fields.vip_obc03.fields.power * 10);
	printf("[VD4] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc04.fields.volt, response.fields.vip_obc04.fields.current, response.fields.vip_obc04.fields.power * 10);
	printf("[VD5] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc05.fields.volt, response.fields.vip_obc05.fields.current, response.fields.vip_obc05.fields.power * 10);
	printf("[VD6] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc06.fields.volt, response.fields.vip_obc06.fields.current, response.fields.vip_obc06.fields.power * 10);
	printf("[VD7] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc07.fields.volt, response.fields.vip_obc07.fields.current, response.fields.vip_obc07.fields.power * 10);
	printf("[VD8] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc08.fields.volt, response.fields.vip_obc08.fields.current, response.fields.vip_obc08.fields.power * 10);
	printf("[VD9] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc09.fields.volt, response.fields.vip_obc09.fields.current, response.fields.vip_obc09.fields.power * 10);
	printf("[VD10] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc10.fields.volt, response.fields.vip_obc10.fields.current, response.fields.vip_obc10.fields.power * 10);
	printf("[VD11] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc11.fields.volt, response.fields.vip_obc11.fields.current, response.fields.vip_obc11.fields.power * 10);
	printf("[VD12] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc12.fields.volt, response.fields.vip_obc12.fields.current, response.fields.vip_obc12.fields.power * 10);
	printf("[VD13] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc13.fields.volt, response.fields.vip_obc13.fields.current, response.fields.vip_obc13.fields.power * 10);
	printf("[VD14] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc14.fields.volt, response.fields.vip_obc14.fields.current, response.fields.vip_obc14.fields.power * 10);
	printf("[VD15] \t\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc15.fields.volt, response.fields.vip_obc15.fields.current, response.fields.vip_obc15.fields.power * 10);

	printf("\n\rMaximum Power Point Trackers\n\r");
	printf("Channel\tVoltage In [mV]\tCurrent In [mA]\tVoltage Out [mV]\tCurrent Out [mA]\n\r");

	printf("\n\r");

	return TRUE;
}

static Boolean _gethousekeepingeng_pbu( void )
{
	isismepsv2_ivid5_pbu__gethousekeepingdataeng__from_t response;

	int error = isismepsv2_ivid5_pbu__gethousekeepingdataeng(_index, &response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pbu__gethousekeepingeng(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_pbu get housekeeping data engineering response: \n\n\r");
	isismepsv2_ivid5_pdu__replyheader_t* convert_to_pdu;
	convert_to_pdu = (isismepsv2_ivid5_pdu__replyheader_t*)&response.fields.reply_header;
	_print_pdu_respone(convert_to_pdu); //Note that all headers from PDU, PBU and PCU share a structure nut not name.

	printf("Internal EPS board voltage: %u mV\n\r", response.fields.volt_brdsup);
	printf("MCU temperature: %.2f deg. C\n\r", ((double)response.fields.temp) * 0.01);
	printf("EPS input voltage: %d mV \n\r", response.fields.vip_input.fields.volt);
	printf("EPS input current: %d mA \n\r", response.fields.vip_input.fields.current);
	printf("EPS power consumption: %d mW \n\r", response.fields.vip_input.fields.power * 10);

	printf("BU bit flags: %u\r\n", (unsigned short)*response.fields.stat_bu.raw);

	printf("BP1 voltage: %d mW\n\r", response.fields.bp1.fields.vip_bp_input.fields.volt);
	printf("BP1 current: %d mA\n\r", response.fields.bp1.fields.vip_bp_input.fields.current);
	printf("BP1 power: %d mW\n\r", response.fields.bp1.fields.vip_bp_input.fields.power * 10);

	printf("BP1 stat_bp: %u\r\n", response.fields.bp1.fields.stat_bp);
	printf("BP1 volt_cell1: %u\r\n", response.fields.bp1.fields.volt_cell1);
	printf("BP1 volt_cell2: %u\r\n", response.fields.bp1.fields.volt_cell2);
	printf("BP1 volt_cell3: %u\r\n", response.fields.bp1.fields.volt_cell3);
	printf("BP1 volt_cell4: %u\r\n", response.fields.bp1.fields.volt_cell4);
	printf("BP1 bat_temp1: %.2f\r\n", response.fields.bp1.fields.bat_temp1 * 0.01);
	printf("BP1 bat_temp2: %.2f\r\n", response.fields.bp1.fields.bat_temp2 * 0.01);
	printf("BP1 bat_temp3: %.2f\r\n", response.fields.bp1.fields.bat_temp3 * 0.01);

	printf("BP2 voltage: %d mW\n\r", response.fields.bp2.fields.vip_bp_input.fields.volt);
	printf("BP2 current: %d mA\n\r", response.fields.bp2.fields.vip_bp_input.fields.current);
	printf("BP2 power: %d mW\n\r", response.fields.bp2.fields.vip_bp_input.fields.power * 10);

	printf("BP2 stat_bp: %u\r\n", response.fields.bp2.fields.stat_bp);
	printf("BP2 volt_cell1: %u\r\n", response.fields.bp2.fields.volt_cell1);
	printf("BP2 volt_cell2: %u\r\n", response.fields.bp2.fields.volt_cell2);
	printf("BP2 volt_cell3: %u\r\n", response.fields.bp2.fields.volt_cell3);
	printf("BP2 volt_cell4: %u\r\n", response.fields.bp2.fields.volt_cell4);
	printf("BP2 bat_temp1: %.2f\r\n", response.fields.bp2.fields.bat_temp1 * 0.01);
	printf("BP2 bat_temp2: %.2f\r\n", response.fields.bp2.fields.bat_temp2 * 0.01);
	printf("BP2 bat_temp3: %.2f\r\n", response.fields.bp2.fields.bat_temp3 * 0.01);

	printf("BP3 voltage: %d mW\n\r", response.fields.bp3.fields.vip_bp_input.fields.volt);
	printf("BP3 current: %d mA\n\r", response.fields.bp3.fields.vip_bp_input.fields.current);
	printf("BP3 power: %d mW\n\r", response.fields.bp3.fields.vip_bp_input.fields.power * 10);

	printf("BP3 stat_bp: %u\r\n", response.fields.bp3.fields.stat_bp);
	printf("BP3 volt_cell1: %u\r\n", response.fields.bp3.fields.volt_cell1);
	printf("BP3 volt_cell2: %u\r\n", response.fields.bp3.fields.volt_cell2);
	printf("BP3 volt_cell3: %u\r\n", response.fields.bp3.fields.volt_cell3);
	printf("BP3 volt_cell4: %u\r\n", response.fields.bp3.fields.volt_cell4);
	printf("BP3 bat_temp1: %.2f\r\n", response.fields.bp3.fields.bat_temp1 * 0.01);
	printf("BP3 bat_temp2: %.2f\r\n", response.fields.bp3.fields.bat_temp2 * 0.01);
	printf("BP3 bat_temp3: %.2f\r\n", response.fields.bp3.fields.bat_temp3 * 0.01);

	printf("\n\r");

	return TRUE;
}

static Boolean _gethousekeepingeng_pcu( void )
{
	isismepsv2_ivid5_pcu__gethousekeepingdataeng__from_t response;

	int error = isismepsv2_ivid5_pcu__gethousekeepingdataeng(_index, &response);
	if( error )
	{
		TRACE_ERROR("isismepsv2_ivid5_pcu__gethousekeepingeng(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_pcu get housekeeping data engineering response: \n\n\r");
	isismepsv2_ivid5_pdu__replyheader_t* convert_to_pdu;
	convert_to_pdu = (isismepsv2_ivid5_pdu__replyheader_t*)&response.fields.reply_header;
	_print_pdu_respone(convert_to_pdu);

	printf("Internal EPS board voltage: %u mV\n\r", response.fields.volt_brdsup);
	printf("MCU temperature: %.2f deg. C\n\r", ((double)response.fields.temp) * 0.01);
	printf("vip_output voltage: %d mV \n\r", response.fields.vip_output.fields.volt);
	printf("vip_output current: %d mA \n\r", response.fields.vip_output.fields.current);
	printf("vip_output consumption: %d mW \n\r", response.fields.vip_output.fields.power * 10);

	printf("\n\rMaximum Power Point Trackers\n\r");
	printf("Channel\tVoltage In [mV]\tCurrent In [mA]\tVoltage Out [mV]\tCurrent Out [mA]\n\r");

	printf("1 \t%d\t\t%d\t\t%d\t\t\t%d\n\r", response.fields.cc1.fields.volt_in_mppt, response.fields.cc1.fields.curr_in_mppt,
			response.fields.cc1.fields.volt_out_mppt, response.fields.cc1.fields.curr_out_mppt);
	printf("2 \t%d\t\t%d\t\t%d\t\t\t%d\n\r", response.fields.cc2.fields.volt_in_mppt, response.fields.cc2.fields.curr_in_mppt,
			response.fields.cc2.fields.volt_out_mppt, response.fields.cc2.fields.curr_out_mppt);
	printf("3 \t%d\t\t%d\t\t%d\t\t\t%d\n\r", response.fields.cc3.fields.volt_in_mppt, response.fields.cc3.fields.curr_in_mppt,
			response.fields.cc3.fields.volt_out_mppt, response.fields.cc3.fields.curr_out_mppt);
	printf("4 \t%d\t\t%d\t\t%d\t\t\t%d\n\r", response.fields.cc4.fields.volt_in_mppt, response.fields.cc4.fields.curr_in_mppt,
			response.fields.cc4.fields.volt_out_mppt, response.fields.cc4.fields.curr_out_mppt);

	printf("\n\r");

	return TRUE;
}

void WDT_kick_task(void* arguments)
{
	isismepsv2_ivid5_pdu__replyheader_t response;

	(void)arguments;

	while(1)
	{
		isismepsv2_ivid5_pdu__resetwatchdog(_index, &response);
		vTaskDelay( WDT_KICK_PERIOD );
	}
}


static Boolean _start_WDT_kick(void)
{
	if(WDTTaskHandle == NULL)
	{
		xTaskCreate(WDT_kick_task, (const signed char*)"Task_WDT", 1024, NULL, configMAX_PRIORITIES-2, &WDTTaskHandle);
	}
	else
	{
		TRACE_ERROR("WDT Task already started");
	}

	return TRUE;
}

static Boolean selectAndExecuteIsis_EpsDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 1 - Software Reset \n\r");
	printf("\t 2 - Cancel Operation \n\r");
	printf("\t 3 - Watchdog Kick \n\r");
	printf("\t 4 - Output Bus Channel On \n\r");
	printf("\t 5 - Output Bus Channel Off \n\r");
	printf("\t 6 - Switch To Nominal \n\r");
	printf("\t 7 - Switch To Safety \n\r");
	printf("\t 8 - Get System Status \n\r");
	printf("\t 9 - Get Housekeeping Data - Raw pdu \n\r");
	printf("\t 10 - Get Housekeeping Data - Engineering pdu \n\r");
	printf("\t 11 - Get Housekeeping Data - Engineering pcu \n\r");
	printf("\t 12 - Get Housekeeping Data - Engineering pbu \n\r");
	printf("\t 13 - Start a watchdog kick task. This cannot be stopped without reboot. \n");
	printf("\t 14 - Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 14) == 0);

	switch(selection)
	{
		case 1:
			offerMoreTests = _reset();
			break;
		case 2:
			offerMoreTests = _cancel();
			break;
		case 3:
			offerMoreTests = _watchdog();
			break;
		case 4:
			offerMoreTests = _outputbuschannelon();
			break;
		case 5:
			offerMoreTests = _outputbuschanneloff();
			break;
		case 6:
			offerMoreTests = _switchtonominal();
			break;
		case 7:
			offerMoreTests = _switchtosafety();
			break;
		case 8:
			offerMoreTests = _getsystemstatus();
			break;
		case 9:
			offerMoreTests = _gethousekeepingraw_pdu();
			break;
		case 10:
			offerMoreTests = _gethousekeepingeng_pdu();
			break;
		case 11:
			offerMoreTests = _gethousekeepingeng_pcu();
			break;
		case 12:
			offerMoreTests = _gethousekeepingeng_pbu();
			break;
		case 13:
			offerMoreTests = _start_WDT_kick();
			break;
		case 14:
			offerMoreTests = FALSE;
			break;
		default:
			break;
	}

	return offerMoreTests;
}

Boolean isismepsv2_ivid5_pdu__demo__init(void)
{
    int retValInt = 0;

	//Initialize the I2C
	printf("\nI2C Initialize\n\r");
	retValInt = I2C_start(200000, 10);

	pdu_subsystem[0].i2cAddr = I2CADDR_IMEPS_DU;
	pcu_subsystem[0].i2cAddr = I2CADDR_IMEPS_CU;
	pbu_subsystem[0].i2cAddr = I2CADDR_IMEPS_BU;

	if(retValInt != 0)
	{
		TRACE_FATAL("\n\rI2Ctest: I2C_start_Master for ISIS_EPS test: %d! \n\r", retValInt);
	}
		
	retValInt = ISISMEPSV2_IVID5_PDU_Init( pdu_subsystem, 1);
	retValInt |= ISISMEPSV2_IVID5_PCU_Init( pcu_subsystem, 1);
	retValInt |= ISISMEPSV2_IVID5_PBU_Init( pbu_subsystem, 1);

	if(retValInt == driver_error_reinit)
	{
		printf("\nISIS_EPS a subsystem has already been initialised.\n\r");
	}
	else if(retValInt != driver_error_none )
	{
		printf("\nisismepsv2_ivid5_pdu_Init(...) returned error %d! \n\r", retValInt);
		return FALSE;
	}

	return TRUE;
}

void isismepsv2_ivid5_pdu__demo__loop(void)
{
	Boolean offerMoreTests = FALSE;

	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);

	while(1)
	{
		offerMoreTests = selectAndExecuteIsis_EpsDemoTest(); // show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)							// was exit/back selected?
		{
			break;
		}
	}
}

Boolean isismepsv2_ivid5_pdu__demo__main(void)
{
	if(!isismepsv2_ivid5_pdu__demo__init())
	{
		return FALSE;
	}

	isismepsv2_ivid5_pdu__demo__loop();

	return TRUE;
}

Boolean isismepsv2_ivid5_pdu__test(void)
{
	return isismepsv2_ivid5_pdu__demo__main();
}

