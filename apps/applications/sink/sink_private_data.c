/**********************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_private_data.c

DESCRIPTION
    This module works as a container for all private and common data that is used across sink
    qpplication. It provides API to access the data from outside. It interacts with configuration
    entities to read/write configuration data. More over this module contains module specific
    run time data (as a common runtime data for entire sink application), and those data also
    exposed using defined API's

 NOTES
      Module does not have any intelligence to manipulate the data, or verify the data or its
      contents ,it is the user to decide hoe to use this data and when to update the data.
*/

#include <stdlib.h>
#include <ps.h>
#include <vmtypes.h>
#include <bdaddr.h>
#include <byte_utils.h>

#include "sink_private_data.h"
#include "sink_dut.h"
#include "sink_malloc_debug.h"
#include "sink_configmanager.h"

#include <local_device.h>

#ifdef DEBUG_SINK_PRIVATE_DATA
#define SINK_DATA_DEBUG(x) DEBUG(x)
#define SINK_DATA_ERROR(x) TOLERATED_ERROR(x)
#else
#define SINK_DATA_DEBUG(x)
#define SINK_DATA_ERROR(x)
#endif

/* Referance to Global Data for sink private module */
typedef struct __sinkdata_globaldata_t
{
    unsigned panic_reconnect:1; /* Are we using panic action? Bit to inidcate panic reconnection action is used  */
    unsigned paging_in_progress:1; /* Bit to indicate that device is curretly paging whilst in connectable state*/
    unsigned powerup_no_connection:1; /* Bit to indicate device has powered and no connections yet */
    unsigned confirmation:1; /* Bit to indicate user auth confirmation status */
    unsigned SinkInitialising:1; /* Bit to indicate sink is in initialising state */
    unsigned PowerOffIsEnabled:1; /* Bit to indicate power off is enabled */
    unsigned debug_keys_enabled:1; /* Bit to indicate debug keys enabled */
    unsigned stream_protection_state:2; /* Holds stream protection state */
    unsigned MultipointEnable:1; /* Bit to indicate multipoint enabled */
    unsigned _spare1_:6;
    unsigned gEventQueuedOnConnection:8; /* variable to hold evet queued while in connection */
    unsigned dfu_access:1;              /* Link Policy expedites DFU data transfer */
    unsigned display_link_keys:1; /* Bit used to indicate if link keys should be displayed */
    unsigned _spare2_:6;
    uint16 NoOfReconnectionAttempts; /* Holdes current number of reconnection attempts */
    uint16 connection_in_progress;  /* flag used to block role switch requests until all connections are complete or abandoned */
#ifdef ENABLE_SQIFVP
    unsigned               partitions_mounted:8;
    unsigned               unused:8;
#endif
    bdaddr                   *linkloss_bd_addr;  /** bdaddr of a2dp device that had the last link loss. */
    tp_bdaddr                *confirmation_addr;
    bdaddr local_bd_addr; /* Local BD Address of the sink device available in ps */
    power_table              *user_power_table;  /* pointer to user power table if available in ps */
}sinkdata_globaldata_t;

/* PSKEY for BD ADDRESS */
#define PSKEY_BDADDR   0x0001
#define LAP_MSW_OFFSET 0
#define LAP_LSW_OFFSET 1
#define UAP_OFFSET 2
#define NAP_OFFSET 3

/* Global data strcuture element for sink private data */
static sinkdata_globaldata_t gSinkData;
#define GSINKDATA  gSinkData


/**********************************************************************
***************  External Interface Function Implemetations  **********************
***********************************************************************/

/**********************************************************************
        Interfaces for accessing Configurable Items
*/
/**********************************************************************
  Interfaces for Initializing Local Address, which read the local address.
*/
bool  sinkDataInitLocalBdAddrFromPs(void)
{
    bool result = FALSE;

    uint16 size = PS_SIZE_ADJ(sizeof(GSINKDATA.local_bd_addr));
    uint16* bd_addr_data = (uint16*)PanicUnlessNew(bdaddr);
    
    BdaddrSetZero(&GSINKDATA.local_bd_addr);

    if(size == PsFullRetrieve(PSKEY_BDADDR, bd_addr_data, size))
    {
        GSINKDATA.local_bd_addr.nap = bd_addr_data[NAP_OFFSET];
        GSINKDATA.local_bd_addr.uap = bd_addr_data[UAP_OFFSET];
        GSINKDATA.local_bd_addr.lap = MAKELONG(bd_addr_data[LAP_LSW_OFFSET], bd_addr_data[LAP_MSW_OFFSET]);

        SINK_DATA_DEBUG(("CONF: PSKEY_BDADDR [%04x %02x %06lx]\n",
                GSINKDATA.local_bd_addr.nap, GSINKDATA.local_bd_addr.uap, GSINKDATA.local_bd_addr.lap));
        result = TRUE;
    }
    else
    {
        GSINKDATA.local_bd_addr = LocalDeviceGetBdAddr();
    }

    free(bd_addr_data);

    return result;
}

/**********************************************************************
  Interfaces for checking reconnect on panic configuration is enabled or not
*/
bool sinkDataIsReconnectOnPanic(void)
{
    bool reconnect_on_panic = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsReconnectOnPanic()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        reconnect_on_panic = read_configdata->ReconnectOnPanic;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData:reconnect_on_panic = %d \n",reconnect_on_panic));
    return (reconnect_on_panic)?TRUE : FALSE;
}

/**********************************************************************
  Interfaces for checking power of after PDL reset configuration is enabled or not
*/
bool sinkDataIsPowerOffAfterPDLReset(void)
{
    bool poweroff_pdl_reset = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsPowerOffAfterPDLReset()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        poweroff_pdl_reset = read_configdata->PowerOffAfterPDLReset;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: poweroff_pdl_reset = %d \n",poweroff_pdl_reset));
    return (poweroff_pdl_reset)?TRUE : FALSE;
}

/**********************************************************************
  Interfaces  for checking does sink shoule be in discoverable mode all time
  configuration is enabled or not
*/
bool sinkDataIsDiscoverableAtAllTimes(void)
{
    bool discoverable_alltime = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsDiscoverableAtAllTimes()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        discoverable_alltime = read_configdata->RemainDiscoverableAtAllTimes;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: discoverable_alltime = %d\n",discoverable_alltime));
    return (discoverable_alltime)?TRUE : FALSE;
}

/**********************************************************************
  Interfaces  for checking DisablePowerOffAfterPowerOn configuration is enabled or not
*/
bool sinkDataCheckDisablePowerOffAfterPowerOn(void)
{
    bool disable_poweroff = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataCheckDisablePowerOffAfterPowerOn()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        disable_poweroff = read_configdata->DisablePowerOffAfterPowerOn;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: disable_poweroff = %d\n",disable_poweroff));
    return (disable_poweroff)?TRUE : FALSE;
}

/**********************************************************************
  Interfaces  for checking pairing mode on connection failure configuration is enabled or not
*/
bool sinkDataEntrePairingModeOnConFailure(void)
{
    bool failuretoconnect = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataPairingModeOnConnectionFailure()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        failuretoconnect = read_configdata->EnterPairingModeOnFailureToConn;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: failuretoconnect = %d\n",failuretoconnect));
    return (failuretoconnect)?TRUE : FALSE;
}

/**********************************************************************
  Interfaces for checking Power Off OnlyIf VReg Enble is low configuration is enabled or not
*/
bool sinkDataIsPowerOffOnlyIfVRegEnlow(void)
{
    bool vreg_enlow = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsPowerOffOnlyIfVRegEnlow()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        vreg_enlow = read_configdata->PowerOffOnlyIfVRegEnLow;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: vreg_enlow = %d\n",vreg_enlow));
    return (vreg_enlow)?TRUE : FALSE;
}

/**********************************************************************/
bool sinkDataAllowAutomaticPowerOffWhenCharging(void)
{
    bool power_off_when_charging = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataAllowAutomaticPowerOffWhenCharging()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        power_off_when_charging = read_configdata->AllowAutomaticPowerOffWhenCharging;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: power_off_when_charging = %d\n",power_off_when_charging));
    return (power_off_when_charging) ? TRUE : FALSE;
}

/**********************************************************************
  Interfaces for checking auto pairing mode  configuration is enabled or not
*/
bool sinkDataAutoPairModeEnabled(void)
{
    bool parimode_enabled = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataAutoPairModeEnabled()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        parimode_enabled = read_configdata->pair_mode_en;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: parimode_enabled = %d\n",parimode_enabled));
    return (parimode_enabled)?TRUE : FALSE;
}

/**********************************************************************
  Interfaces for checking auto reconnect to last device on power on
  configuration is enabled or not
*/
bool sinkDataCheckAutoReconnectPowerOn(void)
{
    bool auto_poweron_reconnect = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataCheckAutoReconnectPowerOn()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        auto_poweron_reconnect = read_configdata->AutoReconnectPowerOn;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: auto_poweron_reconnect = %d\n",auto_poweron_reconnect));
    return (auto_poweron_reconnect)?TRUE : FALSE;
}

/**********************************************************************
  Interfaces for checking do not disconnect during link loss
  configuration is enabled or not
*/
bool sinkDataCheckDoNotDisconDuringLinkLoss(void)
{
    bool discon_duringlinkloss = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataCheckDoNotDisconDuringLinkLoss()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        discon_duringlinkloss = read_configdata->DoNotDiscoDuringLinkLoss;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: discon_duringlinkloss = %d\n",discon_duringlinkloss));
    return (discon_duringlinkloss)?TRUE:FALSE;
}

/**********************************************************************
  Interfaces for accessing action in pairing  mode timeout configuration
*/
uint8 sinkDataGetActionOnParingmodeTimeout(void)
{
    uint8 disconnect_action= 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetActionOnParingmodeTimeout()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        disconnect_action = read_configdata->PowerDownOnDiscoTimeout;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: disconnect_action = %d\n",disconnect_action));
    return disconnect_action;
}

/**********************************************************************
  Interfaces for accessing action on power on
*/
uint8 sinkDataGetPowerOnAction(void)
{
    uint8 poweron_action = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetPowerOnAction()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        poweron_action = read_configdata->ActionOnPowerOn;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: poweron_action = %d\n",poweron_action));
    return poweron_action;
}

/**********************************************************************
  Interfaces for getting the PDL limit for starting a discovery process
*/
uint8 sinkDataGetPDLLimitForDiscovery(void)
{
    uint8 force_discoverablemode = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetPDLLimitForDiscovery()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        force_discoverablemode = read_configdata->DiscoIfPDLLessThan;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: force_discoverablemode = %d\n",force_discoverablemode));
    return force_discoverablemode;
}

/**********************************************************************
  Interfaces for checking MITM is required for authentication
*/
bool sinkDataIsMITMRequired(void)
{
    bool mitm_enabled = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsMITMRequired()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        mitm_enabled = read_configdata->ManInTheMiddle;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: mitm_enabled = %d\n",mitm_enabled));
    return (mitm_enabled)? TRUE:FALSE;
}

/**********************************************************************
  Interfaces for checking to send disconnect event at power on
  configuration is enabled or not
*/
bool sinkDataDiffConnEventAtPowerOnEnabled(void)
{
    bool poweron_event = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataDiffConnEventAtPowerOnEnabled()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        poweron_event = read_configdata->UseDiffConnectedEventAtPowerOn;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: poweron_event = %d\n",poweron_event));
    return(poweron_event)?TRUE:FALSE;
}

/**********************************************************************
  Interfaces for checking secure pairing configuration is enabled or not
*/
bool sinkDataIsSecureParingEnabled(void)
{
    bool secure_pariring_enabled =FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsSecureParingEnabled()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        secure_pariring_enabled = read_configdata->SecurePairing;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: secure_pariring_enabled = %d\n",secure_pariring_enabled));
    return(secure_pariring_enabled)? TRUE:FALSE;
}

/**********************************************************************
  Interfaces for checking reset after poweroff  configuration is enabled or not
*/
bool sinkDataIsResetAfterPowerOff(void)
{
    bool reset_afterpoweroff = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsResetAfterPowerOff()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        reset_afterpoweroff = read_configdata->ResetAfterPowerOffComplete;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: reset_afterpoweroff = %d\n",reset_afterpoweroff));
    return(reset_afterpoweroff)? TRUE:FALSE;
}

/**********************************************************************
  Interfaces for checking auto poweron after init  configuration is enabled or not
*/
bool sinkDataIsAutoPoweronAfterInit(void)
{
    bool auto_pweroff = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsAutoPoweronAfterInit()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        auto_pweroff = read_configdata->AutoPowerOnAfterInitialisation;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: auto_pweroff = %d\n",auto_pweroff));
    return(auto_pweroff)? TRUE:FALSE;
}

/**********************************************************************
  Interfaces for checking auto poweroff after init  configuration is enabled or not
*/
bool sinkDataIsRoleSwitchingDisbaled(void)
{
    bool roleswitch_disabled = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsRoleSwitchingDisbaled()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        roleswitch_disabled = read_configdata->DisableRoleSwitching;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: roleswitch_disabled = %d\n",roleswitch_disabled));
    return(roleswitch_disabled)?TRUE:FALSE;
}

/**********************************************************************
  Interfaces for checking connectable action on link loss configuration is enabled or not
*/
bool sinkDataGoConnectableDuringLinkLoss(void)
{
    bool connectable_linkloss = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGoConnectableDuringLinkLoss()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        connectable_linkloss = read_configdata->GoConnectableDuringLinkLoss;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: connectable_linkloss = %d\n",connectable_linkloss));
    return(connectable_linkloss)?TRUE:FALSE;
}

/**********************************************************************
  Interfaces for checking When to enable RSSI pairing as per PDL entry
*/
uint8 sinkDataGetPDLLimitforRSSIPairing(void)
{
    uint8 pdl_limitforRSSI = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetPDLLimitforRSSIPairing()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        pdl_limitforRSSI = read_configdata->PairIfPDLLessThan;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: pdl_limitforRSSI = %d\n",pdl_limitforRSSI));
    return pdl_limitforRSSI;
}

/**********************************************************************
  Interfaces for checking panic reset action
*/
uint8 sinkDataGetPanicResetAction(void)
{
    uint8 panicreset_action = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetPanicResetAction()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        panicreset_action = read_configdata->ActionOnPanicReset;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: panicreset_action = %d\n",panicreset_action));
    return panicreset_action;
}

/**********************************************************************
  Interfaces for getting number of reconnect last attempts
*/
uint8 sinkDataGetReconnectLastAttempts(void)
{
    uint8 lastreconnect_attempts = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetReconnectLastAttempts()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        lastreconnect_attempts = read_configdata->ReconnectLastAttempts;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: lastreconnect_attempts = %d\n",lastreconnect_attempts));
    return lastreconnect_attempts;

}

/**********************************************************************
  Interfaces for getting reconnections attempts
*/
uint16 sinkDataGetReconnectionAttempts(void)
{
    uint8 reconnect_attempt = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetReconnectionAttempts()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        reconnect_attempt = read_configdata->ReconnectionAttempts;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: reconnect_attempt = %d\n",reconnect_attempt));
    return reconnect_attempt;
}

bool sinkDataSetReconnectionAttempts(uint16 attempts)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetReconnectionAttempts()\n"));

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->ReconnectionAttempts = attempts;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    Interfaces for getting  defragment key size
*/
uint8 sinkDataGetDefragKeySize(void)
{
    uint8 key_size = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetDefragKeySize()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        key_size = read_configdata->defrag.key_size;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: key_size = %d\n",key_size));
    return key_size;
}

/**********************************************************************
    Interfaces for getting  defragment key Min
*/
uint8 sinkDataGetDefragKeyMin(void)
{
    uint8 key_min =0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetDefragKeyMin()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        key_min = read_configdata->defrag.key_minimum;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: key_min = %d\n",key_min));
    return key_min;
}

/**********************************************************************
    Interfaces for getting ssr slc params
*/
void sinkDataGetSsrSlcParams(ssr_params_t*slcParams)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetSsrSlcParams()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        memcpy(slcParams,&read_configdata->ssr_data.slc_params,sizeof(ssr_params_t));
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    return;
}

/**********************************************************************
    Interfaces for getting ssr sco params
*/
void sinkDataGetSsrScoParams(ssr_params_t*scoParams)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetSsrScoParams()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        memcpy(scoParams,&read_configdata->ssr_data.sco_params,sizeof(ssr_params_t));
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    return;
}
/**********************************************************************
    Interfaces for getting local bd address
*/
void sinkDataGetLocalBdAddress(bdaddr *local_bd_addr)
{
    *local_bd_addr = GSINKDATA.local_bd_addr;
}

/*************************** Timer Interfaces *****************************/

/**********************************************************************
    Interfaces for getting Auto Switch off timer value
*/
uint16 sinkDataGetAutoSwitchOffTimeout(void)
{
    uint16 autoswitchoff_timer = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        autoswitchoff_timer = read_configdata->private_data_timers.AutoSwitchOffTime_s;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }

    return autoswitchoff_timer;
}

bool sinkDataSetAutoSwitchOffTimeout(uint16 timeout)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->private_data_timers.AutoSwitchOffTime_s = timeout;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    Interfaces for getting Limbo time out values
*/
uint16 sinkDataGetLimboTimeout(void)
{
    uint16 limbo_timer = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        limbo_timer = read_configdata->private_data_timers.LimboTimeout_s;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    return limbo_timer;
}

bool sinkDataSetLimboTimeout(uint16 timeout)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->private_data_timers.LimboTimeout_s = timeout;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    Interfaces for getting power off disable timeout
*/
uint16 sinkDataGetDisablePoweroffAfterPoweronTimeout(void)
{
    uint16 disable_poweroff_timer =0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        disable_poweroff_timer = read_configdata->private_data_timers.DisablePowerOffAfterPOnTime_s;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    return disable_poweroff_timer;
}

bool sinkDataSetDisablePoweroffAfterPoweronTimeout(uint16 timeout)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->private_data_timers.DisablePowerOffAfterPOnTime_s = timeout;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    Interfaces for getting pair mode timeout
*/
uint16 sinkDataGetPairModeTimeout(void)
{
    uint16 pairmode_timer = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        pairmode_timer = read_configdata->private_data_timers.PairModeTimeout_s;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }

    return pairmode_timer;
}

bool sinkDataSetPairModeTimeout(uint16 timeout)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->private_data_timers.PairModeTimeout_s = timeout;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    Interfaces for getting connectable timeout
*/
uint16 sinkDataGetConnectableTimeout(void)
{
    uint16 connectable_timer = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        connectable_timer = read_configdata->private_data_timers.ConnectableTimeout_s;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    return connectable_timer;
}

bool sinkDataSetConnectableTimeout(uint16 timeout)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->private_data_timers.ConnectableTimeout_s = timeout;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    Interfaces for getting pairmode PDL_empty timeout
*/
uint16 sinkDataGetPairModePDLTimeout(void)
{
    uint16 pairmodePDL_timer = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        pairmodePDL_timer = read_configdata->private_data_timers.PairModeTimeoutIfPDL_s;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    return pairmodePDL_timer;
}

bool sinkDataSetPairModePDLTimeout(uint16 timeout)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->private_data_timers.PairModeTimeoutIfPDL_s = timeout;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    Interfaces for getting encryption refresh timeout
*/
uint16 sinkDataGetEncryptionRefreshTimeout(void)
{
    uint16 encryptionrefresh_timer = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        encryptionrefresh_timer = read_configdata->private_data_timers.EncryptionRefreshTimeout_m;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    return encryptionrefresh_timer;
}

bool sinkDataSetEncryptionRefreshTimeout(uint16 timeout)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->private_data_timers.EncryptionRefreshTimeout_m = timeout;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    Interfaces for amp power down timeout
*/
uint16 sinkDataAmpPowerDownTimeout(void)
{
    uint16 audioamp_timer = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        audioamp_timer = read_configdata->private_data_timers.AudioAmpPDownTimeInLimbo_s;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    return audioamp_timer;
}

bool sinkDataSetAmpPowerDownTimeout(uint16 timeout)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->private_data_timers.AudioAmpPDownTimeInLimbo_s = timeout;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    Interfaces for getting second ag conenct delay timeout
*/
uint16 sinkDataSecondAGConnectDelayTimeout(void)
{
    uint16 secondAg_timer = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        secondAg_timer = read_configdata->private_data_timers.SecondAGConnectDelayTime_s;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    return secondAg_timer;
}

bool sinkDataSetSecondAGConnectDelayTimeout(uint16 timeout)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->private_data_timers.SecondAGConnectDelayTime_s = timeout;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************
    Interfaces for getting defrag check timeout
*/
uint16 sinkDataDefragCheckTimout(void)
{
    uint16 defrag_timer = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        defrag_timer = read_configdata->private_data_timers.DefragCheckTimer_s;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }

    return defrag_timer;
}

bool sinkDataSetDefragCheckTimout(uint16 timeout)
{
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (void **)&read_configdata, 0))
    {
        read_configdata->private_data_timers.DefragCheckTimer_s = timeout;
        configManagerUpdateWriteableConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*****************      Runtime Data Interfaces      ****************************/


/**********************************************************************
    Interfaces for getting user power table values
*/
void sinkDataSetUserPowerTable(power_table *powerTable)
{
    GSINKDATA.user_power_table = powerTable;
}

/**********************************************************************
    Interfaces for setting user power table
*/
power_table * sinkDataGetUserPowerTable(void)
{
    return GSINKDATA.user_power_table;
}

/**********************************************************************
    Interfaces for setting number of reconnection attempts
*/
void sinkDataSetNoOfReconnectionAttempts(uint16 reconenctAttempts)
{
    GSINKDATA.NoOfReconnectionAttempts = reconenctAttempts;
}

/**********************************************************************
    Interfaces for getting number of reconnection attempts
*/
uint16 sinkDataGetNoOfReconnectionAttempts(void)
{
    return GSINKDATA.NoOfReconnectionAttempts;
}

/**********************************************************************
    Interfaces for getting confirmation address
*/
void sinkDataSetSMConfirmationAddress(tp_bdaddr *confirmation_addr)
{
    GSINKDATA.confirmation_addr = confirmation_addr;
}

/**********************************************************************
    Interfaces for setting confirmation address
*/
tp_bdaddr *sinkDataGetSMConfirmationAddress(void)
{
    return GSINKDATA.confirmation_addr;
}

/**********************************************************************
    Interfaces for setting link loss BD address
*/
void sinkDataSetLinkLossBDaddr(bdaddr *linkloss_bd_addr)
{
    GSINKDATA.linkloss_bd_addr = linkloss_bd_addr;
}

/**********************************************************************
    Interfaces for getting link loss bd address
*/
bdaddr *sinkDataGetLinkLossBDaddr(void)
{
    return GSINKDATA.linkloss_bd_addr;
}
/**********************************************************************
    Interfaces for setting power off enable
*/
void sinkDataSetPoweroffEnabled(bool poweroffEnable)
{
    GSINKDATA.PowerOffIsEnabled = poweroffEnable;
}
/**********************************************************************
    Interfaces for getting power off enable
*/
bool sinkDataIsPowerOffEnabled(void)
{
    return(GSINKDATA.PowerOffIsEnabled)? TRUE:FALSE;
}

/**********************************************************************
    Interfaces for setting sink Init status
*/
void sinkDataSetSinkInitialisingStatus(bool initialising)
{
     GSINKDATA.SinkInitialising = initialising;
}

/**********************************************************************
    Interfaces for getting sink Init status
*/
bool sinkDataGetSinkInitialisingStatus(void)
{
    return (GSINKDATA.SinkInitialising)? TRUE: FALSE;
}

/**********************************************************************
    Interfaces for setting auth confirmation flag
*/
void sinkDataSetAuthConfirmationFlag(bool authCfm)
{
    GSINKDATA.confirmation = authCfm;
}

/**********************************************************************
    Interfaces for getting auth confirmation flag
*/
bool sinkDataGetAuthConfirmationFlag(void)
{
    return (GSINKDATA.confirmation) ? TRUE:FALSE;
}

/**********************************************************************
    Interfaces for setting debug key enabled
*/
void sinkDataSetDebugKeyEnabledFlag(bool debugKey)
{
    GSINKDATA.debug_keys_enabled = debugKey;
}

/**********************************************************************
    Interfaces for geting debug key enabled
*/
bool sinkDataIsDebugKeyEnabled(void)
{
    return( GSINKDATA.debug_keys_enabled)?TRUE:FALSE;
}

/**********************************************************************
    Interface to set dfu access state
*/
void sinkDataSetDfuAccess(bool access)
{
    GSINKDATA.dfu_access = access;
}

/**********************************************************************
    @brief Interface to get dfu access state
*/
bool sinkDataGetDfuAccess(void)
{
     return( GSINKDATA.dfu_access)?TRUE:FALSE;
}

/**********************************************************************
    Interfaces for geting DisableCompletePowerOff.
*/
bool sinkDataIsDisableCompletePowerOff(void)
{
    bool DisableCompletePowerOff = FALSE;
    sinkdata_readonly_config_def_t *read_configdata = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsDisableCompletePowerOff()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        DisableCompletePowerOff = read_configdata->DisableCompletePowerOff;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }

    SINK_DATA_DEBUG(("SinkData: Disable complete power off = %d\n",DisableCompletePowerOff));
    return(DisableCompletePowerOff)?TRUE:FALSE;
}
/**********************************************************************
    Interfaces for setting session data for multipoint enabled
*/
void sinkDataSetSessionMultiPointEnabled(void)
{
    sinkdata_writeable_config_def_t *writeable_data;
    SINK_DATA_DEBUG(("SinkData:sinkDataSetSessionMultiPointEnabled()\n"));

    if (configManagerGetWriteableConfig(SINKDATA_WRITEABLE_CONFIG_BLK_ID, (void **)&writeable_data, 0))
    {
        writeable_data->multipoint_enable = GSINKDATA.MultipointEnable;
        configManagerUpdateWriteableConfig(SINKDATA_WRITEABLE_CONFIG_BLK_ID);
    }
}
/**********************************************************************
    Interfaces for getting session data for multipoint enabled
*/
bool sinkDataGetSessionMultiPointEnabled(void)
{
    bool multipoint_enabled = FALSE;
    sinkdata_writeable_config_def_t *writeable_data;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetSessionMultiPointEnabled()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_WRITEABLE_CONFIG_BLK_ID, (const void **)&writeable_data))
    {
        multipoint_enabled = writeable_data->multipoint_enable;
        configManagerReleaseConfig(SINKDATA_WRITEABLE_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: multipoint_enabled = %d\n",multipoint_enabled));
    return (multipoint_enabled)?TRUE:FALSE;
}

/**********************************************************************
    Interfaces for geting EncryptOnSLCEstablishment.
*/
bool sinkDataIsEncryptOnSLCEstablishment(void)
{
    bool result = FALSE;
    sinkdata_readonly_config_def_t *ro_data = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataIsEncryptOnSLCEstablishment()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        result = ro_data->EncryptOnSLCEstablishment;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: EncryptOnSLCEstablishment = %d\n",result));
    return (result)?TRUE:FALSE;
}

/**********************************************************************
    Interfaces for geting EncryptOnSLCEstablishment.
*/
uint16 sinkDataGetPDLSize(void)
{
    uint16 result = 0;
    sinkdata_readonly_config_def_t *ro_data = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetPDLSize()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        result = ro_data->pdl_size;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: sinkDataGetPDLSize = %d\n",result));
    return result;
}
/**********************************************************************
    Interfaces for setting multipoint enabled
*/
void sinkDataSetMultipointEnabled(bool multipointEnable)
{
    GSINKDATA.MultipointEnable = multipointEnable;
}

/**********************************************************************
    Interfaces for checking multipoint enabled
*/
bool sinkDataIsMultipointEnabled(void)
{
    return (GSINKDATA.MultipointEnable)?TRUE:FALSE;
}

/**********************************************************************
    Interfaces for getting session data for multipoint enabled
*/
void sinkDataSetPowerUpNoConnectionFlag(bool powerupConenction)
{
    GSINKDATA.powerup_no_connection = powerupConenction;
}
/**********************************************************************
    Interfaces for setting power up no connection flag
*/
bool sinkDataGetPowerUpNoConnectionFlag(void)
{
    return (GSINKDATA.powerup_no_connection)?TRUE:FALSE;
}
/**********************************************************************
    Interfaces for setting paging in progress flag
*/
void sinkDataSetPagingInProgress(bool paging)
{
    GSINKDATA.paging_in_progress = paging;
}
/**********************************************************************
    Interfaces for checking paging in progress flag
*/
bool sinkDataIsPagingInProgress(void)
{
    return (GSINKDATA.paging_in_progress)?TRUE:FALSE;
}
/**********************************************************************
    Interfaces for setting queue event
*/
void sinkDataSetEventQueuedOnConnection(uint8 event)
{
    GSINKDATA.gEventQueuedOnConnection= event;
}
/**********************************************************************
    Interfaces for getting queue event status
*/
uint8 sinkDataGetEventQueuedOnConnection(void)
{
    return GSINKDATA.gEventQueuedOnConnection;
}

/**********************************************************************
    Interfaces for setting stream protection state
 */
void sinkDataSetStreamProtectionState(linkloss_stream_protection_state_t streamState)
{
    GSINKDATA.stream_protection_state = streamState;
}
/**********************************************************************
    Interfaces for getting stream protection state
 */
linkloss_stream_protection_state_t sinkDataGetStreamProtectionState(void)
{
    return GSINKDATA.stream_protection_state;
}
/**********************************************************************
    Interfaces for setting sqif partition mount
 */
#ifdef ENABLE_SQIFVP
void sinkDataSetSquifPartitionMount(uint8 mount)
{
     GSINKDATA.partitions_mounted = mount;
}
/**********************************************************************
    Interfaces for getting sqif partition mount
 */
uint8 sinkDataGetSquifMountedPartitions(void)
{
    return GSINKDATA.partitions_mounted;
}

/**********************************************************************
    Interfaces for getting free sqif partition
 */
uint8 sinkDataGetSquifFreePartitions(void)
{
    uint8 result = 0;
    sinkdata_readonly_config_def_t *ro_data = NULL;
    SINK_DATA_DEBUG(("SinkData:sinkDataGetSquifFreePartitions()\n"));

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&ro_data))
    {
        result = ro_data->partitions_free;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }
    SINK_DATA_DEBUG(("SinkData: sinkDataGetSquifFreePartitions = %d\n",result));
    return result;
}
#endif /* ENABLE_SQIFVP */

/**********************************************************************
    Interfaces for setting connection in progress flag
 */
void sinkDataSetConnectionInProgress(bool connectionProgress)
{
    GSINKDATA.connection_in_progress = connectionProgress;
}
/**********************************************************************
    Interfaces for getting connection in progress flag address
 */
uint16 *sinkDataIsConnectionInProgress(void)
{
    return &GSINKDATA.connection_in_progress;
}


/**********************************************************************
    Interfaces for setting panic reconnect flag
 */
void sinkDataSetPanicReconnect(bool panicReset)
{
   GSINKDATA.panic_reconnect = panicReset;
}
/**********************************************************************
    Interfaces for getting panic rest flag
 */
bool sinkDataGetPanicReconnect(void)
{
    return GSINKDATA.panic_reconnect;
}

/**********************************************************************
    Interface to get display link keys flag
 */
bool sinkDataGetDisplayLinkKeys(void)
{
    return GSINKDATA.display_link_keys;
}

/**********************************************************************
    Interface to set display link keys flag
 */
void sinkDataSetDisplayLinkKeys(bool displayLinkKeys)
{
    GSINKDATA.display_link_keys = displayLinkKeys;
}

/**********************************************************************
    Interface to get pairing mode reminder interval
 */
uint16 sinkDataGetPairingReminderInterval(void)
{
    uint16 interval = 0;
    sinkdata_readonly_config_def_t *read_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINKDATA_READONLY_CONFIG_BLK_ID, (const void **)&read_configdata))
    {
        interval = read_configdata->private_data_timers.PairingModeReminderInterval_s;
        configManagerReleaseConfig(SINKDATA_READONLY_CONFIG_BLK_ID);
    }

    return interval;
}
