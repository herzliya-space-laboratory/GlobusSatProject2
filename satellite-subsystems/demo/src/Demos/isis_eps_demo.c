/*
 * isis_eps_demo.c
 *
 *  Created on: 4 may. 2013
 *      Author: pbot
 */

#include "isis_eps_demo.h"
#include "input.h"
#include "common.h"

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <hal/Drivers/I2C.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Utility/util.h>
#include <hal/boolean.h>

#include <satellite-subsystems/imepsv2_piu.h>

static uint8_t _index;

static void _print_eps_response(imepsv2_piu__replyheader_t* replyheader)
{
	printf("System Type Identifier: %u \n\r", replyheader->fields.stid);
	printf("Interface Version Identifier: %u \n\r", replyheader->fields.ivid);
	printf("Response Code: %u \n\r", replyheader->fields.rc);
	printf("Board Identifier: %u \n\r", replyheader->fields.bid);
	printf("Command error: %u \n\r", replyheader->fields.cmderr);
	printf("\n\r");
}

static Boolean _reset__tmtc( void )
{
	imepsv2_piu__replyheader_t replyheader;

	printf("\nReset command sent to EPS, no reply available.\n\n\r");

	int error = imepsv2_piu__reset(0, &replyheader);
	if( error )
	{
		TRACE_ERROR("imepsv2_piu__reset(...) return error (%d)!\n\r",error);
		return FALSE;
	}
	
	return TRUE;
}

static Boolean _cancel__tm( void )
{
	imepsv2_piu__replyheader_t response;

	printf("\n\rNote: switches off any output bus channels that have been switched on after the system powered up up.\n\r");


	int error = imepsv2_piu__cancel(_index,&response);
	if( error )
	{
		TRACE_ERROR("imepsv2_piu__cancel(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps cancel operation response: \n\n\r");
	_print_eps_response(&response);
	
	return TRUE;
}

static Boolean _watchdog__tm( void )
{
	imepsv2_piu__replyheader_t response;

	printf("\n\rNote: resets the watchdog timer, keeping the system from performing a reset.\n\r");

	int error = imepsv2_piu__resetwatchdog(_index,&response);
	if( error )
	{
		TRACE_ERROR("imepsv2_piu__resetwatchdog(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps watchdog kick response: \n\n\r");
	_print_eps_response(&response);
	
	return TRUE;
}

static Boolean _outputbuschannelon__tmtc( void )
{
	imepsv2_piu__imeps_channel_t params;
	imepsv2_piu__replyheader_t response;

	printf("\n\rNote: turn ON a single output bus channel using the bus channel index. The iMTQ is index 2 and the LED board is index 9.\n\r");

	params = INPUT_GetINT8("Single Output Bus Channel Index: ");

	int error = imepsv2_piu__outputbuschannelon(_index,params,&response);
	if( error )
	{
		TRACE_ERROR("imepsv2_piu__outputbuschannelon(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps output bus channel on response: \n\n\r");
	_print_eps_response(&response);
	
	return TRUE;
}

static Boolean _outputbuschanneloff__tmtc( void )
{
	imepsv2_piu__imeps_channel_t params;
	imepsv2_piu__replyheader_t response;

	printf("\n\rNote: turn OFF a single output bus channel using the bus channel index. The iMTQ is index 2 and the LED board is index 9.\n\r");

	params = INPUT_GetINT8("Single Output Bus Channel Index: ");

	int error = imepsv2_piu__outputbuschanneloff(_index,params,&response);
	if( error )
	{
		TRACE_ERROR("imepsv2_piu__outputbuschanneloff(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps output bus channel off response: \n\n\r");
	_print_eps_response(&response);
	
	return TRUE;
}

static Boolean _switchtonominal__tm( void )
{
	imepsv2_piu__replyheader_t response;

	printf("\n\rNote: move system to nominal mode. This provides full control of all output buses. The system automatically enters nominal mode after startup mode or when the PDU system is in safety mode or emergency low power mode and the PDU rail voltage exceeds their respective high threshold set in the configuration parameter system.\n\r");

	int error = imepsv2_piu__switchtonominal(_index,&response);
	if( error )
	{
		TRACE_ERROR("imepsv2_piu__switchtonominal(...) return error (%d)!\n\r",error);
		return FALSE;
	}
		
	printf("\nisis_eps switch to nominal response: \n\n\r");
	_print_eps_response(&response);
	
	return TRUE;
}

static Boolean _switchtosafety__tm( void )
{
	imepsv2_piu__replyheader_t response;

	int error = imepsv2_piu__switchtosafety(_index,&response);
	if( error )
	{
		TRACE_ERROR("imepsv2_piu__switchtosafety(...) return error (%d)!\n\r",error);
		return FALSE;
	}
		
	printf("\nisis_eps switch to safety response: \n\n\r");
	_print_eps_response(&response);
	
	return TRUE;
}

static Boolean _getsystemstatus__tm( void )
{
	const char* mode_string[] = {"STARTUP", "NOMINAL", "SAFETY", "EMLOPO", "UNKNOWN"};
	const char* reset_string[] = {"POWER-ON", "WATCHDOG", "COMMAND", "CONTROL", "EMLOPO", "UNKNOWN"};

	uint8_t string_index;

	imepsv2_piu__getsystemstatus__from_t response;

	printf("\n\rNote: returns system status information\n\r");

	int error = imepsv2_piu__getsystemstatus(_index,&response);
	if( error )
	{
		TRACE_ERROR("imepsv2_piu__getsystemstatus(...) return error (%d)!\n\r",error);
		return FALSE;
	}
		
	printf("\nisis_eps get system status response: \n\n\r");
	_print_eps_response(&(response.fields.reply_header));

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
/*
 * prints all the parameters in the structure of imepsv2_piu__gethousekeepingraw__from_t.
 * */
static Boolean _gethousekeepingraw__tm( void )
{
	imepsv2_piu__gethousekeepingraw__from_t response;

	int error = imepsv2_piu__gethousekeepingraw(_index,&response);
	if( error )
	{
		TRACE_ERROR("imepsv2_piu__gethousekeepingraw(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps get housekeeping data raw response: \n\n\r");
	_print_eps_response(&(response.fields.reply_header));

	printf("(UINT8) reserved : %u \n\r", response.fields.reserved);
	printf("(INT16) volt_brdsup : %d \n\r", response.fields.volt_brdsup);
	printf("(INT16) temp : %d \n\r", response.fields.temp);
	printf("(INT16) dist_input.fields.volt : %d \n\r", response.fields.dist_input.fields.volt);
	printf("(INT16) dist_input.fields.current : %d \n\r", response.fields.dist_input.fields.current);
	printf("(INT16) dist_input.fields.power : %d \n\r", response.fields.dist_input.fields.power);
	printf("(INT16) batt_input.fields.volt : %d \n\r", response.fields.batt_input.fields.volt);
	printf("(INT16) batt_input.fields.current : %d \n\r", response.fields.batt_input.fields.current);
	printf("(INT16) batt_input.fields.power : %d \n\r", response.fields.batt_input.fields.power);
	printf("(UINT16) stat_obc_on : %u \n\r", response.fields.stat_obc_on);
	printf("(UINT16) stat_obc_ocf : %u \n\r", response.fields.stat_obc_ocf);
	printf("(UINT16) bat_stat : %u \n\r", response.fields.bat_stat);
	printf("(INT16) temp2 : %d \n\r", response.fields.temp2);
	printf("(INT16) temp3 : %d \n\r", response.fields.temp3);
	printf("(INT16) volt_vd0 : %d \n\r", response.fields.volt_vd0);
	printf("(INT16) volt_vd1 : %d \n\r", response.fields.volt_vd1);
	printf("(INT16) volt_vd2 : %d \n\r", response.fields.volt_vd2);
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
	printf("(INT16) cc1.fields.volt_in_mppt : %d \n\r", response.fields.cc1.fields.volt_in_mppt);
	printf("(INT16) cc1.fields.curr_in_mppt : %d \n\r", response.fields.cc1.fields.curr_in_mppt);
	printf("(INT16) cc1.fields.volt_out_mppt : %d \n\r", response.fields.cc1.fields.volt_out_mppt);
	printf("(INT16) cc1.fields.curr_out_mppt : %d \n\r", response.fields.cc1.fields.curr_out_mppt);
	printf("(INT16) cc2.fields.volt_in_mppt : %d \n\r", response.fields.cc2.fields.volt_in_mppt);
	printf("(INT16) cc2.fields.curr_in_mppt : %d \n\r", response.fields.cc2.fields.curr_in_mppt);
	printf("(INT16) cc2.fields.volt_out_mppt : %d \n\r", response.fields.cc2.fields.volt_out_mppt);
	printf("(INT16) cc2.fields.curr_out_mppt : %d \n\r", response.fields.cc2.fields.curr_out_mppt);
	printf("(INT16) cc3.fields.volt_in_mppt : %d \n\r", response.fields.cc3.fields.volt_in_mppt);
	printf("(INT16) cc3.fields.curr_in_mppt : %d \n\r", response.fields.cc3.fields.curr_in_mppt);
	printf("(INT16) cc3.fields.volt_out_mppt : %d \n\r", response.fields.cc3.fields.volt_out_mppt);
	printf("(INT16) cc3.fields.curr_out_mppt : %d \n\r", response.fields.cc3.fields.curr_out_mppt);
	printf("\n\r");
	
	return TRUE;
}
/*
 * prints all the parameters in the structure of imepsv2_piu__gethousekeepingeng__from_t.
 * */
static Boolean _gethousekeepingeng__tm( void )
{
	imepsv2_piu__gethousekeepingeng__from_t response;

	int error = imepsv2_piu__gethousekeepingeng(_index,&response);
	if( error )
	{
		TRACE_ERROR("imepsv2_piu__gethousekeepingeng(...) return error (%d)!\n\r",error);
		return FALSE;
	}

	printf("\nisis_eps get housekeeping data engineering response: \n\n\r");
	_print_eps_response(&(response.fields.reply_header));

	printf("Internal EPS board voltage: %u mV\n\r", response.fields.volt_brdsup);
	printf("MCU temperature: %.2f deg. C\n\r", ((double)response.fields.temp) * 0.01);
	printf("EPS input voltage: %d mV \n\r", response.fields.dist_input.fields.volt);
	printf("EPS input current: %d mA \n\r", response.fields.dist_input.fields.current);
	printf("EPS power consumption: %d mW \n\r", response.fields.dist_input.fields.power * 10);
	printf("Battery voltage: %d mW\n\r", response.fields.batt_input.fields.volt);
	printf("Battery current: %d mA\n\r", response.fields.batt_input.fields.current);
	printf("Battery power: %d mW\n\r", response.fields.batt_input.fields.power * 10);

	printf("\n\rOutput bus channel status: 0x%04x \n\r", response.fields.stat_obc_on);
	printf("Output bus channel overcurrent status: 0x%04x \n\r", response.fields.stat_obc_ocf);
	printf("Battery status flags: 0x%04x \n\r", response.fields.bat_stat);

	printf("\n\rMain battery temperature: %.2f deg. C\n\r", ((double)response.fields.temp2) * 0.01);
	printf("Secondary battery temperature: %.2f deg. C\n\r", ((double)response.fields.temp3) * 0.01);

	printf("\n\rV_BAT: %u mV\n\r", response.fields.volt_vd0);
	printf("5V perm: %u mV\n\r", response.fields.volt_vd1);
	printf("3V3 perm: %u mV\n\r", response.fields.volt_vd2);

	printf("\n\rAll output channels:\n\r");
	printf("Channel\t\tVoltage [mV]\tCurrent [mA]\tPower [mW]\n\r");

	printf("[VD0] V_BAT\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc00.fields.volt, response.fields.vip_obc00.fields.current, response.fields.vip_obc00.fields.power * 10);
	printf("[VD1] 5V perm\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc01.fields.volt, response.fields.vip_obc01.fields.current, response.fields.vip_obc01.fields.power * 10);
	printf("[VD2] 5V SW1\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc02.fields.volt, response.fields.vip_obc02.fields.current, response.fields.vip_obc02.fields.power * 10);
	printf("[VD3] 5V SW2\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc03.fields.volt, response.fields.vip_obc03.fields.current, response.fields.vip_obc03.fields.power * 10);
	printf("[VD4] 5V SW3\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc04.fields.volt, response.fields.vip_obc04.fields.current, response.fields.vip_obc04.fields.power * 10);
	printf("[VD5] 3V3 perm\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc05.fields.volt, response.fields.vip_obc05.fields.current, response.fields.vip_obc05.fields.power * 10);
	printf("[VD6] 3V3 SW1\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc06.fields.volt, response.fields.vip_obc06.fields.current, response.fields.vip_obc06.fields.power * 10);
	printf("[VD7] 3V3 SW2\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc07.fields.volt, response.fields.vip_obc07.fields.current, response.fields.vip_obc07.fields.power * 10);
	printf("[VD8] 3V3 SW3\t%d\t\t%d\t\t%d\n\r", response.fields.vip_obc08.fields.volt, response.fields.vip_obc08.fields.current, response.fields.vip_obc08.fields.power * 10);

	printf("\n\rMaximum Power Point Trackers\n\r");
	printf("Channel\tVoltage In [mV]\tCurrent In [mA]\tVoltage Out [mV]\tCurrent Out [mA]\n\r");
	printf("1 \t%d\t\t%d\t\t%d\t\t\t%d\n\r", response.fields.cc1.fields.volt_in_mppt, response.fields.cc1.fields.curr_in_mppt,
			response.fields.cc1.fields.volt_out_mppt, response.fields.cc1.fields.curr_out_mppt);
	printf("2 \t%d\t\t%d\t\t%d\t\t\t%d\n\r", response.fields.cc2.fields.volt_in_mppt, response.fields.cc2.fields.curr_in_mppt,
			response.fields.cc2.fields.volt_out_mppt, response.fields.cc2.fields.curr_out_mppt);
	printf("3 \t%d\t\t%d\t\t%d\t\t\t%d\n\r", response.fields.cc3.fields.volt_in_mppt, response.fields.cc3.fields.curr_in_mppt,
			response.fields.cc3.fields.volt_out_mppt, response.fields.cc3.fields.curr_out_mppt);

	printf("\n\r");

	return TRUE;
}

/**
 * Get the status of the voltage on the EPS according to the voltage thresholds that we set in the PDR of Tavel2
 * */
static Boolean GetStatusEPS(void)
{
	imepsv2_piu__gethousekeepingeng__from_t responseEPS;

	int error = imepsv2_piu__gethousekeepingeng(0,&responseEPS); //gets the data structure of the responseEPS
	// if you get an error you print what error do you have. else it prints the status.
	if( error )
			TRACE_ERROR("imepsv2_piu__gethousekeepingeng(...) return error (%d)!\n\r",error);
	else
	{
		//get voltage in mV
		if(responseEPS.fields.batt_input.fields.volt > 7400) //print for Operational mode
			printf("EPS status is Operational \nThe volt battery equal to %d [mV]\r\n", responseEPS.fields.batt_input.fields.volt);
		else if(responseEPS.fields.batt_input.fields.volt > 7000) //print for Cruise mode
			printf("EPS status is Cruise \nThe volt battery equal to %d [mV]\r\n", responseEPS.fields.batt_input.fields.volt);
		else //print for Power Safe mode
			printf("The satellite is in Power Safe mode \nThe volt battery equal to %d [mV]\r\n", responseEPS.fields.batt_input.fields.volt);
	}
	return TRUE;
}

/**
 * get configuration parameter that his value get is from type uint16_t, and prints the name of the parameter and the value of the parameter.
 *
 * @param[in] name= config_parmeter_id; type= uint16_t; put the id of the parameter according to ISIS.EPSv2.ICD.SW. Software ICD v1.0 from page 89 to page 92
 * @param[in] name= name; type= char[]; put the name of the parameter you put the id of.
 * */
static void GetConfigurationParmeter(uint16_t config_parmeter_id, char name[])
{
	imepsv2_piu__getconfigurationparameter__from_t response;
	print_error(imepsv2_piu__getconfigurationparameter(0, config_parmeter_id, &response)); ///gets the data structure of the parameter to response according to the id of the parameter

	printf("%s: ", name); //prints the name
	short response_short;
	memcpy(&response_short, response.fields.par_val, 2); //instead of printing an array we do memcpy and get the array in the right places to create a new value and put it in response_short
	printf(response_short + "\r\n"); //prints the value.
}

/**
 * prints the value in these 4 config parameters.
 * */
static Boolean GetConfigurationParameters(void)
{
	GetConfigurationParmeter(0x4002, "OBUS_FORCE_ENA_BF");

	GetConfigurationParmeter(0x4004, "OBUS_STARTUP_ENA_BF");

	GetConfigurationParmeter(0x480D, "SAFETY_VOLT_LOTHR"); //in mV

	GetConfigurationParmeter(0x480E, "SAFETY_VOLT_HITHR"); //in mV

	return TRUE;
}
/*
 * Asks the user which test he wants or if he wants to exit the test loop.
 * all the functions returns TRUE while the exit is FALSE.
 * @return type= Boolean; offerMoreTest that get to an infinite loop and the loop ends if the function return FALSE.
 * */
static Boolean selectAndExecuteIsis_EpsDemoTest()
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Software Reset \n\r");
	printf("\t 2) Cancel Operation \n\r");
	printf("\t 3) Watchdog Kick \n\r");
	printf("\t 4) Output Bus Channel On \n\r");
	printf("\t 5) Output Bus Channel Off \n\r");
	printf("\t 6) Switch To Nominal \n\r");
	printf("\t 7) Switch To Safety \n\r");
	printf("\t 8) Get System Status \n\r");
	printf("\t 9) Get Housekeeping Data - Raw \n\r");
	printf("\t 10) Get Housekeeping Data - Engineering \n\r");
	printf("\t 11) Get status \n\r");
	printf("\t 12) Get configuration parameters \n\r");
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 12) == 0); //you have to write a number between the two numbers include or else it ask you to enter a number between the two.

	switch(selection)
	{
		case 0:
			offerMoreTests = FALSE;
			break;
		case 1:
			offerMoreTests = _reset__tmtc();
			break;
		case 2:
			offerMoreTests = _cancel__tm();
			break;
		case 3:
			offerMoreTests = _watchdog__tm();
			break;
		case 4:
			offerMoreTests = _outputbuschannelon__tmtc();
			break;
		case 5:
			offerMoreTests = _outputbuschanneloff__tmtc();
			break;
		case 6:
			offerMoreTests = _switchtonominal__tm();
			break;
		case 7:
			offerMoreTests = _switchtosafety__tm();
			break;
		case 8:
			offerMoreTests = _getsystemstatus__tm();
			break;
		case 9:
			offerMoreTests = _gethousekeepingraw__tm();
			break;
		case 10:
			offerMoreTests = _gethousekeepingeng__tm();
			break;
		case 11:
			offerMoreTests = GetStatusEPS();
			break;
		case 12:
			offerMoreTests = GetConfigurationParameters();
			break;
		default:
			break;
	}

	return offerMoreTests;
}

Boolean isis_eps__demo__init(void)
{
    int retValInt = 0;

	//Initialize the I2C
	printf("\nI2C Initialize\n\r");
	retValInt = I2C_start(200000, 10);

	if(retValInt != 0)
	{
		TRACE_FATAL("\n\rI2Ctest: I2C_start_Master for ISIS_EPS test: %d! \n\r", retValInt);
	}

	IMEPSV2_PIU_t subsystem[1]; // One instance to be initialised.
	subsystem[0].i2cAddr = 0x20; // I2C address defined to 0x20.
		
	retValInt = IMEPSV2_PIU_Init(subsystem, 1);
	if(retValInt == driver_error_reinit)
	{
		printf("\nISIS_EPS subsystem have already been initialised.\n\r");
	}
	else if(retValInt != driver_error_none )
	{
		printf("\nISIS_EPS_Init(...) returned error %d! \n\r", retValInt);
		return FALSE;
	}

	return TRUE;
}

void isis_eps__demo__loop()
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

Boolean isis_eps__demo__main(void)
{
	if(!isis_eps__demo__init())
	{
		return FALSE;
	}

	isis_eps__demo__loop();

	return TRUE;
}

Boolean isis_eps__test()
{
	return isis_eps__demo__main();
}

