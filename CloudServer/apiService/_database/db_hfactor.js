var async = require('async');
var UTILS = require('../_global/utils');
var JWT   = require('jsonwebtoken');

var HFactor = new CHFactor();

module.exports = HFactor;

var CONFIG;
var GROUP;
var USER;
var ROLE;
var RULE;
var PUBLISHER;
var GATE;
var NODE;
var SENSOR;

function CHFactor() {
    this.client = null;
}

CHFactor.prototype.Init = function (callback) {
    console.log('Connecting HFactor Database...');
    async.waterfall([
      function (next){

        var keyJWT = "IhA1@iuTUZ/TUbZ$q8UUJSuDCfit#jtK";
        var oMsg = {}
        oMsg.uid = 2;
        oMsg.gwid = 'GW01';
        oMsg.desc = 'Only testing'
        
        console.log(JSON.stringify(oMsg));
        var msg = JWT.sign(oMsg, keyJWT, {});
        console.log(msg);

        next();
      },
      function (next) {
        //fnConnectDatabase('mongodb://factor:factor%40123@127.0.0.1:27017/factor_db',next); 
        fnConnectDatabase('mongodb://factor:factor%40123@209.126.13.94:27017/factor_db',next);        
      },
      function (next) {
        CONFIG    = require('../models/config');
        CONFIG.setDatabase(HFactor.client);

        GROUP     = require('../models/group');
        GROUP.setDatabase(HFactor.client);

        USER      = require('../models/user');
        USER.setDatabase(HFactor.client);
  
        ROLE      = require('../models/role');
        ROLE.setDatabase(HFactor.client);
  
        RULE      = require('../models/rule');
        RULE.setDatabase(HFactor.client);

        PUBLISHER= require('../models/publisher');
        PUBLISHER.setDatabase(HFactor.client);

        GATE      = require('../models/gateway');
        GATE.setDatabase(HFactor.client);

        NODE      = require('../models/node');
        NODE.setDatabase(HFactor.client);

        SENSOR      = require('../models/sensor');
        SENSOR.setDatabase(HFactor.client);


        fnInitDatabaseDefault(next);
      }
    ], callback);
  }

function fnConnectDatabase(connString,callback) {
    var mongoose = require('mongoose');
    mongoose.Promise = global.Promise;
    var MongoOptions = {
      poolSize: 10,
      useNewUrlParser: true,
      useUnifiedTopology: true,
      useCreateIndex: true,
      useFindAndModify: false
    };
    var conn = mongoose.createConnection(connString,MongoOptions);
    conn.on('error', function (err) {
      console.log('...MONGO EVENT ERROR');
      callback(err);
    });
    conn.on('connected', function() {
      if (HFactor.client) console.log('...MONGO [' + HFactor.client.name + '] Connected!');
    });
    conn.on('disconnected', function() {
      if (HFactor.client) console.log('...MONGO [' + HFactor.client.name + '] Disconnected!');
    });
    conn.then(function(_db) {
      console.log('...MONGO Actived : [' + _db.name + ']');
      //console.log(conn);
      HFactor.client = _db;
      callback(null);
  
    });
}

function fnInitDatabaseDefault(callback) {
  async.waterfall([
    function (next) {
      fnCreateGroupDefault(next);
    },
    function (next) {
      fnCreateUserDefault(next);
    },
    function (next) {
      fnCreateRoleRuleDefaull(next);
    },
    function (next) {
      fnCreatePublisherDefault(next);
    },
    function (next) {
      fnCreateGateDefault(next);
    },
    function (next) {
      fnCreateNodeDefault(next);
    },
    function (next) {
      fnCreateSensorDefault(next);
    }
  ],function (err,result) {
    callback();
  });
}

function fnCreateGroupDefault(callback) {
  async.waterfall([
    function (next) {
      GROUP.create('Administrators','Admin Group',next);
    },
    function (oData,next) {
      GROUP.create('Operators','Operators Group',next);
    },
    function (oData,next) {
      GROUP.create('Users','Users Group',next);
    }
  ],function (err,result) {
    callback();
  });
}

function fnCreateUserDefault(callback) {
  async.waterfall([
    function (next) {
      GROUP.getGroupIdByName('Administrators',next);
    },
    function (gid,next) {
      var userData = {
        gid: gid,
        username : 'admin',
        email: 'admin@h-factor.vn',
        password:'sure0hf',
        fullname: 'H-Factor\'s Administrator',
        kind: 1
      }
      USER.Local_Create(userData,next);
    }
  ],function (err,result) {
    /*if (err){ 
      console.log(err);
      console.log(result);
    }*/
    callback();
  });
}

function fnCreateRoleRuleDefaull(callback) {
  var RoleIds = [];
  var RuleIds = [];
  async.waterfall([
    function (next) {
      ROLE.create('Quản trị hệ thống',0,1,next);
    },
    function (mRole,next) {
      RoleIds.push(mRole.rid);
      var roles = [{name:'Quản lý Roles',parent:mRole.rid},{name:'Quản lý Rules',parent:mRole.rid}];
      async.each(roles, function (role, cb) {
        ROLE.create(role.name,role.parent,1,function (err,_data) {
          RoleIds.push(_data.rid);
          cb();
        });
      }, next);
    },
    function (next) {
      var ruleRole1 = {roleid: RoleIds[1], name: 'Tạo mới Role', uid: 1};
      var ruleRole2 = {roleid: RoleIds[1], name: 'Cập nhật Role', uid: 1};
      var ruleRole3 = {roleid: RoleIds[2], name: 'Tạo mới Rule', uid: 1};
      var ruleRole4 = {roleid: RoleIds[2], name: 'Cập nhật Rule', uid: 1};
      var rules = [ruleRole1,ruleRole2,ruleRole3,ruleRole4];
      async.each(rules, function (rule, cb) {
        RULE.create(rule,function (err,_data) {
          RuleIds.push(_data.rid);
          cb();
        });
      }, next);
    },
    function (next) {
      USER.AddRuleByRuleIds(1,RuleIds,next);
    },
    function (next) {
      USER.AddRoleByRoleIds(1,RoleIds,next);
    }
  ],function (err,result) {
    callback();
  });
}

function fnCreatePublisherDefault(callback) {
  var data = {};
  async.waterfall([
    function (next) {
      data.name = 'Website';
      data.site = 'h-factor.vn';
      data.key_jwt = UTILS.GenerateKey(32);
      PUBLISHER.create(data,next);
    },
    function (oPublisher,next) {
      data.name = 'Windows';
      data.site = 'h-factor.vn';
      data.key_jwt = UTILS.GenerateKey(32);
      PUBLISHER.create(data,next);
    },
    function (oPublisher,next) {
      data.name = 'Android';
      data.site = 'h-factor.vn';
      data.key_jwt = UTILS.GenerateKey(32);
      PUBLISHER.create(data,next);
    },
    function (oPublisher,next) {
      data.name = 'IOS';
      data.site = 'h-factor.vn';
      data.key_jwt = UTILS.GenerateKey(32);
      PUBLISHER.create(data,next);
    }
  ],function (err,result) {
    callback();
  });
}

function fnCreateGateDefault(callback) {
  var data = {};
  async.waterfall([
    function (next) {
      data.uid = 1;
      data.gwid = 'GW01';
      data.desc = 'Only testing'
      GATE.Create(data,next);
    },
    function (mGate,next) {
      console.log(mGate);
      next();
    }
  ],function (err,result) {
    //console.log('---- GATE CREATE DEFAULT ----');
    //console.log(err);
    //console.log(result);
    callback();
  });
}

function fnCreateNodeDefault(callback) {
  var data = {};
  async.waterfall([
    function (next) {
      data.gid = 1;
      data.nodeid = 'Node01';
      data.desc = 'Only testing';
      NODE.Create(data,next);
    },
    function (mNode,next) {
      console.log(mNode);
      next();
    }
  ],function (err,result) {
    //console.log('---- NODE CREATE DEFAULT ----');
    //console.log(err);
    //console.log(result);
    callback();
  });
}

function fnCreateSensorDefault(callback) {
  return callback();
  var data = {};
  async.waterfall([
    function (next) {
      data.nid = 1;
      data.gid = 1;
      data.sensors = {
        field1:123,
        field2:456,
        field3:324324,
      }
      SENSOR.Create(data,next);
    },
    function (mSensor,next) {
      console.log(mSensor);
      next();
    }
  ],function (err,result) {
    //console.log('---- GATE CREATE DEFAULT ----');
    //console.log(err);
    //console.log(result);
    callback();
  });
}