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
#define GAS       "0%-1:24%.2%.1.*%(.+%).*%((%d+%.%d+)"

Dsm::Dsm( Stream &port) {
  p1 = &port;
  i = 0;
  id = 0;
  laag = hoog = laagTerug = hoogTerug = gas = 0.0;  
}

// read bytes, return true if a complete telegram has been receiverd
bool Dsm::read() {
  while( p1->available()) {
    char ch = p1->read();
    ch &= 0x7f;
    if( ch == '!' || i >= BUF_SZ-1) {    // eof or max size reached
      buf[ i] = '\0';
      i=0;
      parseMsg();
      return true;
    }
    else { 
      buf[i++] = ch;
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
    id = jenkins_one_at_a_time_hash( meterid);          // hash the meter id
  }
 
  if( ms.Match( LAAG) == 1)
    laag = atof( ms.GetCapture(buf, 0));

  if( ms.Match( HOOG) == 1)
    hoog = atof( ms.GetCapture(buf, 0));

  if( ms.Match( LAAGTERUG) == 1)
    laagTerug = atof( ms.GetCapture (buf, 0));

  if( ms.Match( HOOGTERUG) == 1)
    hoogTerug = atof( ms.GetCapture (buf, 0));

  if( ms.Match( GAS) == 1)
    gas = atof( ms.GetCapture (buf, 0));
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
  printf("hash=%d\n", hash);
  return hash;
}


void Dsm::debug( ) {
    printf( "meterid: %X\n", id);
    printf( "laag: %.3f\n", laag);
    printf( "hoog: %.3f\n", hoog);
    printf( "laagterug: %.3f\n", laagTerug);
    printf( "hoogterug: %.3f\n", hoogTerug);
    printf( "gas: %.3f\n", gas); 
}
