var mongoose    = require('mongoose');
var async       = require('async');

var configSchema = mongoose.Schema({
  nextGroupId     : Number,
  nextUserId      : Number,
  nextRoleId      : Number,
  nextRuleId      : Number,
  nextPublisherId : Number,
  nextGatewayId   : Number,
  nextNodeId      : Number,
  nextSensorId    : Number
});


var Config = new CConfig();
module.exports = Config;

var _ConfigModel = null;


function CConfig() {

}

CConfig.prototype.setDatabase = function (_client) {
  _ConfigModel = _client.model('config', configSchema, 'config');
}

CConfig.prototype.incrField = function (field,value,callback) {
  if (typeof field !== 'string') {
    field = field.toString();
  }
  // if there is a '.' in the field name it inserts subdocument in mongo, replace '.'s with \uff0E
  field = field.replace(/\./g, '\uff0E');
  var data = {};
  data[field] = value;

  _ConfigModel.findOneAndUpdate({}, { $inc: data }, { new: true, upsert: true}).exec(function (err,oGlobal) {
    callback(err, oGlobal ? oGlobal[field] : null);
  });
}

CConfig.prototype.get = function (callback) {
  _ConfigModel.findOne({}).exec(callback);
}