var async   = require('async');
var REQUEST = require('request')
var BASE64  = require('base64url');
var GLOBAL  = require('../_global/global');
var UTILS   = require('../_global/utils');

var JWT     = require('../_global/jwt');
//var XOR4    = require('../_global/xor4');

var PUBLISH = require('../models/publisher');
var USER    = require('../models/user');
var GATE    = require('../models/gateway');
var NODE    = require('../models/node');
const SENSOR = require('../models/sensor');

//website pid=1 jwt="nyhULDQ6uwt2gn39W@bPAxGT$uj#z/qt"
exports.Test = function(req, res) {
    var ip_connected = req.headers['x-forwarded-for'];
    console.log(ip_connected);
    console.log(req.query);

    async.waterfall([
      function (next) {
        next();
      }
    ],function (err, result) {
      if (err) return GLOBAL.ReturnError(res,err,result);
      return GLOBAL.ReturnClear(res,result);
    });    
}

function fnAnalysisUserLoginJWT(req,key,callback) {

}

function fnGetPayload(isHTTPS,m,key){
  if (isHTTPS) return [UTILS.JsonParse(m),{}];
  return JWT.GetPayload(m,key);
}

exports.Local_Register = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  //var isHTTPS = req.headers['x-forwarded-proto'] == 'https';

  var keyJWT;
  var publisherId;
  var r_ = 0;

  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');
      publisherId = req.body.p || 1; //website
      PUBLISH.getKeyJWT(publisherId,next);
    },
    function (_keyJWT,next) {
      keyJWT = _keyJWT;      
      
      var oMsg = JWT.GetPayload(req.body.m,keyJWT);//fnGetPayload(isHTTPS,req.body.m,keyJWT);//
      if (!oMsg || oMsg.length != 2)           return next(-2,'Dữ liệu lỗi');
      var oPayload = oMsg[0];
      var oHeader = oMsg[1];
      if (!oPayload || !oHeader)               return next(-3,'Dữ liệu không chính xác');
      if (oPayload.u === undefined || oPayload.p === undefined || oPayload.e === undefined) 
                                               return next(-4,'Thiếu tham số');
      if (oPayload.u.length < 3)               return next(-5,'Tên đăng nhập phải 3 ký tự trở lên');
      if (oPayload.p.length < 6)               return next(-6,'Mật khẩu phải 6 ký tự trở lên');
      if (!GLOBAL.IsUsernameValid(oPayload.u)) return next(-7,'User không hợp lệ');
      if (!GLOBAL.IsEmailValid(oPayload.e))    return next(-8,'Email không hợp lệ');

      next(null,oHeader,oPayload);      
    },
    function (oHeader,oPayload,next){
      console.log(oPayload);
      var oNewUser = {};
      oNewUser['username'] = oPayload.u;
      oNewUser['password'] = oPayload.p;
      oNewUser['email']    = oPayload.e;
      oNewUser['ip']       = ipaddress;
      oNewUser['gid']       = 3;//User Group
      USER.Create(oNewUser,next);      
    },
    function (mUser, next){
      console.log(mUser);
      next(null,'Tạo tài khoản thành công');
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}

exports.Local_Login = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';

  var isHTTPS = req.headers['x-forwarded-proto'] == 'https';

  var keyJWT;
  var publisherId;  
  var foundUser = {};
  var oSession = {};

  var r_ = 0;
  var m_ = GLOBAL.hfactor;
  //console.log('HTTPS:' + isHTTPS + ' REMOTE IP: ' + ipaddress);
  //console.log(req.headers);
  //console.log(res.req.headers);

  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');
      publisherId = req.body.p || 1; //website
      PUBLISH.getKeyJWT(publisherId,next);
    },
    function (_keyJWT,next) {
      keyJWT = _keyJWT;
      //console.log("JWT: " + keyJWT);
      
      var oMsg = JWT.GetPayload(req.body.m,keyJWT);//fnGetPayload(isHTTPS,req.body.m,keyJWT);//
      if (!oMsg || oMsg.length != 2) return next(-2,'Dữ liệu lỗi');
      var oPayload = oMsg[0];
      var oHeader = oMsg[1];
      if (!oPayload || !oHeader) return next(-3,'Dữ liệu không chính xác');
      if (oPayload.e === undefined || oPayload.p === undefined) return next(-4,'Thiếu tham số');
      if (GLOBAL.IsEmailValid(oPayload.e) === false) return next(-5,'Email không hợp lệ');
      if (oPayload.p.length != 32) return next(-6,'Mật khẩu lỗi');      
      next(null,oHeader,oPayload);      
    },
    function (oHeader,oPayload,next){
      //console.log(oHeader);
      //console.log(oPayload);
      USER.Authentication(oPayload.e,oPayload.p,next);      
    },
    function (oUser,next){
      //console.log(oUser);
      foundUser = {
        uid: oUser.uid,
        gid: oUser.gid,
        username: oUser.local.username,
        email: oUser.local.email,
        fullname: oUser.local.fullname,
        ip_last: oUser.ip_lastlogin,
        date_last: oUser.date_lastlogin
      };
      //console.log(foundUser);
      var d = new Date();
      oSession['uid'] = foundUser.uid;
      oSession['gid'] = foundUser.gid;
      oSession['time'] = ~~(d.getTime() / 1000 );//d.getTime();      
      if (publisherId) oSession['p'] = publisherId;

      oUser.ip_lastlogin = ipaddress;
      oUser.date_lastlogin = d.getTime();
      oUser.save(function (err,data) {
        next();
      });      
    },
    function (next) {
      
      var sessionJWT = UTILS.HWT_Encode(JSON.stringify(oSession));//XOR4.Encode(oSession,GLOBAL.keyJWTSession,true);
      foundUser['token'] = sessionJWT; 
      
      console.log(foundUser);
      if (isHTTPS) next(null,foundUser);
      else{
        r_ = 1;
        if (publisherId == 1) m_ = JWT.Create(foundUser,0,keyJWT);        
        else m_ = JWT.Create(foundUser,1,keyJWT);        
        next(null,m_);
      }           
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}

function fnHFactorLogin(email,pass,callback) {
  async.waterfall([
    function (next) {
      var _body = {
        email:email,
        password: pass
      };
      var _options = {
        url: "https://h-factor.duong.link/api/auth/login",
        headers: {'content-type' : 'application/x-www-form-urlencoded'},
        form: _body
      };
      REQUEST.post(_options,next);
    },
    function (res,data,next) {
      console.log(data);
      if (res.statusCode !== 200){
        var _res = JSON.parse(data.toString());
        console.log(_res);
        return next(-101,_res.error);
      }
      else {
        var oData = JSON.parse(data.toString());
        var oUser = {
          hfid: oData.user.id,
          username: oData.user.name,
          email: oData.user.email,
          fullname: oData.user.first_name + ' ' + oData.user.last_name
        };
        next(null,oUser);
      }
    }
  ],callback);
}

exports.User_Login = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';

  var isHTTPS = req.headers['x-forwarded-proto'] == 'https';

  var keyJWT;
  var publisherId;  
  var foundUser = {};
  var oSession = {};

  var r_ = 0;
  var m_ = GLOBAL.hfactor;
  //console.log('HTTPS:' + isHTTPS + ' REMOTE IP: ' + ipaddress);
  //console.log(req.headers);
  //console.log(res.req.headers);

  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');
      publisherId = req.body.p || 1; //website
      PUBLISH.getKeyJWT(publisherId,next);
    },
    function (_keyJWT,next) {
      keyJWT = _keyJWT;
      //console.log("JWT: " + keyJWT);
      
      var oMsg = JWT.GetPayload(req.body.m,keyJWT);//fnGetPayload(isHTTPS,req.body.m,keyJWT);//
      if (!oMsg || oMsg.length != 2) return next(-2,'Dữ liệu lỗi');
      var oPayload = oMsg[0];
      var oHeader = oMsg[1];
      if (!oPayload || !oHeader) return next(-3,'Dữ liệu không chính xác');
      if (oPayload.e === undefined || oPayload.p === undefined) return next(-4,'Thiếu tham số');
      if (GLOBAL.IsEmailValid(oPayload.e) === false) return next(-5,'Email không hợp lệ');
      if (oPayload.p.length < 6) return next(-6,'Mật khẩu lỗi');      
      next(null,oHeader,oPayload);      
    },
    function (oHeader,oPayload,next){
      //console.log(oHeader);
      console.log(oPayload);
      fnHFactorLogin(oPayload.e,oPayload.p,next);          
    },
    function (oUser,next){
      console.log(oUser);      
      var HF_User = {
        gid: 3,
        ip : ipaddress,
        hfid: oUser.hfid,
        username: oUser.username,
        email: oUser.email,
        fullname: oUser.fullname,
      };
      console.log(HF_User);
      USER.HFactor_AddIfNew(HF_User,next);
    },
    function (mUser,next){
      console.log(mUser);
      foundUser = {
        gid: mUser.gid,        
        uid: mUser.uid,
        username: mUser.hfactor.username,
        email: mUser.hfactor.email,
        fullname: mUser.hfactor.fullname,
      };;
      var d = new Date();
      oSession['uid'] = mUser.uid;
      oSession['gid'] = mUser.gid;
      oSession['time'] = ~~(d.getTime() / 1000 );//d.getTime();      
      if (publisherId) oSession['p'] = publisherId;    
      
      var sessionJWT = UTILS.HWT_Encode(JSON.stringify(oSession));//XOR4.Encode(oSession,GLOBAL.keyJWTSession,true);
      foundUser['token'] = sessionJWT; 
      
      console.log(foundUser);
      if (isHTTPS) next(null,foundUser);
      else{
        r_ = 1;
        if (publisherId == 1) m_ = JWT.Create(foundUser,0,keyJWT);        
        else m_ = JWT.Create(foundUser,1,keyJWT);        
        next(null,m_);
      }           
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}

exports.User_Get = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  var foundUser = {};
  var oMsg = null;
  var r_ = 0;
  var m_ = GLOBAL.hfactor;

  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');
      oMsg = UTILS.JsonParse(req.body.m);//fnGetPayload(isHTTPS,req.body.m,keyJWT);//
      if (!oMsg) return next(-2,'Dữ liệu lỗi');
      if (oMsg.email === undefined) return next(-4,'Thiếu tham số');
      if (GLOBAL.IsEmailValid(oMsg.email) === false) return next(-5,'Email không hợp lệ');
      var uname = oMsg.email.match(/^([^@]*)@/)[1];
      var HF_User ={
        username : uname,
        email : oMsg.email,
        hfid : 0,
        fullname: oMsg.email
      }  
      USER.HFactor_AddIfNew(HF_User,next);    
    },
    function (mUser,next){
      //console.log(mUser);
      foundUser = {
        gid: mUser.gid,        
        uid: mUser.uid,
        username: mUser.hfactor.username,
        email: mUser.hfactor.email,
        fullname: mUser.hfactor.fullname,
      };
      next(null,foundUser);           
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}

exports.User_Register = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  //var isHTTPS = req.headers['x-forwarded-proto'] == 'https';

  var keyJWT;
  var publisherId;
  var r_ = 0;

  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');
      publisherId = req.body.p || 1; //website
      PUBLISH.getKeyJWT(publisherId,next);
    },
    function (_keyJWT,next) {
      keyJWT = _keyJWT;      
      
      var oMsg = JWT.GetPayload(req.body.m,keyJWT);//fnGetPayload(isHTTPS,req.body.m,keyJWT);//
      if (!oMsg || oMsg.length != 2)           return next(-2,'Dữ liệu lỗi');
      var oPayload = oMsg[0];
      var oHeader = oMsg[1];
      if (!oPayload || !oHeader)               return next(-3,'Dữ liệu không chính xác');
      if (oPayload.u === undefined || oPayload.p === undefined || oPayload.e === undefined) 
                                               return next(-4,'Thiếu tham số');
      if (oPayload.u.length < 3)               return next(-5,'Tên đăng nhập phải 3 ký tự trở lên');
      if (oPayload.p.length < 6)               return next(-6,'Mật khẩu phải 6 ký tự trở lên');
      if (!GLOBAL.IsUsernameValid(oPayload.u)) return next(-7,'User không hợp lệ');
      if (!GLOBAL.IsEmailValid(oPayload.e))    return next(-8,'Email không hợp lệ');

      next(null,oHeader,oPayload);      
    },
    function (oHeader,oPayload,next){
      console.log(oPayload);
      var oNewUser = {};
      oNewUser['username'] = oPayload.u;
      oNewUser['password'] = oPayload.p;
      oNewUser['email']    = oPayload.e;
      oNewUser['ip']       = ipaddress;
      oNewUser['gid']       = 3;//User Group
      USER.Create(oNewUser,next);      
    },
    function (mUser, next){
      console.log(mUser);
      next(null,'Tạo tài khoản thành công');
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}

exports.User_Gate_Gets = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  var r_ = 0;
  var oDataRet = {};
  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');      
      var oMsg = UTILS.JsonParse(req.body.m);        
      if (!oMsg) return next(-2,'Dữ liệu lỗi');
      if (oMsg.uid === undefined)  return next(-2,'Thiếu tham số');
      GATE.GetFilters(oMsg,next);      
    },    
    function (mGates,pages,next) {
      oDataRet['pages'] = pages;
      oDataRet['data'] = mGates;
      next(null,oDataRet);
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}

exports.User_Gate_Node_Gets = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  var r_ = 0;
  var oMsg = null;
  var oDataRet = {};
  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');      
      oMsg = UTILS.JsonParse(req.body.m);        
      if (!oMsg) return next(-2,'Dữ liệu lỗi');
      if (oMsg.uid === undefined || oMsg.gid === undefined)  return next(-3,'Thiếu tham số');
      GATE.GetGateById(oMsg.gid,next);
           
    },
    function (mGate,next){
      if (!mGate) return next(-4,'Gateway không tồn tại');
      if (mGate.uid !== oMsg.uid) return next(-5,'Gateway không thuộc tài khoản này');
      NODE.GetFilters(oMsg,next); 
    },    
    function (mNodes,pages,next) {
      oDataRet['pages'] = pages;
      oDataRet['data'] = mNodes;
      next(null,oDataRet);
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}

///////GATEWAY/////////
exports.Gate_Add = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  //var isHTTPS = req.headers['x-forwarded-proto'] == 'https';

  var keyJWT;
  var publisherId;
  var oHWT = null;
  var r_ = 0;
  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');
      publisherId = req.body.p || 1; //website
      PUBLISH.getKeyJWT(publisherId,next);
    },
    function (_keyJWT,next) {
      keyJWT = _keyJWT;
      //console.log(req.headers);
      var token = req.headers['hfactor-t'];
      //console.log(token);
      if (!token) return next(-2,'Yêu cầu đăng nhập');
      var t_ = UTILS.HWT_Decode(token);
      //console.log(t_);
      if (!t_) return next(-3,'What the f@ck A?');
      oHWT = UTILS.JsonParse(t_);
      if (!oHWT) return next(-4,'What the f@ck B?');
      //console.log(oHWT);
      var oMsg = JWT.GetPayload(req.body.m,keyJWT);//fnGetPayload(isHTTPS,req.body.m,keyJWT);//
      if (!oMsg || oMsg.length != 2) return next(-5,'Dữ liệu lỗi');
      var oPayload = oMsg[0];
      var oHeader = oMsg[1];
      if (!oPayload || !oHeader) return next(-6,'Dữ liệu không chính xác');
      if (oPayload.gwid === undefined) return next(-7,'Thiếu tham số');
      
      var oGate = {};
      oGate.uid = oHWT.uid;
      oGate.gwid = oPayload.gwid;
      oGate.desc = oPayload.desc | 'Gateway';
      GATE.Create(oGate,next);
    },
    function (mGate,next){
      //console.log(mGate);      
      next(null,'Tạo Gateway thành công');
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}
exports.Gate_Get = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  var r_ = 0;
  var oDataRet = {};
  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');      
      var oMsg = UTILS.JsonParse(req.body.m);        
      if (!oMsg) return next(-2,'Dữ liệu lỗi');
      if (oMsg.uid === undefined)  return next(-2,'Thiếu tham số');
      GATE.GetFilters(oMsg,next);      
    },    
    function (mGates,pages,next) {
      oDataRet['pages'] = pages;
      oDataRet['data'] = mGates;
      next(null,oDataRet);
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}
///////NODE/////////
exports.Node_Add = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  //var isHTTPS = req.headers['x-forwarded-proto'] == 'https';

  var keyJWT;
  var publisherId;
  var oHWT = null;
  var r_ = 0;
  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');
      publisherId = req.body.p || 1; //website
      PUBLISH.getKeyJWT(publisherId,next);
    },
    function (_keyJWT,next) {
      keyJWT = _keyJWT;
      //console.log(req.headers);
      var token = req.headers['hfactor-t'];
      //console.log(token);
      if (!token) return next(-2,'Yêu cầu đăng nhập');
      var t_ = UTILS.HWT_Decode(token);
      //console.log(t_);
      if (!t_) return next(-3,'What the f@ck A?');
      oHWT = UTILS.JsonParse(t_);
      if (!oHWT) return next(-4,'What the f@ck B?');
      //console.log(oHWT);
      var oMsg = JWT.GetPayload(req.body.m,keyJWT);
      if (!oMsg || oMsg.length != 2) return next(-5,'Dữ liệu lỗi');
      var oPayload = oMsg[0];
      var oHeader = oMsg[1];
      if (!oPayload || !oHeader) return next(-6,'Dữ liệu không chính xác');
      
      //console.log(oPayload);
      if (oPayload.gid === undefined || oPayload.nodeid === undefined) return next(-7,'Thiếu tham số');
      
      var oNode = {};
      oNode.gid = oPayload.gid;
      oNode.nodeid = oPayload.nodeid;
      oNode.desc = oPayload.desc || 'Node';

      NODE.Create(oNode,next);
    },
    function (mNode,next){
      //console.log(mNode);      
      next(null,'Tạo Node thành công');
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}
exports.Node_Get = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  var r_ = 0;
  var oDataRet = {};
  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');      
      var oMsg = UTILS.JsonParse(req.body.m);        
      if (!oMsg) return next(-2,'Dữ liệu lỗi');
      if (oMsg.uid === undefined || oMsg.gid === undefined)  return next(-2,'Thiếu tham số');
      NODE.GetFilters(oMsg,next);      
    },    
    function (mNodes,pages,next) {
      oDataRet['pages'] = pages;
      oDataRet['data'] = mNodes;
      next(null,oDataRet);
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}
exports.Node_View_Sensor = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  var nId = req.params.nId;
  var r_ = 0;
  var oMsg = null;
  var oDataRet = {};
  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');      
      oMsg = UTILS.JsonParse(req.body.m);        
      if (!oMsg) return next(-2,'Dữ liệu lỗi');
      if (oMsg.uid === undefined)  return next(-3,'Thiếu tham số');
      NODE.GetNodeById(nId,next);      
    }, 
    function (mNode,next){
      if (!mNode) return next(-4,'Node không tồn tại');
      GATE.GetGateById(mNode.gid,next);
    },
    function (mGate,next){
      if (!mGate) return next(-5,'Gateway không tồn tại');
      if (mGate.uid !== oMsg.uid) return next(-6,'Node không thuộc tài khoản này');
      //oMsg['gid'] = mGate.gid;
      oMsg['nid'] = nId;
      SENSOR.GetFilters(oMsg,next)
    },  
    function (mSensors,pages,next) {
      oDataRet['pages'] = pages;
      oDataRet['data'] = mSensors;
      next(null,oDataRet);
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}

function fnGetSensors(gid,nids,oFilter,cb){
  //console.log("-------START--------");
  //console.log(oFilter);
  var uniqueIds =  [...new Set(nids)];
  var oDataRet = {};
  async.each(uniqueIds,
    function (item, next){      
      var oQuery = {gid:gid,nid:item};
      SENSOR.QueryNode(oQuery,oFilter,function(err,oData){
        //console.log('item: ' + item);
        oDataRet[item] = oData;
        //oDataRet.push(o);        
        next();
      });              
    },
    function (err){
      cb(null,oDataRet);
  });
}
exports.Node_Views = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  var nId = req.params.nId;
  var r_ = 0;
  var oMsg = null;
  var oDataRet = {};
  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');      
      oMsg = UTILS.JsonParse(req.body.m);        
      if (!oMsg) return next(-2,'Dữ liệu lỗi');
      if (oMsg.uid === undefined || oMsg.gid === undefined || oMsg.nids === undefined)  return next(-3,'Thiếu tham số');
      if (!(oMsg.nids instanceof Array)) return next(-4,'nids phải là array');
      GATE.GetGateById(oMsg.gid,next);            
    },
    function (mGate,next){
      if (!mGate) return next(-5,'Gateway không tồn tại');
      if (mGate.uid !== oMsg.uid) return next(-6,'Gateway không thuộc tài khoản này');
      var oFilter = {};
      oFilter.start = oMsg.start || 0;
      oFilter.limit = oMsg.limit || 1;
      fnGetSensors(oMsg.gid,oMsg.nids,oFilter,next);      
    },      
    function (data,next) {
      //console.log('******');
      //console.log(data);
      next(null,data);
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}

///////SENSOR/////////
exports.Sensor_Add = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  //var isHTTPS = req.headers['x-forwarded-proto'] == 'https';

  var keyJWT;
  var publisherId;
  
  var r_ = 0;
  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');
      publisherId = req.body.p || 1; //website
      PUBLISH.getKeyJWT(publisherId,next);
    },
    function (_keyJWT,next) {
      keyJWT = _keyJWT;     
      //console.log('p:' + publisherId + ' KEY:' + keyJWT); 
      //console.log(req.body.m); 
      var oMsg = JWT.GetPayload(req.body.m,keyJWT);      
      if (!oMsg || oMsg.length != 2) return next(-2,'Dữ liệu lỗi');
      var oPayload = oMsg[0];
      var oHeader = oMsg[1];
      if (!oPayload || !oHeader) return next(-3,'Dữ liệu không chính xác');
      if (oPayload.n === undefined || oPayload.g === undefined || oPayload.s === undefined) return next(-4,'Thiếu tham số');
      
      var oSensor = {};
      oSensor.nid = oPayload.n;
      oSensor.gid = oPayload.g;
      oSensor.sensors = oPayload.s;
      SENSOR.Create(oSensor,next);
    },
    function (mSensor,next){
      //console.log(mSensor);      
      next(null,'Tạo Sensor thành công');
    }
  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}

exports.Sensor_Get = function(req, res) {
  var ipaddress = req.headers['x-real-ip'] ? req.headers['x-real-ip'] : '';
  //var isHTTPS = req.headers['x-forwarded-proto'] == 'https';
  
  var r_ = 0;
  async.waterfall([
    function (next) {
      if (!req.body) return next(-1,'Không có dữ liệu');
      console.log(req.body.m);
      var oMsg = UTILS.JsonParse(req.body.m);  
      console.log(oMsg);    
      if (!oMsg) return next(-2,'Dữ liệu lỗi');
      if (oMsg.gids === undefined)  return next(-2,'Thiếu tham số');
      
      console.log(oMsg.gids);
      console.log(typeof oMsg.gids);


      next();
    },

  ],function (err, result) {
    if (err) return GLOBAL.ReturnError(res,err,result);
    return GLOBAL.ReturnEncrypt(res,result,r_);
  });
}