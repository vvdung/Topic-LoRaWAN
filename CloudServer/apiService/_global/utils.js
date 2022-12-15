
var CryptoJS    = require('crypto-js');
var Base64Url   = require('base64url');


module.exports = new CUtils();

function CUtils() {
    this.chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-/=!@#$';
    this.HEXChars = '0123456789ABCDEFabcdef';
    this.DELIMITER_CHARS = '=~!@#$^*()+{}[]|/<>.,';
}
CUtils.prototype.MD5 = function (txt) {
    return CryptoJS.MD5(txt).toString();
}

CUtils.prototype.GetDelimiter = function(enCode){
    if (!enCode) return this.DELIMITER_CHARS[0];
    var bytes = this.String2UTF8Bytes(enCode);
    var len = bytes.length;
    while (len < 4) {bytes.push(0); ++len;}
    var word = bytes[3] << 24 | bytes[2] << 16 | bytes[1] << 8 | bytes[0];
    var i = (word % 21);    
	return this.DELIMITER_CHARS[i];
}
CUtils.prototype.GenerateHexValue = function (nSize) {
  var nCount = nSize * 4;
  var rnd = CryptoJS.lib.WordArray.random(nCount);
  var value = new Array(nSize);
  var len = this.HEXChars.length;
  for (var i = 0; i < nSize; i++) {
    var k = rnd.words[i] % len;
    if (k < 0) k = k * (-1);
    value[i] = this.HEXChars[k];
  };
  return value.join('');
}

CUtils.prototype.GenerateKey = function (nSize) {
    var nCount = nSize * 4;
    var rnd = CryptoJS.lib.WordArray.random(nCount);
    var value = new Array(nSize);
    var len = this.chars.length;
    for (var i = 0; i < nSize; i++) {
        var k = rnd.words[i] % len;
        if (k < 0) k = k * (-1);
        value[i] = this.chars[k];
    };
    return value.join('');
}

CUtils.prototype.WordArray2Bytes = function (wa) {
    var bytes = [];
    if (wa.words === undefined || wa.sigBytes === undefined) return bytes;
    var k = 0;
    for (var i = 0; i < wa.words.length; ++i){
        if (k >= wa.sigBytes) break;
        var x = wa.words[i];
        if (k++ < wa.sigBytes) bytes.push((x >> 24) & 0xFF); else break;
        if (k++ < wa.sigBytes) bytes.push((x >> 16) & 0xFF); else break;
        if (k++ < wa.sigBytes) bytes.push((x >> 8) & 0xFF); else break;
        if (k++ < wa.sigBytes) bytes.push(x & 0xFF); else break;
    }
    return bytes;
}

CUtils.prototype.String2UTF8Bytes = function (txt) {
    var txtUTF8 = CryptoJS.enc.Utf8.parse(txt);
    return this.WordArray2Bytes(txtUTF8);
}
CUtils.prototype.Bytes2WordArray = function (bytes) {
    var iSize = bytes.length;
    var tmpBytes = bytes;
    var iPad = (4 - (bytes.length % 4)) % 4;
    var i = 0;
    for (i = 0; i < iPad; ++i) tmpBytes.push(0);
    var wa = {};
    wa['words'] = [];
    for (i = 0; i < tmpBytes.length; i += 4){
        var b1 = tmpBytes[i] << 24;
        var b2 = tmpBytes[i + 1] << 16;
        var b3 = tmpBytes[i + 2] << 8;
        var b4 = tmpBytes[i + 3];
        var x = b1 | b2 | b3 | b4;
        wa['words'].push(x);
    }
    wa['sigBytes'] = iSize;
    return wa;
}
CUtils.prototype.Bytes2String = function (bytes) {
    var wa = this.Bytes2WordArray(bytes);
    return CryptoJS.enc.Utf8.stringify(wa);
}
CUtils.prototype.Bytes2HexString = function (bytes) {
    var strHex = '';
    for (var i = 0; i < bytes.length; ++i){
        strHex += ('0' + (bytes[i] & 0xFF).toString(16)).slice(-2);
    }
    return strHex;
}
CUtils.prototype.Bytes2Base64Url = function (bytes) {
    return Base64Url.encode(bytes);
}
CUtils.prototype.Base64Url2Bytes = function (txt64Url) {
    return Base64Url.toBuffer(txt64Url);
}
CUtils.prototype.Base64Url2HexString = function (txt64Url) {
    var bytes = Base64Url.toBuffer(txt64Url);
    return this.Bytes2HexString(bytes);
}
CUtils.prototype.MD5 = function (txt) {
    return CryptoJS.MD5(txt).toString();
    //var waUTF8 = CryptoJS.enc.Utf8.parse(txt);
    //return CryptoJS.MD5(waUTF8).toString();
}
CUtils.prototype.JsonParse = function (txt) {
    if (txt === undefined) return null;
    try{
        return JSON.parse(txt);
    }catch (e){
        console.log('ERROR JsonParse [' + txt + '] ' + txt.length);
        return null;
    }
}

CUtils.prototype.isNumber = function(n){
    return !isNaN(parseFloat(n)) && isFinite(n);
}
CUtils.prototype.isInteger = function(n){
    return (+n===parseInt(n));
}
CUtils.prototype.isPositiveInteger = function(n){
    var v = parseInt(n);
    if (+n===v) return v;
    return 0;
}
CUtils.prototype.isAlphaNumeric = function( str ) {
    return /^[0-9a-zA-Z]+$/.test(str);
}

//return timestamp
CUtils.prototype.addDaysByTimestamp = function( timestamp, days ) {
  var dt_new = new Date(timestamp + (days * 86400000));
  return dt_new.getTime();
}

CUtils.prototype.GetHash = function (msg,key) {
    var waMsg = CryptoJS.enc.Utf8.parse(msg);
    var waKey = CryptoJS.enc.Utf8.parse(key);
    var hash = CryptoJS.HmacSHA256(waMsg,waKey);
    var bytes = this.WordArray2Bytes(hash);
    return Base64Url.encode(bytes);
}
CUtils.prototype.Checksum = function(msg){
    if (!msg || msg === undefined) return 0;
    var bytes = this.String2UTF8Bytes(msg);
  
    var sum = 0;
      var swappem = 0;
    var len = bytes.length;
    var k = 0;
      
      sum = bytes[k++] << 8;		
      len--;
      ++swappem;
    
    while (len > 1) {
      var word = bytes[k+1] << 8 | bytes[k];    
          sum += word;
          k   += 2;
          len -= 2;
      }
    
    if (len > 0) sum += bytes[k];
    
    /*  Fold 32-bit sum to 16 bits */
      while (sum >> 16)
      sum = (sum & 0xffff) + (sum >> 16);
    
    if (swappem)
          sum = ((sum & 0xff00) >> 8) + ((sum & 0x00ff) << 8);
    
      return sum;
}

var XOR4        = require('./xor4');

CUtils.prototype.HWT_Encode = function(msg){
    if (!msg || msg === undefined) return null;
    var key = this.GenerateHexValue(4);
    var enCode = XOR4.Encode(msg,key,true);
    var szKey = key + enCode;
    var uSum = this.Checksum(szKey);
    var delimiter = this.GetDelimiter(enCode);
    //console.log(delimiter + ' GetDelimiter');
    var szSum = key + uSum.toString(16);
    var bySum = this.String2UTF8Bytes(szSum);
    var c = bySum[0];
    bySum[0] = bySum[4];
    bySum[4] = c;
    var szSum_ = this.Bytes2String(bySum);
    return (enCode + delimiter + szSum_);
}
CUtils.prototype.HWT_Decode = function(enCode){
    if (!enCode || enCode === undefined) return null;
    var delimiter = this.GetDelimiter(enCode);
    var a = enCode.split(delimiter);    
    if (a.length != 2) return null;
    if (a[1].length < 4) return null;
    var b = a[1].split("");
    var ch = b[0];
    b[0] = b[4];
    b[4] = ch;
    var c = b.join('');
    var key = c.slice(0,4);
    var szKey = key + a[0];
    var uSum = parseInt(c.slice(4), 16);
    var uSum_ = this.Checksum(szKey);
    if (uSum != uSum_) return null;
    return XOR4.Decode(a[0],key,true);
}