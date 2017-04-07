/****************************************************************************
 *
 * Copyright (c) 2001-2013
 * Sigma Designs, Inc.
 * All Rights Reserved
 *
 *---------------------------------------------------------------------------
 *
 * Description:       Functions for controlling application board via the UART
 *
 * Last Changed By:  $Author: tro $
 * Revision:         $Revision: 25261 $
 * Last Changed:     $Date: 2013-04-03 12:57:19 +0200 (on, 03 apr 2013) $
 *
 ****************************************************************************/

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include "ZW_SerialAPI.h"
#include "conhandle.h"
#include "serial.h"
#include "common.h"
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>


/****************************************************************************/
/*                      PRIVATE TYPES and DEFINITIONS                       */
/****************************************************************************/
/* serial Protocol handler states */
enum
{
  stateSOFHunt = 0,
  stateLen = 1,
  stateType = 2,
  stateCmd = 3,
  stateData = 4,
  stateChecksum = 5
};


/****************************************************************************/
/*                              PRIVATE DATA                                */
/****************************************************************************/
char 					serBuf[SERBUF_MAX];
int 					serBufLen, con_state;
char 					bChecksum_RX;
static long 	timeOutRX_ACKStart;
static long 	timeOutRX_BYTEStart;
int 					rxActive = 0;
int 					AckNakNeeded = 0;


/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/
long timeOutRX_ACK 	= RX_ACK_TIMEOUT_DEFAULT;
long timeOutRX_BYTE = RX_BYTE_TIMEOUT_DEFAULT;



/****************************************************************************/
/*                            PRIVATE FUNCTIONS                             */
/****************************************************************************/
//Start sending frame to the host.

//static int serial_fd = -1;
static int serial_fd = 0;
static void ZW_SerialPutByte(char x) {
	serial_write(serial_fd, x);
}
static void ZW_SerialFlush(){
	serial_flush(serial_fd);
}
static bool ZW_SerialCheck() {
	struct timeval tv = {0, 8000};
	fd_set fds;
	FD_ZERO(&fds);
	//FD_SET(serial_fd, &fds);
	FD_SET(serial_fd, &fds);
	int ret = select(serial_fd + 1, &fds, NULL, NULL, &tv);
	if (ret < 0) {
		return false;
	}
	if (ret == 0) {
		return false;
	}
	return true;
}
static int  ZW_SerialGetByte() {
	unsigned char x;
	serial_read(serial_fd, &x);
	return x;
}
static void ZW_InitSerialIf(int bBaudRate) {
	serial_open("/dev/ttyUSB0", 115200);
}
static void ZW_FinishSerialIf() {
	return;
}
/*===============================  Tick Get   ============================
**    Get then system time tick
**
**--------------------------------------------------------------------------*/
static long getTickTime() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

/*===============================   TimerReset   ============================
**    Reset startTick to current timer tick
**
**--------------------------------------------------------------------------*/
static void                          /*RET Nothing */
TimerReset(long *pwTimerStartTick)   /*IN  Nothing */
{
  *pwTimerStartTick = getTickTime();
}


/*=============================   TimerGetElapsedTime   ======================
**    Get elapsed time
**
**--------------------------------------------------------------------------*/
static long                                /* RET  Elapsed Time in 10 ms ticks */
TimerGetElapsedTime(long wTimerStartTick)  /* IN   Nothing */
{
  return (getTickTime() - wTimerStartTick);
}


/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/

/*===============================   ConTxFrame   =============================
**
**   Transmit frame via serial port by adding SOF, Len, Type, cmd and Chksum.
**   Frame  : SOF-Len-Type-Cmd-DATA-Chksum
**    where SOF is Start of frame byte
**          Len is length of bytes between and including Len to last DATA byte
**          Type is Response or Request
**          Cmd Serial application command describing what DATA is
**          DATA as it says
**          Chksum is a XOR checksum taken on all BYTEs from Len to last DATA byte
**
**          NOTE: If Buf is NULL then the previously used cmd, type, Buf and len
**          is used again (Retransmission)
**
**--------------------------------------------------------------------------*/
void          /*RET Nothing */
ConTxFrame(
  char cmd,   /* IN Command */
  char type,  /* IN frame Type to send (Response or Request) */
  char *Buf, /* IN pointer to BYTE buffer containing DATA to send */
  int len)   /* IN the length of DATA to transmit */
{
  static char wCmd, wType, *wBuf;
  static int wLen;
	int i;
  char bChecksum;


  if (Buf != NULL)
  {
    wBuf = Buf;
    wLen = len;
    wCmd = cmd;
    wType = type;
  }
  bChecksum = 0xFF; /* Initialize checksum */
	
  ZW_SerialPutByte(SOF);
  ZW_SerialPutByte(wLen + 3);  // Remember the 'len', 'type' and 'cmd' bytes
  bChecksum ^= wLen + 3;
  ZW_SerialPutByte(wType);
  bChecksum ^= wType;
  ZW_SerialPutByte(wCmd);
  bChecksum ^= wCmd;
  for (i = 0; (wBuf != NULL) && (i < wLen); i++)
  {
    ZW_SerialPutByte(wBuf[i]);
    bChecksum ^= wBuf[i];
  }
  ZW_SerialPutByte(bChecksum);       // XOR checksum of
  ZW_SerialFlush();                //Start sending frame to the host.
  AckNakNeeded = 1;  // Now we need an ACK...
  TimerReset(&timeOutRX_ACKStart); /* Reset ACK timeout */
}


/*==============================   ConUpdate   =============================
**
**   Parses serial data sent from external controller module (PC-controller).
**   Should be frequently called by main loop.
**
**   Return: conIdle          if nothing has happened
**           conFrameReceived if valid frame was received
**           conFrameSent     if transmitted frame was ACKed by other end
**           conFrameErr      if received frame has error in Checksum
**           conRxTimeout     if a Rx timeout happened
**           conTxTimeout     if a Tx timeout waiting for ACK happened
**
**--------------------------------------------------------------------------*/
enum T_CON_TYPE     /*RET conState - See above */
ConUpdate(
  char acknowledge) /* IN do we send acknowledge and handle frame if received correctly */
{
  char c;
  enum T_CON_TYPE retVal = conIdle;


  if (ZW_SerialCheck())
  {
    do
    {
      c = ZW_SerialGetByte();

      switch (con_state)
      {
        case stateSOFHunt :
          if (c == SOF)
          {
            bChecksum_RX = 0xff;
            serBufLen = 0;
            rxActive = 1;  // now we're receiving - check for timeout
            con_state++;
            TimerReset(&timeOutRX_BYTEStart); /* Reset Inframe Byte timeout */
          }
          else
          {
            if (AckNakNeeded)
            {
              if (c == ACK)
              {
                retVal = conFrameSent;
                AckNakNeeded = 0;  // Done
              }
              else if (c == NAK)
              {
                retVal = conTxTimeout;
                AckNakNeeded = 0;
              }
              else
              {
                // Bogus character received...
              }
            }
          }
          break;

        case stateLen :
          // Check for length to be inside valid range
          if ((c < FRAME_LENGTH_MIN) || (c > FRAME_LENGTH_MAX))
          {
            con_state = stateSOFHunt; // Restart looking for SOF
            rxActive = 0;  // Not really active now...
            break;
          }
          // Drop through...

        case stateType :
          if (serBufLen && (c > RESPONSE))
          {
            con_state = stateSOFHunt; // Restart looking for SOF
            rxActive = 0;  // Not really active now...
            break;
          }
          // Drop through...

        case stateCmd :
          con_state++;
          // Drop through...

        case stateData :
          if (serBufLen < SERBUF_MAX)
          {
            serBuf[serBufLen] = c;
            serBufLen++;
            bChecksum_RX ^= c;
            if (serBufLen >= serFrameLen)
            {
              con_state++;
            }
          }
          else
          {
            con_state++;
          }
          TimerReset(&timeOutRX_BYTEStart); /* Reset Inframe Byte timeout */
          break;

        case stateChecksum :
          /* Do we send ACK/NAK according to checksum... */
          /* if not then the received frame is dropped! */
          if (acknowledge)
          {
            if (c == bChecksum_RX)
            {
              ZW_SerialPutByte(ACK);
              ZW_SerialFlush();                //Start sending frame to the host.
              retVal = conFrameReceived;  // Tell THE world that we got a packet
            }
            else
            {
              ZW_SerialPutByte(NAK);       // Tell them something is wrong...
              ZW_SerialFlush();                //Start sending frame to the host.
              retVal = conFrameErr;
            }
          }
          else
          {
            // We are in the process of looking for an acknowledge to a callback request
            // Drop the new frame we received - we don't have time to handle it.
            // Send a CAN to indicate what is happening...
            ZW_SerialPutByte(CAN);
            ZW_SerialFlush();                //Start sending frame to the host.
          }
          // Drop through

        default :
          con_state = stateSOFHunt; // Restart looking for SOF
          rxActive = 0;  // Not really active now...
          break;
      }
    } while ((retVal == conIdle) && ZW_SerialCheck());
  }
  /* Check for timeouts - if no other events detected */
  if (retVal == conIdle)
  {
    /* Are in the middle of collecting a frame and have we timed out? */
    if (rxActive && (TimerGetElapsedTime(timeOutRX_BYTEStart) >= timeOutRX_BYTE))
    {
      /* Reset to SOF hunting */
      con_state = stateSOFHunt;
      rxActive = 0; /* Not inframe anymore */
      retVal = conRxTimeout;
    }
    /* Are we waiting for ACK and have we timed out? */
    if (AckNakNeeded && (TimerGetElapsedTime(timeOutRX_ACKStart) >= timeOutRX_ACK))
    {
      /* Not waiting for ACK anymore */
      AckNakNeeded = 0;
      /* Tell upper layer we could not get the frame through */
      retVal = conTxTimeout;
    }
  }
  return retVal;
}


/*==============================   ConInit   =============================
**
**   Initialize the module.
**
**--------------------------------------------------------------------------*/
void                /*RET  Nothing */
ConInit(
  int bBaudRate)  /* IN  Baud rate / 100 (e.g. 96=>9600baud/s, 1152=>115200baud/s) */
{

  //0x0658idVendor=sigma. Insert your own idVendor here;
  //0x0200: idProduct=SerialAPI. Insert your own idProduct here;
  //Version 0x001
  //ZW_Init_USB_Vendor(0x0658, 0x0200,0x001);
  ZW_InitSerialIf(bBaudRate);
  ZW_FinishSerialIf();

  serBufLen = 0;
  con_state = stateSOFHunt;
}
