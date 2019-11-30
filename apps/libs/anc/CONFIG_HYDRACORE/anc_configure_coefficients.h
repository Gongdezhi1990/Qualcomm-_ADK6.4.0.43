/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_configure_coefficients.h

DESCRIPTION

*/

#ifndef ANC_CONFIGURE_COEFFICIENTS_H_
#define ANC_CONFIGURE_COEFFICIENTS_H_

#include <app/audio/audio_if.h>
#include "anc_config_data.h"

anc_instance_config_t * getInstanceConfig(audio_anc_instance instance);
void ancConfigureMutePathGains(void);
void ancConfigureCoefficientsAndGains(void);

#endif /* ANC_CONFIGURE_COEFFICIENTS_H_ */
