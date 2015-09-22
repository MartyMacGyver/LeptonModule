#include "Lepton_I2C.h"

bool _connected;

LEP_CAMERA_PORT_DESC_T _port;

LEP_SYS_SHUTTER_POSITION_E _shutter_pos;

int lepton_connect() {
	LEP_OpenPort(1, LEP_CCI_TWI, 400, &_port);
	_connected = true;
	return 0;
}

void lepton_disable_auto_ffc() {
	if(!_connected) {
		lepton_connect();
	}

	LEP_SYS_FFC_SHUTTER_MODE_OBJ_T smo;
	smo.shutterMode = LEP_SYS_FFC_SHUTTER_MODE_AUTO;
	smo.tempLockoutState = LEP_SYS_SHUTTER_LOCKOUT_INACTIVE;
	smo.videoFreezeDuringFFC = LEP_SYS_ENABLE;
	smo.ffcDesired = LEP_SYS_DISABLE;
	smo.desiredFfcPeriod = (LEP_UINT32)9999000;
	smo.explicitCmdToOpen = (LEP_BOOL)1;
	smo.desiredFfcTempDelta = 0;
	smo.imminentDelay = 300;
	LEP_SetSysFfcShutterModeObj(&_port, smo);
}

void lepton_perform_ffc() {
	if(!_connected) {
		lepton_connect();
	}
	LEP_RunSysFFCNormalization(&_port);
}

int lepton_enable_telemetry(int header) {

	if(!_connected){
		lepton_connect();
	}

	LEP_RESULT tel_location;
	LEP_RESULT tel_enbl = LEP_SetSysTelemetryEnableState(&_port, LEP_TELEMETRY_ENABLED);

	if(header == 0){
		tel_location = LEP_SetSysTelemetryLocation(&_port, LEP_TELEMETRY_LOCATION_HEADER);
	}
	else if(header == 1){
		tel_location = LEP_SetSysTelemetryLocation(&_port, LEP_TELEMETRY_LOCATION_FOOTER);
	}

	if(tel_enbl == LEP_OK && tel_location == LEP_OK){
		return 0;
	}else{
		return -1;
	}

}

LEP_SYS_FFC_SHUTTER_MODE_OBJ_T lepton_get_ffc_shutter_mode() {
	LEP_SYS_FFC_SHUTTER_MODE_OBJ_T ret;

	if(!_connected){
		lepton_connect();
	}

	LEP_GetSysFfcShutterModeObj(&_port, &ret);
	return ret;
}

LEP_SYS_SHUTTER_POSITION_E lepton_toggle_shutter() {
	if(!_connected){
		lepton_connect();
	}

	if (_shutter_pos != LEP_SYS_SHUTTER_POSITION_CLOSED) {
		LEP_SetSysShutterPosition(&_port, LEP_SYS_SHUTTER_POSITION_CLOSED);
	} else {
		LEP_SetSysShutterPosition(&_port, LEP_SYS_SHUTTER_POSITION_OPEN);
	}
	LEP_GetSysShutterPosition(&_port, &_shutter_pos);

	return _shutter_pos;
}

LEP_SYS_FPA_TEMPERATURE_CELCIUS_T lepton_get_aux_temperature() {
	LEP_SYS_FPA_TEMPERATURE_CELCIUS_T ret;

	if(!_connected){
		lepton_connect();
	}

	LEP_GetSysAuxTemperatureCelcius(&_port, &ret);
	return ret;
}

LEP_SYS_FPA_TEMPERATURE_CELCIUS_T lepton_get_fpa_temperature() {
	LEP_SYS_FPA_TEMPERATURE_CELCIUS_T ret;

	if(!_connected){
		lepton_connect();
	}

	LEP_GetSysFpaTemperatureCelcius(&_port, &ret);
	return ret;
}

//presumably more commands could go here if desired
