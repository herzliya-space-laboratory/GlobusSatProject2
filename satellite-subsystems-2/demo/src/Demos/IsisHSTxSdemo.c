/*
 * IsisTxSdemo.c
 *
 *  Created on: 4 mrt. 2015
 *      Author: malv
 */
#include "common.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/boards/ISIS_OBC_G20/board.h>

#include <hal/Utility/util.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/LED.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <satellite-subsystems/isis_hstxs.h>
#include <satellite-subsystems/isis_hstxs_spi.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Test Functions
static Boolean resetSupHSTxsTest(void)
{
	printf("\r\n Reset of the HSTxS supervisor board\r\n");
	print_error(isis_hstxs__reset(0));

	return TRUE;
}

static Boolean toggleHSTxsSupStateTest(void)
{
	static Boolean toggle_supflag = 0;

	if(toggle_supflag)
	{
	    print_error(isis_hstxs__set_sup_mode(0, isis_hstxs__sup_mode__sup_only));
	    toggle_supflag = FALSE;
	}
	else
	{
	    print_error(isis_hstxs__set_sup_mode(0, isis_hstxs__sup_mode__standby));
	    toggle_supflag = TRUE;
	}

	return TRUE;
}

static Boolean getSupTelemHSTxSTest(void)
{
	unsigned short telemetryValue = 0;
	unsigned int uptime = 0;
	unsigned int pwrtime = 0;
	float eng_value = 0.0;
	isis_hstxs__get_telemetry__from_t tlm_supervisor;

	// Telemetry values are presented as raw values
	printf("\r\nGet all Telemetry\r\n\r\n");
	print_error(isis_hstxs__get_telemetry(0, &tlm_supervisor));


	telemetryValue = tlm_supervisor.fields.reflPower;
	eng_value = ((float)telemetryValue/4095.0) * 2500;
	printf("- Reflected output power = %.2f mV\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.forwPower;
	eng_value = ((float)telemetryValue/4095.0) * 2500;
	printf("- Forward output power = %.2f mV\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.volt3v3;
	eng_value = ((float)telemetryValue/4095.0) * 2500 * 8;
	printf("- 3V3 line Voltage = %.2f mV\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.volt3v3sw;
	eng_value = ((float)telemetryValue/4095.0) * 2500 * 8;
	printf("- 3V3 SW line Voltage = %.2f mV\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.volt5v * 8;
	eng_value = ((float)telemetryValue/4095.0) * 2500;
	printf("- 5V SW line Voltage = %.2f mV\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.voltBat * 8;
	eng_value = ((float)telemetryValue/4095.0) * 2500;
	printf("- Battery bus line Voltage = %.2f mV\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.cur3v3 * 1/(0.05 * 100);
	eng_value = ((float)telemetryValue/4095.0) * 2500;
	printf("- 3V3 line current = %.2f mA\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.cur3v3sw * 1/(0.015 * 100);
	eng_value = ((float)telemetryValue/4095.0) * 2500;
	printf("- 3V3 SW line current = %.2f mA\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.cur5v * 1/(0.01 * 100);
	eng_value = ((float)telemetryValue/4095.0) * 2500;
	printf("- 5V SW line current = %.2f mA\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.curBat * 1/(0.01 * 100);
	eng_value = ((float)telemetryValue/4095.0) * 2500;
	printf("- Battery bus line current = %.2f mA\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.voltCtl;
	eng_value = ((float)telemetryValue/4095.0) * 2500;
	printf("- Voltage Control line Voltage = %.2f mV\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.tempDriver;
	eng_value = (float)telemetryValue * -0.07669 + 195.6037;
	printf("- Driver Temperature = %.2f deg. C\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.tempPD;
	eng_value = (((((float)telemetryValue/4095.0) * 2500) - 1.4)/0.005) + 25;
	printf("- PD Temperature = %.2f deg. C \r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.tempPA;
	eng_value = (float)telemetryValue * -0.07669 + 195.6037;
	printf("- Power Amplifier Temperature = %.2f deg. C\r\n", eng_value);

	telemetryValue = tlm_supervisor.fields.tempTXCO;
	eng_value = (float)telemetryValue * -0.07669 + 195.6037;
	printf("- Crystal Temperature = %.2f deg. C\r\n", eng_value);

	uptime = tlm_supervisor.fields.uptime;
	printf("- HSTxS supervisor uptime = %d seconds\r\n", uptime);

	pwrtime =  tlm_supervisor.fields.mssPwrTime;
	printf("- HSTxS MSS power time = %d seconds\r\n", pwrtime);

	if(tlm_supervisor.fields.StatusFlags.fields.currmode)
	{
		printf("- Stand-by mode state\r\n");
	}
	else
	{
		printf("- On mode state\r\n");
	}

	if(tlm_supervisor.fields.StatusFlags.fields.good3v3Sw)
	{
		printf("- 3V3 SW line good line high\r\n");
	}
	else
	{
		printf("- 3V3 SW line good line low\r\n");
	}

	if(tlm_supervisor.fields.StatusFlags.fields.good5vSw)
	{
		printf("- 5V SW line good line high\r\n");
	}
	else
	{
		printf("- 5V SW line good line low\r\n");
	}

	if(tlm_supervisor.fields.StatusFlags.fields.paOverTemp)
	{
		printf("- PA over temperature activated \r\n");
	}

	if(tlm_supervisor.fields.StatusFlags.fields.tcxoOverTemp)
	{
		printf("- TCXO over temperature activated \r\n");
	}

	if(tlm_supervisor.fields.StatusFlags.fields.oc5v)
	{
		printf("- 5V line over current activated \r\n");
	}

	printf("- HSTxS MSS WDT reset counter = %d times\r\n", tlm_supervisor.fields.WDTResetCount);


	return TRUE;
}

static Boolean toggleHSTxsMSS_UptimeTest(void)
{
	uint32_t mss_uptime = 0;

	print_error(isis_hstxs__get_uptime(0, &mss_uptime));

	printf("- Current MSS uptime is:  %d\r\n", (unsigned int)mss_uptime);

	return TRUE;
}

static Boolean toggleHSTxsMSS_StateTest(void)
{
	static Boolean toggle_mssflag = 0;

	if(toggle_mssflag)
	{
	    print_error(isis_hstxs__set_tx_mode(0, isis_hstxs__mode__standby));
		toggle_mssflag = FALSE;
	}
	else
	{
	    print_error(isis_hstxs__set_tx_mode(0, isis_hstxs__mode__on));
		toggle_mssflag = TRUE;
	}

	return TRUE;
}

static Boolean toggleHSTxsMSSFECFTest(void)
{
	static Boolean toggle_fecfflag = FALSE;

	if(toggle_fecfflag)
	{
		printf("Turn FECF ON\n\r");
		print_error(isis_hstxs__set_fecf(0, isis_hstxs__enabled__enabled));
	}
	else
	{
		printf("Turn FECF OFF\n\r");
		print_error(isis_hstxs__set_fecf(0, isis_hstxs__enabled__disabled));
	}

	return TRUE;
}

static Boolean toggleHSTxsMSSModTest(void)
{
	static Boolean toggle_mssModflag = 0;

	if(toggle_mssModflag)
	{
		print_error(isis_hstxs__set_modulation_scheme(0, isis_hstxs__modscheme__oqpsk));
		printf("\r\n Set OQPSK modulation scheme\r\n");
	    toggle_mssModflag = FALSE;
	}
	else
	{
	    print_error(isis_hstxs__set_modulation_scheme(0, isis_hstxs__modscheme__bpsk));
		printf("\r\n Set BPSK modulation scheme\r\n");
	    toggle_mssModflag = TRUE;
	}

	return TRUE;
}

static Boolean setSymbolrateHSTxsTest(void)
{
	int selection = 0;

	printf("\n\r Select a symbolrate to set:\n\r");
	printf("\t 0) One eight symbol rate [625 ksps]\n\r");
	printf("\t 1) One quarter symbol rate [1250 ksps]\n\r");
	printf("\t 2) One half symbol rate [2500 ksps]\n\r");
	printf("\t 3) Maximum symbolrate [5000 ksps]\n\r");
	printf("\t 4) Don't change symbolrate\n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 4) == 0);

	if(selection >= 0 && selection < 4)
	{
		print_error(isis_hstxs__set_symbolrate(0, selection));
	}

	return TRUE;
}

static Boolean getMSSStatusHSTxSTest(void)
{
	isis_hstxs__get_general_status__from_t status_mss;

	printf("\r\nMSS current status values\r\n\r\n");
	print_error(isis_hstxs__get_general_status(0, &status_mss));

	printf("- Uptime: %d seconds\r\n", (unsigned int)status_mss.fields.uptime);

	if(status_mss.fields.modulation)
	{
		printf("- Current Modulation is OQPSK\r\n");
	}
	else
	{
		printf("- Current Modulation is BPSK\r\n");
	}

	if(status_mss.fields.tx_on)
	{
		printf("- HSTxS Currently on Tx-ON mode\r\n");
	}
	else
	{
		printf("- HSTxS Currently on Stand_By mode\r\n");
	}

	switch(status_mss.fields.symbolrate)
	{
		case isis_hstxs__symbolrate__eighth:
			printf("- Eighth of the full bitrate\r\n");
			break;
		case isis_hstxs__symbolrate__quarter:
			printf("- Quarter of the full bitrate\r\n");
			break;
		case isis_hstxs__symbolrate__half:
			printf("- Half of the full bitrate\r\n");
			break;
		case isis_hstxs__symbolrate__full:
			printf("- Full bitrate \r\n");
			break;
	}

	printf("- Current external attenuator value %d\r\n", status_mss.fields.attenuator);
	printf("- Modulator attenuation level value %d\r\n", status_mss.fields.mod_out_power);
	printf("- LUT Choice 0x%02x \r\n", status_mss.fields.lut_choice);
	printf("- SCID level value %d\r\n", status_mss.fields.scid);
	printf("- Serial DAC control voltage value %d\r\n", status_mss.fields.volt_serial_DAC);
	printf("- Engine status value 0x%02x \r\n", (unsigned int)status_mss.fields.engine_stat);
	printf("- MSS uptime: %d\r\n", (unsigned int)status_mss.fields.uptime);
	printf("- PLL locked error counter: %d\r\n", status_mss.fields.locked_err_cnt);
	printf("- PLL frequency error counter: %d\r\n", status_mss.fields.freq_err_cnt);
	printf("- SPI FIFO error counter: %d\r\n", (unsigned int)status_mss.fields.spi_err_cnt);

	return TRUE;
}

static Boolean toggleHSTxsMSSSetFreqTest(void)
{
	static Boolean toggle_mssSetFreqflag = 0;
	unsigned int freq_2245000 = 2245000;
	unsigned int freq_2200000 = 2200000;

	if(toggle_mssSetFreqflag)
	{
	    print_error(isis_hstxs__set_tx_frequency(0, freq_2200000));
		toggle_mssSetFreqflag = FALSE;
	}
	else
	{
	    print_error(isis_hstxs__set_tx_frequency(0, freq_2245000));
		toggle_mssSetFreqflag = TRUE;
	}

	return TRUE;
}

static Boolean getHSTxsMSSFreqTest(void)
{
	uint32_t freq = 0;

	print_error(isis_hstxs__get_tx_frequency(0, &freq));

	printf("- Current transmitter frequency is: %d kHz\r\n", (unsigned int)freq);

	return TRUE;
}

static Boolean getHSTxsMSSEngStatusTest(void)
{
	isis_hstxs__get_engine_status__from_t status_reg = {{0}};

	unsigned int* raw_eng_status = (unsigned int*)(&status_reg.raw[0]);

	print_error(isis_hstxs__get_engine_status(0, &status_reg));

	printf("- Current engine status reg is:  0x%x\r\n", *raw_eng_status);

	return TRUE;
}

static Boolean HSTxsMSSSendFramesTest(Boolean use_spi)
{
	const int frame_payload_size = 217;

	int selection = 0;
	int vcid, frame_count, byte_count;
	uint8_t status_out;
	driver_error_t driver_error;

	isis_hstxs__ccsds_frameheader_t ccsds_header = {{0}};
	isis_hstxs_spi__sendframe__to_t spi_frame;
	isis_hstxs__write_and_send_frame__to_t i2c_frame;

	i2c_frame.fields.data = malloc(frame_payload_size);
	if(i2c_frame.fields.data == NULL)
	{
		printf("Error allocating memory for I2C frame\n\r");
		return TRUE;
	}

	printf("\n\r How many frames should be sent per virtual channel? [1 - 100000]\n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 100000) == 0);

	for(vcid = 1; vcid <= 7; vcid++)
	{
		ccsds_header.fields.virtual_channel = vcid;

		printf("Sending %d frames with VCID %d to the HSTXS over ", selection, vcid);
		if(use_spi)
		{
			printf("SPI\n\r");

			// Set frame header
			spi_frame.fields.header = ccsds_header;

			for(frame_count = 0; frame_count < selection; frame_count++)
			{
				for(byte_count = 0; byte_count < frame_payload_size; byte_count++)
				{
					spi_frame.fields.data[byte_count] = frame_count + byte_count;
				}

				driver_error = driver_error_none;
				do
				{
					driver_error = isis_hstxs_spi__sendframe(&spi_frame, TRUE);
				}
				while(driver_error != driver_error_none);
			}
		}
		else
		{
			printf("I2C\n\r");

			// Set frame header
			i2c_frame.fields.header = ccsds_header;

			for(frame_count = 0; frame_count < selection; frame_count++)
			{
				for(byte_count = 0; byte_count < frame_payload_size; byte_count++)
				{
					i2c_frame.fields.data[byte_count] = frame_count + byte_count;
				}
				isis_hstxs__write_and_send_frame(0, &i2c_frame, frame_payload_size, &status_out);
			}
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
	}

	return TRUE;
}

static Boolean selectAndExecuteTxSDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform:\n\r");
	printf("\t 1) HSTxS Supervisor reset (Supervisor)\n\r");
	printf("\t 2) HSTxS Toggle between on mode and stand_by mode (Supervisor)\n\r");
	printf("\t 3) HSTxS Request supervisor telemetry (Supervisor)\n\r");
	printf("\t 4) HSTxS MSS uptime (MSS/Fabric)\n\r");
	printf("\t 5) HSTxS Toggle between FECF on and off (MSS/Fabric)\n\r");
	printf("\t 6) HSTxS Toggle between stand_by and tx_on mode (MSS/Fabric)\n\r");
	printf("\t 7) HSTxS Toggle between OQPSK and BPSK modulation scheme (MSS/Fabric)\n\r");
	printf("\t 8) HSTxS Set symbolrate (MSS/Fabric)\n\r");
	printf("\t 9) HSTxS Request MSS current status (MSS/Fabric)\n\r");
	printf("\t 10) HSTxS Toggle TX Frequency between 2,245 GHz and 2,2 GHz (MSS/Fabric)\n\r");
	printf("\t 11) HSTxS Get TX Frequency (MSS/Fabric)\n\r");
	printf("\t 12) HSTxS Get engine status (MSS/Fabric)\n\r");
	printf("\t 13) HSTxS Send data frames over I2C (MSS/Fabric)\n\r");
	printf("\t 14) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 14) == 0);

	switch(selection) {
	case 1:
		offerMoreTests = resetSupHSTxsTest();
		break;
	case 2:
		offerMoreTests = toggleHSTxsSupStateTest();
		break;
	case 3:
		offerMoreTests = getSupTelemHSTxSTest();
		break;
	case 4:
		offerMoreTests = toggleHSTxsMSS_UptimeTest();
		break;
	case 5:
		offerMoreTests = toggleHSTxsMSSFECFTest();
		break;
	case 6:
		offerMoreTests = toggleHSTxsMSS_StateTest();
		break;
	case 7:
		offerMoreTests = toggleHSTxsMSSModTest(); // Function needs to be addd to driver
		break;
	case 8:
		offerMoreTests = setSymbolrateHSTxsTest();
		break;
	case 9:
		offerMoreTests = getMSSStatusHSTxSTest();
		break;
	case 10:
		offerMoreTests = toggleHSTxsMSSSetFreqTest();
		break;
	case 11:
		offerMoreTests = getHSTxsMSSFreqTest();
		break;
	case 12:
		offerMoreTests = getHSTxsMSSEngStatusTest();
		break;
	case 13:
		offerMoreTests = HSTxsMSSSendFramesTest(FALSE);
		break;
	case 14:
		offerMoreTests = FALSE;
		break;

	default:
		break;
	}

	return offerMoreTests;
}


Boolean IsisHSTxSdemoInit(void)
{
	ISIS_HSTXS_t i2c_addresses;

	i2c_addresses.mssAddr = 0x45;
	i2c_addresses.supAddr = 0x46;
    int rv;

    rv = ISIS_HSTXS_Init(&i2c_addresses, 1);
    if(rv != driver_error_none && rv != driver_error_reinit)
    {
    	// we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
    	// hanging the code
    	TRACE_ERROR("\n\r IsisHSTxsInitialize() failed; err=%d! Exiting ...\n\r", rv);
    	return FALSE;
    }

    return TRUE;
}

void IsisHSTxSdemoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteTxSDemoTest(); // show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)  // was exit/back
		{
			break;
		}
	}
}

Boolean IsisHSTxSdemoMain(void)
{
	if(IsisHSTxSdemoInit())                                 // initialize of I2C and IsisTRXVU subsystem drivers succeeded?
	{
		IsisHSTxSdemoLoop();                                // show the main IsisTRXVU demo interface and wait for user input
		return TRUE;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}


Boolean HSTxStest(void)
{
	IsisHSTxSdemoMain();
	return TRUE;
}
