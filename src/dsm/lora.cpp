/*--------------------------------------------------------------------
  This file is part of the TTN-Apeldoorn Sound Sensor.

  This code is free software:
  you can redistribute it and/or modify it under the terms of a Creative
  Commons Attribution-NonCommercial 4.0 International License
  (http://creativecommons.org/licenses/by-nc/4.0/) by
  TTN-Apeldoorn (https://www.thethingsnetwork.org/community/apeldoorn/) 

  The program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  --------------------------------------------------------------------*/

/*!
 * \file lora.cpp
 * 
 */

#include <Arduino.h>
#include <basicmac.h>
#include <hal/hal.h>
#include <SPI.h>

#include "lora.h"

#if defined(ARDUINO_ESP32_DEV) 
// Assume Sparkfun LoRa Gateway board
const lmic_pinmap lmic_pins = {
    .nss = 16,
    // RX/TX is controlled through RXTX by the SX1272 directly on the RFM95W
    .tx = LMIC_UNUSED_PIN,
    .rx = LMIC_UNUSED_PIN,
    // RST is hardwarid to MCU reset
    .rst = 5,
    .dio = {26, 33, 32},
    .busy = LMIC_UNUSED_PIN,
    .tcxo = LMIC_UNUSED_PIN,
};
#elif defined(ARDUINO_TTGO_LoRa32_V1)
// Assume TTGO LoRa32 board
const lmic_pinmap lmic_pins = {
    .nss = 18,
    // RX/TX is controlled through RXTX by the SX1272 directly on the RFM95W
    .tx = LMIC_UNUSED_PIN,
    .rx = LMIC_UNUSED_PIN,
    // RST is hardwarid to MCU reset
    .rst = 14,
    .dio = {26, 33, 32},
    .busy = LMIC_UNUSED_PIN,
    .tcxo = LMIC_UNUSED_PIN,
};
#else
  #error "Unknown board, no standard pimap available. Define your own in the main sketch file."
#endif

const int      TxTIMEOUT = 10000;                   /// wait max TxTIMEOUT msec. after a join or send
const uint8_t  DOWNLINK_DATA_SIZE = 52;             ///< Maximum payload size (not enforced)

static bool    txBusy {false};
static uint8_t downlinkPort {0};                    ///< used uplink port
static uint8_t downlinkData[DOWNLINK_DATA_SIZE] {}; ///< uplink data
static uint8_t downlinkDataSize {0};                ///< Size of downlink data received

static void (*_callback)(unsigned int, uint8_t*, unsigned int) = NULL;     // TTN receive handler

LoRa::LoRa() {
 }

void LoRa::begin(){
  // LMIC init
  os_init(nullptr);
  LMIC_reset();

  // Enable this to increase the receive window size, to compensate
  // for an inaccurate clock.  // This compensate for +/- 10% clock
  // error, a lower value will likely be more appropriate.
  // LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);
  // Start join
  LMIC_startJoining();

  // Optionally wait for join to complete (uncomment this is you want
  // to run the loop while joining).
 // while ((LMIC.opmode & (OP_JOINING))){
    os_runstep();
 // }

  // now we have joined enable ADR and link-check
  LMIC_setAdrMode(1);
  LMIC_setLinkCheckMode(1);
}

LoRa::~LoRa() {
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
}

bool LoRa::sendMsg(int port, uint8_t* buf, int len){
  os_runstep();
 
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & (OP_JOINING|OP_TXRXPEND)){
    printf("OP_TXRXPEND or OP_JOINING, not sending\n");
    return false;
  }
  else{
    // Prepare upstream data transmission at the next possible time.
    txBusy= true;
    LMIC_setTxData2( port, buf, len, 0);
    uint32_t start = millis();
    while ( txBusy && (millis() - start < TxTIMEOUT) ) {         //MM,  wait max TxTIMEOUT ms
      os_runstep();
    }
    if ( txBusy) {
      LMIC_reset();
      txBusy = false;
      printf( "ttn send failed\n");
      return false;
    }
  }
  return true;
}

void LoRa::process() {
  // Let LMIC handle background tasks
  os_runstep();
}

void LoRa::receiveHandler( void (*callback)(unsigned int, uint8_t*, unsigned int)) {
  _callback = callback;
}

// handle TTN keys
void convertStringToByteArray(unsigned char* byteBuffer, const char* str);
void os_getJoinEui (u1_t* buf) { convertStringToByteArray( buf, APPEUI);}
void os_getDevEui (u1_t* buf) { convertStringToByteArray( buf, DEVEUI);}
void os_getNwkKey (u1_t* buf) { convertStringToByteArray( buf, APPKEY);}

// The region to use
u1_t os_getRegion (void) { return REGCODE_EU868; }

void onLmicEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            txBusy = false;           // MM added, a succesful join must also reset this flag
            Serial.println(F("EV_JOINED"));
            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            txBusy = false;
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              printf("Received %d bytes of payload\n", LMIC.dataLen);
              downlinkDataSize = LMIC.dataLen;
              downlinkPort = LMIC.frame[LMIC.dataBeg-1];
              memcpy( downlinkData, &LMIC.frame[LMIC.dataBeg], downlinkDataSize);
              //for( int i = 0; i < downlinkDataSize; i++)
              //  downlinkData[i] = LMIC.frame[LMIC.dataBeg+i];
              // MM callback added
              if( _callback != NULL) {
               _callback( downlinkPort, downlinkData, downlinkDataSize);
              }
            }
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        case EV_SCAN_FOUND:
            Serial.println(F("EV_SCAN_FOUND"));
            break;
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXDONE:
            Serial.println(F("EV_TXDONE"));
            break;
        case EV_DATARATE:
            Serial.println(F("EV_DATARATE"));
            break;
        case EV_START_SCAN:
            Serial.println(F("EV_START_SCAN"));
            break;
        case EV_ADR_BACKOFF:
            Serial.println(F("EV_ADR_BACKOFF"));
            break;

         default:
            Serial.print(F("Unknown event: "));
            Serial.println(ev);
            break;
    }
}

// convert TTN keys strings to byte array
void convertStringToByteArray(unsigned char* byteBuffer, const char* str){
  int len = strlen(str);
  for (int i = 0; i < len; i+=2){
    char substr[3];
    const char* strSel = &(str[i]);
    strncpy(substr, strSel, 2);
    substr[2] = '\0';
    if( len == 16){
      byteBuffer[((len-1)-i)/2] = (unsigned char)(strtol(substr, NULL, 16));   // reverse the TTN KEY string !!
    }else if( len == 32){
      byteBuffer[i/2] = (unsigned char)(strtol(substr, NULL, 16));             // don't reverse the TTN KEY
    }else{
      printf( "Incorrect TTN key length: %s\n", str);
      break;
    }  
  }
}
