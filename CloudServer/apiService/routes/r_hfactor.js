var express = require('express');
var router = express.Router();

var HFACTOR = require('../controllers/c_hfactor');


router.post('/test', HFACTOR.Test);

router.post('/local/login', HFACTOR.Local_Login);
//router.post('/local/register', HFACTOR.Local_Register);

router.post('/user/login', HFACTOR.User_Login);
router.post('/user/get', HFACTOR.User_Get);
//router.post('/user/register', HFACTOR.User_Register);
router.post('/user/gate/gets', HFACTOR.User_Gate_Gets);
router.post('/user/gate/node/gets', HFACTOR.User_Gate_Node_Gets);

router.post('/gate/add', HFACTOR.Gate_Add);
//router.post('/gate/get', HFACTOR.Gate_Get);
//router.post('/gate/view', HFACTOR.Gate_View);

router.post('/node/add', HFACTOR.Node_Add);
//router.post('/node/get', HFACTOR.Node_Get);
router.post('/node/view/:nId', HFACTOR.Node_View_Sensor);
router.post('/node/views', HFACTOR.Node_Views);


router.post('/sensor/add', HFACTOR.Sensor_Add);
//router.post('/sensor/get', HFACTOR.Sensor_Get);

module.exports = router;