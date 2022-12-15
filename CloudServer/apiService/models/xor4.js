var CryptoJS    = require('crypto-js');
var BASE64URL   = require('base64url');
var UTILS       = require('./utils');
var BASE64HF    = require("../_global/base64hf");

module.exports = new CXor4();

function CXor4() {}

CXor4.prototype.Encode = function (msg,key,isHF) {

    var msgWords = CryptoJS.enc.Utf8.parse(msg);
    var keyWords = CryptoJS.enc.Utf8.parse(key);
    var uKey = keyWords.words[0];
    
    var wa = {};
    wa["words"] = [];
    wa["sigBytes"] = msgWords.sigBytes;
    for (var i = 0; i < msgWords.words.length; ++i){
      var x = msgWords.words[i] ^ uKey;
      wa.words.push(x);
    }
    if (!isHF || isHF === undefined) isHF = false;
    if (!isHF) return BASE64URL.encode(UTILS.WordArray2Bytes(wa));
    return BASE64HF.Encode(UTILS.WordArray2Bytes(wa))    
}


CXor4.prototype.Decode = function (msg64Url,key,isHF) {
    var keyWords = CryptoJS.enc.Utf8.parse(key);
    var uKey = keyWords.words[0];
    if (!isHF || isHF === undefined) isHF = false;
    var msg64Bytes;
    if (!isHF) { 
      var msg64Buffer = BASE64URL.toBuffer(msg64Url);
      msg64Bytes = Array.prototype.slice.call(msg64Buffer);
    }
    else msg64Bytes = BASE64HF.Decode(msg64Url);

    var wa = UTILS.Bytes2WordArray(msg64Bytes);
    for (var i = 0; i < wa.words.length; ++i){
      wa.words[i] = wa.words[i] ^ uKey;
    }       
    return CryptoJS.enc.Utf8.stringify(wa);
}