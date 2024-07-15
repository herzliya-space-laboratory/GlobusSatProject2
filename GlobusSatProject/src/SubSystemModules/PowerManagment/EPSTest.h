#ifndef EPS_TEST_H
#define EPS_TEST_H

#define P_ERR_NUM 3

typedef union __attribute__ ((__packed__)){
	int raw[P_ERR_NUM];
	struct {
		int alphaError;
		int TresholdError;
		int battError;
	}fields;
}Error_List;

/**
 * @brief total check of all the EPS systems. checking the FRAM for alpha and threshold
 * and the battery voltage
 * @return a list of all the errors that happen during runtime
 */
Error_List EPSTest();

/**
 * @brief a test for the alpha in FRAM
 * @return error according to hal/errors.h
 */
int alphaTest();

/**
 * @brief a test for the threshold in FRAM
 * @return error according to hal/errors.h
 */
int thresholdTest();

/*
 * @brief. checks if the battery returns a reasonable voltage
 * @return error according to hal/errors.h
 */
int batteryTest();

#endif
