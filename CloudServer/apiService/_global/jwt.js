var UTILS       = require('./utils');
var XOR4        = require('./xor4');
var AES         = require('./aes');
var BASE64      = require('base64url');

module.exports = new CJWT();

function CJWT(){
}

//type: 0:base64, 1:xor4, 2 aes
CJWT.prototype.Create = function(oPayload,type,keyJWT,keyPayload){
  if (!oPayload || type === undefined || !keyJWT) return null;
  var txtPayload = JSON.stringify(oPayload);
  if (!txtPayload) return null;
  var oHeader = {};//{alg:'HS256',typ:'JWT'};
  var szPayload64Url = '';
  //oHeader['alg'] = 'HS256';
  //oHeader['typ'] = 'JWT';
  oHeader['t'] = type;
  switch (type){
      case 1:{// XOR
          if (keyPayload) oHeader['k'] = XOR4.Encode(keyPayload,keyJWT);
          else keyPayload = keyJWT;
          szPayload64Url = XOR4.Encode(txtPayload,keyPayload);
          break;
      }
      case 2:{//EAS
          if (keyPayload) oHeader['k'] = AES.Encode(keyPayload,keyJWT);
          else keyPayload = keyJWT;
          szPayload64Url = AES.Encode(txtPayload,keyPayload);
          //console.log('JWTCreate TYPE === 2');
          break;
      }
      default:
          szPayload64Url= BASE64.encode(UTILS.String2UTF8Bytes(txtPayload));
          //console.log('JWTCreate TYPE === default');
  }
  var szHeader64Url = BASE64.encode(JSON.stringify(oHeader));
  var szMsg = szHeader64Url + '.' + szPayload64Url;
  var szSign = UTILS.GetHash(szMsg,keyJWT);
  return szMsg + '.' + szSign;
}

CJWT.prototype.Verify = function(strJWT,key){
  var arrStr = strJWT.split('.',3);
  if (arrStr.length != 3) return null;
  var strHeaderUrl = arrStr[0];
  var strPayloadUrl = arrStr[1];
  var msg = strHeaderUrl + '.' + strPayloadUrl;
  if (arrStr[2] !== UTILS.GetHash(msg,key)) return null;
  return arrStr;
}

CJWT.prototype.GetPayload = function(strJWT,key) {
  //Chu y sai key trong config collection db
  if (!strJWT) return null;
  var arrStr = this.Verify(strJWT,key);
  //console.log(arrStr);
  if (arrStr === null) return null;
  var oHeader = UTILS.JsonParse(BASE64.decode(arrStr[0]));
  if (!oHeader) return null;
  //console.log(oHeader);
  var type = oHeader.t || 0;
  var keyPayload = key;
  var oPayload;
  switch (type){
      case 1:{//XOR
          if (oHeader.k){
            keyPayload = XOR4.Decode(oHeader.k,key);
            oHeader.k = keyPayload;
          }
          oPayload = UTILS.JsonParse(XOR4.Decode(arrStr[1],keyPayload));
          break;
      }
      case 2:{//AES
          if (oHeader.k){
            keyPayload = AES.Decode(oHeader.k,key);
            oHeader.k = keyPayload;
          }
          oPayload = UTILS.JsonParse(AES.Decode(arrStr[1],keyPayload));
          break;
      }
      default:
          oPayload = UTILS.JsonParse(BASE64.decode(arrStr[1]));
  }
  //console.log(oPayload);
  oHeader['t'] = type;
  return [oPayload,oHeader];
}