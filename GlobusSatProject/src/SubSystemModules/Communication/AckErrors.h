/*
 * AckErrors.h
 *
 *  Created on: 17 9 2024
 *      Author: maayan
 */

#ifndef ACKERRORS_H_
#define ACKERRORS_H_

#define ERROR_READ_FROM_FRAM			1
#define ERROR_WRITE_TO_FRAM				2
#define ERROR_WRITTEN_IN_FRAM_WRONG 	3

#define ERROR_WRITE_TO_I2C				10

#define ERROR_CANT_GET_TIME				15

#define ERROR_WRONG_LENGTH_DATA			17 //the packet sent wrong length of the data field in data

#define ERROR_CANT_SET_END_TIME			20

#define ERROR_CANT_DO					30 //can't do what we want it to

#define ERROR_OUT_OF_BOUND 				31
#endif /* ACKERRORS_H_ */
