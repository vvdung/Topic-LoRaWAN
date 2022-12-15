//var HS256       = require('./hs256');
var bcrypt      = require('bcrypt');

var GLOBAL = new CGlobal();

module.exports = GLOBAL;

function CGlobal() {
    this.chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-/=!@#$';
    this.hfactor = 'H-FACTOR.VN @ Copyright 2021. All rights reserved.';
    this.keyJWTAdmin    = 'ee4a9521a4f4a352649754932e8abd8c';
    this.keyJWTSession  = '!decode#session$key@aigo#crypto!';
  }

  CGlobal.prototype.ReturnError = function(res,code,msg){
    console.log(code);
    console.log(msg);
    var oResult = {};
    oResult.r = code || -1;
    oResult.m = msg || this.hfactor;
    return res.status(302).json(oResult);
}

CGlobal.prototype.ReturnClear = function(res,msg){
    var oResult = {};
    oResult.r = 0;
    oResult.m = msg || this.hfactor;
    return res.status(200).json(oResult);
}

CGlobal.prototype.ReturnEncrypt = function(res,msg,r){
    var oResult = {};
    oResult.r = r;//1;
    oResult.m = msg;
    return res.status(200).json(oResult);
}

CGlobal.prototype.bcryptPassword = function (clearpass,callback) {
    if (clearpass === undefined || clearpass === null | clearpass.length === 0) return callback(null,null);
    // generate a salt
    bcrypt.genSalt(10,function (err,_salt) {
      if (err) return callback(err,err.message);
      //console.log(_salt);
      // hash the password along with our new salt
      bcrypt.hash(clearpass, _salt, function(err, _hash) {
        if (err) return callback(err,err.message);
        callback(null,_hash);
      });
    });
}
CGlobal.prototype.IsEmailValid = function(email){
  if (email === undefined || email.length === 0 || typeof email !== 'string') return false;
  var re = /^\w+([\.-]?\w+)*@\w+([\.-]?\w+)*(\.\w{2,3})+$/;
  return re.test(email);
}
CGlobal.prototype.IsUsernameValid = function(username){
  if (username === undefined || username.length === 0 || typeof username !== 'string') return false;
  if (username.indexOf(" ") !== -1) return false;
  var re = /^['"\s\-+.*0-9\u00BF-\u1FFF\u2C00-\uD7FF\w]+$/;
  return re.test(username);
}
CGlobal.prototype.IsAlphanumeric = function(strId){
  if (strId === undefined || strId.length === 0 || typeof strId !== 'string') return false;
  var regex = /^[a-z0-9]+$/i;
  return regex.test(strId);
}