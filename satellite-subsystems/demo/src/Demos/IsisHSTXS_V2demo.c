/*
 * IsisHSTXS_V2demo.c
 *
 *  Created on: 18 oct. 2023
 *      Author: obar
 */

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
#include <hal/errors.h>

#include <satellite-subsystems/isis_hstxs_v2.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TXS_WAITTIME_MINIMUM		1
#define TXS_PACKET_MAXRETRIES		5
#define TXS_CCSDSFRAME_SIZE			223 ///< Size in bytes of a CCSDS frame
#define TXS_STATIC_WAIT_TIME		10 //ms

static isis_hstxs_v2__ccsds_frameheader_t ccsds_header = {{0}};
static float txs_bitrate_bps = 625000.0;

// Test Functions
static Boolean softResetHSTxSV2Test(void)
{
	printf("\r\n Software Reset of the TxS microcontroller \r\n");
	print_error(isis_hstxs_v2__reset(0));

	return TRUE;
}

static Boolean halfMaxBitrateHSTxSV2Test(void)
{
	printf("\r\n The current bitrate has been set to half its maximum value \r\n");
	print_error(isis_hstxs_v2__set_symbolrate(0, isis_hstxs_v2__symbolrate__half));
	txs_bitrate_bps = 2150000.0;

	return TRUE;
}

static Boolean MaxBitrateHSTxSV2Test(void)
{
	printf("\r\n The current bitrate has been set to its maximum value \r\n");
	print_error(isis_hstxs_v2__set_symbolrate(0, isis_hstxs_v2__symbolrate__full));
	txs_bitrate_bps = 4300000.0;

	return TRUE;
}

static Boolean toggleHSTxSV2ModulationScheme(void)
{
	static Boolean toggle_flag = 0;

	if(toggle_flag)
	{
		printf("\r\n Setting modscheme to BPSK \r\n");
		print_error(isis_hstxs_v2__set_modulation_scheme(0, isis_hstxs_v2__modscheme__bpsk));
		toggle_flag = FALSE;
	}
	else
	{
		printf("\r\n Setting modscheme to OQPSK \r\n");
		print_error(isis_hstxs_v2__set_modulation_scheme(0, isis_hstxs_v2__modscheme__oqpsk));
		toggle_flag = TRUE;
	}

	return TRUE;
}

static Boolean toggleHSTxSV2TXMode(void)
{
	static Boolean toggle_flag = 0;

	if(toggle_flag)
	{
	    print_error(isis_hstxs_v2__set_tx_mode(0, isis_hstxs_v2__mode__standby));
		toggle_flag = FALSE;
	}
	else
	{
	    print_error(isis_hstxs_v2__set_tx_mode(0, isis_hstxs_v2__mode__on));
		toggle_flag = TRUE;
	}

	return TRUE;
}

static Boolean toggleHSTxSToggleStandbyMode(void)
{
	static Boolean toggle_flag = 0;

	if(toggle_flag)
	{
	    print_error(isis_hstxs_v2__set_standby_mode(0, isis_hstxs_v2__sup_mode__sup_only));
		toggle_flag = FALSE;
	}
	else
	{
	    print_error(isis_hstxs_v2__set_standby_mode(0, isis_hstxs_v2__sup_mode__standby));
		toggle_flag = TRUE;
	}

	return TRUE;
}

static Boolean sendTestFrame(void)
{
	unsigned char test_data[10] =  {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
	unsigned char data_size = 10;

	void* newdata = NULL;
	unsigned short retrycount;
	driver_error_t txs_error;
	unsigned char addframe_status;

	newdata = malloc(data_size + 1);
	if(newdata != NULL)
	{
		// Copy data and add a size byte
		memcpy(newdata, &(data_size), sizeof(unsigned char));
		memcpy(newdata + 1, test_data, data_size);

		data_size++;

		// Try send packet through the HS TxS v2
		isis_hstxs_v2__write_and_send_frame__to_t hstxs_frame;
		hstxs_frame.data = newdata;
		hstxs_frame.header = ccsds_header;
		hstxs_frame.header.fields.virtual_channel = 0x01; // VCID to use

		txs_error = isis_hstxs_v2__write_and_send_frame(0, &hstxs_frame, data_size, &addframe_status);

		for(retrycount = 0; retrycount < TXS_PACKET_MAXRETRIES && txs_error != driver_error_none && addframe_status != 0; retrycount++)
		{
			// Wait a bit before trying again, in reality this should depend on the symbol rate
			vTaskDelay(TXS_STATIC_WAIT_TIME / portTICK_RATE_MS);

			// Retry transmission
			txs_error = isis_hstxs_v2__write_and_send_frame(0, &hstxs_frame, data_size, &addframe_status);
		}
	}

	return TRUE;
}

static Boolean getAllTelemHSTxSV2Test(void)
{
	unsigned short telemetryValue;
	float eng_value = 0.0;
	isis_hstxs_v2__get_general_status__from_t status_telemetry;
	isis_hstxs_v2__get_telemetry__from_t telemetry;

	printf("\r\nGet all Telemetry \r\n\r\n");
	print_error(isis_hstxs_v2__get_general_status(0, &status_telemetry));

    printf("\r\n HSTxS V2 Telemetry: \r\n");

    printf("Modulation Scheme: ");
    switch(status_telemetry.fields.modulation)
    {
    	case isis_hstxs_v2__modscheme__bpsk:
    		printf("BPSK");
    		break;
    	case isis_hstxs_v2__modscheme__oqpsk:
    		printf("OQPSK");
    		break;
    	default:
    		printf("UNKNOWN");
    		break;
    }

    printf("\r\n Mode: ");
    switch(status_telemetry.fields.tx_on)
    {
    	case isis_hstxs_v2__mode__standby:
    		printf("STANDBY");
    		break;
    	case isis_hstxs_v2__mode__on:
    		printf("ON");
    		break;
    	default:
    		printf("UNKNOWN");
    		break;
    }

    printf("\r\n Symbol Rate: ");
    switch(status_telemetry.fields.symbolrate)
    {
    	case isis_hstxs_v2__symbolrate__thirtysecondth:
    		printf("THIRTYSECOND - 156.25 ksps");
    		break;
    	case isis_hstxs_v2__symbolrate__sixteenth:
    		printf("SIXTEENTH - 312.5 ksps");
    		break;
    	case isis_hstxs_v2__symbolrate__eighth:
    		printf("EIGHTH - 625 ksps");
    		break;
    	case isis_hstxs_v2__symbolrate__quarter:
    		printf("QUARTER - 1250 ksps");
    		break;
    	case isis_hstxs_v2__symbolrate__half:
    		printf("HALF - 2500 ksps");
    		break;
    	case isis_hstxs_v2__symbolrate__full:
    		printf("FULL - 5000 ksps");
    		break;
    	default:
    		printf("UNKNOWN");
    		break;
    }

    printf("\r\n Modulation Output Power: %hhu", status_telemetry.fields.mod_out_power);

    printf("\r\n LUT Choice: ");
    switch(status_telemetry.fields.lut_choice)
    {
    	case isis_hstxs_v2__lut___0_5:
    		printf("Roll off Factor: 0.5");
    		break;
    	case isis_hstxs_v2__lut___0_35:
    		printf("Roll off Factor: 0.35");
    		break;
    	default:
    		printf("UNKNOWN");
    		break;
    }

    printf("\r\n Spacecraft ID: %hu", status_telemetry.fields.scid);
    printf("\r\n PCL Setpoint: %hu", status_telemetry.fields.PCLsetpoint);
    printf("\r\n Engine Status: %u", (unsigned int)status_telemetry.fields.engine_stat);
    printf("\r\n Uptime %u sec  \r\n\r\n", (unsigned int)status_telemetry.fields.uptime);

    printf("\r\n Modulator PLL Lock error Count: %hu", status_telemetry.fields.locked_err_cnt);
    printf("\r\n Modulator Frequency Error Count: %hu", status_telemetry.fields.freq_err_cnt);
    printf("\r\n SPI Error Count: %u \r\n", (unsigned int)status_telemetry.fields.spi_err_cnt);


	print_error(isis_hstxs_v2__get_telemetry(0, &telemetry));


    telemetryValue = telemetry.fields.reflPower;
    eng_value = (float)telemetryValue * 0.6105006105;
    printf("\r\n\r\n Reflected Power: %.2f", eng_value);

    telemetryValue = telemetry.fields.forwPower;
    eng_value = (float)telemetryValue * 0.6105006105;
    printf("\r\n Forward Power: %.2f", eng_value);

    telemetryValue = telemetry.fields.volt3v3;
    eng_value = (float)telemetryValue * 0.004884;
    printf("\r\n Voltage 3v3: %.2f V", eng_value);

    telemetryValue = telemetry.fields.volt3v3sw;
    eng_value = (float)telemetryValue * 0.004884;
    printf("\r\n Voltage 3v3 (switched): %.2f V", eng_value);

    telemetryValue = telemetry.fields.volt5v;
    eng_value = (float)telemetryValue * 0.004884;
    printf("\r\n Voltage 5v: %.2f V", eng_value);

    telemetryValue = telemetry.fields.voltBat;
    eng_value = (float)telemetryValue * 0.004884;
    printf("\r\n Voltage Battery: %.2f V", eng_value);

    telemetryValue = telemetry.fields.cur3v3;
    eng_value = (float)telemetryValue * 0.1221001221001221;
    printf("\r\n Current 3v3: %.2f mA", eng_value);

    telemetryValue = telemetry.fields.cur3v3sw;
    eng_value = (float)telemetryValue * 0.407;
    printf("\r\n Current 3v3 (switched): %.2f mA", eng_value);

    telemetryValue = telemetry.fields.cur5v;
    eng_value = (float)telemetryValue * 0.6105;
    printf("\r\n Current 5v: %.2f mA", eng_value);

    telemetryValue = telemetry.fields.curBat;
    eng_value = (float)telemetryValue * 0.6105;
    printf("\r\n Current Battery: %.2f mA", eng_value);

    telemetryValue = telemetry.fields.voltCtl;
    eng_value = (float)telemetryValue * 0.6105;
    printf("\r\n Voltage Control Loop: %.2f mV", eng_value);

    telemetryValue = telemetry.fields.tempDriver;
    eng_value = ((float)telemetryValue * -0.07669) + 195.6037;
    printf("\r\n Temperature Driver: %.2f C", eng_value);

    telemetryValue = telemetry.fields.tempPD;
    eng_value = ((float)telemetryValue * 0.1221) - 255;
    printf("\r\n Temperature Power Detector: %.2f C", eng_value);

    telemetryValue = telemetry.fields.tempPA;
    eng_value = ((float)telemetryValue * -0.07669) + 195.6037;
    printf("\r\n Temperature Power Amplifier: %.2f C", eng_value);

    telemetryValue = telemetry.fields.tempTXCO;
    eng_value = ((float)telemetryValue * -0.07669) + 195.6037;
    printf("\r\n Temperature TCXO: %.2f C", eng_value);

    printf("\r\n Supervisor Uptime %u sec  \r\n\r\n", (unsigned int)telemetry.fields.uptime);
    printf("\r\n MSS Power Time %u sec  \r\n\r\n", (unsigned int)telemetry.fields.mssPwrTime);
	printf("\r\n Raw Status Flags value = 0x%x \r\n", (unsigned int)telemetry.fields.StatusFlags.raw[0]);
    printf("\r\n Watchdog Timer Reset Count: %hhu \r\n\r\n", telemetry.fields.WDTResetCount);

	return TRUE;
}


static Boolean selectAndExecuteHSTxSV2DemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 1) HSTxSV2 software reset  \n\r");
	printf("\t 2) Set bit rate to half its maximum value \n\r");
	printf("\t 3) Set bit rate to maximum value \n\r");
	printf("\t 4) Toggle Modscheme (BPSK, OQPSK) \n\r");
	printf("\t 5) Send Sample Frame - Note: Use one of the set bit-rate functions first \n\r");
	printf("\t 6) Toggle TX mode (0 - Standby, 1 - Idle) \n\r");
	printf("\t 7) Toggle Standby Mode (0 - SUP Only, 1 - Standby \n\r");
	printf("\t 8) Request Housekeeping data \n\r");
	printf("\t 9) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 9) == 0);

	switch(selection) {
	case 1:
		offerMoreTests = softResetHSTxSV2Test();
		break;
	case 2:
		offerMoreTests = halfMaxBitrateHSTxSV2Test();
		break;
	case 3:
		offerMoreTests = MaxBitrateHSTxSV2Test();
		break;
	case 4:
		offerMoreTests = toggleHSTxSV2ModulationScheme();
		break;
	case 5:
		offerMoreTests = sendTestFrame();
		break;
	case 6:
		offerMoreTests = toggleHSTxSV2TXMode();
		break;
	case 7:
		offerMoreTests = toggleHSTxSToggleStandbyMode();
		break;
	case 8:
		offerMoreTests = getAllTelemHSTxSV2Test();
		break;
	case 9:
		offerMoreTests = FALSE;
		break;

	default:
		break;
	}

	return offerMoreTests;
}

ISIS_HSTXS_V2_t i2c_addresses;


Boolean IsisHSTxSV2demoInit(void)
{
	i2c_addresses.mssAddr = 0x45;
	i2c_addresses.supAddr = 0x46;
    int rv;

    rv = ISIS_HSTXS_V2_Init(&i2c_addresses, 1);
    if(rv != E_NO_SS_ERR && rv != E_IS_INITIALIZED && rv != driver_error_reinit)
    {
    	// we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
    	// hanging the code
    	TRACE_ERROR("\n\r IsisTxsInitialize() failed; err=%d! Exiting ... \n\r", rv);
    	return FALSE;
    }

    return TRUE;
}

void IsisHSTxSV2demoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteHSTxSV2DemoTest(); // show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)  // was exit/back
		{
			break;
		}
	}
}

Boolean IsisHSTxSV2demoMain(void)
{
	if(IsisHSTxSV2demoInit())                                 // initialize of I2C and IsisTRXVU subsystem drivers succeeded?
	{
		IsisHSTxSV2demoLoop();                                // show the main IsisTRXVU demo interface and wait for user input
		return TRUE;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}
