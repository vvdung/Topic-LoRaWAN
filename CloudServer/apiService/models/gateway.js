var mongoose    = require('mongoose');
var async       = require('async');

var GLOBAL      = require('../_global/global');
var UTILS       = require('../_global/utils');

var gateSchema = mongoose.Schema({    
    gid             : {type:Number,required:true,unique: true},
    gwid            : {type:String,required:true},
    uid             : {type:Number,required:true},   
    gwkey           : {type:String,required:true}, //su dung trong code cua gate (raspberry)
    date_created    : Number,
    date_lastseen   : Number,
    description     : String,
    frequency       : String,    
    location        :{
        latitude    : Number,
        longitude   : Number,
        public      : Boolean
    },
    public          : Boolean
});

gateSchema.index({ uid: 1, gwid : 1 },{unique:true});

var GATE = new CGate();
module.exports = GATE;

var _GateModel = null;

var CONFIG      = require('./config');
var USER        = require('./user');

function CGate() {

}

CGate.prototype.setDatabase = function (_client) {
    _GateModel = _client.model('gate', gateSchema, 'gates');
}

CGate.prototype.GetGate = function (uid_,gwid_,callback) {
    _GateModel.findOne({uid:uid_,gwid:gwid_}).exec(callback);
}
CGate.prototype.GetGateById = function (gid_,callback) {
    _GateModel.findOne({gid:gid_}).exec(callback);
}

CGate.prototype.Create = function (data,callback) {

    async.waterfall([
        function (next) {
            if (!data) return next(-101,'Lỗi dữ liệu.');
            if (!data.gwid || !data.uid ) return next(-102,'Thiếu thông tin');
            if (data.gwid.length < 2 || data.gwid.length > 8) return next(-103,'GWID từ 2-8 ký tự');
            data.gwid = data.gwid.toLowerCase();
            console.log(data.gwid);
            if (!GLOBAL.IsAlphanumeric(data.gwid)) return next(-104,'GWID chỉ gồm ký tự thường và số');
            USER.getByUserId(data.uid,next);
        },
        function (mUser,next){
            if (!mUser) return next(-105,'Tài khoản không tồn tại');
            GATE.GetGate(data.uid,data.gwid,next);            
        },
        function (mGate,next){
            if (mGate) return next(-106,'Gateway đã đăng ký');            
            CONFIG.incrField('nextGatewayId',1, next);
        },
        function (_newUID,next) {
            var newGate = new _GateModel;
            newGate.gid = _newUID;
            newGate.uid = data.uid;
            newGate.gwid = data.gwid;
            newGate.description = data.desc || '';
            newGate.date_created = new Date().getTime();
            var oKey = {};
            oKey['g'] = newGate.gid;
            oKey['u'] = newGate.uid;
            //oKey['gwid'] = newGate.gwid;
            newGate.gwkey = UTILS.HWT_Encode(JSON.stringify(oKey));
            newGate.save(next);
        }
    ],callback);
}

CGate.prototype.GetFilters = function (oFilters,callback) {
    if (oFilters.uid === undefined) return callback(null,[],[]);
    var _query = {};
    var _fields = {_id:0, __v: 0};  //loc fields
    var _sort = {date_created:1};
    var _start = oFilters.start || 0;
    var _limit = oFilters.limit || 10;
    if ( _limit < 1 || _limit > 50) _limit = 50;
    _query.uid = oFilters.uid;

    var Pages = {};
    async.waterfall([
        function (next) {
            _GateModel.countDocuments(_query,next);
        },
        function (_count,next) {
          Pages.index = _start;
          Pages.limit = _limit;
          Pages.total = _count;
          _GateModel.find(_query, _fields).limit(_limit).skip(_start).sort(_sort).exec(next);
        },
        function (mGates,next) {
            console.log(Pages);
            console.log(mGates);
          next(null,mGates,Pages);
        }
    ],callback);
}