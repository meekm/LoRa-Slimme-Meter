/* 
 * Define here your configuration
 * - cyclecount 
 * - LoRa keys
 * - Baudrate Port
 * Marcel Meek, July 2020
 * 
 */

#ifndef _CONFIG_h /* Prevent loading library twice */
#define _CONFIG_h

// specify here TTN keys
#define APPEUI "0000000000000000"
#define DEVEUI "0000000000000000"
#define APPKEY "00000000000000000000000000000000"

// define in milleseconds how often a message will be sent
#define CYCLETIME 600000  //10 minutes


// You must use Hardware Serial if baudrate > 57.6K
// use HW serial
#undef SW_SERIAL
#define BAUDRATE 115200
#define VERSION "1.1"
#endif
