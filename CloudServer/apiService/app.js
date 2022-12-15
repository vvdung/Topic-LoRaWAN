var express     = require('express');
var bodyParser  = require('body-parser');
var async       = require('async');

var GLOBAL      = require('./_global/global');
var HFACTOR     = require('./_database/db_hfactor');

var rHFactor	= require('./routes/r_hfactor');

var app = express();

app.Init = function (callback) {
  async.waterfall([
    function (next) {
        HFACTOR.Init(next);
    }
  ], callback);
}

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

//LINK - http://api.h-factor.vn/v1    https://h-factor.vn/api/v1
app.use('/',rHFactor);                
//app.use('/user/',rPackage);        //LINK/user


app.use(function (req,res) {
	return GLOBAL.ReturnError(res);
});


module.exports = app;