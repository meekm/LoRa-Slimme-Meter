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
 * \file lora.h
 * \brief BasicMAC wrapper class for TTN-Apeldoorn Sound Sensor.
 * 
 * \author Remko Welling
 * \date See revision table
 * \version see revision table
 * 
 * ## version
 * 
 * version | date       | who            | Comment
 * --------|------------|----------------|-------------------------------------
 * 0.1     | 10-7-2020  | Remko Welling  | Initial version
 * 0.2     | 31-5-2020  | Marcel Meek    | timout added when a join or send fails
 * 0.3     | 1-6-2020   | Marcel Meek    | callback for downlink added, static buffers moved from .h file to .cpp file
 *
 * # References
 *
 * -
 *
 * # Dependencies
 * 
 * This class depends on BasicMAC by Matthijs Kooyman. Download BasicMAC from https://github.com/pe1mew/basicmac
 * and install the Arduino library as described in the readme.
 * 
 * Hardware configuration of the RFM95 is managed in file lora.cpp in a similar manner as
 * BasicMAC is delivering the default board configuration.
 */

#ifndef __LORA_H_
#define __LORA_H_

#include <stdint.h> // uint8_t type
#include "config.h"

/// \brief C++ wrapper around LMIC LoRaWAN stack
class LoRa{
  public:
    
    LoRa();
    ~LoRa();
    
    /// \brief start BasicMAC stack
    /// Starting with OTAA
    void begin();
    
    /// \brief function to be called to allow BasicMAC OS to operated.
    /// Shall be called periodically to let BasicMAC operate.
    void process();

    /// \brief send data to LoRaWAN
    /// \param [in] port application port in LoRaWAN application (1 to 99)
    /// \param [in] buf pointer to character array that contains payload to be sent.
    /// \param [in] len Lenght of payload to be sent.
    bool sendMsg( int port, uint8_t* buf, int len);
 
    /// \brief specify callback funftion, this function is called when downlink data has been receieved
    /// \param port
    /// \param buffer
    /// \param length of buffer
    void receiveHandler( void (*callback)(unsigned int, uint8_t*, unsigned int));

  private:
};

#endif // __LORA_H_
