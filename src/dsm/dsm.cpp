/*--------------------------------------------------------------------
  De Slimme Meter
  Ths class reads and parses the Dutch Smart Meter

  Author Marcel Meek
  Date 12/7/2020
  --------------------------------------------------------------------*/

#include <Regexp.h>
#include "dsm.h"

// define regular expression for ISKRA meter
#define METERID   "0%-0:96%.1%.1%((.%x+)%)"
#define LAAG      "1%-0:1%.8%.1%((%d+%.%d+)%*"
#define HOOG      "1%-0:1%.8%.2%((%d+%.%d+)%*"
#define LAAGTERUG "1%-0:2%.8%.1%((%d+%.%d+)%*"
#define HOOGTERUG "1%-0:2%.8%.2%((%d+%.%d+)%*"
//#define GAS       "%c%((%d+%.%d+)%)"
#define GAS       "0%-1:24%.2%.1.*%(.+%).*%((%d+%.%d+)"

Dsm::Dsm( Stream &port) {
  p1 = &port;
  id = i = 0;
  laag = hoog = laagTerug = hoogTerug = gas = 0.0;  
}

// read telegram, return true if a complete telegram is received
bool Dsm::read() {
  stx = false;
  i = 0;
  int milliCount = millis();
  bool complete = false;   // end of message
  while( millis() - milliCount < 15000 && !complete )
    complete = rawRead();

  if (complete)
    parseMsg();

  return complete;
}

// read bytes, call n times until end of message messags is received
bool Dsm::rawRead() {
  while( p1->available()) {
    buf[ i] = p1->read() & 0x7f;
    
    // check start of message
    if( buf[ i] == '/' ||                                 // start of message
        ( i>=2 && strncmp( &buf[i-2], "KFM", 3)==0) ) {   // start of messgae i.c.o. Kaifa Meters
      //printf( "ch=%c, i=%d\n", buf[ i], i);
      stx = true;
      i = 0;
    }    
    else if( stx &&  buf[ i] == '!') {      // end of message reached
      //printf( "eof=%c, i=%d\n", buf[ i], i);
      buf[ i] = '\0';
      //printf("buf=%s\n", buf);
      i=0;
      stx = false;
      return true;
    }
    else {
      i++;
    }
    
    if( i >= BUF_SZ-1 ) {   // buffer overflow, skip all
      printf("buffer overlow %d\n", i);
      i=0;
      stx = 0;
      return false;
    }
  }
  return false;
}

// save paramters from telegram
void Dsm::parseMsg( ) {
  MatchState ms (buf);

 // get meter values
  if( ms.Match( METERID) == 1) {
    char meterid[40];
    convertHexToBytes( meterid, ms.GetCapture(buf, 0));  // convert hex ascii to ascii
    //id = jenkins_one_at_a_time_hash( meterid);          // hash the meter id
    id = atoi(&meterid[ strlen(meterid) -9]);  // get last 9 decimal numbers and put it in an uint32
    //printf( "meterid=%s, %d\n", meterid, id);
  }
 
  if( ms.Match( LAAG) == 1) {
    laag = atof( ms.GetCapture(buf, 0));
  }

  if( ms.Match( HOOG) == 1) {
    hoog = atof( ms.GetCapture(buf, 0));
  }

  if( ms.Match( LAAGTERUG) == 1) {
    laagTerug = atof( ms.GetCapture (buf, 0));
  }
  
  if( ms.Match( HOOGTERUG) == 1) {
    hoogTerug = atof( ms.GetCapture (buf, 0));
  }

  if( ms.Match( GAS) == 1) {
    gas = atof( ms.GetCapture (buf, 0));
  }
}

void Dsm::convertHexToBytes( char* dst, char* src){
  int len = strlen( src);
  for (int i = 0; i < len; i+=2){
    char substr[3];
    char* strSel = &(src[i]);
    strncpy(substr, strSel, 2);
    substr[2] = '\0';
    dst[i/2] = (char)(strtol(substr, NULL, 16));            
  }
  dst[len/2] = '\0';
}
/*
uint32_t Dsm::jenkins_one_at_a_time_hash( char* key) {
  uint32_t hash, i;
  int len = strlen( key);
  for (hash = i = 0; i < len; ++i) {
    hash += key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  //printf("hash=%d\n", hash);
  return hash;
}
*/

void Dsm::debug( ) {
    printf( "meterid: %X\n", id);
    printf( "laag: %.3f\n", laag);
    printf( "hoog: %.3f\n", hoog);
    printf( "laagterug: %.3f\n", laagTerug);
    printf( "hoogterug: %.3f\n", hoogTerug);
    printf( "gas: %.3f\n", gas); 
}
