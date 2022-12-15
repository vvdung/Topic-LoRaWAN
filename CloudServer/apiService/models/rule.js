var mongoose    = require('mongoose');
var async       = require('async');


var ruleSchema = mongoose.Schema({
  rid             : {type:Number,required:true,unique: true},
  roleid          : Number,
  name            : String,
  uid_created     : Number,
  date_created    : Number
});

ruleSchema.index({ roleid: 1, rid: -1 });

var Rule = new CRule();
module.exports = Rule;

var _RuleModel = null;

var CONFIG      = require('./config');
var USER        = require('./user');
var ROLE        = require('./role');

function CRule() {}

CRule.prototype.setDatabase = function (_client) {
  _RuleModel = _client.model('rule', ruleSchema, 'rules');
}

CRule.prototype.getByName = function (name,callback) {
  _RuleModel.findOne({name:name}).exec(callback);
}

CRule.prototype.getByRuleId = function (rid,callback) {
  _RuleModel.findOne({rid:rid}).exec(callback);
}

CRule.prototype.getByRuleIds = function (rids,callback) {
  _RuleModel.find({rid:{$in:rids}}).exec(callback);
}

CRule.prototype.create = function (data,callback) {
  async.waterfall([
    function (next) {
      if (!data) return next(-101,'Lỗi dữ liệu');
      Rule.getByName(data.name,next);
    },
    function (mRule,next) {
      if (mRule) return next(-102,'Rule đã tồn tại');
      ROLE.getByRoleId(data.roleid,next);
    },
    function (mRole,next) {
      if (!mRole) return next(-103,'Role không tồn tại');
      USER.getByUserId(data.uid,next);
    },
    function (mUser,next) {
      if (!mUser) return next(-104,'User không tồn tại');
      CONFIG.incrField('nextRuleId',1,next);
    },
    function (_newRuleId,next) {
      var newRule = new _RuleModel;
      newRule.rid = _newRuleId;
      newRule.roleid = data.roleid;
      newRule.name = data.name;
      newRule.uid_created = data.uid;
      newRule.date_created = new Date().getTime();
      newRule.save(next);
    }
  ],callback);
}

CRule.prototype.createByUserId = function (uid,name,mRole,callback) {
  async.waterfall([
    function (next) {
      if (!mRole) return next(-101,'Role không tồn tại');
      Rule.getByName(name,next);
    },
    function (mRule,next) {
      if (mRule) return next(-102,'Rule đã tồn tại');
      USER.getByUserId(uid,next);
    },
    function (mUser,next) {
      if (!mUser) return next(-103,'User không tồn tại');
      CONFIG.incrField('nextRuleId',1,next);
    },
    function (_newRuleId,next) {
      var newRule = new _RuleModel;
      newRule.rid = _newRuleId;
      newRule.roleid = mRole.rid;
      newRule.name = name;
      newRule.uid_created = uid;
      newRule.date_created = new Date().getTime();
      newRule.save(function (err,_data) {
        if (err) return next(-100,'Lỗi lưu dữ liệu');
        else return next(null,_data);
      });
    }
  ],callback);
}

CRule.prototype.getByUserModel = function (mUser,callback) {
  async.waterfall([
    function (next) {
      if (!mUser) return next(-101,'User không tồn tại');
      if (!mUser.rules || mUser.rules.length === 0) return next(null,[]);
      Rule.getByRuleIds(mUser.rules,next);
    },
    function (mRules,next) {
      if (mUser.roles.length !== mRules.length){
        //console.log('Update Rules now');
        var realRules = [];
        for (var i = 0; i < mRules.length; ++i){
          realRules.push(mRules[i].rid);
        }
        mUser.rules = realRules;
        mUser.save(function (err,_data) {
          return next(null,mRules);
        });
      }
      else next(null,mRules);
    }
  ],callback);
}