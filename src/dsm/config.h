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
#define APPEUI "70B3D57ED0031D76"
#define DEVEUI "0000000000000001"
#define APPKEY "5D80E9639C45212F05F1230D4DCBC259"

// define in milleseconds how often a message will be sent
#define CYCLETIME 600000  //10 minutes

#define BAUDRATE 9600
#define VERSION "1.0"

#endif
