var mongoose    = require('mongoose');
var async       = require('async');
var bcrypt      = require('bcrypt');

var GLOBAL      = require('../_global/global');
var UTILS       = require('../_global/utils');

var userSchema = mongoose.Schema({
    kind            : {type:String,required:true},
    uid             : {type:Number,required:true,unique: true},
    gid             : Number,
    date_created    : Number,
    ip_created      : String,
    date_lastlogin  : Number,
    ip_lastlogin    : String,
    roles           : [Number],
    rules           : [Number],
    local           : {
      username      : String,
      email         : String,
      password      : String,
      fullname      : String,
    },
    hfactor:{
      uid           : Number,
      username      : String,
      email         : String,      
      fullname      : String,
    }
});

userSchema.index({kind: 1, uid: 1 });

var User = new CUser();
module.exports = User;

var _UserModel = null;

var CONFIG    = require('./config');
var GROUP     = require('./group');
var ROLE      = require('./role');
var RULE      = require('./rule');

function CUser() {

}

CUser.prototype.setDatabase = function (_client) {
  _UserModel = _client.model('user', userSchema, 'users');
}

CUser.prototype.getByUserId = function (uid,callback) {
  _UserModel.findOne({uid:uid}).exec(callback);
}

CUser.prototype.getByGroupId = function (gid,callback) {
  _UserModel.find({gid:gid}).exec(callback);
}

CUser.prototype.getKindUsers = function (_kind,callback) {
  _UserModel.find({kind:_kind}).exec(callback);
}

CUser.prototype.getUser = function (uname,kind,callback) {
  //_UserModel.findOne({username:uname}).exec(callback);
    var _query = {};
    _query.kind = kind;
    var field = kind + '.username';
    _query[field] = uname;
    _UserModel.findOne(_query).exec(callback);
}

CUser.prototype.getEmail = function (email,kind,callback) {
  //_UserModel.findOne({email:email}).exec(callback);
    var _query = {};
    _query.kind = kind;
    var field = kind + '.email';
    _query[field] = email;    
    _UserModel.findOne(_query).exec(callback);
}

CUser.prototype.Local_Create = function (data,callback) {  
  var passMD5_;
  async.waterfall([
    function (next) {
      if (!data) return next(-101,'Lỗi dữ liệu.');
      if (!data.username || !data.email || !data.password) return next(-102,'Thiếu thông tin');
      if (data.username < 3 || GLOBAL.IsUsernameValid(data.username) === false) return next(-103,'Tài khoản không hợp lệ');
      if (!data.gid) return next(-104,'Nhóm không hợp lệ');
      if (GLOBAL.IsEmailValid(data.email) === false) return next(-105,'Email không hợp lệ');
      if (data.password.length < 6) return next(-106,'Mật khẩu phải từ 6 ký tự trở lên');

      data.username = data.username.toLowerCase();
      User.getUser(data.username,'local',next); 
    },
    function (oUser,next) {
      if (oUser) return next(-107,'Tài khoản đã tồn tại');
      data.email = data.email.toLowerCase();
      User.getEmail(data.email,'local',next);      
    },
    function (oUser,next) {
      if (oUser) return next(-108,'Email đã tồn tại');
      GROUP.getByGroupId(data.gid,next);
    },
    function (oGroup,next) {
      if (!oGroup) return next(-109,'Group không tồn tại');      
      var passMD5 = UTILS.MD5(data.password);
      GLOBAL.bcryptPassword(passMD5,next);//data.password
    },
    function (_password,next) {
      passMD5_ = _password;
      CONFIG.incrField('nextUserId',1, next);
    },
    function (_newUID,next) {        
      var newUser = new _UserModel;
      newUser.kind = 'local';
      newUser.uid = _newUID;
      newUser.ip_lastlogin = data.ip ? data.ip : '';
      newUser.ip_created = newUser.ip_lastlogin;
      newUser.gid = data.gid;

      newUser.local.username = data.username;
      newUser.local.email = data.email;
      newUser.local.password = passMD5_;
      newUser.local.fullname = data.fullname ? data.fullname : '';
      
      newUser.date_created = new Date().getTime();
      newUser.date_lastlogin = newUser.date_created;

      newUser.save(next);
    }
  ],callback);
}

CUser.prototype.HFactor_AddIfNew = function (data,callback) {
  async.waterfall([
    function (next) {
      data.gid = data.gid | 3;
      console.log(data);
      if (!data || !data.username || !data.email) return next(-101,'Thiếu dữ liệu');
      User.getEmail(data.email,'hfactor',next);
    },
    function (mUser,next) {
      if (mUser) {
        mUser.ip_lastlogin = data.ip | '';
        mUser.date_lastlogin = new Date().getTime();
        mUser.save(function(err,oUser) {
          return callback(null,mUser);
        });
      }
      else GROUP.getByGroupId(data.gid,next)
    },
    function (mGroup,next) {
      if (!mGroup) return next(-102,'Group không tồn tại');      
      CONFIG.incrField('nextUserId',1, next);
    },
    function (_newUID,next) {
      var newUser = new _UserModel;
      newUser.uid = _newUID;
      newUser.kind = 'hfactor'
      newUser.ip_lastlogin = data.ip ? data.ip : '';
      newUser.ip_created = newUser.ip_lastlogin;
      newUser.gid = data.gid;

      newUser.hfactor.uid = data.hfid;
      newUser.hfactor.username = data.username;
      newUser.hfactor.email = data.email;      
      newUser.hfactor.fullname = data.fullname ? data.fullname : '';
      
      newUser.date_created = new Date().getTime();
      newUser.date_lastlogin = newUser.date_created;

      newUser.save(next);
    }
  ],callback);
}

CUser.prototype.AddRole = function (uid,roleModel,callback) {
    async.waterfall([
      function (next) {
        if (!roleModel) return next(-101,'Lỗi dữ liệu');
        User.getByUserId(uid,next);
      },
      function (mUser,next) {
        if (!mUser) return next(-102,'User không tồn tại');
        if (!mUser.roles || mUser.roles.indexOf(roleModel.rid) === -1)
        {
          if (roleModel.parent && mUser.roles.indexOf(roleModel.parent) === -1)
            return next(-103,'Role cha chưa được phép');
          else{
            mUser.roles.push(roleModel.rid);
            mUser.save(next);
          }
        }
        else next(null,mUser);
      }
    ],callback);
}
CUser.prototype.AddRoleByRoleIds = function (uid,_roleids,callback) {
    var roleids = [];
    async.waterfall([
      function (next) {
  
        if (Array.isArray(_roleids)) roleids = Array.from(new Set(_roleids));
        else roleids.push(_roleids);
        if (roleids.length === 0) return next(-101,'Không có gì để làm');
  
        User.getByUserId(uid,next);
      },
      function (mUser,next) {
        if (!mUser) return next(-102,'User không tồn tại');
        ROLE.getByRoleIds(roleids,next);
      },
      function (mRoles,next) {
        async.each(mRoles, function (mRole, cb) {
          User.AddRole(uid,mRole,cb);
        }, next);
      }
    ],callback);
}
CUser.prototype.GetRoles = function (uid,callback) {
    async.waterfall([
      function (next) {
        User.getByUserId(uid,next);
      },
      function (mUser,next) {
        ROLE.getByUserModel(mUser,next);
      }
    ],callback);
}
CUser.prototype.RemRole = function (uid,roleModel,callback) {
    async.waterfall([
      function (next) {
        if (!roleModel) return next(-101,'Lỗi dữ liệu');
        User.getByUserId(uid,next);
      },
      function (mUser,next) {
        if (!mUser) return next(-102,'User không tồn tại');
        if (mUser.roles)
        {
          var idx = mUser.roles.indexOf(roleModel.rid);
          if (idx >= 0){
            mUser.roles.splice(idx, 1);
            return mUser.save(next);
          }
          else return next(null,mUser);
        }
        else return next(null,mUser);
      }
    ],callback);
}
CUser.prototype.AddRule = function (uid,ruleModel,callback) {
    async.waterfall([
      function (next) {
        if (!ruleModel) return next(-101,'Lỗi dữ liệu');
        User.getByUserId(uid,next);
      },
      function (mUser,next) {
        if (!mUser) return next(-102,'User không tồn tại');
        if (!mUser.rules || mUser.rules.indexOf(ruleModel.rid) === -1)
        {
          mUser.rules.push(ruleModel.rid);
  
  //        if (!mUser.roles || mUser.roles.indexOf(ruleModel.roleid) === -1)
  //          mUser.roles.push(ruleModel.roleid);
          return mUser.save(next);
        }
        else next(null,mUser);
      }
    ],callback);
}
CUser.prototype.AddRuleByRuleIds = function (uid,_ruleids,callback) {
    var ruleids = [];
    async.waterfall([
      function (next) {
        if (Array.isArray(_ruleids)) ruleids = Array.from(new Set(_ruleids));
        else ruleids.push(_ruleids);
        if (ruleids.length === 0) return next(-101,'Không có gì để làm');
  
        User.getByUserId(uid,next);
      },
      function (mUser,next) {
        if (!mUser) return next(-102,'User không tồn tại');
        RULE.getByRuleIds(ruleids,next);
      },
      function (mRules,next) {
        async.each(mRules, function (mRule, cb) {
          User.AddRule(uid,mRule,cb);
        }, next);
      }
    ],callback);
}
CUser.prototype.GetRules = function (uid,callback) {
    async.waterfall([
      function (next) {
        User.getByUserId(uid,next);
      },
      function (mUser,next) {
        RULE.getByUserModel(mUser,next);
      }
    ],callback);
}
CUser.prototype.RemRule = function (uid,ruleModel,callback) {
    async.waterfall([
      function (next) {
        if (!ruleModel) return next(-101,'Lỗi dữ liệu');
        User.getByUserId(uid,next);
      },
      function (mUser,next) {
        if (!mUser) return next(-102,'User không tồn tại');
        if (mUser.rules)
        {
          var idx = mUser.rules.indexOf(ruleModel.rid);
          if (idx >= 0){
            mUser.rules.splice(idx, 1);
            return mUser.save(next);
          }
          else return next(null,mUser);
        }
        else return next(null,mUser);
      }
    ],callback);
}
CUser.prototype.IsAdministrators = function (uid,callback) {
    async.waterfall([
      function (next) {
        User.getByUserId(uid,next);
      },
      function (mUser,next) {
        if (!mUser) return next(-101,'User không tồn tại');
        GROUP.getByGroupId(mUser.gid,next);
      },
      function (mGroup,next) {
        if (!mGroup) return next(-102,'Group không tồn tại');
        var bAdmin = false;
        if (mGroup.name === 'Administrators') bAdmin = true;
        next(null,bAdmin);
      }
    ],callback);
}
CUser.prototype.IsGroup = function (modelUser,groupName,callback) {
    async.waterfall([
      function (next) {
        if (!modelUser) return next(-101,'User không tồn tại');
        GROUP.getByGroupId(modelUser.gid,next);
      },
      function (mGroup,next) {
        if (!mGroup) return next(-102,'Group không tồn tại');
        var bGroup = false;
        if (mGroup.name === groupName) bGroup = true;
        next(null,bGroup);
      }
    ],callback);
}
CUser.prototype.UpdatePassword = function (uid,newPass,callback) {
    var hashPass;
    async.waterfall([
      function (next) {
        if (!newPass || newPass.length < 6) return next(-101,'Mật khẩu phải từ 6 ký tự trở lên');
        var passMD5 = UTILS.MD5(newPass);
        GLOBAL.bcryptPassword(passMD5,next);
      },
      function (_pass,next) {
        hashPass = _pass;
        User.getByUserId(uid,next);
      },
      function (mUser,next) {
        mUser.local.password = hashPass;
        mUser.save(function (err,_data) {
          if (err) return next(-100,'Lỗi lưu dữ liệu');
          else return next();
        });
      }
    ],callback);
}

CUser.prototype.Authentication = function (email_,pass_,callback) {
  async.waterfall([
    function (next) {
      //console.log(uname_ + '/' + pass_ + ' KIND:' + kind_);
      User.getEmail(email_,'local',next);      
    },
    function (oUser,next) {
      if (!oUser) return next(-1,'Tài khoản không tồn tại');
      //console.log(oUser);
      bcrypt.compare(pass_, oUser.local.password, function(err, isMatch) {
        if (err) return next(err,err.message);
        if (!isMatch) return next(-2,'Mật khẩu không chính xác');
        next(null, oUser);
      });
    }
  ],callback);
}
////////////USER - HFACTOR//////////////////////////////
CUser.prototype.searchUsers = function (filter,callback) {
    var _query = {};
    var _fields = {_id:0, __v: 0,roles:0,rules:0,kind:0};  //loc fields
    var _sort = {'username':-1};
    var _start = filter.start || 0;
    var _limit = filter.limit || 50;
    if ( _limit < 1 || _limit > 50) _limit = 50;
  
    if (filter != null){
      if (filter.username){
        _query['username'] = { $regex : new RegExp(filter.username, 'i') }
      }
  
      if (filter.tmincreated && filter.tmaxcreated){
        _sort = {date_created:1};
        _query.date_created = {$gte: filter.tmincreated, $lte: filter.tmaxcreated }
      }else if (filter.tminlogined && filter.tmaxlogined){
        _sort = {date_lastlogin:1};
        _query.date_lastlogin = {$gte: filter.tminlogined, $lte: filter.tmaxlogined }
      }
    }
    //console.log(filter);
    //console.log(_query);
    var Pages = {};
    async.waterfall([
      function (next) {
        _UserModel.countDocuments(_query,next);
      },
      function (_count,next) {
        Pages.index = _start;
        Pages.limit = _limit;
        Pages.total = _count;
        _UserModel.find(_query, _fields).limit(_limit).skip(_start).sort(_sort).exec(next);
      },
      function (mUsers,next) {
        next(null,mUsers,Pages);
      }
    ],callback);
}