#include "power_private.h"
#include "power_charger.h"

#define PSKEY_CHARGER_TRIM_INVALID  (0xFF)

/****************************************************************************
NAME
    powerChargerGetTrim

DESCRIPTION
    Set ps_chg_trim to invalid.

RETURNS
    void
*/
void powerChargerGetTrim(void)
{
    power->ps_chg_trim = PSKEY_CHARGER_TRIM_INVALID;
}
