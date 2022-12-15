var async = require('async');
var mongoose    = require('mongoose');

var groupSchema = mongoose.Schema({

  gid   : {type:Number, required:true, unique:true},
  name  : {type:String, required:true},
  desc  : String
});

groupSchema.index({ name: 1, gid: -1 });

var Group = new CGroup();
module.exports = Group;

var _GroupModel = null;

var CONFIG = require('./config');

function CGroup() {

}

CGroup.prototype.setDatabase = function (_client) {
  _GroupModel = _client.model('group', groupSchema, 'groups');
}

CGroup.prototype.getByName = function (name,callback) {
  _GroupModel.findOne({name:name},{_id:0}).exec(callback);
}
CGroup.prototype.getGroupIdByName = function (name,callback) {
  _GroupModel.findOne({name:name},{_id:0, gid:1}).exec(function (err,oGroup) {
    callback(err, oGroup ? oGroup.gid : null);
  });
}
CGroup.prototype.getByGroupId = function (gid,callback) {
  _GroupModel.findOne({gid:gid},{_id:0}).exec(callback);
}

CGroup.prototype.getAll = function (callback) {
  _GroupModel.find({},{_id:0,__v:0}).exec(callback);
}

CGroup.prototype.create = function (name,desc,callback) {
  async.waterfall([
    function (next) {
      Group.getByName(name,next);
    },
    function (oGroup,next) {
      if (oGroup) return next(-101,'Tên nhóm đã tồn tại');
      CONFIG.incrField('nextGroupId',1,next);
    },
    function (_newGid,next) {
      var newGroup = new _GroupModel;
      newGroup.gid = _newGid;
      newGroup.name = name;
      newGroup.desc = desc || '';
      newGroup.save(next);
    }
  ],callback);
}