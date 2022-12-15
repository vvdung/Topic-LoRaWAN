var mongoose    = require('mongoose');
var async       = require('async');
var UTILS       = require('../_global/utils');
var GLOBAL      = require('../_global/global');

var publisherSchema = mongoose.Schema({
  pid             : {type:Number,required:true,unique: true},
  name            : String,
  site            : String,
  key_jwt         : String,
  date_created    : Number
});

var Publisher = new CPublisher();
module.exports = Publisher;

var _PublisherModel = null;

var CONFIG  = require('./config');

function CPublisher() {}

CPublisher.prototype.setDatabase = function (_client) {
  _PublisherModel = _client.model('publisher', publisherSchema, 'publishers');
}

CPublisher.prototype.getByName = function (pname,callback) {
  _PublisherModel.findOne({name:pname}).exec(callback);
}

CPublisher.prototype.getByPublisherId = function (pid,callback) {
  _PublisherModel.findOne({pid:pid}).exec(callback);
}
CPublisher.prototype.getByPublisherIds = function (pids,callback) {
  _PublisherModel.find({pid:{$in:pids}}).exec(callback);
}
CPublisher.prototype.getPublishers = function (callback) {
  _PublisherModel.find({}).exec(callback);
}
CPublisher.prototype.create = function (data,callback) {
  var pubname;
  var key_jwt;
  async.waterfall([
    function (next) {
      if (!data)      return next(-101,'Lỗi dữ liệu.');
      pubname = data.name;
      if (!pubname)   return next(-102,'Tên nhà phát hành đâu?');
      if (pubname.length < 2 || pubname.length > 255 || pubname.indexOf(':') !== -1 || pubname.indexOf('/') !== -1 || !UTILS.isAlphaNumeric(pubname))
        return next(-103,'Tên nhà phát hành không hợp lệ');
      pubname = pubname
      key_jwt = data.key_jwt ? data.key_jwt : null;
      Publisher.getByName(pubname,next);
    },
    function (oPublisher,next) {
      if (oPublisher) return next(-104,'Nhà phát hành đã tồn tại');
      CONFIG.incrField('nextPublisherId',1, next);
    },
    function (newPubId,next) {
      var newPublish = new _PublisherModel;

      newPublish.pid = newPubId;
      newPublish.name = pubname;
      newPublish.date_created = new Date().getTime();
      newPublish.site = data.site || '';
      if (key_jwt) newPublish.key_jwt = key_jwt;

      newPublish.save(next);
    }
  ],callback);
}

CPublisher.prototype.getKeyJWT = function (pid,callback) {
  async.waterfall([
    function (next) {
      Publisher.getByPublisherId(pid,next);
    },
    function (oPublisher,next) {
      next(null, oPublisher && oPublisher.key_jwt ? oPublisher.key_jwt : GLOBAL.keyJWTAdmin);
    }
  ],callback);
}