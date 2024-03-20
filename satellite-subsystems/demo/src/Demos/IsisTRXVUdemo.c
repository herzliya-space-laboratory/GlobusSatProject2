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

#include <hal/Utility/util.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/LED.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisTRXVU.h>

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

#define MICROCHIP_SLAVE 0x61
static xTaskHandle watchdogKickTaskHandle = NULL;
static xSemaphoreHandle trxvuInterruptTrigger = NULL;

// Test Function
static Boolean softResetVUTest(void)
{
	printf("\r\n Soft Reset of both receiver and transmitter microcontrollers \r\n");
	print_error(IsisTrxvu_componentSoftReset(0, trxvu_rc));
	vTaskDelay(1 / portTICK_RATE_MS);
	print_error(IsisTrxvu_componentSoftReset(0, trxvu_tc));

	return TRUE;
}

static Boolean vutc_sendBeacon(void)
{
	unsigned char data[10]  = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	IsisTrxvu_tcSetAx25BeaconDefClSign(0, data, 10, 20);


	return TRUE;
}

static Boolean vutc_stopSendingBeacon(void)
{
	IsisTrxvu_tcClearBeacon(0);
	return TRUE;
}

static Boolean vutc_sendEmptyPacketTest(void)
{
	//Buffers and variables definition
	unsigned char testBuffer1[10]  = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	unsigned char txCounter = 0;
	unsigned char avalFrames = 0;
	unsigned int timeoutCounter = 0;

	while(txCounter < 5 && timeoutCounter < 5)
	{
		printf("\r\n Transmission of single buffers with default callsign. AX25 Format. \r\n");
		print_error(IsisTrxvu_tcSendAX25DefClSign(0, testBuffer1, 10, &avalFrames));

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



static Boolean vutc_sendPacketInsertedByTheUser(void)
{
	//Buffers and variables definition
	unsigned char testBuffer2[10];
	int amountOfRepetitions;
	unsigned char txCounter = 0;
	unsigned char avalFrames = 0;
	int timeoutCounter = 0;
	int i;
	unsigned int temp;
	printf("\r\nEnter buffer: \r\n");
	for(i = 0; i < 10 ; i++)
	{
		if (UTIL_DbguGetHexa32(&temp) == 1)
			testBuffer2[i] = (unsigned char)temp;
		else
			i--;
	}
	printf("\r\nEnter amount of repetitions: \r\n");
	while(UTIL_DbguGetIntegerMinMax(&amountOfRepetitions, 1, 1000) == 0);
	while(txCounter < amountOfRepetitions && timeoutCounter < amountOfRepetitions)
	{
		printf("\r\n Transmission of single buffers with default callsign. AX25 Format. \r\n");
		print_error(IsisTrxvu_tcSendAX25DefClSign(0, testBuffer2, 10, &avalFrames));

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
	    print_error(IsisTrxvu_tcSetIdlestate(0, trxvu_idle_state_off));
		toggle_flag = FALSE;
	}
	else
	{
	    print_error(IsisTrxvu_tcSetIdlestate(0, trxvu_idle_state_on));
		toggle_flag = TRUE;
	}

	return TRUE;
}
static Boolean Get_Uptime(void) {
	unsigned int uptime;

	int error = IsisTrxvu_rcGetUptime(0, &uptime);
	if(!error) printf("\r\n there is an error getting the uptime, %d \r\n", error);
	printf("\r\n the uptime is: %d seconds \r\n", uptime);
	return TRUE;
}

static Boolean vurc_getFrameCountTest(void)
{
	unsigned short RxCounter = 0;
	unsigned int timeoutCounter = 0;

	while(timeoutCounter < 4*TIMEOUT_UPBOUND)
	{
	    print_error(IsisTrxvu_rcGetFrameCount(0, &RxCounter));

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
	ISIStrxvuRxFrame rxFrameCmd = {0,0,0, rxframebuffer};

	print_error(IsisTrxvu_rcGetFrameCount(0, &RxCounter));

	printf("\r\nThere are currently %d frames waiting in the RX buffer\r\n", RxCounter);

	while(RxCounter > 0)
	{
		print_error(IsisTrxvu_rcGetCommandFrame(0, &rxFrameCmd));

		printf("Size of the frame is = %d \r\n", rxFrameCmd.rx_length);

		printf("Frequency offset (Doppler) for received frame: %.2f Hz\r\n", ((double)rxFrameCmd.rx_doppler) * 13.352 - 22300.0); // Only valid for rev. B & C boards
		printf("Received signal strength (RSSI) for received frame: %.2f dBm\r\n", ((double)rxFrameCmd.rx_rssi) * 0.03 - 152);

		printf("The received frame content is = ");

		for(i = 0; i < rxFrameCmd.rx_length; i++)
		{
			printf("%02x ", rxFrameCmd.rx_framedata[i]);
		}
		printf("\r\n");

		print_error(IsisTrxvu_rcGetFrameCount(0, &RxCounter));

		vTaskDelay(10 / portTICK_RATE_MS);
	}

	return TRUE;
}

static Boolean vurc_getFrameCmdAndTxTest(void)
{
	unsigned short RxCounter = 0;
	unsigned int i = 0;
	unsigned char rxframebuffer[SIZE_RXFRAME] = {0};
	unsigned char trxvuBuffer = 0;
	ISIStrxvuRxFrame rxFrameCmd = {0,0,0, rxframebuffer};

	print_error(IsisTrxvu_rcGetFrameCount(0, &RxCounter));

	printf("\r\nThere are currently %d frames waiting in the RX buffer\r\n", RxCounter);

	while(RxCounter > 0)
	{
		print_error(IsisTrxvu_rcGetCommandFrame(0, &rxFrameCmd));

		printf("Size of the frame is = %d \r\n", rxFrameCmd.rx_length);

		printf("Frequency offset (Doppler) for received frame: %.2f Hz\r\n", ((double)rxFrameCmd.rx_doppler) * 13.352 - 22300.0); // Only valid for rev. B & C boards
		printf("Received signal strength (RSSI) for received frame: %.2f dBm\r\n", ((double)rxFrameCmd.rx_rssi) * 0.03 - 152);

		rxframebuffer[26] = '-';
		rxframebuffer[27] = 'O';
		rxframebuffer[28] = 'B';
		rxframebuffer[29] = 'C';

		IsisTrxvu_tcSendAX25DefClSign(0, rxframebuffer, SIZE_RXFRAME, &trxvuBuffer);

		printf("The received frame content is = ");

		for(i = 0; i < rxFrameCmd.rx_length; i++)
		{
			printf("%02x ", rxFrameCmd.rx_framedata[i]);
		}
		printf("\r\n");

		print_error(IsisTrxvu_rcGetFrameCount(0, &RxCounter));

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
    ISIStrxvuRxFrame rxFrameCmd = {0,0,0, rxframebuffer};
    Boolean continueRunning = TRUE;

    //Using a do-while loop here to ensure we empty the TRXVU of frames before
    // we wait for interrupts
    do
    {
        printf("\r\nFrame ready pin = %d\r\n", TRXVU_FR_IsReady());

        print_error(IsisTrxvu_rcGetFrameCount(0, &RxCounter));

        printf("There are currently %d frames waiting in the RX buffer\r\n", RxCounter);

        for (; RxCounter > 0; RxCounter--)
        {
            print_error(IsisTrxvu_rcGetCommandFrame(0, &rxFrameCmd));

            printf("Size of the frame is = %d \r\n", rxFrameCmd.rx_length);

            printf("Frequency offset (Doppler) for received frame: %.2f Hz\r\n", ((double)rxFrameCmd.rx_doppler) * 13.352 - 22300.0); // Only valid for rev. B & C boards
            printf("Received signal strength (RSSI) for received frame: %.2f dBm\r\n", ((double)rxFrameCmd.rx_rssi) * 0.03 - 152);

            printf("The received frame content is = ");

            for(i = 0; i < rxFrameCmd.rx_length; i++)
            {
                printf("%02x ", rxFrameCmd.rx_framedata[i]);
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
	ISIStrxvuRxTelemetry telemetry;
	int rv;

	// Telemetry values are presented as raw values
	printf("\r\nGet all Telemetry at once in raw values \r\n\r\n");
	rv = IsisTrxvu_rcGetTelemetryAll(0, &telemetry);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	telemetryValue = telemetry.fields.rx_doppler;
	eng_value = ((float)telemetryValue) * 13.352 - 22300;
	printf("Receiver doppler = %f Hz\r\n", eng_value);

	telemetryValue = telemetry.fields.rx_rssi;
	eng_value = ((float)telemetryValue) * 0.03 - 152;
	printf("Receiver RSSI = %f dBm\r\n", eng_value);

	telemetryValue = telemetry.fields.bus_volt;
	eng_value = ((float)telemetryValue) * 0.00488;
	printf("Bus voltage = %f V\r\n", eng_value);

	telemetryValue = telemetry.fields.vutotal_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Total current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.vutx_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Transmitter current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.vurx_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Receiver current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.vupa_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("PA current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.pa_temp;
	eng_value = ((float)telemetryValue) * -0.07669 + 195.6037;
	printf("PA temperature = %f degC\r\n", eng_value);

	telemetryValue = telemetry.fields.board_temp;
	eng_value = ((float)telemetryValue) * -0.07669 + 195.6037;
	printf("Local oscillator temperature = %f degC\r\n", eng_value);

	return TRUE;
}

static Boolean vutc_getTxTelemTest_revD(void)
{
	unsigned short telemetryValue;
	float eng_value = 0.0;
	ISIStrxvuTxTelemetry telemetry;
	int rv;

	// Telemetry values are presented as raw values
	printf("\r\nGet all Telemetry at once in raw values \r\n\r\n");
	rv = IsisTrxvu_tcGetTelemetryAll(0, &telemetry);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	telemetryValue = telemetry.fields.tx_reflpwr;
	eng_value = ((float)(telemetryValue * telemetryValue)) * 5.887E-5;
	printf("RF reflected power = %f mW\r\n", eng_value);

	telemetryValue = telemetry.fields.tx_fwrdpwr;
	eng_value = ((float)(telemetryValue * telemetryValue)) * 5.887E-5;
	printf("RF forward power = %f mW\r\n", eng_value);

	telemetryValue = telemetry.fields.bus_volt;
	eng_value = ((float)telemetryValue) * 0.00488;
	printf("Bus voltage = %f V\r\n", eng_value);

	telemetryValue = telemetry.fields.vutotal_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Total current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.vutx_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Transmitter current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.vurx_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Receiver current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.vupa_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("PA current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.pa_temp;
	eng_value = ((float)telemetryValue) * -0.07669 + 195.6037;
	printf("PA temperature = %f degC\r\n", eng_value);

	telemetryValue = telemetry.fields.board_temp;
	eng_value = ((float)telemetryValue) * -0.07669 + 195.6037;
	printf("Local oscillator temperature = %f degC\r\n", eng_value);

	return TRUE;
}

static Boolean TransponderOn()
{
	unsigned char data[] = {0x38, 0x02};
	if(I2C_write(MICROCHIP_SLAVE, data, 2))
		printf("ohhh no. \r\n");
	else
		printf("It work \r\n");

	return TRUE;
}

static Boolean TransponderOff()
{
	unsigned char data[] = {0x38, 0x01};
	if(I2C_write(MICROCHIP_SLAVE, data, 2))
		printf("ohhh no. \r\n");
	else
		printf("It work \r\n");

	return TRUE;
}
static Boolean SetTransponderThreshold(void){
	int input;
	short threshold;
	unsigned char data[] = {0x52, 0,0};
		printf("set the Transponder Threshold range is between 0-4095 \r\n");
		UTIL_DbguGetIntegerMinMax(&input, 0, 4095);
		threshold = (short)input;
		memcpy(data + 1,&threshold, sizeof(threshold));
		if(I2C_write(MICROCHIP_SLAVE, data, 3))
			printf("ohhh no. \r\n");
		else
			printf("It work \r\n");
	return TRUE;
}
static Boolean Get_Tx_Telemetry_Value_Array(void) {
	unsigned short telemetryValue;
	float eng_value = 0.0;
	ISIStrxvuTxTelemetry telemetry;
	int rv;

	// Telemetry values are presented as raw values
	printf("\r\nGet all Telemetry at once in raw values \r\n\r\n");
	rv = IsisTrxvu_tcGetLastTxTelemetry(0, &telemetry);
	if(rv)
	{
		printf("Subsystem call failed. rv = %d", rv);
		return TRUE;
	}

	telemetryValue = telemetry.fields.tx_reflpwr;
	eng_value = ((float)(telemetryValue * telemetryValue)) * 5.887E-5;
	printf("RF reflected power = %f mW\r\n", eng_value);

	telemetryValue = telemetry.fields.tx_fwrdpwr;
	eng_value = ((float)(telemetryValue * telemetryValue)) * 5.887E-5;
	printf("RF forward power = %f mW\r\n", eng_value);

	telemetryValue = telemetry.fields.bus_volt;
	eng_value = ((float)telemetryValue) * 0.00488;
	printf("Bus voltage = %f V\r\n", eng_value);

	telemetryValue = telemetry.fields.vutotal_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Total current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.vutx_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Transmitter current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.vurx_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("Receiver current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.vupa_curr;
	eng_value = ((float)telemetryValue) * 0.16643964;
	printf("PA current = %f mA\r\n", eng_value);

	telemetryValue = telemetry.fields.pa_temp;
	eng_value = ((float)telemetryValue) * -0.07669 + 195.6037;
	printf("PA temperature = %f degC\r\n", eng_value);

	telemetryValue = telemetry.fields.board_temp;
	eng_value = ((float)telemetryValue) * -0.07669 + 195.6037;
	printf("Local oscillator temperature = %f degC\r\n", eng_value);

	return TRUE;
}
static Boolean printTransmitterState() {
	ISIStrxvuTransmitterState currentstate;
	print_error(IsisTrxvu_tcGetState(0, &currentstate));
	switch (currentstate.fields.transmitter_idle_state) {
	        case trxvu_idle_state_off:
            printf("\r\n Transmitter is in idle state OFF\r\n");
            break;
        case trxvu_idle_state_on:
            printf("\r\n Transmitter is in idle state ON\r\n");
            break;
        default:
            printf("\r\n Unknown transmitter idle state\r\n");
            break;
	}
        switch (currentstate.fields.transmitter_beacon) {
        	case trxvu_beacon_none:
        		printf("\r\n Transmitter beacon mode: None\r\n");
       			break;
      		case trxvu_beacon_active:
      			printf("\r\n Transmitter beacon mode: Active\r\n");
       			break;
	       	default:
	   			printf("\r\n Unknown transmitter beacon mode\r\n");
    			break;
	        }
        switch(currentstate.fields.transmitter_bitrate) {
            case trxvu_bitratestatus_1200:
                printf("Transmission Bitrate: 1200 bps\r\n");
                break;
            case trxvu_bitratestatus_2400:
                printf("Transmission Bitrate: 2400 bps\r\n");
                break;
            case trxvu_bitratestatus_4800:
                printf("Transmission Bitrate: 4800 bps\r\n");
                break;
            case trxvu_bitratestatus_9600:
                printf("Transmission Bitrate: 9600 bps\r\n");
                break;
            default:
                printf("Invalid bitrate status\r\n");
                break;
	       		}
	 return TRUE;
}
static Boolean IsisTrxvu_tcEstimateTransmissionTimeTest(void){
	printf("\r\n the current time it takes to send a package size of  8 with a 9600bitrate is %d \r\n", IsisTrxvu_tcEstimateTransmissionTime(8, trxvu_bitrate_9600));
	return TRUE;
}
static Boolean selectAndExecuteTRXVUDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Soft Reset TRXVU both microcontrollers \n\r");
	printf("\t 2) Empty Callsign Send Test\n\r");
	printf("\t 3) Toggle Idle state \n\r");
	printf("\t 4) Get frame count \n\r");
	printf("\t 5) Get command frame \n\r");
	printf("\t 6) Get command frame and retransmit \n\r");
	printf("\t 7) (revD) Get command frame by interrupt \n\r");
	printf("\t 8) (revD) Get receiver telemetry \n\r");
	printf("\t 9) (revD) Get transmitter telemetry \n\r");
	printf("\t 10) Send packet inserted by the user\n\r");
	printf("\t 11) Get the transponder on \n\r");
	printf("\t 12) Get the transponder off \n\r");
	printf("\t 13) Set Transponder Threshold \n\r");
	printf("\t 14) send beacon every 20 seconds \n\r");
	printf("\t 15) Stop sending beacon \n\r");
	printf("\t 16) get uptime \n\r");
	printf("\t 17) prints Get Tx Telemetry Value Array \n\r");
	printf("\t 18) print Transmitter State \n\r");
	printf("\t 19) checks for estimate time 8 bytes by 9600 bitrate \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 19) == 0);

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = softResetVUTest();
		break;
	case 2:
		offerMoreTests = vutc_sendEmptyPacketTest();
		break;
	case 3:
		offerMoreTests = vutc_toggleIdleStateTest();
		break;
	case 4:
		offerMoreTests = vurc_getFrameCountTest();
		break;
	case 5:
		offerMoreTests = vurc_getFrameCmdTest();
		break;
	case 6:
		offerMoreTests = vurc_getFrameCmdAndTxTest();
		break;
    case 7:
        offerMoreTests = vurc_getFrameCmdInterruptTest();
        break;
	case 8:
		offerMoreTests = vurc_getRxTelemTest_revD();
		break;
	case 9:
		offerMoreTests = vutc_getTxTelemTest_revD();
		break;
	case 10:
		offerMoreTests = vutc_sendPacketInsertedByTheUser();
		break;
	case 11:
		offerMoreTests = TransponderOn();
		break;
	case 12:
		offerMoreTests = TransponderOff();
		break;
	case 13:
		offerMoreTests = SetTransponderThreshold();
		break;
	case 14:
		offerMoreTests = vutc_sendBeacon();
		break;
	case 15:
		offerMoreTests = vutc_stopSendingBeacon();
		break;
	case 16:
		offerMoreTests = Get_Uptime();
		break;
	case 17:
		offerMoreTests = Get_Tx_Telemetry_Value_Array();
		break;
	case 18:
		offerMoreTests = printTransmitterState();
		break;
	case 19:
		offerMoreTests = IsisTrxvu_tcEstimateTransmissionTimeTest();
		break;
	default:
		break;
	}

	return offerMoreTests;
}

static void _WatchDogKickTask(void *parameters)
{
	(void)parameters;
	// Kick radio I2C watchdog by requesting uptime every 10 seconds
	portTickType xLastWakeTime = xTaskGetTickCount ();
	for(;;)
	{
		unsigned int uptime;
		(void)IsisTrxvu_tcGetUptime(0, &uptime);
		vTaskDelayUntil(&xLastWakeTime,10000);
	}
}

Boolean IsisTRXVUdemoInit(void)
{
    // Definition of I2C and TRXVU
	ISIStrxvuI2CAddress myTRXVUAddress[1];
	ISIStrxvuFrameLengths myTRXVUBuffers[1];
	ISIStrxvuBitrate myTRXVUBitrates[1];
    int rv;

	//I2C addresses defined
	myTRXVUAddress[0].addressVu_rc = 0x60;
	myTRXVUAddress[0].addressVu_tc = MICROCHIP_SLAVE;

	//Buffer definition
	myTRXVUBuffers[0].maxAX25frameLengthTX = SIZE_TXFRAME;
	myTRXVUBuffers[0].maxAX25frameLengthRX = SIZE_RXFRAME;

	//Bitrate definition
	myTRXVUBitrates[0] = trxvu_bitrate_1200;

	//Initialize the trxvu subsystem
	rv = IsisTrxvu_initialize(myTRXVUAddress, myTRXVUBuffers, myTRXVUBitrates, 1);
	if(rv != E_NO_SS_ERR && rv != E_IS_INITIALIZED)
	{
		// we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
		// hanging the code
		TRACE_ERROR("\n\r IsisTrxvu_initialize() failed; err=%d! Exiting ... \n\r", rv);
		return FALSE;
	}

	// Start watchdog kick task
	xTaskCreate( _WatchDogKickTask,(signed char*)"WDT", 2048, NULL, tskIDLE_PRIORITY, &watchdogKickTaskHandle );

	return TRUE;
}

void IsisTRXVUdemoLoop(void)
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

Boolean IsisTRXVUdemoMain(void)
{
	if(IsisTRXVUdemoInit())									// initialize of I2C and IsisTRXVU subsystem drivers succeeded?
	{
		IsisTRXVUdemoLoop();								// show the main IsisTRXVU demo interface and wait for user input
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

Boolean TRXVUtest(void)
{
	IsisTRXVUdemoMain();
	return TRUE;
}
