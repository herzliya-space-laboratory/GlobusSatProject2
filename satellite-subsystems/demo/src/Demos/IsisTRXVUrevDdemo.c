/*
 * IsisTRXVUdemo.c
 *
 *  Created on: 6 feb. 2015
 *      Author: malv
 */

#include "common.h"
#include "trxvu_frame_ready.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/peripherals/dbgu/dbgu.h>

#include <hal/Utility/util.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/LED.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <satellite-subsystems/isis_vu_d.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if USING_GOM_EPS == 1
#include <SatelliteSubsystems/GomEPS.h>
#endif

////General Variables
#define TX_UPBOUND				30
#define TIMEOUT_UPBOUND			10

#define SIZE_RXFRAME	30
#define SIZE_TXFRAME	235

static xTaskHandle watchdogKickTaskHandle = NULL;
static xSemaphoreHandle trxvuInterruptTrigger = NULL;

// Test Function
static Boolean softResetVUTest(void)
{
	printf("\r\n Soft Reset of both receiver and transmitter microcontrollers \r\n");
	print_error(isis_vu_d__reset_sw_rx(0));
	vTaskDelay(1 / portTICK_RATE_MS);
	print_error(isis_vu_d__reset_sw_tx(0));

	return TRUE;
}

static Boolean hardResetVUTest(void)
{
	printf("\r\n Hard Reset of both receiver and transmitter microcontrollers \r\n");
	print_error(isis_vu_d__reset_hw_rx(0));
	vTaskDelay(1 / portTICK_RATE_MS);
	print_error(isis_vu_d__reset_sw_tx(0));

	return TRUE;
}

static Boolean vutc_sendDefClSignTest(void)
{
	//Buffers and variables definition
	unsigned char testBuffer1[10]  = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x40};
	unsigned char txCounter = 0;
	unsigned char avalFrames = 0;
	unsigned int timeoutCounter = 0;

	while(txCounter < 5 && timeoutCounter < 5)
	{
		printf("\r\n Transmission of single buffers with default callsign. AX25 Format. \r\n");
		print_error(isis_vu_d__send_frame(0, testBuffer1, 10, &avalFrames));

		if ((avalFrames != 0)&&(avalFrames != 255))
		{
			printf("\r\n Number of frames in the buffer: %d  \r\n", avalFrames);
			txCounter++;
		}
		else
		{
			vTaskDelay(100 / portTICK_RATE_MS);
			timeoutCounter++;
		}
	}

	return TRUE;
}

static Boolean vutc_toggleIdleStateTest(void)
{
	static Boolean toggle_flag = 0;

	if(toggle_flag)
	{
	    print_error(isis_vu_d__set_idle_state(0, isis_vu_d__onoff__off));
		toggle_flag = FALSE;
	}
	else
	{
	    print_error(isis_vu_d__set_idle_state(0, isis_vu_d__onoff__on));
		toggle_flag = TRUE;
	}

	return TRUE;
}

static Boolean vutc_setTxBitrate9600Test(void)
{
    print_error(isis_vu_d__set_bitrate(0, isis_vu_d__bitrate__9600bps));

	return TRUE;
}

static Boolean vutc_setTxBitrate1200Test(void)
{
    print_error(isis_vu_d__set_bitrate(0, isis_vu_d__bitrate__1200bps));

	return TRUE;
}

static Boolean vurc_getFrameCountTest(void)
{
	unsigned short RxCounter = 0;
	unsigned int timeoutCounter = 0;

	while(timeoutCounter < 4*TIMEOUT_UPBOUND)
	{
	    print_error(isis_vu_d__get_frame_count(0, &RxCounter));

		timeoutCounter++;

		vTaskDelay(10 / portTICK_RATE_MS);
	}
	printf("\r\n There are currently %d frames waiting in the RX buffer \r\n", RxCounter);

	return TRUE;
}

static Boolean vurc_getFrameCmdTest(void)
{
	unsigned short RxCounter = 0;
	unsigned int i = 0;
	unsigned char rxframebuffer[SIZE_RXFRAME] = {0};
	isis_vu_d__get_frame__from_t rxFrameCmd = {0,0,0, rxframebuffer};

	print_error(isis_vu_d__get_frame_count(0, &RxCounter));

	printf("\r\nThere are currently %d frames waiting in the RX buffer\r\n", RxCounter);

	while(RxCounter > 0)
	{
		print_error(isis_vu_d__get_frame(0, &rxFrameCmd));

		printf("Size of the frame is = %d \r\n", rxFrameCmd.length);

		printf("Frequency offset (Doppler) for received frame: %.2f Hz\r\n", ((double)rxFrameCmd.doppler) * 13.352 - 22300.0); // Only valid for rev. B & C boards
		printf("Received signal strength (RSSI) for received frame: %.2f dBm\r\n", ((double)rxFrameCmd.rssi) * 0.03 - 152);

		printf("The received frame content is = ");

		for(i = 0; i < rxFrameCmd.length; i++)
		{
			printf("%02x ", rxFrameCmd.data[i]);
		}
		printf("\r\n");

		vTaskDelay(10 / portTICK_RATE_MS);
		print_error(isis_vu_d__remove_frame(0));
		vTaskDelay(10 / portTICK_RATE_MS);

		print_error(isis_vu_d__get_frame_count(0, &RxCounter));

		printf("There are currently %d more frames waiting in the RX buffer\r\n", RxCounter);

		vTaskDelay(10 / portTICK_RATE_MS);
	}

	return TRUE;
}

static Boolean vurc_getFrameCmdAndTxTest(void)
{
    unsigned char avalFrames = 0;
	unsigned short RxCounter = 0;
	unsigned int i = 0;
	unsigned char rxframebuffer[SIZE_RXFRAME] = {0};
	isis_vu_d__get_frame__from_t rxFrameCmd = {0,0,0, rxframebuffer};

	print_error(isis_vu_d__get_frame_count(0, &RxCounter));

	printf("\r\nThere are currently %d frames waiting in the RX buffer\r\n", RxCounter);

	while(RxCounter > 0)
	{
		print_error(isis_vu_d__get_frame(0, &rxFrameCmd));

		printf("Size of the frame is = %d \r\n", rxFrameCmd.length);

		printf("Frequency offset (Doppler) for received frame: %.2f Hz\r\n", ((double)rxFrameCmd.doppler) * 13.352 - 22300.0); // Only valid for rev. B & C boards
		printf("Received signal strength (RSSI) for received frame: %.2f dBm\r\n", ((double)rxFrameCmd.rssi) * 0.03 - 152);

		rxframebuffer[26] = '-';
		rxframebuffer[27] = 'O';
		rxframebuffer[28] = 'B';
		rxframebuffer[29] = 'C';

		isis_vu_d__send_frame(0, rxframebuffer, SIZE_RXFRAME, &avalFrames);

		printf("The received frame content is = ");

		for(i = 0; i < rxFrameCmd.length; i++)
		{
			printf("%02x ", rxFrameCmd.data[i]);
		}
		printf("\r\n");
		vTaskDelay(10 / portTICK_RATE_MS);
		print_error(isis_vu_d__remove_frame(0));
		vTaskDelay(10 / portTICK_RATE_MS);

		print_error(isis_vu_d__get_frame_count(0, &RxCounter));

		vTaskDelay(10 / portTICK_RATE_MS);
	}

	return TRUE;
}

static void vurc_revDInterruptCallback()
{
    portBASE_TYPE higherPriorityTaskWoken = pdFALSE;
    //Because the callback is from an interrupt context, we need to use FromISR
    xSemaphoreGiveFromISR(trxvuInterruptTrigger, &higherPriorityTaskWoken);
    //This forces a context switch to a now woken task
    // This should improve the response time for incoming packets
    if (higherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

static Boolean vurc_getFrameCmdInterruptTest(void)
{
    //Using a binary semaphore for syncronization between the interrupt and this task
    vSemaphoreCreateBinary(trxvuInterruptTrigger);
    //Create will do a give by itself. We need to counteract that
    xSemaphoreTake(trxvuInterruptTrigger, 0);
    //Enable the interrupt by giving it the callback we want to use
    TRXVU_FR_Enable(vurc_revDInterruptCallback);

    unsigned short RxCounter = 0;
    unsigned int i = 0;
    unsigned char rxframebuffer[SIZE_RXFRAME] = {0};
    isis_vu_d__get_frame__from_t rxFrameCmd = {0,0,0, rxframebuffer};
    Boolean continueRunning = TRUE;

    //Using a do-while loop here to ensure we empty the TRXVU of frames before
    // we wait for interrupts
    do
    {
        printf("\r\nFrame ready pin = %d\r\n", TRXVU_FR_IsReady());

        print_error(isis_vu_d__get_frame_count(0, &RxCounter));

        printf("There are currently %d frames waiting in the RX buffer\r\n", RxCounter);

        for (; RxCounter > 0; RxCounter--)
        {
            print_error(isis_vu_d__get_frame(0, &rxFrameCmd));

            printf("Size of the frame is = %d \r\n", rxFrameCmd.length);

            printf("Frequency offset (Doppler) for received frame: %.2f Hz\r\n", ((double)rxFrameCmd.doppler) * 13.352 - 22300.0); // Only valid for rev. B & C boards
            printf("Received signal strength (RSSI) for received frame: %.2f dBm\r\n", ((double)rxFrameCmd.rssi) * 0.03 - 152);

            printf("The received frame content is = ");

            for(i = 0; i < rxFrameCmd.length; i++)
            {
                printf("%02x ", rxFrameCmd.data[i]);
            }
            printf("\r\n");
        }
        //Ensure some time is passing after last frame retrieve to ensure
        // the frame ready pin has sufficient time to go low
        // The wait requirement is about 0.5ms. To ensure that we actually need
        // to use 2ms as the argument here. That will result in 1-2ms wait
        vTaskDelay(2 / portTICK_RATE_MS);

        //In some cases, new frames arriving while processing the existing ones
        // This will essentially emulate the interrupt occurring again
        if(TRXVU_FR_IsReady())
        {
            xSemaphoreGive(trxvuInterruptTrigger);
        }

        //Check for a new interrupt, and letting it timeout every 10th second.
        // Blocking using portMAX_DELAY is possible, but it is not recommended
        // to rely solely on the pin and interrupts. For demo purposes we do
        // however rely on the pin and the interrupts, and instead check for
        // user input in order to be able to escape the demo
        printf("Press ESC if you want to stop the loop (you may have to wait 10 second)\r\n");
        do
        {
            if (DBGU_IsRxReady() && DBGU_GetChar() == 0x1B)
            {
                continueRunning = FALSE;
                printf("Exiting the demo\r\n");
            }
        }
        while (continueRunning
                && xSemaphoreTake(trxvuInterruptTrigger, 10000 / portTICK_RATE_MS) == pdFALSE);
    }
    while(continueRunning);

    TRXVU_FR_Disable();
    vSemaphoreDelete(trxvuInterruptTrigger);

    return TRUE;
}

static Boolean vurc_getRxTelemTest_revD(void)
{
	unsigned short telemetryValue;
	float eng_value = 0.0;
	uint32_t uptime = 0;
	isis_vu_d__get_rx_telemetry__from_t telemetry;
	int rv;

	// Telemetry values are presented as raw values
	printf("\r\nGet all Telemetry at once in raw values \r\n\r\n");
	rv = isis_vu_d__get_rx_telemetry(0, &telemetry);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	rv = isis_vu_d__rx_uptime(0, &uptime);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}
	printf("Receiver uptime = %lu s\r\n", uptime);

	telemetryValue = telemetry.fields.doppler;
	eng_value = ((float)telemetryValue) * 13.352 - 22300;
	printf("Receiver doppler = %f Hz\r\n", eng_value);

	telemetryValue = telemetry.fields.rssi;
	eng_value = ((float)telemetryValue) * 0.03 - 152;
	printf("Receiver RSSI = %f dBm\r\n", eng_value);

	telemetryValue = telemetry.fields.voltage;
	eng_value = ((float)telemetryValue) * 0.00488;
	printf("Bus voltage = %f V\r\n", eng_value);

	telemetryValue = telemetry.fields.current_total;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Total current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.current_tx;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Transmitter current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.current_rx;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Receiver current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.current_pa;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("PA current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.temp_pa;
	eng_value = ((float)telemetryValue) * -0.07669 + 195.6037;
	printf("PA temperature = %f deg. C\r\n", eng_value);

	telemetryValue = telemetry.fields.temp_board;
	eng_value = ((float)telemetryValue) * -0.07669 + 195.6037;
	printf("Board temperature = %f deg. C\r\n", eng_value);

	return TRUE;
}

static Boolean vutc_getTxTelemTest_revD(void)
{
	unsigned short telemetryValue;
	float eng_value = 0.0;
	uint32_t uptime = 0;
	isis_vu_d__get_tx_telemetry__from_t telemetry;
	int rv;

	// Telemetry values are presented as raw values
	printf("\r\nGet all Telemetry at once in raw values \r\n\r\n");
	rv = isis_vu_d__get_tx_telemetry(0, &telemetry);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	rv = isis_vu_d__tx_uptime(0, &uptime);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}
	printf("Transmitter uptime = %lu s\r\n", uptime);

	telemetryValue = telemetry.fields.reflected_power;
	eng_value = ((float)(telemetryValue * telemetryValue)) * 5.887E-5;
	printf("RF reflected power = %f mW\r\n", eng_value);

	telemetryValue = telemetry.fields.forward_power;
	eng_value = ((float)(telemetryValue * telemetryValue)) * 5.887E-5;
	printf("RF forward power = %f mW\r\n", eng_value);

	telemetryValue = telemetry.fields.voltage;
	eng_value = ((float)telemetryValue) * 0.00488;
	printf("Bus voltage = %f V\r\n", eng_value);

	telemetryValue = telemetry.fields.current_total;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Total current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.current_tx;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Transmitter current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.current_rx;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Receiver current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.current_pa;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("PA current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.temp_pa;
	eng_value = ((float)telemetryValue) * -0.07669 + 195.6037;
	printf("PA temperature = %f deg. C\r\n", eng_value);

	telemetryValue = telemetry.fields.temp_board;
	eng_value = ((float)telemetryValue) * -0.07669 + 195.6037;
	printf("Board temperature = %f deg. C\r\n", eng_value);

	return TRUE;
}


static void _WatchDogKickTask(void *parameters)
{
    (void)parameters;
    // Kick radio I2C watchdog by requesting uptime every 10 seconds
    portTickType xLastWakeTime = xTaskGetTickCount ();
    for(;;)
    {
        (void)isis_vu_d__reset_wdg_rx(0);
        (void)isis_vu_d__reset_wdg_tx(0);
        vTaskDelayUntil(&xLastWakeTime,10000);
	}
}

static Boolean _start_WDT_kick(void)
{
	if(watchdogKickTaskHandle == NULL)
	{
		// Start watchdog kick task
		printf("\n\rWDT Task started.\n\r");
		xTaskCreate( _WatchDogKickTask,(signed char*)"WDT", 2048, NULL, tskIDLE_PRIORITY, &watchdogKickTaskHandle );
	}
	else
	{
		printf("\n\rWDT Task already started.\n\r");
	}

	return TRUE;
}

static Boolean selectAndExecuteTRXVUDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 1) Soft Reset TRXVU both microcontrollers \n\r");
	printf("\t 2) Hard Reset TRXVU both microcontrollers \n\r");
	printf("\t 3) Default Callsign Send Test\n\r");
	printf("\t 4) Toggle Idle state \n\r");
	printf("\t 5) Change transmission bitrate to 9600  \n\r");
	printf("\t 6) Change transmission bitrate to 1200 \n\r");
	printf("\t 7) Get frame count \n\r");
	printf("\t 8) Get command frame \n\r");
	printf("\t 9) Get command frame and retransmit \n\r");
	printf("\t 10) Get command frame by interrupt \n\r");
	printf("\t 11) Get receiver telemetry \n\r");
	printf("\t 12) Get transmitter telemetry \n\r");
	printf("\t 13) Start a watchdog kick task. This cannot be stopped without reboot. \n");
	printf("\t 14) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 14) == 0);

	switch(selection) {
	case 1:
		offerMoreTests = softResetVUTest();
		break;
	case 2:
		offerMoreTests = hardResetVUTest();
		break;
	case 3:
		offerMoreTests = vutc_sendDefClSignTest();
		break;
	case 4:
		offerMoreTests = vutc_toggleIdleStateTest();
		break;
	case 5:
		offerMoreTests = vutc_setTxBitrate9600Test();
		break;
	case 6:
		offerMoreTests = vutc_setTxBitrate1200Test();
		break;
	case 7:
		offerMoreTests = vurc_getFrameCountTest();
		break;
	case 8:
		offerMoreTests = vurc_getFrameCmdTest();
		break;
	case 9:
		offerMoreTests = vurc_getFrameCmdAndTxTest();
		break;
    case 10:
        offerMoreTests = vurc_getFrameCmdInterruptTest();
        break;
	case 11:
		offerMoreTests = vurc_getRxTelemTest_revD();
		break;
	case 12:
		offerMoreTests = vutc_getTxTelemTest_revD();
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

Boolean IsisTRXVUrevDdemoInit(void)
{
    // Definition of I2C and TRXUV
    ISIS_VU_D_t myTRXVU[1];
    int rv;

	//I2C addresses defined
    myTRXVU[0].rxAddr = 0x60;
    myTRXVU[0].txAddr = 0x61;

	//Buffer definition
    myTRXVU[0].maxSendBufferLength = SIZE_TXFRAME;
    myTRXVU[0].maxReceiveBufferLength = SIZE_RXFRAME;

	//Initialize the trxvu subsystem
	rv = ISIS_VU_D_Init(myTRXVU, 1);
	if(rv != driver_error_none && rv != driver_error_reinit)
	{
		// we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
		// hanging the code
		TRACE_ERROR("\n\r IsisTrxvu_initialize() failed; err=%d! Exiting ... \n\r", rv);
		return FALSE;
	}

	return TRUE;
}

void IsisTRXVUrevDdemoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteTRXVUDemoTest();	// show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)							// was exit/back selected?
		{
			break;
		}
	}
}

Boolean IsisTRXVUrevDdemoMain(void)
{
	if(IsisTRXVUrevDdemoInit())									// initialize of I2C and IsisTRXVU subsystem drivers succeeded?
	{
		IsisTRXVUrevDdemoLoop();								// show the main IsisTRXVU demo interface and wait for user input
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

Boolean TRXVUrevDtest(void)
{
	IsisTRXVUrevDdemoMain();
	return TRUE;
}
