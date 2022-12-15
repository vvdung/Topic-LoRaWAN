var mongoose    = require('mongoose');
var async       = require('async');

var GLOBAL      = require('../_global/global');
var UTILS       = require('../_global/utils');

var sensorSchema = mongoose.Schema({    
    sid             : {type:Number,required:true,unique: true},    
    nid             : {type:Number,required:true}, //nId - Node
    gid             : {type:Number,required:true}, //gId - Gateway
    date_created    : Number,    
    sensors         : {}
});

sensorSchema.index({ sid: -1, nid : -1, gid : -1 });

var SENSOR = new CSensor();
module.exports = SENSOR;

var _SensorModel = null;

var CONFIG      = require('./config');
var GATE        = require('./gateway');
var NODE        = require('./node');

function CSensor() {

}

CSensor.prototype.setDatabase = function (_client) {
    _SensorModel = _client.model('sensor', sensorSchema, 'sensors');
}


CSensor.prototype.Create = function (data,callback) {

    async.waterfall([
        function (next) {
            if (!data) return next(-101,'Lỗi dữ liệu.');
            if (!data.nid || !data.gid || !data.sensors ) return next(-102,'Thiếu thông tin');
            GATE.GetGateById(data.gid,next); 
        },
        function (mGate,next){
            if (!mGate) return next(-105,'Gateway không tồn tại'); 
            NODE.GetNodeById(data.nid,next);            
        },
        function (mNode,next){
            if (!mNode) return next(-106,'Node không tồn tại'); 
            CONFIG.incrField('nextSensorId',1, next);
        },
        function (_newUID,next) {
            var newSensor = new _SensorModel;
            newSensor.sid = _newUID;
            newSensor.nid = data.nid;            
            newSensor.gid = data.gid;            
            newSensor.date_created = new Date().getTime();
            newSensor.sensors = data.sensors;
            newSensor.save(next);
        }
    ],callback);
}

CSensor.prototype.Get = function (data,callback) {

}

function CheckWarning(mSensors){
    var size = mSensors.length;
    if (size === 0) return mSensors;
    var oWarning =[];
    mSensors.forEach(item => {
        var oSensors = item['sensors'];        
        var oPH = {};
        var ph = oSensors['ph'];
        if (ph !== undefined){
            oPH.value = ph;
            if (ph < 5.0 || ph > 10.5) oPH.level = 3;
            else if (ph < 5.5 || ph > 9.5) oPH.level = 2;
            else if (ph < 6.0 || ph > 9.0) oPH.level = 1;
            else oPH.level = 0;
        }
        var oNH3 = {};
        var nh3 = oSensors['nh3'];
        if (nh3 !== undefined){
            oNH3.value = nh3;
            if (nh3 > 2.0) oNH3.level = 3;
            else if (nh3 > 1.0) oNH3.level = 2;
            else if (nh3 > 0.05) oNH3.level = 1;
            else oNH3.level = 0;  
        }
        var oH2S = {};
        var h2s = oSensors['h2s'];
        if (h2s !== undefined){
            oH2S.value = h2s;
            if (h2s > 0.1) oH2S.level = 3;
            else if (h2s > 0.03) oH2S.level = 2;
            else if (h2s > 0.01) oH2S.level = 1;
            else oH2S.level = 0;  
        }
        var oNO2 = {};
        var no2 = oSensors['no2'];
        if (no2 !== undefined){
            oNO2.value = no2;
            if (no2 > 3.0) oNO2.level = 3;
            else if (no2 > 1.0) oNO2.level = 2;
            else if (no2 > 0.5) oNO2.level = 1;
            else oNO2.level = 0;  
        }
        var oDO = {};
        var do_ = oSensors['do'];
        if (do_ !== undefined){
            oDO.value = do_;
            if (do_ < 2.0) oDO.level = 3;
            else if (do_ < 3.0) oDO.level = 2;
            else if (do_ < 4.0) oDO.level = 1;
            else oDO.level = 0;  
        }
        var oWater_temp = {};
        var temp = oSensors['water_temp'];
        if (temp !== undefined){
            oWater_temp.value = temp;
            if (temp < 10.0) oWater_temp.level = 3;
            else if (temp < 15.0) oWater_temp.level = 2;
            else if (temp < 20.0 || temp > 33.0) oWater_temp.level = 1;
            else oWater_temp.level = 0;  
        }
        var oWater_flow = {};
        var flow = oSensors['water_flow'];
        if (flow !== undefined){
            oWater_flow.value = flow;
            if (flow > 2.0) oWater_flow.level = 3;
            else if (flow > 1.0 || flow < 0.1) oWater_flow.level = 2;
            else if (flow > 0.5 || flow < 0.2) oWater_flow.level = 1;
            else oWater_flow.level = 0;  
        }
        var oItem = {
            ph:oPH,
            nh3:oNH3,
            h2s:oH2S,
            no2:oNO2,
            do:oDO,
            water_temp:oWater_temp,
            water_flow:oWater_flow
        }
        item['sensors'] = oItem;
        oWarning.push(item);
    });
    return oWarning;    
}

CSensor.prototype.GetFilters = function (oFilters,callback) {
    if (oFilters.nid === undefined) return callback(null,[],{});
    var _query = {};
    var _fields = {_id:0, __v: 0};  //loc fields
    var _sort = {sid:-1};
    var _start = oFilters.start || 0;
    var _limit = oFilters.limit || 10;
    if ( _limit < 1 || _limit > 50) _limit = 50;    
    _query.nid = oFilters.nid;
	var isWarning = oFilters.r === undefined ? true : false; 
    var Pages = {};
    async.waterfall([
        function (next) {
            _SensorModel.countDocuments(_query,next);
        },
        function (_count,next) {
          Pages.index = _start;
          Pages.limit = _limit;
          Pages.total = _count;
          _SensorModel.find(_query, _fields).skip(_start).sort(_sort).limit(_limit).exec(next);
        },
        function (mSensors,next){
            if (isWarning){
                var oItems = CheckWarning(mSensors);
                next(null,oItems);
            }
            else next(null,mSensors);
        },
        function (mSensors,next) {
            //console.log(mSensors);
            next(null,mSensors,Pages);
        }
    ],callback);
}

CSensor.prototype.QueryNode = function(oQuery,oFilters,cb){ 
    var oDataRet = {};
    var oPages = {};
    var _count = 0;   
    _SensorModel.countDocuments(oQuery,function (err,cnt){        
        _count = cnt;
        var _fields = {_id:0, __v: 0};  //loc fields
        var _sort = {sid:-1};
        var _start = oFilters.start || 0;
        var _limit = oFilters.limit || 1;
        if ( _limit < 1 || _limit > 50) _limit = 50;
        oPages.index = _start;
        oPages.limit = _limit;
        oPages.total = _count; 
        if (_count <= 0){
            oDataRet['pages'] = oPages;
            oDataRet['data'] = [];
            return cb(null,oDataRet);
        }       
        _SensorModel.find(oQuery, _fields).skip(_start).sort(_sort).limit(_limit).exec(function (err,mSensors){
            oDataRet['pages'] = oPages;            
            oDataRet['data'] = CheckWarning(mSensors);//mSensors;
            cb(null,oDataRet);
        });        
    });
}
