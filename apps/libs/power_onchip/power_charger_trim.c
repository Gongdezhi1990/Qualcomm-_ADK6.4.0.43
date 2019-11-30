#include <ps.h>

#include "power_private.h"
#include "power_charger.h"

/* Voltage level: 0 - under limit or 1 - equal to or over limit */
#define PSKEY_CHARGER_TRIM          (0x03B7)
#define PSKEY_CHARGER_TRIM_INVALID  (0xFF)

/****************************************************************************
NAME
    powerChargerGetTrim

DESCRIPTION
    Read PSKEY_CHARGER_TRIM or set to invalid if not present.

RETURNS
    void
*/
void powerChargerGetTrim(void)
{
    if(!PsFullRetrieve(PSKEY_CHARGER_TRIM, &power->ps_chg_trim, sizeof(uint8)))
        power->ps_chg_trim = PSKEY_CHARGER_TRIM_INVALID;
}
