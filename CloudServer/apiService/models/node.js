var mongoose    = require('mongoose');
var async       = require('async');

var GLOBAL      = require('../_global/global');
var UTILS       = require('../_global/utils');

var nodeSchema = mongoose.Schema({    
    nid             : {type:Number,required:true,unique: true},    
    gid             : {type:Number,required:true}, //gatewayId 
    nodeid          : {type:String,required:true},
    nkey            : {type:String,required:true}, //su dung trong node Arduino
    date_created    : Number,
    date_lastseen   : Number,
    description     : String    
});

nodeSchema.index({ gid: 1, nodeid : -1 },{unique:true});

var NODE = new CNode();
module.exports = NODE;

var _NodeModel = null;

var CONFIG      = require('./config');
var GATE        = require('./gateway');

function CNode() {

}

CNode.prototype.setDatabase = function (_client) {
    _NodeModel = _client.model('node', nodeSchema, 'nodes');
}

CNode.prototype.GetNode = function (gid_,nodeid_,callback) {
    _NodeModel.findOne({gid:gid_,nodeid:nodeid_}).exec(callback);
}
CNode.prototype.GetNodeById = function (nid_,callback) {
    _NodeModel.findOne({nid:nid_}).exec(callback);
}

CNode.prototype.Create = function (data,callback) {

    async.waterfall([
        function (next) {
            if (!data) return next(-101,'Lỗi dữ liệu.');
            if (!data.nodeid || !data.gid ) return next(-102,'Thiếu thông tin');
            if (data.nodeid.length < 2 || data.nodeid.length > 8) return next(-103,'NODEID từ 2-8 ký tự');
            data.nodeid = data.nodeid.toLowerCase();
            console.log(data.nodeid);
            if (!GLOBAL.IsAlphanumeric(data.nodeid)) return next(-104,'NODEID chỉ gồm ký tự thường và số');
            GATE.GetGateById(data.gid,next); 
        },
        function (mGate,next){
            if (!mGate) return next(-105,'Gateway không tồn tại'); 
            NODE.GetNode(data.gid,data.nodeid,next);            
        },
        function (mNode,next){
            if (mNode) return next(-106,'Node đã tồn tại'); 
            CONFIG.incrField('nextNodeId',1, next);
        },
        function (_newUID,next) {
            var newNode = new _NodeModel;
            newNode.nid = _newUID;
            newNode.gid = data.gid;
            newNode.nodeid = data.nodeid;
            newNode.description = data.desc || '';
            newNode.date_created = new Date().getTime();
            var oKey = {};
            oKey['g'] = newNode.gid;
            oKey['n'] = newNode.nid;            
            newNode.nkey = UTILS.HWT_Encode(JSON.stringify(oKey));
            newNode.save(next);
        }
    ],callback);
}

CNode.prototype.GetFilters = function (oFilters,callback) {
    if (oFilters.gid === undefined) return callback(null,[],{});
    var _query = {};
    var _fields = {_id:0, __v: 0};  //loc fields
    var _sort = {date_created:1};
    var _start = oFilters.start || 0;
    var _limit = oFilters.limit || 10;
    if ( _limit < 1 || _limit > 50) _limit = 50;    
    _query.gid = oFilters.gid;

    var Pages = {};
    async.waterfall([
        function (next) {
            _NodeModel.countDocuments(_query,next);
        },
        function (_count,next) {
          Pages.index = _start;
          Pages.limit = _limit;
          Pages.total = _count;
          _NodeModel.find(_query, _fields).limit(_limit).skip(_start).sort(_sort).exec(next);
        },
        function (mNodes,next) {
          next(null,mNodes,Pages);
        }
    ],callback);
}

