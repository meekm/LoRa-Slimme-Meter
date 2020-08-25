/*--------------------------------------------------------------------
  De Slimme Meter DSM
  Reads the Dutch Smart Meter and send data to LoRa network

  Author Marcel Meek
  Date 12/7/2020
  --------------------------------------------------------------------*/
#include <Arduino.h>
#include <SoftwareSerial.h>

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
SoftwareSerial P1(35, 13);    // 35=rx, 13=tx
LoRa lora;
Dsm dsm( P1);                 //  De slimme meter


long milliCount = 0;
long cycleTime = CYCLETIME;
bool ttnOk = false;
bool dsmOk = false;

// LoRa receive handler (downnlink)
void loracallback( unsigned int port, unsigned char* msg, unsigned int len) {
  printf("lora download message received port=%d len=%d\n", port, len);

  // change cycle time in seconds with a remote TTN downlink command
  // port is 40, byte 0 is low byte 1 is high byte
  if ( port == 40 && len >= 2) {
    int value = msg[0] + 256 * msg[1];
    if ( value >= 20 && value <= 3600) {
      cycleTime = 1000 * value;
      printf( "cycleTime changed to %d sec.\n" , value);
    }
  }
}

void setup() {

  //initialize Serial Monitor
  Serial.begin(115200);
  P1.begin( BAUDRATE);

  // LoRa send LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite( LED_BUILTIN, LOW);

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

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
  display.setCursor(0, 22);  display.printf("Starting up ...");
  display.display();
  lora.receiveHandler( loracallback);     // set LoRa receive handler (downnlink)
  lora.begin();                           // Lora join
  printf("Initializing OK!\n");
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
  display.setCursor(0, 44); display.printf( " Id %X %d", dsm.id, atoi(DEVEUI));
  display.setCursor(0, 55); display.printf( " DSM %s", (dsmOk) ? "ok" : "fail");
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
  digitalWrite( LED_BUILTIN, HIGH);
  ttnOk = lora.sendMsg( 40, (unsigned char*)&payload, sizeof( payload));  // use port 40
  digitalWrite( LED_BUILTIN, LOW);
}

void loop() {

  // cycle time is 10 min. when connected to ttn, cycle time is 20sec when ttn fails
  if ( millis() - milliCount > ((ttnOk) ? cycleTime : 20000)) {
    milliCount = millis();
    sendToTTN();
    displayOLED();    // display after ttn send
    dsmOk = false;
  }

  if( dsm.read()) {
    dsmOk = true;    // complete telegram received
    displayOLED();   // display it
  }

  lora.process();
}
