#include <connection.h>
#include <ps.h>
#include "gaia_private.h"
#include "gaia_register_custom_sdp.h"

/*************************************************************************
NAME
    register_custom_sdp

DESCRIPTION
    Register a custom SDP record retrieved from PS

    The first word in the PS data is the length after unpacking.  This
    allows us to retrieve an odd number of bytes and allows some sanity
    checking.
*/
void register_custom_sdp(uint16 pskey)
{
    uint16 ps_length;

    ps_length = PsFullRetrieve(pskey, NULL, 0);
    GAIA_DEBUG(("gaia: m %04x sdp %d\n", pskey, ps_length));

    if (ps_length > 1)
    {
        uint16 sr_length = 2 * (ps_length - 1);
        uint16 *sr = malloc(sr_length);

        if (sr && PsFullRetrieve(pskey, sr, ps_length) && (sr_length - sr[0] < 2))
        {
        /*  Unpack into uint8s, preserving overlapping word  */
            uint16 idx;
            uint16 tmp = sr[1];

            sr_length = sr[0];
            for (idx = ps_length; idx > 1; )
            {
                --idx;
                sr[2 * idx - 1] = LOW(sr[idx]);
                sr[2 * idx - 2] = HIGH(sr[idx]);
            }

            sr[0] = HIGH(tmp);

            gaia->custom_sdp = TRUE;
            ConnectionRegisterServiceRecord(&gaia->task_data, sr_length, (uint8 *) sr);
        /*  NOTE: firmware will free the slot  */
        }

        else
        {
            free(sr);
            GAIA_DEBUG(("gaia: bad sr\n"));
        }
    }
}
