#ifndef EPSOPERATIONMODES_H_
#define EPSOPERATIONMODES_H_


#include "GlobalStandards.h"
#include "SubSystemModules/Payload/payload_drivers.h"
#include <stdio.h>


#define CHANNELS_OFF 0x00 	//!< channel state when all systems are off
#define CHNNELS_ON	 0XFF	//!< channel
#define SYSTEM0		 0x01	//!< channel state when 'SYSTEM0' is on
#define SYSTEM1		 0x02	//!< channel state when 'SYSTEM1' is on
#define SYSTEM2 	 0x04	//!< channel state when 'SYSTEM2' is on
#define SYSTEM3		 0x08	//!< channel state when 'SYSTEM3' is on
#define SYSTEM4		 0x10	//!< channel state when 'SYSTEM4' is on
#define SYSTEM5		 0x20	//!< channel state when 'SYSTEM5' is on
#define SYSTEM6 	 0x40	//!< channel state when 'SYSTEM6' is on
#define SYSTEM7 	 0x80	//!< channel state when 'SYSTEM7' is on

typedef enum{
	OperationalMode,
	CruiseMode,
	PowerSafeMode
}EpsState_t;

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Operational mode
 * @return	0
 */
int EnterOperationalMode();

/*!
 * @brief Check if we need to be in Operational or Cruise
 * @return	0
 */
int BetweenOperationalToCruise();

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Cruise mode
 * @return	0
 */
int EnterCruiseMode();

/*!
 * @brief Check if we need to be in Cruise or Power safe mode
 * @return	0
 */
int BetweenCruiseToPowerSafeMode();

/*!
 * @brief Executes the necessary procedure in order to initiate the system into Power Safe mode
 * @return	0
 */
int EnterPowerSafeMode();


/*!
 * returns the current system state according to the EpsState_t enumeration
 * @return system state according to EpsState_t
 */
EpsState_t GetSystemState();


/*
 * Get if the Tx flag is on or off for us to put in the CheckAllowed in the TRXVU.c code.
 * */
Boolean GetTxFlag();

/*
 * Get if the Payload flag is on or off.
 * */
Boolean GetPayloadFlag();

#endif /* EPSOPERATIONMODES_H_ */
