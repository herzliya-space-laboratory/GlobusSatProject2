/*
 * isisRXSrevCdemo.c
 *
 *  Created on: 23 Nov 2023
 *      Author: obar
 */

#include "common.h"
#include "isisRXSrevCdemo.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>

#include <hal/Utility/util.h>
#include <hal/checksum.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/LED.h>
#include <hal/Drivers/UART.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <satellite-subsystems/isis_rxs_c_types.h>
#include <satellite-subsystems/isis_rxs_c.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SIZE_RXFRAME 			200
#define	_RXS_UART_BUFFERSIZE	220 //framesize + last_rssi + last_doppler + frame + starter

//RXS UART
/*
 * RXS_UARTBUS set to specific UART
 * I2CADDR_RXS_RX Set to the correct i2c address for the RXS
 */
#define RXS_UARTBUS bus2_uart

//RXS i2c
#define I2CADDR_RXS_RX		0x60

#define _RXS_UART_PILOT_BYTE 0xCA

static const UARTconfig uartBusRXS =
{
		.mode = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT,
		.baudrate = 460800,
		.busType = rs422_noTermination_uart,
		.rxtimeout = 8,	//1 byte timeout
		.timeGuard = 50
};

static xTaskHandle watchdogKickTaskHandle = NULL;
static unsigned short LUT_TCCRC[256];

/*
 * Soft Reset
 */
static Boolean softResetRXSTest(void)
{
	printf("\r\n Soft Reset of RXS \r\n");
	print_error(isis_rxs_c__reset_wdg_rx(0));

	return TRUE;
}

/*
 * power cycle
 */
static Boolean powerCycleRXSTest(void)
{
	printf("\r\n Power Cycle of RXS \r\n");
	print_error(isis_rxs_c__reset_hw_rx(0));

	return TRUE;
}

static Boolean getRXSTelemetryTest(void)
{
	unsigned short telemetryValue;
	float eng_value = 0.0;
	isis_rxs_c__get_rx_telemetry__from_t telemetry;
	int rv;

	// Telemetry values are presented as raw values
	printf("\r\nGet all Telemetry at once in engineering values \r\n\r\n");
	rv = isis_rxs_c__get_rx_telemetry(0, &telemetry);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	telemetryValue = telemetry.fields.doppler;
	eng_value = ((float)telemetryValue) * -38.1469726563;
	printf("Receiver Doppler = %f Hz\r\n", eng_value);

	telemetryValue = telemetry.fields.rssi;
	eng_value = ((float)telemetryValue) * (-0.5) - 28;
	printf("Receiver RSSI = %f dBm\r\n", eng_value);

	telemetryValue = telemetry.fields.voltage;
	eng_value = ((float)telemetryValue) * 0.004912686 + 0.06;
	printf("Bus voltage = %f V\r\n", eng_value);

	telemetryValue = telemetry.fields.current_total;
	eng_value = ((float)telemetryValue) * 0.076312576;
	printf("Total current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.temp_rf;
	eng_value = ((float)telemetryValue) * 0.061035 - 50;
	printf("Transmitter temp RF = %f deg. C\r\n", eng_value);

	telemetryValue = telemetry.fields.temp_board;
	eng_value = ((float)telemetryValue) * 0.061035 - 50;
	printf("Receiver temp board = %f deg. C\r\n", eng_value);

	telemetryValue = telemetry.fields.last_doppler;
	eng_value = ((float)telemetryValue) * (-38.1469726563);
	printf("Last packet Doppler = %f Hz\r\n", eng_value);

	telemetryValue = telemetry.fields.last_rssi;
	eng_value = ((float)telemetryValue) * (-0.5) - 28;
	printf("Last packet RSSI = %f dbm\r\n", eng_value);

	return TRUE;
}

static Boolean getRXSSWInfoTest(void)
{
	isis_rxs_c__get_rx_swinfo__from_t telemetry;
	int rv;

	// Telemetry values are presented as raw values
	printf("\r\nGet SW info \r\n\r\n");
	rv = isis_rxs_c__get_rx_swinfo(0, &telemetry);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	printf("SW Info:");
	printf("%s", telemetry.fields.swinfo);
	printf("\r\n");

	return TRUE;
}

static Boolean getRXSUptimeTest(void)
{
	uint32_t uptime;
	int rv;

	rv = isis_rxs_c__rx_uptime(0, &uptime);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	printf("\r\n Uptime: %u seconds", (unsigned int)uptime);

	return TRUE;

}

static Boolean getFrameCountTest(void)
{
	uint16_t frame_count;
	int rv;

	rv = isis_rxs_c__get_frame_count(0, &frame_count);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	printf("\r\n Current Frame count: %hu \r\n", frame_count);

	return TRUE;
}

static Boolean removeAllFramesTest(void)
{
	int rv;

	rv = isis_rxs_c__remove_frame_all(0);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	printf("\r\n All Frames Removed! \r\n");

	return TRUE;
}

static void print_bitrate(isis_rxs_c__rxs_bitrate_t bitrate)
{
	switch(bitrate)
	{
	case isis_rxs_c__rxs_bitrate__9k6:
		printf("9k6\r\n");
		break;
	case isis_rxs_c__rxs_bitrate__19k2:
		printf("19k2\r\n");
		break;
	case isis_rxs_c__rxs_bitrate__38k4:
		printf("38k4\r\n");
		break;
	case isis_rxs_c__rxs_bitrate__76k8:
		printf("76k8\r\n");
		break;
	default:
		printf("Unknown Bitrate %d\r\n", (int)bitrate);
	}
}

static Boolean changeBitRateTest(void)
{
	isis_rxs_c__rxs_bitrate_t current_bitrate = 0, new_bitrate = 0;
	uint8_t status;
	int rv;

	rv = isis_rxs_c__get_bitrate(0, &current_bitrate);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	printf("\r\nPrevious Bitrate was: ");
	print_bitrate(current_bitrate);

	new_bitrate = current_bitrate + 1;
	if(new_bitrate > isis_rxs_c__rxs_bitrate__76k8)
	{
		new_bitrate = isis_rxs_c__rxs_bitrate__9k6;
	}

	vTaskDelay(5 / portTICK_RATE_MS);

	isis_rxs_c__set_bitrate(0, new_bitrate, &status);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	vTaskDelay(5 / portTICK_RATE_MS);

	rv = isis_rxs_c__get_bitrate(0, &current_bitrate);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}
	printf("New Bitrate is: ");
	print_bitrate(current_bitrate);

	return TRUE;
}


/**
 * Example get RXS frame
 */
static Boolean getRXSframe(void)
{
	unsigned short RxCounter = 0;
	unsigned int i = 0;
	unsigned char rxframebuffer[SIZE_RXFRAME] = {0};
	isis_rxs_c__get_frame__from_t rxFrameCmd = {0,0,0, rxframebuffer};

	print_error(isis_rxs_c__get_frame_count(0, &RxCounter));

	printf("\r\nThere are currently %d frames waiting in the RX buffer\r\n", RxCounter);

	while(RxCounter > 0)
	{
		print_error(isis_rxs_c__get_frame(0, &rxFrameCmd));

		printf("Size of the frame is = %d \r\n", rxFrameCmd.length);

		printf("Frequency offset (Doppler) for received frame: %.2f Hz\r\n", ((double)rxFrameCmd.doppler) * -38.1469726563); // Only valid for rev. B & C boards
		printf("Received signal strength (RSSI) for received frame: %.2f dBm\r\n", ((double)rxFrameCmd.rssi) * -0.5 - 28);

		printf("The received frame content is = ");

		for(i = 0; i < rxFrameCmd.length; i++)
		{
			printf("%02x ", rxFrameCmd.data[i]);
		}
		printf("\r\n");

		vTaskDelay(10 / portTICK_RATE_MS);
		print_error(isis_rxs_c__remove_frame(0));
		vTaskDelay(10 / portTICK_RATE_MS);

		print_error(isis_rxs_c__get_frame_count(0, &RxCounter));

		printf("There are currently %d more frames waiting in the RX buffer\r\n", RxCounter);

		vTaskDelay(10 / portTICK_RATE_MS);
	}

	printf("\r\n END of command frames in buffer \r\n");


	return TRUE;
}


/**
 * Blocks on UART read until a RXS uart frame is read
 */
static Boolean getRXSUARTframe(void)
{
	unsigned char data[_RXS_UART_BUFFERSIZE];
	int error, i, pilot_byte_location;

	//Task to setup the incoming UART, and triggers the TCPoll tasks if ready
	if(UART_start(RXS_UARTBUS, uartBusRXS))
	{
		//UART Buffer failed to initialize?
		TRACE_ERROR("UART_start failed to initialize");
	}

	error = UART_read(RXS_UARTBUS, data, _RXS_UART_BUFFERSIZE);

	if(error != 0)
	{
		TRACE_ERROR("Error on UART read: %d", error);
	}
	else
	{
		//Determine valid frame size. In this case we just accept what ever
		if(0)
		{
			TRACE_ERROR("Received Frame of Invalid Size!");
		}
		else
		{
			pilot_byte_location = -1;
			for(i = 0; i < _RXS_UART_BUFFERSIZE; i++)
			{
				//Example finding the pilot byte for a valid frame
				//This is used to ensure the alignment of the data
				if(data[i] == _RXS_UART_PILOT_BYTE && pilot_byte_location < 0)
				{
					pilot_byte_location = i;
					break;
				}
			}
			if(pilot_byte_location >= 0)
			{
				/*
				 * Incoming frame structure:
				 * pilot_byte_location - offset to start frame
				 * Pilot Byte - 1 byte
				 * Length - 2 bytes
				 * doppler - 2 bytes
				 * rssi - 2 bytes
				 *
				 * data - length bytes
				 *
				 * crc16 - 2 bytes
				 */
				isis_rxs_c__get_frame__from_t *raw_frame_ptr = (void*)&data[pilot_byte_location+1];

				//Length of frame, plus header, plus size of the CRC check to ensure we dont overrun.
				//Checking this here, to ensure we dont overrun the buffer in case of bad frame
				//pilot_byte location + 7 (header inc pilot) + length of frame + size of CRC
				if(pilot_byte_location + 7 + raw_frame_ptr->length + 2 > _RXS_UART_BUFFERSIZE)
				{
					printf("\r\nSize too large, skipping printout\r\n");
					return TRUE;
				}

				printf("\r\n\t Frame Length: %hu", raw_frame_ptr->length);
				printf("\r\n\t Frame Doppler: %d", (int)raw_frame_ptr->doppler);
				printf("\r\n\t Frame rssi: %d\r\n", (int)raw_frame_ptr->rssi);

				for(i = 0; i < raw_frame_ptr->length; i++)
				{
					//Data starts pilot byte+1, plus size of the header data+1+6=7
					printf(" (0x%02X) ", data[i+pilot_byte_location+7]);

					if((i+1)%10 == 0)
					{
						printf("\r\n");
					}
				}

				//Check the CRC
				uint16_t uartframe_crc;

				// Extract CRC at the end of the frame
				memcpy(&uartframe_crc, &data[pilot_byte_location + 7 + raw_frame_ptr->length], sizeof(uartframe_crc));
				uartframe_crc = (uartframe_crc >> 8) | (uartframe_crc << 8); // swap bytes, this CRC16 is MSB

				if(uartframe_crc == checksum_calculateCRC16LUT((void*)&data[pilot_byte_location+1], raw_frame_ptr->length + 6, LUT_TCCRC, CRC16_DEFAULT_STARTREMAINDER, TRUE))
				{
					printf("\r\nCRC check passed:  (0x%04X)\r\n", uartframe_crc);
				}
				else
				{
					printf("\r\nCRC check failed: (0x%04X)\r\n", uartframe_crc);
				}
			}
		}
	}

	return TRUE;
}

static Boolean selectAndExecuteRXSDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 1) Soft Reset RXS \n\r");
	printf("\t 2) Hard Reset RXS \n\r");
	printf("\t 3) Get RXS telemetry \n\r");
	printf("\t 4) Get SW info test \n\r");
	printf("\t 5) Get uptime \n\r");
	printf("\t 6) Get frame count \n\r");
	printf("\t 7) Remove All Frames Test \n\r");
	printf("\t 8) Change bitrate test (cycles through the bitrates) \n\r");
	printf("\t 9) Get command frame over I2C \n\r");
	printf("\t 10) Get command frame over UART (This is blocking) \r\n");
	printf("\t 11) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 11) == 0);

	switch(selection) {
	case 1:
		offerMoreTests = softResetRXSTest();
		break;
	case 2:
		offerMoreTests = powerCycleRXSTest();
		break;
	case 3:
		offerMoreTests = getRXSTelemetryTest();
		break;
	case 4:
		offerMoreTests = getRXSSWInfoTest();
		break;
	case 5:
		offerMoreTests = getRXSUptimeTest();
		break;
	case 6:
		offerMoreTests = getFrameCountTest();
		break;
	case 7:
		offerMoreTests = removeAllFramesTest();
		break;
	case 8:
		offerMoreTests = changeBitRateTest();
		break;
	case 9:
		offerMoreTests = getRXSframe();
		break;
	case 10:
		offerMoreTests = getRXSUARTframe();
		break;
	case 11:
		offerMoreTests = FALSE;
		break;

	default:
		break;
	}

	return offerMoreTests;
}

static void _RXSWatchDogKickTask(void *parameters)
{
	(void)parameters;
	// Kick radio I2C watchdog by requesting uptime every 10 seconds
	portTickType xLastWakeTime = xTaskGetTickCount ();
	for(;;)
	{
	    uint32_t uptime;
		(void)isis_rxs_c__rx_uptime(0, &uptime);
		vTaskDelayUntil(&xLastWakeTime,10000);
	}
}

Boolean IsisRXSrevCdemoInit(void)
{
    // Definition of I2C and RXS
	ISIS_RXS_C_t myRXS[1];
    int rv;

	//I2C addresses defined
    myRXS[0].i2cAddr = I2CADDR_RXS_RX;

	//Buffer definition
    myRXS[0].maxReceiveBufferLength = SIZE_RXFRAME;

	//Initialize the trxvu subsystem
	rv = ISIS_RXS_C_Init(myRXS, 1);
	if(rv != driver_error_none && rv != driver_error_reinit)
	{
		// we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
		// hanging the code
		TRACE_ERROR("\n\r IsisRXS_initialize() failed; err=%d! Exiting ... \n\r", rv);
		return FALSE;
	}

	checksum_prepareLUTCRC16(CRC16_POLYNOMIAL, LUT_TCCRC);

	// Start watchdog kick task
	xTaskCreate( _RXSWatchDogKickTask,(signed char*)"WDT_RXS", 2048, NULL, tskIDLE_PRIORITY, &watchdogKickTaskHandle );

	return TRUE;
}

void IsisRXSrevCdemoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteRXSDemoTest();	// show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)							// was exit/back selected?
		{
			break;
		}
	}
}

Boolean IsisRXSrevCdemoMain(void)
{
	if(IsisRXSrevCdemoInit())									// initialize of I2C and IsisRXS subsystem drivers succeeded?
	{
		IsisRXSrevCdemoLoop();								// show the main IsisRXS demo interface and wait for user input
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

Boolean RXSrevCtest(void)
{
	IsisRXSrevCdemoMain();
	return TRUE;
}


