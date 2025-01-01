
#ifndef TELEMETRYFILES_H_
#define TELEMETRYFILES_H_
//	---general

#define END_FILE_NAME_TX					"tx"
#define END_FILE_NAME_RX					"rx"
#define END_FILE_NAME_ANTENNA_SIDE_ZERO		"ata"
#define END_FILE_NAME_ANTENNA_SIDE_ONE		"atb"
#define END_FILENAME_WOD_TLM				"wod"
#define END_FILENAME_EPS_TLM				"eps"
#define	END_FILENAME_SOLAR_PANELS_TLM		"slr"
#define	END_FILENAME_LOGS					"log"
#define END_FILENAME_RADFET_TLM				"rad"
#define END_FILENAME_SEL_TLM				"sel"
#define END_FILENAME_SEU_TLM				"seu"


typedef enum {
// don't change the position of these
	tlm_eps, // 0
	tlm_tx,
	tlm_ants0,
	tlm_solar,
	tlm_wod,
	tlm_radfet,
	tlm_sel,
	tlm_seu,
// don't change the position of these

	tlm_rx, // 8
	tlm_ants1,
	tlm_log
}tlm_type_t;
#endif /* TELEMETRYFILES_H_ */
