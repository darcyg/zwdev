/*
 * SerialLink.h
 *
 *  Created on: 2016-5-17
 *      Author: root
 */

#ifndef SERIALLINK_H_
#define SERIALLINK_H_

#if defined __cplusplus
extern "C"

#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <stdbool.h>
#include "Serial.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define SL_READ(PDATA)		serial_read(serial_fd, PDATA)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef enum
{
	E_SL_MSG_VERSION_REQUEST = 0,
	E_SL_MSG_VERSION = 1,
	E_SL_MSG_IPV4 = 100,
	E_SL_MSG_IPV6 = 101,
	E_SL_MSG_CONFIG = 102,
	E_SL_MSG_RUN_COORDINATOR = 103,
	E_SL_MSG_RESET = 104,
	E_SL_MSG_ADDR = 105,
	E_SL_MSG_CONFIG_REQUEST = 106,
	E_SL_MSG_SECURITY = 107,
	E_SL_MSG_LOG = 108,
	E_SL_MSG_PING = 109,
	E_SL_MSG_PROFILE = 110,
	E_SL_MSG_RUN_ROUTER = 111,
	E_SL_MSG_RUN_COMMISIONING = 112,
	E_SL_MSG_ACTIVITY_LED = 113,
	E_SL_MSG_SET_RADIO_FRONTEND = 114,
	E_SL_MSG_ENABLE_DIVERSITY = 115,

	E_SL_MSG_NETWORK_KEY_REQ = 116,
	E_SL_MSG_METWORK_KEY_ACK = 117,
	E_SL_MSG_PANID_REQ = 118,
	E_SL_MSG_PANID_ACK = 119,
	E_SL_MSG_CHANNEL_REQ = 120,
	E_SL_MSG_CHANNEL_ACK = 121,
	E_SL_MSG_LOCK_TO_HOST	= 122,


	E_SL_MSG_RUN_STATUS = 20,

	E_SL_MSG_LOCK_JOIN = 30,
	E_SL_MSG_LOCK_LEFT = 31,
	E_SL_MSG_LOCK_OUTPUT = 32,
	E_SL_MSG_LOCK_DATA = 33,
	E_SL_MSG_LOCK_UNIXTIME = 34,
	E_SL_MSG_LOCK_BATTERY = 35,
	E_SL_MSG_LOCK_GET_BATTERY = 36,

	E_SL_MSG_COMMISSION_ENABLE = 40,

	E_SL_MSG_DATA_SetRes = 50,
	E_SL_MSG_DATA_GetRes = 51,
	E_SL_MSG_DATA_RESULT = 52,

	E_SL_MSG_STACK_STARTED = 60,
	E_SL_MSG_STACK_JOINED = 61,
} teSL_MsgType;

#ifndef FALSE
#define FALSE false
#define TRUE  true
#endif
/** Typedef bool to builtin integer */
/*
typedef enum
{
#ifdef FALSE
#undef FALSE
#endif
	FALSE = 0,
#ifdef TRUE
#undef TRUE
#endif
	TRUE = 1,
} bool;
*/
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

bool bSL_ReadMessage(uint8_t *pu8Type, uint16_t *pu16Length,
		uint16_t u16MaxLength, uint8_t *pu8Message);
void vSL_WriteMessage(uint8_t u8Type, uint16_t u16Length, uint8_t *pu8Data);

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif /* SERIALLINK_H_ */
