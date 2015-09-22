#ifndef LEPTON_I2C
#define LEPTON_I2C

#include "leptonSDKEmb32PUB/LEPTON_SDK.h"
#include "leptonSDKEmb32PUB/LEPTON_SYS.h"
#include "leptonSDKEmb32PUB/LEPTON_Types.h"

void lepton_disable_auto_ffc();
void lepton_perform_ffc();
int lepton_enable_telemetry(int header);
LEP_SYS_FFC_SHUTTER_MODE_OBJ_T lepton_get_ffc_shutter_mode();
LEP_SYS_SHUTTER_POSITION_E lepton_toggle_shutter();
LEP_SYS_FPA_TEMPERATURE_CELCIUS_T lepton_get_aux_temperature();
LEP_SYS_FPA_TEMPERATURE_CELCIUS_T lepton_get_fpa_temperature();

#endif
