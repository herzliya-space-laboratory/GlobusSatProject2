/*
 * IsisMTQv2demo.c
 *
 *  Created on: 13 mrt. 2015
 *      Author: malv
 */
#include "IsisMTQv2demo.h"

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

#include <satellite-subsystems/isis_mtq_v2.h>

#include <Demos/common.h>

#include <stddef.h>			// for size_t
#include <stdlib.h>
#include <stdio.h>
#include <string.h>			// used for memset/memcpy/strncpy functions
#include <stdbool.h>		// 'normal' boolean values; note: these are incompatible with the ones defined by Boolean.h in the hal library

#include <math.h>			// for the exp function used in the user input parser function ing()
#include <float.h>			// floating point min and max values
#include <stdint.h>			// integer data type min and max values
#include <inttypes.h>		// printf format codes

//#if USING_GOM_EPS == 1
//#include <SatelliteSubsystems/GomEPS.h>
//#endif



///***************************************************************************************************************************
///
/// Implementers notice:
/// This file contains example code that aids building understanding of how to interface with the subsystem
/// using the subsystems library. Validation code has been kept to a minimum in an effort not to obfuscate
/// the code that is needed for calling the subsystem. Be cautious and critical when using portions of this
/// code in your own projects. It is *not* recommended to copy/paste this code into your projects,
/// instead use the example as a guide while building your modules separately.
///
///***************************************************************************************************************************


///***************************************************************************************************************************
///
/// Notes on the parameter system:
/// For getting and resetting a parameter an unsigned short param_id_in is required, while
/// for setting a parameter, a pointer to a structure with a param_id and param_value is expected.
/// All three operations require a pointer to an output structure, in which the reply_header, the
/// param_id, and the actual param_value will be received.
///
/// note that the parameter index consists of: param-type in the highest nibble (4 bits), and the ordinal index in the
/// lower 3 nibbles (12 bits). Optionally the fifth to highest bit of the 16 bits is used to indicate read/only
/// e.g.: 0xA802 = third read-only double, 0xA002 = third read/write double
///
/// example 1: getting param 0x1000 (which is the first int8):
///		unsigned short param_id = 0x1000;
///		isis_mtq_v2__get_parameter__from_t rsp;
///
///		rv = isis_mtq_v2__get_parameter(0, param_id, &rsp);
///
/// example 2: setting param 0xA000 (which is the first 8 byte double):
///		unsigned short param_id = 0xA000;
///     isis_mtq_v2__set_parameter__to_t par_data = { .fields = { .param_id = param_id } }; // byte array for storing the parameter data we want to set; our maximum size is 8 bytes (e.g. double, long ...)
///     isis_mtq_v2__set_parameter__from_t par_data_out;                                    // byte array for storing the parameter data that was actually set; our maximum size is 8 bytes (e.g. double, long ...)
///
///		rv = isis_mtq_v2__set_parameter(0, &par_data, &par_data_out);
///
/// example 3: setting param 0x7003 (which is the fourth 4 byte float) using a (little endian) byte array as input:
///     isis_mtq_v2__set_parameter__to_t par_data = { .fields = { .param_id = 0x7003, .param_value = { 0x10, 0x32, 0x54, 0x76 } } };
///     isis_mtq_v2__set_parameter__from_t par_data_out;
///
///		rv = isis_mtq_v2__set_parameter(0, &par_data, &par_data_out);
///
///***************************************************************************************************************************


///***************************************************************************************************************************
///
/// Demo helper functions:
/// below are helper functions specific to the demo application for interacting with the user and presenting results
///
/// The subsystem calls can be found below!
///
///***************************************************************************************************************************

static void _parse_resp(isis_mtq_v2__replyheader_t* p_rsp_code)
{
	// this function parses the response that is provided as the result of
	// issuing a command to the subsystem. It provides information on whether the response
	// was accepted for processing, not necessarily whether the command succeeded because
	// generally too much processing time is required for executing the command to allow
	// waiting for a response. Generally separate calls can be made to verify successful
	// command execution, usually that call includes any output data gather during command
	// execution.
	// In case of getting measurement results the inva status indicates whether issues
	// were encountered during measurement causing the axis value to become suspect
	if(p_rsp_code == NULL)
	{
		TRACE_ERROR(" internal error: p_rsp_code is NULL");
		return;
	}

	if(p_rsp_code->fields.new_flag)																// is the new flag set?
	{
		printf("   - new_flag = %d (new response/data)\r\n", p_rsp_code->fields.new_flag);			// indicate its a never before retrieved response
	}
	else
	{
		printf("   - new_flag = %d (old response/data)\r\n", p_rsp_code->fields.new_flag);			// indicate we've read this response before
	}

	printf("   - IVA x,y,z = %d, %d, %d \r\n", p_rsp_code->fields.iva_x, p_rsp_code->fields.iva_y,
				p_rsp_code->fields.iva_z); // parse axis invalid markers

	switch(p_rsp_code->fields.cmd_error)
	{
	case isis_mtq_v2__errorcode__accepted: 		///< Accepted
		printf("   - cmd_error = %d (isis_mtq_v2__errorcode__accepted) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case isis_mtq_v2__errorcode__rejected: 		///< Rejected: no reason indicated
		printf("   - cmd_error = %d (!REJECTED! isis_mtq_v2__errorcode__rejected) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case isis_mtq_v2__errorcode__invalid: ///< Rejected: invalid command code
		printf("   - cmd_error = %d (!REJECTED! isis_mtq_v2__errorcode__invalid) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case isis_mtq_v2__errorcode__parmetermissing: ///< Rejected: parameter missing
		printf("   - cmd_error = %d (!REJECTED! isis_mtq_v2__errorcode__parmetermissing) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case isis_mtq_v2__errorcode__parameterinvalid: ///< Rejected: parameter invalid
		printf("   - cmd_error = %d (!REJECTED! isis_mtq_v2__errorcode__parameterinvalid) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case isis_mtq_v2__errorcode__cc_unavailable: ///< Rejected: CC unavailable in current mode
		printf("   - cmd_error = %d (!REJECTED! isis_mtq_v2__errorcode__cc_unavailable) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case isis_mtq_v2__errorcode__reserved: 	   ///< Reserved value
		printf("   - cmd_error = %d (!REJECTED! isis_mtq_v2__errorcode__reserved) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case isis_mtq_v2__errorcode__internalerror: ///< Internal error occurred during processing
		printf("   - cmd_error = %d (!REJECTED! isis_mtq_v2__errorcode__internalerror) \r\n", p_rsp_code->fields.cmd_error);
		break;
	}
}

static void _parse_selftest_step(isis_mtq_v2__selftestdata_t *p_step)
{
	// this function shows the self-test result information for a self-test-step to the user.

	if(p_step == NULL)										// if no valid step pointer is provided we'll only print out generic information
	{
		// only print info on the step data
		printf("   - error = %s \r\n", "error bitflag indicating any problems that the IMTQv2 encountered while performing the self-test-step.");
		printf("   - step = %s \r\n", "self-test-step indicating the coil operation that was being performed at the time the measurements were taken");
		printf("   - raw_mag_(x,y,z) = %s \r\n", "magnetometer field strength in raw counts for x, y, and z");
		printf("   - calibrated_mag_(x,y,z) = %s \r\n", "magnetometer field strength for x, y, and z [nT / 10E-9 T]");
		printf("   - coil_current_(x,y,z) = %s \r\n", "coil current for x, y, and z [10E-4 A]");
		printf("   - coil_temp_(x,y,z) = %s \r\n", "coil temperature for x, y, and z [deg. C]");

		return;
	}

	_parse_resp(&p_step->fields.reply_header);																		// step response information

	printf("   - error = %04x", p_step->fields.error);																// error field information (this is a bitflag field which can have multiple flags set at the same time)
	if(p_step->fields.error == isis_mtq_v2__selftesterror__noerror) printf(" ( none ) \r\n");
	else
	{
		printf(" ( ");
		if(p_step->fields.error & isis_mtq_v2__selftesterror__i2c_failure) printf("isis_mtq_v2__selftesterror__i2c_failure ");
		if(p_step->fields.error & isis_mtq_v2__selftesterror__spi_failure) printf("isis_mtq_v2__selftesterror__spi_failure ");
		if(p_step->fields.error & isis_mtq_v2__selftesterror__adc_failure) printf("isis_mtq_v2__selftesterror__adc_failure ");
		if(p_step->fields.error & isis_mtq_v2__selftesterror__pwm_failure) printf("isis_mtq_v2__selftesterror__pwm_failure ");
		if(p_step->fields.error & isis_mtq_v2__selftesterror__tc_failure) printf("isis_mtq_v2__selftesterror__tc_failure ");
		if(p_step->fields.error & isis_mtq_v2__selftesterror__mtm_outofrange) printf("isis_mtq_v2__selftesterror__mtm_outofrange ");
		if(p_step->fields.error & isis_mtq_v2__selftesterror__coil_outofrange) printf("isis_mtq_v2__selftesterror__coil_outofrange ");
		if(p_step->fields.error & 0x80) printf("!UNKNOWN! ");
		printf(")\r\n");
	}

	printf("   - step = %04x", p_step->fields.step);
	switch(p_step->fields.step)
	{
	case isis_mtq_v2__step__init: printf(" ( init = isis_mtq_v2__step__init ) \r\n"); break;
	case isis_mtq_v2__step__x_positive: printf(" ( +x = isis_mtq_v2__step__x_positive ) \r\n"); break;
	case isis_mtq_v2__step__x_negative: printf(" ( -x = isis_mtq_v2__step__x_negative ) \r\n"); break;
	case isis_mtq_v2__step__y_positive: printf(" ( +y = isis_mtq_v2__step__y_positive ) \r\n"); break;
	case isis_mtq_v2__step__y_negative: printf(" ( -y = isis_mtq_v2__step__y_negative ) \r\n"); break;
	case isis_mtq_v2__step__z_positive: printf(" ( +z = isis_mtq_v2__step__z_positive ) \r\n"); break;
	case isis_mtq_v2__step__z_negative: printf(" ( -z = isis_mtq_v2__step__z_negative ) \r\n"); break;
	case isis_mtq_v2__step__fina: printf(" ( fina = isis_mtq_v2__step__fina ) \r\n"); break;
	default: printf(" ( !UNKNOWN! ) \r\n"); break;
	}

	printf("   - raw_mag_(x,y,z) = %ld, %ld, %ld [-] \r\n", p_step->fields.raw_mag_x, p_step->fields.raw_mag_y, p_step->fields.raw_mag_z);
	printf("   - calibrated_mag_(x,y,z) = %ld, %ld, %ld [10E-9 T] \r\n", p_step->fields.calibrated_mag_x, p_step->fields.calibrated_mag_y, p_step->fields.calibrated_mag_z);
	printf("   - coil_current_(x,y,z) = %d, %d, %d [10E-4 mA] \r\n", p_step->fields.coil_current_x, p_step->fields.coil_current_y, p_step->fields.coil_current_z);
	printf("   - coil_temp_(x,y,z) = %d, %d, %d [degC] \r\n", p_step->fields.coil_temp_x, p_step->fields.coil_temp_y, p_step->fields.coil_temp_z);
}

/*!
 * Union for storing the common parameters.
 */
typedef union __attribute__((__packed__)) _mtq_general_axis_inputs
{
    unsigned char raw[8];
    struct __attribute__ ((__packed__))
    {
        int16_t input_x; /*!< X-direction setting */
        int16_t input_y; /*!< Y-direction setting */
        int16_t input_z; /*!< Z-direction setting */
        uint16_t duration; /*!< Actuation duration. 0 = infinite */
    } fields;
} mtq_general_axis_inputs;

static void _get_axis_inputs(void* inp)
{
	// gets the axis information required for actuation
	mtq_general_axis_inputs* p_inp = inp;

	double value = 0;

	// unHACK: input function test
//	ing_debug_show_input_test();

	if(p_inp == NULL)
	{
		TRACE_ERROR(" internal error: p_inp is NULL");
		return;
	}

	memset(p_inp, 0, sizeof(mtq_general_axis_inputs));

	if(ing(" x-axis value [0]: ", &value, -32768, 32767, 0))
	{
		TRACE_ERROR("\r\n invalid number!\r\n");
		return;
	}
	printf("\r\n");
	p_inp->fields.input_x = (signed short) value;

	if(ing(" y-axis value [0]: ", &value, -32768, 32767, 0))
	{
		TRACE_ERROR("\r\n invalid number!\r\n");
		return;
	}
	printf("\r\n");
	p_inp->fields.input_y = (signed short) value;

	if(ing(" z-axis value [0]: ", &value, -32768, 32767, 0))
	{
		TRACE_ERROR("\r\n invalid number!\r\n");
		return;
	}
	printf("\r\n");
	p_inp->fields.input_z = (signed short) value;

	if(ing(" duration ms (0=inf) [0]: ", &value, 0, 65535, 0))
	{
		TRACE_ERROR("\r\n invalid number!\r\n");
		return;
	}
	printf("\r\n");
	p_inp->fields.duration = (unsigned short) value;
}

///***************************************************************************************************************************
///
/// Demo helper functions end
///
///***************************************************************************************************************************

///***************************************************************************************************************************
///
/// IsisMTQv2 command demo functions:
/// below are the IsisMTQv2 interface commands making up the messaging interface.
/// these generally send the corresponding command to the IMTQv2 and present the results to the user
///
///***************************************************************************************************************************

static Boolean _softReset(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_RESET_SW_ID;
    isis_mtq_v2__replyheader_t rsp_stat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information isis_mtq_v2__reset_sw *** \r\n");
		printf(" Sends the softReset command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Used to perform a software reset of the IMTQv2 restarting the system. \r\n");
		printf(" Note: all information stored in volatile memory (e.g. configuration data) \r\n");
		printf(" is reset to their startup defaults. \r\n");

		return TRUE;
	}


	printf("\r\n Perform isis_mtq_v2__reset_sw \r\n");

	rv = isis_mtq_v2__reset_sw(0, &rsp_stat);			// reset the ISIS-EPS configuration to hardcoded defaults
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);

	return TRUE;
}

static Boolean _noOperation(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_NO_OP_ID;
	isis_mtq_v2__replyheader_t rsp_stat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information no_op *** \r\n");
		printf(" Sends the no-operation command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Does not affect the IMTQv2 other than providing a 'success' reply. \r\n");
		printf(" Can be used to verify availability of the IMTQv2 in a non-intrusive manner. \r\n");

		return TRUE;
	}

	printf("\r\n Perform no_op \r\n");

	rv = isis_mtq_v2__no_op(0, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);

	return TRUE;
}

static Boolean _cancelOperation(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_CANCEL_OP_ID;
	isis_mtq_v2__replyheader_t rsp_stat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information cancel_op *** \r\n");
		printf(" Sends the cancel-operation command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Stops any actuation and returns to idle mode. \r\n");

		return TRUE;
	}

	printf("\r\n Perform cancelOperation \r\n");

	rv = isis_mtq_v2__cancel_op(0, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);

	return TRUE;
}

static Boolean _startMTMMeasurement(Boolean info)
{
    uint8_t cmd_code_start[] = ISIS_MTQ_V2_START_MTM_ID;
    uint8_t cmd_code_raw[] = ISIS_MTQ_V2_GET_RAW_MTM_DATA_ID;
    uint8_t cmd_code_cal[] = ISIS_MTQ_V2_GET_CAL_MTM_DATA_ID;
	isis_mtq_v2__replyheader_t rsp_stat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information start_mtm *** \r\n");
		printf(" Sends the mtm start command %#04x to the IMTQv2 \r\n", cmd_code_start[0]);
		printf(" Starts a magnetometer measurement. Only available in Idle mode \r\n");
		printf(" Use getRawMTMData %#04x or getCalMTMData %#04x to get the result \r\n", cmd_code_raw[0], cmd_code_cal[0]);
		printf(" once integration has completed. Integration duration can be configured.");

		return TRUE;
	}

	printf("\r\n Perform start_mtm \r\n");

	rv = isis_mtq_v2__start_mtm(0, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);

	return TRUE;
}

static Boolean _startMTQActuationCurrent(Boolean info)
{
    uint8_t cmd_code_start[] = ISIS_MTQ_V2_START_ACTUATION_CURRENT_ID;
    uint8_t cmd_code_cancel[] = ISIS_MTQ_V2_CANCEL_OP_ID;
	isis_mtq_v2__replyheader_t rsp_stat;
	isis_mtq_v2__start_actuation_current__to_t input;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information startMTQActuationCurrent *** \r\n");
		printf(" Sends the actuation start current command %#04x to the IMTQv2 \r\n", cmd_code_start[0]);
		printf(" Starts coil actuation providing actuation level in coil currents. Only available in Idle mode \r\n");
		printf(" Provide positive values for positive dipoles in IMTQv2 frame, negative values for negative dipoles. \r\n");
		printf(" An actuation duration in milliseconds needs to be provided. \r\n");
		printf(" Provide 0 for duration to keep the coils on indefinitely. \r\n");
		printf(" The cancel command %#04x can be used to stop actuation at any time. \r\n", cmd_code_cancel[0]);

		return TRUE;
	}

	printf("\r\n Perform start_actuation_current \r\n");

	printf(" Provide actuation currents [10E-4 A] \r\n");
	_get_axis_inputs(&input);

	rv = isis_mtq_v2__start_actuation_current(0, &input, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);

	return TRUE;
}
static Boolean _startMTQActuationDipole(Boolean info)
{
    uint8_t cmd_code_start[] = ISIS_MTQ_V2_START_ACTUATION_DIPOLE_ID;
    uint8_t cmd_code_get[] = ISIS_MTQ_V2_GET_CMD_ACTUATION_DIPOLE_ID;
    uint8_t cmd_code_cancel[] = ISIS_MTQ_V2_CANCEL_OP_ID;
	isis_mtq_v2__replyheader_t rsp_stat;
	isis_mtq_v2__start_actuation_dipole__to_t input;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information startMTQActuationDipole *** \r\n");
		printf(" Sends the actuation start dipole command %#04x to the IMTQv2 \r\n", cmd_code_start[0]);
		printf(" Starts coil actuation providing actuation level in coil dipole. Only available in Idle mode \r\n");
		printf(" Provide positive values for positive dipoles in IMTQv2 frame, negative values for negative dipoles. \r\n");
		printf(" The provided dipole is collinearly reduced if it is bigger than the torque that the IMTQ can produce. \r\n");
		printf(" Use command getCmdActuationDipole %#04x to retrieve the dipole that is actually being used for torquing. \r\n", cmd_code_get[0]);
		printf(" An actuation duration in milliseconds needs to be provided, \r\n");
		printf(" or 0 can be supplied to keep the coils on indefinitely. \r\n");
		printf(" The cancel command %#04x can be used to stop actuation at any time. \r\n", cmd_code_cancel[0]);

		return TRUE;
	}

	printf("\r\n Perform startMTQActuationDipole \r\n");

	printf(" Provide actuation dipole [10E-4 Am2] \r\n");
	_get_axis_inputs(&input);

	rv = isis_mtq_v2__start_actuation_dipole(0, &input, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);

	return TRUE;
}
static Boolean _startMTQActuationPWM(Boolean info)
{
    uint8_t cmd_code_start[] = ISIS_MTQ_V2_START_ACTUATION_PWM_ID;
    uint8_t cmd_code_cancel[] = ISIS_MTQ_V2_CANCEL_OP_ID;
	isis_mtq_v2__replyheader_t rsp_stat;
	isis_mtq_v2__start_actuation_pwm__to_t input;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information startMTQActuationPWM *** \r\n");
		printf(" Sends the actuation start pwm command %#04x to the IMTQv2 \r\n", cmd_code_start[0]);
		printf(" Starts coil actuation providing actuation level in on-percentage. Only available in Idle mode \r\n");
		printf(" Provide positive values for positive dipoles in IMTQv2 frame, negative values for negative dipoles. \r\n");
		printf(" An actuation duration in milliseconds needs to be provided. \r\n");
		printf(" Provide 0 for duration to keep the coils on indefinitely. \r\n");
		printf(" The cancel command %#04x can be used to stop actuation at any time. \r\n", cmd_code_cancel[0]);

		return TRUE;
	}

	printf("\r\n Perform startMTQActuationDipole \r\n");

	printf(" Provide actuation [0.1%%] \r\n");
	_get_axis_inputs(&input);

	rv = isis_mtq_v2__start_actuation_pwm(0, &input, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);

	return TRUE;
}

static Boolean _startSelfTest(Boolean info)
{
    uint8_t cmd_code_start[] = ISIS_MTQ_V2_START_SELF_TEST_ID;
    uint8_t cmd_code_get[] = ISIS_MTQ_V2_GET_SELF_TEST_RESULT_ALL_ID;
	isis_mtq_v2__replyheader_t rsp_stat;
	isis_mtq_v2__axis_t axdir;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information startSelfTest *** \r\n");
		printf(" Sends the startSelfTest command %#04x to the IMTQv2 \r\n", cmd_code_start[0]);
		printf(" Switches IMTQv2 into self-test mode, actuating axes and cross-checking with measured magnetic field. \r\n");
		printf(" Can only be started from Idle mode and automatically switches back to Idle mode upon completion. \r\n");
		printf(" Provide isis_mtq_v2__axis__all (%d) to actuate and verify all axes-directions (i.e. a positive and a negative dipole generated per axis) \r\n", isis_mtq_v2__axis__all);
		printf(" sequentially using a single command. Otherwise a single axis can be verified per command. \r\n");
		printf(" After completion the result of the self-test needs to be retrieved using command getSelftestData %#04x. \r\n", cmd_code_get[0]);
		printf(" NOTE: The STAT byte response is *not* the result of the self-test, it only indicates start command acceptance. \r\n");

		return TRUE;
	}

	printf("\r\n Perform startSelfTest \r\n");

	// *** get axis test user input
	{
		double value;
		char tmp[255] = {0};

		snprintf(tmp, sizeof(tmp), " Provide self-test axis (%d=all,%d=+x,%d=-x,%d=+y,%d=-y,%d=+z,%d=-z) [%d]: ",
		        isis_mtq_v2__axis__all, isis_mtq_v2__axis__x_positive, isis_mtq_v2__axis__x_negative, isis_mtq_v2__axis__y_positive, isis_mtq_v2__axis__y_negative, isis_mtq_v2__axis__z_positive, isis_mtq_v2__axis__z_negative,
		        isis_mtq_v2__axis__all);

		while(1)
		{
			rv = ing(tmp, &value, isis_mtq_v2__axis__all, isis_mtq_v2__axis__z_negative, isis_mtq_v2__axis__all);	// request info from the user
			if(rv == INGRV_esc) {printf("<ESC> \r\n"); return TRUE;}								// esc? exit function
			else if(rv == INGRV_val) {printf("\r\n"); break;}										// valid? continue
		}

		axdir = (isis_mtq_v2__axis_t) value;															// cast the supplied axis-direction value
	}

	rv = isis_mtq_v2__start_self_test(0, axdir, &rsp_stat);	// issue the command
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);

	return TRUE;
}
static Boolean _startDetumble(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_START_BDOT_ID;
	isis_mtq_v2__replyheader_t rsp_stat;
	unsigned short duration;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information startDetumble *** \r\n");
		printf(" Sends the startDetumble command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Switches IMTQv2 into detumble mode, providing autonomous detumble operation. \r\n");
		printf(" Detumble mode implements the b-dot algorithm using magnetic measurements and coil actuation to reduce the spin of a satellite. \r\n");
		printf(" Can only be started from Idle mode. Returns to Idle mode after duration expires. \r\n");
		printf(" Duration can *not* be set to infinite, instead startBDOT needs to be re-issued to update its duration. \r\n");

		return TRUE;
	}

	printf("\r\n Perform startDetumble \r\n");

	// *** get axis test user input
	{
		double value;

		while(1)
		{
			rv = ing(" Provide duration in seconds [0]: ", &value, 0, 65535, 0);					// request info from the user
			if(rv == INGRV_esc) {printf("<ESC> \r\n"); return TRUE;}								// esc? exit function
			else if(rv == INGRV_val) {printf("\r\n"); break;}										// valid? continue
		}

		duration = value;																			// cast the supplied axis-direction value
	}

	rv = isis_mtq_v2__start_bdot(0, duration, &rsp_stat);											// issue the command
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);													// non-zero return value means error!
		return TRUE;																				// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);

	return TRUE;
}

static Boolean _getSystemState(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_STATE_ID;
    isis_mtq_v2__get_state__from_t system_state;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getSystemState *** \r\n");
		printf(" Sends the getMTQSystemState command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Provides system state information on the IMTQv2. \r\n");
		printf(" The following information is returned: \r\n");
		printf("   - mode = %s \r\n", "current mode of the IMTQv2");
		printf("   - error = %s \r\n", "first internal error encountered during last control iteration");
		printf("   - conf = %s \r\n", "1 when the in-memory configuration been altered by the user since start-up");
		printf("   - uptime = %s \r\n", "uptime in seconds");

		return TRUE;
	}

	printf("\r\n Perform getSystemState \r\n");

	rv = isis_mtq_v2__get_state(0, &system_state);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&system_state.fields.reply_header);

	printf("   - mode = %d ", system_state.fields.mode);
	switch(system_state.fields.mode)
	{
	case isis_mtq_v2__mode__idle: printf("( idle = isis_mtq_v2__mode__idle ) \r\n"); break;
	case isis_mtq_v2__mode__selftest: printf("( selftest = isis_mtq_v2__mode__selftest ) \r\n"); break;
	case isis_mtq_v2__mode__detumble: printf("( detumble = isis_mtq_v2__mode__detumble ) \r\n"); break;
	default: printf("( !UNKNOWN! ) \r\n"); break;
	}

	printf("   - error = %d \r\n", system_state.fields.err);
	printf("   - conf = %d ", system_state.fields.conf);
	if(system_state.fields.conf) printf("( config params were changed ) \r\n"); else printf("( no config param changes ) \r\n");
	printf("   - uptime = %d [s] \r\n", system_state.fields.uptime);

	return TRUE;
}
static Boolean _getRawMTMData(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_RAW_MTM_DATA_ID;
	isis_mtq_v2__get_raw_mtm_data__from_t field_raw;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getRawMTMData *** \r\n");
		printf(" Sends the getRawMTMData command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Returns MTM measurement results from a previously started measurement using startMTMMeasurement. \r\n");
		printf(" Measurement results become available after the configurable integration time completes. \r\n");
		printf(" The following information is returned: \r\n");
		printf("   - raw_mag_(x,y,z) = %s \r\n", "magnetometer field strength in raw counts for x, y, and z.");
		printf("   - coilact = %s \r\n", "coil actuation detected during measurement");

		return TRUE;
	}

	printf("\r\n Perform getRawMTMData \r\n");

	rv = isis_mtq_v2__get_raw_mtm_data(0, &field_raw);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&field_raw.fields.reply_header);

	printf("   - raw_mag_(x,y,z) = %ld, %ld, %ld [-] \r\n", field_raw.fields.raw_mag_x, field_raw.fields.raw_mag_y, field_raw.fields.raw_mag_z);
	printf("   - coilact = %d \r\n", field_raw.fields.coilact);

	return TRUE;
}
static Boolean _getCalMTMData(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_CAL_MTM_DATA_ID;
	isis_mtq_v2__get_cal_mtm_data__from_t field_cal;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getCalMTMData *** \r\n");
		printf(" Sends the getCalMTMData command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Returns MTM measurement results from a previously started measurement using startMTMMeasurement. \r\n");
		printf(" Measurement results become available after the configurable integration time completes. \r\n");
		printf(" The following information is returned: \r\n");
		printf("   - calibrated_mag_(x,y,z) = %s \r\n", "magnetometer field strength in nano-tesla for x, y, and z [10E-9 T]");
		printf("   - coilact = %s \r\n", "coil actuation detected during measurement");

		return TRUE;
	}

	printf("\r\n Perform getCalMTMData \r\n");

	rv = isis_mtq_v2__get_cal_mtm_data(0, &field_cal);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&field_cal.fields.reply_header);

	printf("   - calibrated_mag_(x,y,z) = %ld, %ld, %ld [10E-9 T] \r\n", field_cal.fields.calibrated_mag_x, field_cal.fields.calibrated_mag_y, field_cal.fields.calibrated_mag_z);
	printf("   - coilact = %d \r\n", field_cal.fields.coilact);

	return TRUE;
}

static Boolean _getCoilCurrent(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_COIL_CURRENT_ID;
    isis_mtq_v2__get_coil_current__from_t coilcurr;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getCoilCurrent *** \r\n");
		printf(" Sends the getCoilCurrent command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Returns the latest coil current measurement available. \r\n");
		printf(" Measurement results become available automatically after each millisecond. \r\n");
		printf(" The following information is returned: \r\n");
		printf("   - coil_current_(x,y,z) = %s \r\n", "coil current for x, y, and z [10E-4 A]");

		return TRUE;
	}

	printf("\r\n Perform getCoilCurrent \r\n");

	rv = isis_mtq_v2__get_coil_current(0, &coilcurr);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&coilcurr.fields.reply_header);

	printf("   - coil_current_(x,y,z) = %d, %d, %d [10E-4 A] \r\n", coilcurr.fields.coil_current_x, coilcurr.fields.coil_current_y, coilcurr.fields.coil_current_z);

	return TRUE;
}

static Boolean _getCoilTemperature(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_COIL_TEMPS_ID;
	isis_mtq_v2__get_coil_temps__from_t coiltemp;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getCoilTemperature *** \r\n");
		printf(" Sends the getCoilTemperature command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Returns the latest coil temperature measurement available. \r\n");
		printf(" Measurement results become available automatically after each millisecond. \r\n");
		printf(" The following information is returned: \r\n");
		printf("   - coil_temp_(x,y,z) = %s \r\n", "coil temperature in for x, y, and z [deg. C]");

		return TRUE;
	}

	printf("\r\n Perform getCoilTemperature \r\n");

	rv = isis_mtq_v2__get_coil_temps(0, &coiltemp);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&coiltemp.fields.reply_header);

	printf("   - coil_temp_(x,y,z) = %d, %d, %d [deg. C] \r\n", coiltemp.fields.coil_temp_x, coiltemp.fields.coil_temp_y, coiltemp.fields.coil_temp_z);

	return TRUE;
}
static Boolean _getCmdActuationDipole(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_CMD_ACTUATION_DIPOLE_ID;
    isis_mtq_v2__get_cmd_actuation_dipole__from_t cmdact_dip;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getCmdActuationDipole *** \r\n");
		printf(" Sends the getCmdActuationDipole command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Returns the dipole that is being actuated, which might be different from the one commanded by the IMTQv2 master. \r\n");
		printf(" Differences occur due to automatic scaling of the dipole to fall within temperature dependent torque-able limits of the IMTQv2. \r\n");
		printf(" NOTE: Only available after a torque command using dipole, or while in detumble mode.\r\n");
		printf(" The following information is returned: \r\n");
		printf("   - cmd_act_dip_(x,y,z) = %s \r\n", "commanded coil actuation dipole for x, y, and z [10E-4 Am2]");

		return TRUE;
	}

	printf("\r\n Perform getCmdActuationDipole \r\n");

	rv = isis_mtq_v2__get_cmd_actuation_dipole(0, &cmdact_dip);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&cmdact_dip.fields.reply_header);

	printf("   - cmd_act_dip_(x,y,z) = %d, %d, %d [10E-4 Am2] \r\n", cmdact_dip.fields.cmd_act_dip_x, cmdact_dip.fields.cmd_act_dip_y, cmdact_dip.fields.cmd_act_dip_z);

	return TRUE;
}

static Boolean _getSelftestDataSingleAxis(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_SELF_TEST_RESULT_SINGLE_ID;
    uint8_t cmd_code_start[] = ISIS_MTQ_V2_START_SELF_TEST_ID;
    isis_mtq_v2__get_self_test_result_single__from_t seldata;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getSelftestDataSingleAxis *** \r\n");
		printf(" Sends the getSelftestData command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Returns the result of the last completed self-test, started with the command startSelfTest %#04x. \r\n", cmd_code_start[0]);
		printf(" The self-test is a mode which actuates the coils and measures the resulting field to verify hardware functionality. \r\n");
		printf(" At the end of the self-test the IMTQv2 will diagnose the gathered measurement data and indicate whether problems are detected. \r\n");
		printf(" The result data returned can be either a single-axis or all-axis data block, depending on the axis-direction parameter provided to startSelfTest(), \r\n");
		printf(" which should be retrieved using the getSelftestDataSingleAxis() or getSelftestDataAllAxis() functions respectively. \r\n");
		printf(" The following information is returned: \r\n");
		_parse_selftest_step(NULL);						// request only generic information ...

		return TRUE;
	}

	printf("\r\n Perform getSelftestDataSingleAxis \r\n");

	rv = isis_mtq_v2__get_self_test_result_single(0, &seldata);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf("\r\n Results: \r\n");

	printf(" *** step INIT \r\n");
	_parse_selftest_step(&seldata.fields.step_init);
	printf(" *** step AXAC (actuation step results for selected axis-direction) \r\n");
	_parse_selftest_step(&seldata.fields.step_axac);
	printf(" *** step FINA \r\n");
	_parse_selftest_step(&seldata.fields.step_fina);

	return TRUE;
}

static Boolean _getSelftestDataAllAxis(Boolean info)
{
    uint8_t cmd_code_get[] = ISIS_MTQ_V2_GET_SELF_TEST_RESULT_ALL_ID;
    uint8_t cmd_code_start[] = ISIS_MTQ_V2_START_SELF_TEST_ID;
    isis_mtq_v2__get_self_test_result_all__from_t seldata;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getSelftestDataSingleAxis *** \r\n");
		printf(" Sends the getSelftestData command %#04x to the IMTQv2 \r\n", cmd_code_get[0]);
		printf(" Returns the result of the last completed self-test, started with the command startSelfTest %#04x. \r\n", cmd_code_start[0]);
		printf(" The self-test is a mode which actuates the coils and measures the resulting field to verify hardware functionality. \r\n");
		printf(" At the end of the self-test the IMTQv2 will diagnose the gathered measurement data and indicate whether problems are detected. \r\n");
		printf(" The result data returned can be either a single-axis or all-axis data block, depending on the axis-direction \r\n");
		printf(" parameter provided to startSelfTest(), which should be retrieved using the getSelftestDataSingleAxis() or \r\n");
		printf(" getSelftestDataAllAxis() functions respectively. \r\n");
		printf(" The following information is returned: \r\n");
		_parse_selftest_step(NULL);						// request only generic information ...

		return TRUE;
	}

	printf("\r\n Perform getSelftestDataSingleAxis \r\n");

	rv = isis_mtq_v2__get_self_test_result_all(0, &seldata);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf("\r\n Results: \r\n");

	printf(" *** step INIT \r\n");
	_parse_selftest_step(&seldata.fields.step_init);
	printf(" *** step POSX \r\n");
	_parse_selftest_step(&seldata.fields.step_posx);
	printf(" *** step NEGX \r\n");
	_parse_selftest_step(&seldata.fields.step_negx);
	printf(" *** step POSY \r\n");
	_parse_selftest_step(&seldata.fields.step_posy);
	printf(" *** step NEGY \r\n");
	_parse_selftest_step(&seldata.fields.step_negy);
	printf(" *** step POSZ \r\n");
	_parse_selftest_step(&seldata.fields.step_posz);
	printf(" *** step NEGZ \r\n");
	_parse_selftest_step(&seldata.fields.step_negz);
	printf(" *** step FINA \r\n");
	_parse_selftest_step(&seldata.fields.step_fina);

	return TRUE;
}

static Boolean _getDetumbleData(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_DETUMBLE_DATA_ID;
	isis_mtq_v2__get_detumble_data__from_t detdat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getDetumbleData *** \r\n");
		printf(" Sends the getDetumbleData command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Returns the latest measurement and control information produced by the autonomously operating detumble mode. \r\n");
		printf(" The following information is returned: \r\n");
		printf("   - calibrated_mag_(x,y,z) = %s \r\n", "magnetic field strength for x, y, and z [nT / 10E-9 T]");
		printf("   - filtered_mag_(x,y,z) = %s \r\n", "filtered magnetic field strength for x, y, and z [nT / 10E-9 T]");
		printf("   - bdot_(x,y,z) = %s \r\n", "computed b-dot value in nano-tesla per second for x, y, and z [10E-9 T/s]");
		printf("   - cmd_act_dip_(x,y,z) = %s \r\n", "commanded actuation dipole for x, y, and z [10E-4 Am2]");
		printf("   - cmd_current_(x,y,z) = %s \r\n", "commanded actuation current for x, y, and z [10E-4 A]");
		printf("   - meas_current_(x,y,z) = %s \r\n", "measured actuation current for x, y, and z [10E-4 A]");

		return TRUE;
	}

	printf("\r\n Perform getDetumbleData \r\n");

	rv = isis_mtq_v2__get_detumble_data(0, &detdat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&detdat.fields.reply_header);

	printf("   - calibrated_mag_(x,y,z) = %ld, %ld, %ld [10E-9 T] \r\n", detdat.fields.calibrated_mag_x, detdat.fields.calibrated_mag_y, detdat.fields.calibrated_mag_z);
	printf("   - filtered_mag_(x,y,z) = %ld, %ld, %ld [10E-9 T] \r\n", detdat.fields.filtered_mag_x, detdat.fields.filtered_mag_y, detdat.fields.filtered_mag_z);
	printf("   - bdot_(x,y,z) = %ld, %ld, %ld [10E-9 T/s] \r\n", detdat.fields.bdot_x, detdat.fields.bdot_y, detdat.fields.bdot_z);
	printf("   - cmd_act_dip_(x,y,z) = %d, %d, %d [10E-4 Am2] \r\n", detdat.fields.cmd_act_dip_x, detdat.fields.cmd_act_dip_y, detdat.fields.cmd_act_dip_z);
	printf("   - cmd_current_(x,y,z) = %d, %d, %d [10E-4 A] \r\n", detdat.fields.cmd_current_x, detdat.fields.cmd_current_y, detdat.fields.cmd_current_z);
	printf("   - meas_current_(x,y,z) = %d, %d, %d [10E-4 A] \r\n", detdat.fields.meas_current_x, detdat.fields.meas_current_y, detdat.fields.meas_current_z);

	return TRUE;
}
static Boolean _getRawHKData(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_HOUSEKEEPING_ID;
	isis_mtq_v2__get_housekeeping__from_t rawhk;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getRawHKData *** \r\n");
		printf(" Sends the getRawHKData command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Returns the latest house-keeping data in raw form from the IMTQv2. \r\n");
		printf(" The following information is returned: \r\n");
		printf("   - digital_voltage = %s \r\n", "measured digital supply current in raw counts for x, y, and z");
		printf("   - analog_voltage = %s \r\n", "measured analog supply current in raw counts for x, y, and z");
		printf("   - digital_current = %s \r\n", "measured digital supply current in raw counts for x, y, and z");
		printf("   - analog_current = %s \r\n", "measured analog supply current in raw counts for x, y, and z");
		printf("   - meas_current_(x,y,z) = %s \r\n", "measured coil currents in raw counts for x, y, and z");
		printf("   - coil_temp_(x,y,z) = %s \r\n", "measured coil temperature in raw counts for x, y, and z");
		printf("   - mcu_temp = %s \r\n", "measured micro controller unit internal temperature in raw counts for x, y, and z");

		return TRUE;
	}

	printf("\r\n Perform _getRawHKData \r\n");

	rv = isis_mtq_v2__get_housekeeping(0, &rawhk);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rawhk.fields.reply_header);

	printf("   - digital_voltage = %d [-] \r\n", rawhk.fields.digital_voltage);
	printf("   - analog_voltage = %d [-] \r\n", rawhk.fields.analog_voltage);
	printf("   - digital_current = %d [-] \r\n", rawhk.fields.digital_current);
	printf("   - analog_current = %d [-] \r\n", rawhk.fields.analog_current);
	printf("   - meas_current_(x,y,z) = %d, %d, %d [-] \r\n", rawhk.fields.meas_current_x , rawhk.fields.meas_current_y, rawhk.fields.meas_current_z);
	printf("   - coil_temp_(x,y,z) = %d, %d, %d [-] \r\n", rawhk.fields.coil_temp_x , rawhk.fields.coil_temp_y, rawhk.fields.coil_temp_z);
	printf("   - mcu_temp = %d [-] \r\n", rawhk.fields.mcu_temp);

	return TRUE;
}

static Boolean _getEngHKData(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_HOUSEKEEPING_ENGINEERING_ID;
	isis_mtq_v2__get_housekeeping_engineering__from_t enghk;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getEngHKData *** \r\n");
		printf(" Sends the getEngHKData command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Returns the latest house-keeping data as engineering values from the IMTQv2. \r\n");
		printf(" The following information is returned: \r\n");
		printf("   - digital_voltage = %s \r\n", "measured digital supply current for x, y, and z [mV]");
		printf("   - analog_voltage = %s \r\n", "measured analog supply current for x, y, and z [mV]");
		printf("   - digital_current = %s \r\n", "measured digital supply current for x, y, and z [10E-4 A]");
		printf("   - analog_current = %s \r\n", "measured analog supply current for x, y, and z [10E-4 A]");
		printf("   - meas_current_(x,y,z) = %s \r\n", "measured coil currents for x, y, and z [10E-4 A]");
		printf("   - coil_temp_(x,y,z) = %s \r\n", "measured coil temperature  for x, y, and z [deg. C]");
		printf("   - mcu_temp = %s \r\n", "measured micro controller unit internal temperature [deg. C]");

		return TRUE;
	}

	printf("\r\n Perform _getEngHKData \r\n");

	rv = isis_mtq_v2__get_housekeeping_engineering(0, &enghk);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&enghk.fields.reply_header);

	printf("   - digital_voltage = %d [mV] \r\n", enghk.fields.digital_voltage);
	printf("   - analog_voltage = %d [mV] \r\n", enghk.fields.analog_voltage);
	printf("   - digital_current = %d [10E-4 A] \r\n", enghk.fields.digital_current);
	printf("   - analog_current = %d [10E-4 A] \r\n", enghk.fields.analog_current);
	printf("   - meas_current_(x,y,z) = %d, %d, %d [10E-4 A] \r\n", enghk.fields.meas_current_x , enghk.fields.meas_current_y, enghk.fields.meas_current_z);
	printf("   - coil_temp_(x,y,z) = %d, %d, %d [degC] \r\n", enghk.fields.coil_temp_x , enghk.fields.coil_temp_y, enghk.fields.coil_temp_z);
	printf("   - mcu_temp = %d [degC] \r\n", enghk.fields.mcu_temp);

	return TRUE;
}


static Boolean _getParameter(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_GET_PARAMETER_ID;
	unsigned short par_id;							// storage for the param-id
	isis_mtq_v2__get_parameter__from_t  rsp;	    // storage for the command response
	unsigned int rv;

	if(info)
	{
		printf("\r\n Information getParameter \r\n");
		printf(" Sends the getParameter command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Used to get configuration parameter values from the IMTQv2. \r\n");
		printf(" Execution is performed and completed immediately. \r\n");
		printf(" The following information is returned: \r\n");
		printf("   - param-id = %s \r\n", "parameter-id of the parameter under consideration.");
		printf("   - param-value = %s \r\n", "value of the parameter under consideration. Between 1 and 8 bytes.");

		return TRUE;
	}

	printf("\r\n Perform getParameter \r\n");

	if(!config_param_info(CONFIG_PARAM_OP_ask_parid, &par_id, NULL)) return TRUE;	// get the param-id from the user

	rv = isis_mtq_v2__get_parameter(0, par_id, &rsp);	// get the parameter from the IMTQv2
	if(rv)
	{
		TRACE_ERROR("return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp.fields.reply_header);							// parse the command response and show that to the user

	config_param_info(CONFIG_PARAM_OP_print, &par_id, &rsp.fields.param_value);	// show the param-id and corresponding value that we received back

	return TRUE;
}


static Boolean _setParameter(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_SET_PARAMETER_ID;
	isis_mtq_v2__set_parameter__to_t par_data;						// byte array for storing the parameter data we want to set; our maximum size is 8 bytes (e.g. double, long ...)
	isis_mtq_v2__set_parameter__from_t par_data_out;			    // byte array for storing the parameter data that was actually set; our maximum size is 8 bytes (e.g. double, long ...)
	unsigned int rv;

	if(info)
	{
		printf("\r\n Information setParameter \r\n");
		printf("Sends the setParameter command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf("Used to set configuration parameter values within the IMTQv2. \r\n");
		printf("Execution is performed and completed immediately. \r\n");
		printf("The following information is returned: \r\n");
		printf("  - param-id = %s \r\n", "parameter-id of the parameter under consideration.");
		printf("  - param-value = %s \r\n", "new value of the parameter under consideration. Between 1 and 8 bytes.");

		return TRUE;
	}

	printf("\r\n Perform setParameter \r\n");

	if(!config_param_info(CONFIG_PARAM_OP_ask_parid_and_data, &par_data.fields.param_id, &par_data.fields.param_value)) return TRUE;	// get the param-id and new value from the user

	rv = isis_mtq_v2__set_parameter(0, &par_data, &par_data_out);		// send the new parameter data to the IMTQv2
	if(rv)
	{
		TRACE_ERROR("return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&par_data_out.fields.reply_header);							// parse the command response and show that to the user

	if(!config_param_info(CONFIG_PARAM_OP_print, &par_data.fields.param_id, &par_data_out.fields.param_value_new)) return TRUE;	// show the param-id and corresponding value that we received back

	if(memcmp(&par_data.fields.param_value, &par_data_out.fields.param_value_new, sizeof(par_data.fields.param_value)) != 0)		// is the returned parameter value the same?
	{
		unsigned int i;

		printf("Warning: the resulting parameter value differs from the one send in! \r\n");
		printf("this is generally caused because the supplied variable was not in the range \r\n");
		printf("of valid values and has been set to closest valid value by the IMTQv2. \r\n");
		printf("I2C bus noise might be another possible culprit. \r\n");
		printf("Consult the manual and test the I2C bus to determine the cause. \r\n");

		printf("send     = %#04x",  par_data.fields.param_value[0]);
		for(i = 1; i < sizeof(par_data); i++)
		{
			printf(", %#04x",  par_data.fields.param_value[i]);
		}
		printf("\r\n");
		printf("received = %#04x",  par_data_out.fields.param_value_new[0]);
		for(i = 1; i < sizeof(par_data_out); i++)
		{
			printf(", %#04x",  par_data_out.fields.param_value_new[i]);
		}
		printf("\r\n");
	}

	return TRUE;
}


static Boolean _resetParameter(Boolean info)
{
    uint8_t cmd_code[] = ISIS_MTQ_V2_RESET_PARAMETER_ID;
	unsigned short par_id;							            // storage for the param-id
	isis_mtq_v2__reset_parameter__from_t par_data;		        // byte array for storing our parameter data
	unsigned int rv;

	if(info)
	{
		printf("\r\n Information resetParameter \r\n");
		printf(" Sends the resetParameter command %#04x to the IMTQv2 \r\n", cmd_code[0]);
		printf(" Used to reset configuration parameter values back to its hard coded default within the IMTQv2. \r\n");
		printf(" note: this is different from the value stored in non-volatile memory; if required use loadConfig instead. \r\n");
		printf(" Execution is performed and completed immediately. \r\n");
		printf(" The following information is returned: \r\n");
		printf("   - param-id = %s \r\n", "parameter-id of the parameter under consideration.");
		printf("   - param-value = %s \r\n", "value of the parameter under consideration. Between 1 and 8 bytes.");

		return TRUE;
	}

	printf("\r\n Perform resetParameter \r\n");

	if(!config_param_info(CONFIG_PARAM_OP_ask_parid, &par_id, NULL)) return TRUE;	// get the param-id from the user

	rv = isis_mtq_v2__reset_parameter(0, par_id, &par_data);	// command the ISIS-MTQv2 to reset the parameter, and receive the value that it was rest to
	if(rv)
	{
		TRACE_ERROR("return value=%d \r\n", rv);		// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&par_data.fields.reply_header);							// parse the command response and show that to the user

	config_param_info(CONFIG_PARAM_OP_print, &par_id, &par_data);	// show the param-id and corresponding value that we received back

	return TRUE;
}

static Boolean _selectAndExecuteDemoTest(void)
{
	double value;
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;
	static Boolean toggle_is_info = FALSE;

	if(toggle_is_info)
	{
		printf("\n\r ******************* Information Mode ******************* \n\r");
		printf("\n\r While in this mode you can select commands about which \n\r");
		printf(    " you would like to get information without issuing \n\r");
		printf(    " the actual command. To exit information mode select \n\r");
		printf(    " option 1. \n\r");
		printf("\n\r Choose which command to show information for: \n\r");
	}
	else
	{
		printf( "\n\r Select a test to perform: \n\r");
	}

	printf("\t  0) Return to main menu \n\r");

	if(toggle_is_info)	printf("\t  1) Switch back to test mode \n\r");
	else 				printf("\t  1) Switch to information mode \n\r");

	printf("\t  2) softReset \n\r");
	printf("\t  3) noOperation \n\r");
	printf("\t  4) cancelOperation \n\r");
	printf("\t  5) startMTMMeasurement \n\r");
	printf("\t  6) startMTQActuationCurrent \n\r");
	printf("\t  7) startMTQActuationDipole \n\r");
	printf("\t  8) startMTQActuationPWM \n\r");
	printf("\t  9) startSelfTest \n\r");
	printf("\t 10) startDetumble \n\r");
	printf("\t 11) getSystemState \n\r");
	printf("\t 12) getRawMTMData \n\r");
	printf("\t 13) getCalMTMData \n\r");
	printf("\t 14) getCoilCurrent \n\r");;
	printf("\t 15) getCoilTemperature \n\r");
	printf("\t 16) getCmdActuationDipole \n\r");
	printf("\t 17) getSelftestDataSingleAxis \n\r");
	printf("\t 18) getSelftestDataAllAxis \n\r");
	printf("\t 19) getDetumbleData \n\r");
	printf("\t 20) getRawHKData \n\r");
	printf("\t 21) getEngHKData \n\r");
	printf("\t 22) getParameter \n\r");
	printf("\t 23) setParameter \n\r");
	printf("\t 24) resetParameter \n\r");

	while(INGRV_val != ing("\r\n enter selection: ", &value, 0, 24, -1))
	{
		printf("\r\n");
	}

	printf("\r\n");

	selection = value;

	switch(selection)
	{
		case 0: offerMoreTests = FALSE; break;
		case 1: toggle_is_info = !toggle_is_info; offerMoreTests = TRUE; break;

		case 2: offerMoreTests = _softReset(toggle_is_info); break;
		case 3: offerMoreTests = _noOperation(toggle_is_info); break;
		case 4: offerMoreTests = _cancelOperation(toggle_is_info); break;
		case 5: offerMoreTests = _startMTMMeasurement(toggle_is_info); break;
		case 6: offerMoreTests = _startMTQActuationCurrent(toggle_is_info); break;
		case 7: offerMoreTests = _startMTQActuationDipole(toggle_is_info); break;
		case 8: offerMoreTests = _startMTQActuationPWM(toggle_is_info); break;
		case 9: offerMoreTests = _startSelfTest(toggle_is_info); break;
		case 10: offerMoreTests = _startDetumble(toggle_is_info); break;
		case 11: offerMoreTests = _getSystemState(toggle_is_info); break;
		case 12: offerMoreTests = _getRawMTMData(toggle_is_info); break;
		case 13: offerMoreTests = _getCalMTMData(toggle_is_info); break;
		case 14: offerMoreTests = _getCoilCurrent(toggle_is_info); break;
		case 15: offerMoreTests = _getCoilTemperature(toggle_is_info); break;
		case 16: offerMoreTests = _getCmdActuationDipole(toggle_is_info); break;
		case 17: offerMoreTests = _getSelftestDataSingleAxis(toggle_is_info); break;
		case 18: offerMoreTests = _getSelftestDataAllAxis(toggle_is_info); break;
		case 19: offerMoreTests = _getDetumbleData(toggle_is_info); break;
		case 20: offerMoreTests = _getRawHKData(toggle_is_info); break;
		case 21: offerMoreTests = _getEngHKData(toggle_is_info); break;
		case 22: offerMoreTests = _getParameter(toggle_is_info); break;
		case 23: offerMoreTests = _setParameter(toggle_is_info); break;
		case 24: offerMoreTests = _resetParameter(toggle_is_info); break;
		default: TRACE_ERROR("Invalid selection"); break;
	}

	return offerMoreTests;
}

/***
 * Initializes the IMTQv2 subsystem driver
 * The IMTQv2 subsystem driver is a layer that sits on top of the I2C interface driver
 * requiring the I2C interface driver to be initialized once before using any of the
 * subsystem library drivers
 */
Boolean IsisMTQv2demoInit(void)
{
    ISIS_MTQ_V2_t myIMTQ = {.i2cAddr = 0x77};
    int rv;

	rv = ISIS_MTQ_V2_Init(&myIMTQ, 1);
	if(rv != E_NO_SS_ERR && rv != E_IS_INITIALIZED)				// rinse-repeat
	{
		// we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
		// hanging the code
		TRACE_ERROR("\n\r ISIS_MTQ_V2_Init() failed; err=%d! Exiting ... \n\r", rv);
		return FALSE;
	}

	return TRUE;
}

void IsisMTQv2demoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = _selectAndExecuteDemoTest();		// show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)							// was exit/back selected?
		{
			break;
		}
	}
}

Boolean IsisMTQv2demoMain(void)
{
	if(IsisMTQv2demoInit())									// initialize of I2C and IMTQv2 subsystem drivers succeeded?
	{
		IsisMTQv2demoLoop();								// show the main IMTQv2 demo interface and wait for user input
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

Boolean IsisMTQv2test(void)
{
	IsisMTQv2demoMain();
	return TRUE;
}
