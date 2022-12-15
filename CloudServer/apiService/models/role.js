var mongoose    = require('mongoose');
var async       = require('async');
var UTILS       = require('../_global/utils');

var roleSchema = mongoose.Schema({
  rid             : {type:Number,required:true,unique: true},
  name            : String,
  parent          : Number,
  uid_created     : Number,
  date_created    : Number
});

roleSchema.index({ rid: 1 });

var Role = new CRole();
module.exports = Role;

var _RoleModel = null;

var CONFIG      = require('./config');

function CRole() {}

CRole.prototype.setDatabase = function (_client) {
  _RoleModel = _client.model('role', roleSchema, 'roles');
}

CRole.prototype.getByName = function (name,callback) {
  _RoleModel.findOne({name:name}).exec(callback);
}

CRole.prototype.getByRoleId = function (rid,callback) {
  _RoleModel.findOne({rid:rid}).exec(callback);
}

CRole.prototype.getByRoleIds = function (rids,callback) {
  _RoleModel.find({rid:{$in:rids}}).exec(callback);
}

CRole.prototype.create = function (name,parent,uid_created,callback) {
  async.waterfall([
    function (next) {
      //if (!name || name.length < 2 || name.length > 255 || name.indexOf(':') !== -1 || name.indexOf('/') !== -1 || !UTILS.isAlphaNumeric(name))
        //return next(-101,'Tên Role không phù hợp');
      Role.getByName(name,next);
    },
    function (mRole,next) {
      if (mRole) return next(-102,'Role đã tồn tại');
      Role.getByRoleId(parent,next);
    },
    function (mRoleParent,next) {
      if (!mRoleParent) parent = 0;
      CONFIG.incrField('nextRoleId',1,next);
    },
    function (_newRoleId,next) {
      var newRole = new _RoleModel;
      newRole.rid = _newRoleId;
      newRole.name = name;
      newRole.parent = parent;
      newRole.uid_created = uid_created;
      newRole.date_created = new Date().getTime();
      newRole.save(next);
    }
  ],callback);
}

CRole.prototype.getByUserModel = function (mUser,callback) {
  async.waterfall([
    function (next) {
      if (!mUser) return next(-101,'User không tồn tại');
      if (!mUser.roles || mUser.roles.length === 0) return next(null,[]);
      Role.getByRoleIds(mUser.roles,next);
    },
    function (mRoles,next) {
      if (mUser.roles.length !== mRoles.length){
        //console.log('Update Roles now');
        var realRoles = [];
        for (var i = 0; i < mRoles.length; ++i){
          realRoles.push(mRoles[i].rid);
        }
        mUser.roles = realRoles;
        mUser.save(function (err,_data) {
          return next(null,mRoles);
        });
      }
      else next(null,mRoles);
    }
  ],callback);
}
/*
CRole.prototype.getByRoleIds = function (roleids,callback) {
  async.waterfall([
    function (next) {
      _RoleModel.find({rid:{$in:roleids}}).exec(next);
    }
  ],callback);
}*/