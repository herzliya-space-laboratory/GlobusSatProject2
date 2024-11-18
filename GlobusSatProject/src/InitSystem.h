/*
 *
 * @file	InitSystem.h
 * @brief	All crucial initialization functions in one place
 * @note	Order of function calls is important. Read system documents for further analysis
 */

#ifndef INITSYSTEM_H_
#define INITSYSTEM_H_


#define MIN_2_WAIT_BEFORE_DEPLOY 120 // how many minutes to wait before we open the Ants TODO: before flight change to 30
#define RESTART_TIME 3 // how much time does it take to restart the SAT
/*!
 * @brief	Starts the FRAM using drivers, and checks for errors.
 * @see FRAM.h
 */
int StartFRAM();

/*!
 * @brief	writes the default flight parameters to the corresponding FRAM addresses
 * @see FRAM_FlightParameters.h
 */
int WriteDefaultValuesToFRAM();

/*!
 * @brief	Starts the I2C using drivers, and checks for errors.
 * @see	I2C.h
 */
int StartI2C();


/*!
 * @brief	Starts the SPI using drivers, and checks for errors
 * @see	SPI.h
 */
int StartSPI();


/*!
 * @brief	Starts the Time module using drivers, and checks for errors.
 * @see Time.h
 */
int StartTIME();

/*
 * @brief	Starts the supervisor module using drivers, and checks for errors.
 * @see supervisor.h
 */
int InitSupervisor();

/*!
 * @brief	executes all required initializations of systems, including sub-systems, and checks for errors
 * @return	0
 */
int InitSubsystems();

/*!
 * @brief	deployment procedure
 * @return	0 successful deployment
 * 			-1 failed to deploy
 */
int FirstActivition();

#endif /* INITSYSTEM_H_ */
