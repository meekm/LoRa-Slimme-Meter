/*--------------------------------------------------------------------
  De Slimme Meter
  Ths class reads and parses the Dutch Smart Meter

  Author Marcel Meek
  Date 12/7/2020
  --------------------------------------------------------------------*/

#ifndef __DSM_H_
#define __DSM_H_
#include <Arduino.h>

#define BUF_SZ 2500

class Dsm {
  public:
    Dsm( Stream &port);
    bool read();
    void debug();

    uint32_t id;   // hashed meterid
    float laag, hoog, laagTerug, hoogTerug, gas;  

  private:
    bool rawRead();
    void parseMsg();
    void convertHexToBytes( char* dst, char* src);
    uint32_t jenkins_one_at_a_time_hash( char* key);
    Stream *p1;
    bool stx = false;
    char buf[ BUF_SZ];
    int i = 0;
}; 

#endif //_DSM_H_
