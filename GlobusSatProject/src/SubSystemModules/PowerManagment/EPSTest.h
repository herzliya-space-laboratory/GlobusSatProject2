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
 * Warning!!!
 * if it does return an error. there is a high chance that the FRAM value has been changed.
 * reset it
 */
/**
 * @brief total check of all the EPS systems. checking the FRAM for alpha and threshold
 * and the battery voltage
 * @return a list of all the errors that happen during runtime
 */
Error_List EPSTest();

/**
 * @brief a test for the alpha in FRAM the test includes:
 * compares if the save has been successful if fail return -2
 * compares if the param out of bounds works. if fail will return -3
 * if it return something else that is not 0 error will be according to hal/errors.h
 */
int alphaTest();

/**
 * @brief a test for the threshold in FRAM the tests includes
 * compares if the save has been successful if fail return -2
 * compares if the param out of bounds works. if fail will return -3
 * if it return something else that is not 0 error will be according to hal/errors.h
 */
int thresholdTest();

/*
 * @brief. checks if the battery returns a reasonable voltage
 * @return error according to hal/errors.h
 */
int batteryTest();

#endif
