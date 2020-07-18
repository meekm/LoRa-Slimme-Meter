function Decoder(bytes, port) {
   
  // Payload decoder for "De Slimme Meter".
  // Decode binary uplink message (24 bytes) to json message
  // by Marcel Meek, July 2020
   
  function bytesToFloat(bytes) {
    var bits = bytes[3]<<24 | bytes[2]<<16 | bytes[1]<<8 | bytes[0];
    var sign = (bits>>>31 === 0) ? 1.0 : -1.0;
    var exponent = bits>>>23 & 0xff;
    var mantisse = (exponent === 0) ? (bits & 0x7fffff)<<1 : (bits & 0x7fffff) | 0x800000;
    var floating_point = sign * mantisse * Math.pow(2, exponent - 127) / 0x800000;
    return floating_point;
  }
  
  
  function bytesToHex( bytes) {
    var hex = bytes[3].toString(16);
    hex += bytes[2].toString(16);
    hex += bytes[1].toString(16);
    hex += bytes[0].toString(16);
    return hex.toUpperCase();
  }
  

  var decoded = {};

  if( port == 40) {
    
    decoded.meterid = bytesToHex( bytes);
    decoded.laag = bytesToFloat( bytes.slice(4)).toFixed(3);
    decoded.hoog = bytesToFloat( bytes.slice(8)).toFixed(3);
    decoded.laagterug = bytesToFloat( bytes.slice(12)).toFixed(3);
    decoded.hoogterug = bytesToFloat( bytes.slice(16)).toFixed(3);
    decoded.gas = bytesToFloat( bytes.slice(20)).toFixed(3);
  }

  return decoded;
}
