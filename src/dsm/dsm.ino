/*--------------------------------------------------------------------
  De Slimme Meter DSM
  Reads the Dutch Smart Meter and send data to LoRa network

  Author Marcel Meek
  Date 12/7/2020
  changed 16-5-2021
  - MCCI Catena lora stack
  - worker loop changed
  - deveui from chipid
  - dsm parser updated 
  - detect auto baudrate 115K2 or 9600
  - sw serial removed
  --------------------------------------------------------------------*/
#include <Arduino.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "lora.h"
#include "dsm.h"

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16   // !!!! helaas ook RX UART 2 en daarom niet bruikbaar :-(
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

Dsm dsm( Serial);                   // DSM on HW Serial port

int baudrate = 115200;
long cycleTime = CYCLETIME;
bool ttnOk = false;
bool dsmOk = false;
char deveui[32];

// LoRa receive handler (downnlink)
void loracallback( unsigned int port, unsigned char* msg, unsigned int len) {
  printf("lora download message received port=%d len=%d\n", port, len);

  // change cycle time in seconds with a remote TTN downlink command
  // port is 40, byte 0 is low byte 1 is high byte
  if ( port == 40 && len >= 2) {
    int value = msg[0] + 256 * msg[1];
    if ( value >= 20 && value <= 3600) {
      cycleTime = value;
      printf( "cycleTime changed to %d sec.\n" , value);
    }
  }
}

void setup() {
  if( BAUDRATE) 
    baudrate = BAUDRATE;    // fixed baudrate
  else 
    baudrate = 115200;      // auto baudrate, start with 115200
  Serial.begin( baudrate);  // take care, debug port and P1 are the same
  delay(200);

  // LoRa send LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, LOW);   // off

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  // get chip id, to be used for DEVEUI LoRa
  uint64_t chipid = ESP.getEfuseMac();   //The chip ID is essentially its MAC address(length: 6 bytes).
  sprintf(deveui, "%08X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
  printf("deveui=%s\n", deveui);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    printf("SSD1306 allocation failed\n");
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setRotation( 2);  // rotate 180 degrees
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 11);  display.printf("TTN Slimme meter %s", VERSION);
  display.setCursor(0, 22);  display.printf("Id %s", deveui + 4);
  display.setCursor(0, 33);  display.printf("Starting up ...");
  display.display();

  // if auto Baudrate is enabled, check the speed of dsm
  if( !BAUDRATE) {   // test if a message can be read on 115200
    dsmOk = dsm.read();
    if( !dsmOk) {   // if not ok, fall back to 9600 baud 
      baudrate = 9600;
      Serial.end();
      delay( 200);
      Serial.begin( baudrate);
      delay( 200);
    }
  }
  printf("baudrate=%d\n", baudrate);

//initialize LoRa
  loraBegin( APPEUI, deveui, APPKEY);
  loraSetRxHandler( loracallback);    // set LoRa receive handler (downnlink)
  loraSetWorker( loraWorker);         // set Worker handler
  loraSetTxComplete( loraTxComplete); // set Tx complete handler

  printf("Initializing OK!\n");
  loraWorker();                     // Call the worker once from here
}

void displayOLED() {
  display.clearDisplay();
  display.setRotation( 2);  // rotate 180 degrees
  display.setCursor(0, 0);  display.printf( " Laag");
  display.setCursor(64, 0); display.printf( " Hoog");
  display.setCursor(0, 11); display.printf( " %.3f", dsm.laag);
  display.setCursor(64, 11); display.printf( " %.3f", dsm.hoog);
  display.setCursor(0, 22); display.printf( "-%.3f", dsm.laagTerug);
  display.setCursor(64, 22); display.printf( "-%.3f", dsm.hoogTerug);
  display.setCursor(0, 33); display.printf( " Gas %.3f", dsm.gas);
  //display.setCursor(0, 44); display.printf( " Id %X %s", dsm.id, deveui + 4);
  display.setCursor(0, 44); display.printf( " Id %s", deveui + 4);
  display.setCursor(0, 55); 
  if(dsmOk) 
    display.printf( " DSM %d", baudrate);
  else 
    display.printf( " DSM fail");
  display.setCursor(64, 55); display.printf( " TTN %s", (ttnOk) ? "ok" : "fail");
  display.display();
}

void sendToTTN( ) {
  struct {
    uint32_t id;
    float laag, hoog, laagterug, hoogterug, gas;
  } payload;

  payload.id = dsm.id;
  payload.laag = dsm.laag;
  payload.hoog = dsm.hoog;
  payload.laagterug = dsm.laagTerug;
  payload.hoogterug = dsm.hoogTerug;
  payload.gas = dsm.gas;

  printf("send TTN len=%d\n", sizeof( payload));
  loraSend( 40, (unsigned char*)&payload, sizeof( payload));  // use port 40
}

void loraTxComplete( bool ok) {
   ttnOk = ok;
   displayOLED();   // display it
   digitalWrite( LED_BUILTIN, LOW);
}

void loraWorker( ) {
  printf("Worker\n");
  dsmOk = dsm.read();
  //dsm.debug();
  //printf("end read DSM, baudrate=%d, dsmOk=%d\n", baudrate, dsmOk);
  digitalWrite( LED_BUILTIN, HIGH);
  sendToTTN();
  loraSleep( cycleTime);
}

void loop() {
   loraProcess();
}
