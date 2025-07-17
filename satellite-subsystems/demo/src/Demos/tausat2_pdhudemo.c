#include "tausat2_pdhudemo.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <hal/supervisor.h>
#include <hal/Drivers/I2C.h>
#include <hal/Utility/util.h>

#include <satellite-subsystems/isis_hstxs_spi.h>

static const unsigned char pdhu_i2caddr = 0x13;
static const unsigned char pdhusup_i2caddr = 0x14;
static const portTickType pdhu_wrdelay = (10 / portTICK_RATE_MS);

static Boolean PdhuDemoGetSupervisorTest(void)
{
	supervisor_housekeeping_t houseKeeping;

	Supervisor_getHousekeeping(&houseKeeping, 0);

	printf("Supervisor Uptime       : %03d:%02d:%02d \n\r", (int)(houseKeeping.fields.supervisorUptime / 3600), (int)(houseKeeping.fields.supervisorUptime % 3600) / 60, (int)(houseKeeping.fields.supervisorUptime % 3600) % 60);
	printf("IOBC Uptime             : %03d:%02d:%02d \n\r", (int)(houseKeeping.fields.iobcUptime / 3600), (int)(houseKeeping.fields.iobcUptime % 3600) / 60, (int)(houseKeeping.fields.iobcUptime % 3600) % 60);
	printf("IOBC Power Cycle Count  : %d \n\r", (int)(houseKeeping.fields.iobcResetCount));

	supervisor_enable_status_t* temporaryEnable = &(houseKeeping.fields.enableStatus);
	printf("\n\r Supervisor Enable Status \n\r");
	printf("Power OBC               : %d \n\r", temporaryEnable->fields.powerObc);
	printf("Power RTC               : %d \n\r", temporaryEnable->fields.powerRtc);
	printf("Is in Supervisor Mode   : %d \n\r", temporaryEnable->fields.isInSupervisorMode);
	printf("Busy RTC                : %d \n\r", temporaryEnable->fields.busyRtc);
	printf("Power off RTC           : %d \n\r", temporaryEnable->fields.poweroffRtc);

	int16_t adcValue[SUPERVISOR_NUMBER_OF_ADC_CHANNELS] = {0};
	Supervisor_calculateAdcValues(&houseKeeping, adcValue);
	printf("\n\r Analog to Digital Channels [Update Flag: 0x%02X] \n\r", houseKeeping.fields.adcUpdateFlag);
	printf("_temperature_measurement: %04d | %d C \n\r\n\r", houseKeeping.fields.adcData[_temperature_measurement], adcValue[_temperature_measurement]);

	printf("_voltage_measurement_3v3in: %04d | %d mV \n\r", houseKeeping.fields.adcData[_voltage_measurement_3v3in], adcValue[_voltage_measurement_3v3in]);
	printf("_voltage_reference_2v5    : %04d | %d mV \n\r", houseKeeping.fields.adcData[_voltage_reference_2v5], adcValue[_voltage_reference_2v5]);
	printf("_voltage_measurement_rtc  : %04d | %d mV \n\r", houseKeeping.fields.adcData[_voltage_measurement_rtc], adcValue[_voltage_measurement_rtc]);
	printf("_voltage_measurement_3v3  : %04d | %d mV \n\r", houseKeeping.fields.adcData[_voltage_measurement_3v3], adcValue[_voltage_measurement_3v3]);
	printf("_voltage_measurement_1v8  : %04d | %d mV \n\r", houseKeeping.fields.adcData[_voltage_measurement_1v8], adcValue[_voltage_measurement_1v8]);
	printf("_voltage_measurement_1v0  : %04d | %d mV \n\r\n\r", houseKeeping.fields.adcData[_voltage_measurement_1v0], adcValue[_voltage_measurement_1v0]);

	printf("_current_measurement_3v3  : %04d | %d mA \n\r", houseKeeping.fields.adcData[_current_measurement_3v3], adcValue[_current_measurement_3v3]);
	printf("_current_measurement_1v8  : %04d | %d mA \n\r", houseKeeping.fields.adcData[_current_measurement_1v8], adcValue[_current_measurement_1v8]);
	printf("_current_measurement_1v0  : %04d | %d mA \n\r\n\r", houseKeeping.fields.adcData[_current_measurement_1v0], adcValue[_current_measurement_1v0]);

	return TRUE;
}

static Boolean PdhuDemoResetTest(void)
{
	I2Ctransfer transfer;
	unsigned char writedata[1] = {0xAA};

	transfer.slaveAddress = pdhu_i2caddr;
	transfer.writeSize = sizeof(writedata);
	transfer.writeData = writedata;
	transfer.readData = NULL;
	transfer.readSize = 0;
	transfer.writeReadDelay = pdhu_wrdelay;

	I2C_writeRead(&transfer);

	return TRUE;
}

typedef struct __attribute__ ((__packed__))
{
    unsigned int uptime;
    unsigned int free_ram_kb;
} firstproject_telemetry;

static Boolean PdhuDemoTelemetryTest(void)
{
	I2Ctransfer transfer;
	unsigned char writedata[1] = {0xBB};
	unsigned char readdata[8] = {0};

	firstproject_telemetry* telemetry = (firstproject_telemetry*)(&readdata[0]);

	transfer.slaveAddress = pdhu_i2caddr;
	transfer.writeSize = sizeof(writedata);
	transfer.writeData = writedata;
	transfer.readData = readdata;
	transfer.readSize = sizeof(readdata);
	transfer.writeReadDelay = pdhu_wrdelay;

	I2C_writeRead(&transfer);

	printf("PDHU uptime: %d seconds\n\r", telemetry->uptime);
	printf("PDHU free mem: %d kB\n\r", telemetry->free_ram_kb);

	return TRUE;
}

static Boolean PdhuDemoFrameTest(void)
{
	I2Ctransfer transfer;
	int i, j;

	unsigned int cmdsize = 1 + sizeof(isis_hstxs_spi__sendframe__to_t);

	unsigned char writedata[cmdsize];
	unsigned int errorcode;

	writedata[0] = 0xCC;
	isis_hstxs_spi__sendframe__to_t* frame = (isis_hstxs_spi__sendframe__to_t*)(&writedata[1]);

	for(i = 1; i <= 7; i++)
	{
		for(j = 0; j < 217; j++)
		{
			frame->fields.data[j] = i + 2 * j;
		}

		frame->fields.header.fields.virtual_channel = i;

		transfer.slaveAddress = pdhu_i2caddr;
		transfer.writeSize = cmdsize;
		transfer.writeData = writedata;
		transfer.readData = NULL;
		transfer.readSize = 0;
		transfer.writeReadDelay = pdhu_wrdelay;

		errorcode = I2C_writeRead(&transfer);
		vTaskDelay(100 / portTICK_RATE_MS);

		(void)errorcode;
	}

	return TRUE;
}

static Boolean PdhuDemoGenerateTest(void)
{
	I2Ctransfer transfer;
	int errorcode;
	unsigned int cmdsize = 1;
	unsigned char writedata[cmdsize];

	writedata[0] = 0xDD;

	transfer.slaveAddress = pdhu_i2caddr;
	transfer.writeSize = cmdsize;
	transfer.writeData = writedata;
	transfer.readData = NULL;
	transfer.readSize = 0;
	transfer.writeReadDelay = pdhu_wrdelay;

	errorcode = I2C_writeRead(&transfer);
	vTaskDelay(10 / portTICK_RATE_MS);

	(void)errorcode;

	return TRUE;
}

static Boolean selectAndExecuteTAUSAT2PdhuDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform:\n\r");
	printf("\t 1) Get PDHU supervisor telemetry\n\r");
	printf("\t 2) Soft Reset PDHU\n\r");
	printf("\t 3) Get PDHU telemetry\n\r");
	printf("\t 4) Send PDHU->TXS frames\n\r");
	printf("\t 5) Generate TXS frames on PDHU\n\r");
	printf("\t 6) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 6) == 0);

	switch(selection)
	{
		case 1:
			offerMoreTests = PdhuDemoGetSupervisorTest();
			break;

		case 2:
			offerMoreTests = PdhuDemoResetTest();
			break;

		case 3:
			offerMoreTests = PdhuDemoTelemetryTest();
			break;

		case 4:
			offerMoreTests = PdhuDemoFrameTest();
			break;

		case 5:
			offerMoreTests = PdhuDemoGenerateTest();
			break;

		case 6:
			offerMoreTests = FALSE;
			break;

		default:
			break;
	}

	return offerMoreTests;
}

Boolean TAUSAT2PdhuDemoInit(void)
{
	uint8_t pdhusup_i2c[1] = {pdhusup_i2caddr};
	Supervisor_start(pdhusup_i2c, 1);

    return TRUE;
}

void TAUSAT2PdhuDemoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteTAUSAT2PdhuDemoTest(); // show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)  // was exit/back
		{
			break;
		}
	}
}

Boolean TAUSAT2PdhuDemoMain(void)
{
	if(TAUSAT2PdhuDemoInit())                                 // initialize of I2C and IsisTRXVU subsystem drivers succeeded?
	{
		TAUSAT2PdhuDemoLoop();                                // show the main IsisTRXVU demo interface and wait for user input
		return TRUE;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}
