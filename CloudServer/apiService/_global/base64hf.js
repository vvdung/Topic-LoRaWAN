var CryptoJS    = require('crypto-js');
var UTILS       = require('./utils');


module.exports = new CBase64HF();

function CBase64HF(){
    this.CHARS = 'abcdefghijklmnopqrstuvwxyz0123456789-_ABCDEFGHIJKLMNOPQRSTUVWXYZ';
}

CBase64HF.prototype.Encode = function(msg){
    if (!msg || msg === undefined) return null;
    //var waMsg = CryptoJS.enc.Utf8.parse(msg);
    //var byMsg = UTILS.WordArray2Bytes(waMsg);
    var byMsg;
    if (typeof msg === 'string') byMsg = UTILS.WordArray2Bytes(CryptoJS.enc.Utf8.parse(msg));
    else byMsg = msg;
    //console.log(byMsg);

    var in_len = byMsg.length;
	var i = 0;
	var j = 0;
    var k = 0;
	var char_array_3 = [0,0,0];
	var char_array_4 = [0,0,0,0];
    var bytes = [];

    while (in_len--) {
        char_array_3[i++] = byMsg[k++];
        if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++) bytes.push(this.CHARS[char_array_4[i]]);
			i = 0;
        }
    }
	if (i)
	{
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++) bytes.push(this.CHARS[char_array_4[j]]);		
	}
    //console.log(bytes);
    
    return bytes.join("");
}
function IsBase64Url(c){
    return (UTILS.isAlphaNumeric(c) || (c == '-') || (c == '_'));
}

CBase64HF.prototype.Decode = function(msg64){
    if (!msg64) return null;
    var byMsg = msg64.split("");    
    var in_len = byMsg.length;
	var i = 0;
	var j = 0;
    var in_ = 0;
	var char_array_3 = [0,0,0];
	var char_array_4 = [0,0,0,0];
    var bytes = [];
    while (in_len-- && IsBase64Url(byMsg[in_])){
        char_array_4[i++] = byMsg[in_++]; 
		if (i ==4) {
			for (i = 0; i <4; i++)
                char_array_4[i] = this.CHARS.indexOf(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++) bytes.push(char_array_3[i]);
			i = 0;
		}
    }

    if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = this.CHARS.indexOf(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) bytes.push(char_array_3[j]);
	}	
    return bytes;
	//return UTILS.Bytes2String(bytes);
}