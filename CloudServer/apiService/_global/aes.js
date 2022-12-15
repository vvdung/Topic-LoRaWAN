
var CryptoJS    = require('crypto-js');
var UTILS       = require('./utils');
var Base64Url   = require('base64url');
module.exports = new CAes();

function CAes() {}

//type: 16,24,32
CAes.prototype.NormalizeKey = function (key,type) {
    var keylen = 32;
    if (type === 24) keylen = 24;
    else if (type === 16) keylen = 16;
    var szKey = key.substr(0,keylen);
    var baKey = UTILS.String2UTF8Bytes(szKey);
    var iPad = keylen - baKey.length;
    for (var i = 0; i < iPad; ++i) baKey.push(0);
    return UTILS.Bytes2WordArray(baKey);
}
CAes.prototype.Encode = function (msg,key,type) {
    var waMsg = CryptoJS.enc.Utf8.parse(msg);
    var waKey = this.NormalizeKey(key,type);
    var oCipherAES = CryptoJS.AES.encrypt(waMsg,waKey,{mode: CryptoJS.mode.ECB, padding: CryptoJS.pad.ZeroPadding});//
    return UTILS.Bytes2Base64Url(UTILS.WordArray2Bytes(oCipherAES.ciphertext));
}
CAes.prototype.Decode = function (msg64Url,key,type) {
    try {
      //console.log('AES MSG [' + msg64Url + '] ');
      //console.log('AES KEY [' + key + '] ' + key.length);
      var waKey = this.NormalizeKey(key,type);
      var msgHex = UTILS.Base64Url2HexString(msg64Url);
      //console.log(msgHex);
      var bytes  = CryptoJS.AES.decrypt(
        {ciphertext: CryptoJS.enc.Hex.parse(msgHex)},
        waKey,
        {mode: CryptoJS.mode.ECB, padding: CryptoJS.pad.ZeroPadding}//
      );
      var res = bytes.toString(CryptoJS.enc.Utf8);
      //console.log(res.charCodeAt(res.length - 1));
      while (res.charCodeAt(res.length - 1) < 16) res = res.slice(0,-1);
      return res;
    }catch (err){
      console.log('AESDecrypt() ERROR...');
      return null;
    }
}