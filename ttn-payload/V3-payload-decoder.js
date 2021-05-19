
function decodeUplink(input) {
   
  // Payload decoder for "De Slimme Meter".
  // Decode binary uplink message (24 bytes) to json message
  // by Marcel Meek, July 2020
  // 7-5-2021 updated to TTN V3
   
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
  
 function bytesToUInt32(bytes) {
    return bytes[3]<<24 | bytes[2]<<16 | bytes[1]<<8 | bytes[0];
  }
  

  var decoded = {};

  if( input.fPort == 40) {
    var bytes = input.bytes;
    //decoded.meterid = bytesToHex( bytes);
    decoded.meterid = bytesToUInt32( bytes);
    decoded.laag = bytesToFloat( bytes.slice(4));
    decoded.hoog = bytesToFloat( bytes.slice(8));
    decoded.laagterug = bytesToFloat( bytes.slice(12));
    decoded.hoogterug = bytesToFloat( bytes.slice(16));
    decoded.gas = bytesToFloat( bytes.slice(20));
  }

  return { data:decoded, warnings: [],errors: [] };
}
