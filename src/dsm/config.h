/* 
 * Define here your configuration
 * - cyclecount 
 * - LoRa keys
 * - Baudrate Port
 * Marcel Meek, July 2020
 *  update 16-5-2021
 */

#ifndef _CONFIG_h /* Prevent loading library twice */
#define _CONFIG_h

// specify here TTN keys V3
#define APPEUI "70B3D57ED002E7AF"  // buitenbuurt
#define APPKEY "F4E57B1FCDF5117C7189E48699A8311E"
// DEVEU is filled by board id

// define in seconds how often a message will be sent
#define CYCLETIME 600-15         //600 sec

// Baudrate for DSM
#define BAUDRATE 0           // auto baudrate, during setup 115200 is tested. If fails it falls back is 9600 baud
//#define BAUDRATE 115200   // set baudrate fixed to 115200
//#define BAUDRATE 9600     // set baudrate fixed to 9600

#define VERSION "3.0"

#endif
