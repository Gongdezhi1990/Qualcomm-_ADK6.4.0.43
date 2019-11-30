/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/*
  Defines the MessageId's for messages generated in the firmware but
  destined for the VM application
*/


#ifndef APP_SYSTEM_MESSAGE_H__
#define APP_SYSTEM_MESSAGE_H__

#include "app/adc/adc_if.h"
#include "app/usb/usb_if.h"
#include "app/vm/vm_if.h"
#include "app/capacitive_sensor/capacitive_sensor_if.h"
#include "app/partition/partition_if.h"
#include "app/infrared/infrared_if.h"
#include "app/charger/charger_if.h"
#include "app/bitserial/bitserial_if.h"
#include "app/message/subsystem_if.h"
#include <source_.h>
#include <sink_.h>

#define SYSTEM_MESSAGE_BASE_               0x8000 /*!< All system message numbers are relative to this.*/

/*!
  All Bluestack message numbers are relative to this.

  Bluestack messages are sent to the task registered with MessageBlueStackTask(),
  except MESSAGE_BLUESTACK_ATT_PRIM which are routed to the task registered with MessageAttTask().
*/
#define MESSAGE_BLUESTACK_BASE_           (SYSTEM_MESSAGE_BASE_)

#define MESSAGE_BLUESTACK_LC_PRIM         (MESSAGE_BLUESTACK_BASE_ + 1)  /*!< Link Controller primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_LM_PRIM         (MESSAGE_BLUESTACK_BASE_ + 2)  /*!< Link Manager primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_HCI_PRIM        (MESSAGE_BLUESTACK_BASE_ + 3)  /*!< Host Controller Interface primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_DM_PRIM         (MESSAGE_BLUESTACK_BASE_ + 4)  /*!< Device Manager primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_L2CAP_PRIM      (MESSAGE_BLUESTACK_BASE_ + 5)  /*!< L2CAP primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_RFCOMM_PRIM     (MESSAGE_BLUESTACK_BASE_ + 6)  /*!< RFCOM primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_SDP_PRIM        (MESSAGE_BLUESTACK_BASE_ + 7)  /*!< Service Discovery Protocol primitive.*/
#define MESSAGE_BLUESTACK_BCSP_LM_PRIM    (MESSAGE_BLUESTACK_BASE_ + 8)  /*!< BCSP LM primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_BCSP_HQ_PRIM    (MESSAGE_BLUESTACK_BASE_ + 9)  /*!< BCSP HQ primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_BCSP_BCCMD_PRIM (MESSAGE_BLUESTACK_BASE_ + 10) /*!< BCSP BCCMD primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_CALLBACK_PRIM   (MESSAGE_BLUESTACK_BASE_ + 11) /*!< Callback primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_TCS_PRIM        (MESSAGE_BLUESTACK_BASE_ + 12) /*!< TCS primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_BNEP_PRIM       (MESSAGE_BLUESTACK_BASE_ + 13) /*!< BNEP primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_TCP_PRIM        (MESSAGE_BLUESTACK_BASE_ + 14) /*!< TCP primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_UDP_PRIM        (MESSAGE_BLUESTACK_BASE_ + 15) /*!< UDP primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_FB_PRIM         (MESSAGE_BLUESTACK_BASE_ + 16) /*!< FB primitive from Bluestack.*/
#define MESSAGE_BLUESTACK_ATT_PRIM        (MESSAGE_BLUESTACK_BASE_ + 18) /*!< ATT primitive from Bluestack.*/

#define MESSAGE_BLUESTACK_END_            (MESSAGE_BLUESTACK_BASE_ + 19) /*!< End of Bluestack primitives.*/

/*
    Word-oriented message over BCSP#13. Message is the legacy
    word-format with length+type+payload.
*/

/*!
  Message received from the host. The message content has the format
  described at \ref host_messages.

  This message is sent to the task registered with MessageHostCommsTask().
 */
#define MESSAGE_FROM_HOST                 (SYSTEM_MESSAGE_BASE_ + 32)

/*!
  A Source has more data. The message content is a #MessageMoreData.

  This message is sent to any task associated with the Source using MessageSinkTask().

  The frequency of this message can be controlled with #VM_SOURCE_MESSAGES.
 */
#define MESSAGE_MORE_DATA                 (SYSTEM_MESSAGE_BASE_ + 33)

/*!
  A Sink has more space. The message content is a #MessageMoreSpace.

  This message is sent to any task associated with the Sink using MessageSinkTask().

  The frequency of this message can be controlled with #VM_SINK_MESSAGES.
 */
#define MESSAGE_MORE_SPACE                (SYSTEM_MESSAGE_BASE_ + 34)

/*!
  A PIO has changed. The message content is a #MessagePioChanged.

  This message is sent to the task registered with MessagePioTask().
 */
#define MESSAGE_PIO_CHANGED               (SYSTEM_MESSAGE_BASE_ + 35)

/*!
  A message from Kalimba has arrived. The message content is a
  #MessageFromKalimba.

  This message is sent to the task registered with MessageKalimbaTask().
 */
#define MESSAGE_FROM_KALIMBA              (SYSTEM_MESSAGE_BASE_ + 36)

/*!
  A requested ADC measurement has completed. The message content is
  a #MessageAdcResult.

  This message is sent to the task passed to AdcRequest().
 */
#define MESSAGE_ADC_RESULT                (SYSTEM_MESSAGE_BASE_ + 37)

/*!
  A stream has disconnected. The message content is a
  #MessageStreamDisconnect.

  This message is sent to any task associated with the stream using MessageSinkTask().
 */
#define MESSAGE_STREAM_DISCONNECT         (SYSTEM_MESSAGE_BASE_ + 38)

/*!
  The energy level in a SCO stream has changed. The message content is a
  #MessageEnergyChanged.

  This message is sent to any task associated with the stream using MessageSinkTask().

  It is enabled with EnergyEstimationSetBounds().
 */
#define MESSAGE_ENERGY_CHANGED            (SYSTEM_MESSAGE_BASE_ + 39)

/*!
  The values returned by StatusQuery() may have changed. (No message content.)

  This message is sent to the task registered with MessageStatusTask().
 */
#define MESSAGE_STATUS_CHANGED            (SYSTEM_MESSAGE_BASE_ + 40)

/*!
  A Source is empty. The message content is a #MessageSourceEmpty.

  This message is sent to any task associated with the Source using MessageSinkTask().
 */
#define MESSAGE_SOURCE_EMPTY              (SYSTEM_MESSAGE_BASE_ + 41)

/*!
  A long message from Kalimba has arrived. The message content is a
  #MessageFromKalimbaLong.

  This message is sent to the task registered with MessageKalimbaTask().
 */
#define MESSAGE_FROM_KALIMBA_LONG         (SYSTEM_MESSAGE_BASE_ + 42)

/*!
  BlueCore has enumerated as a USB device. The message content is a
  #MessageUsbConfigValue, telling you which configuration has been
  set by the host.

  This message is sent to the task registered with MessageSystemTask().
 */
#define MESSAGE_USB_ENUMERATED            (SYSTEM_MESSAGE_BASE_ + 43)

/*!
  BlueCore has suspended or resumed as a USB device; the message
  is a #MessageUsbSuspended and indicates which.

  This message is not sent if BlueCore is bus powered.

  Note that only the most recent change is reported, so adjacent
  messages may report the same status.

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_USB_SUSPENDED             (SYSTEM_MESSAGE_BASE_ + 44)

/*!
  The charger hardware has changed state.

  Note when user removes the charger, the firmware has already selected
  the battery as the power source, i.e.equivalent to VM executing
  PsuConfigure(PSU_VBAT_SWITCH, PSU_SMPS_INPUT_SEL_VBAT, 1).

  The message content is a #MessageChargerChanged.

  This message is sent to the task registered with MessageChargerTask().
*/
#define MESSAGE_CHARGER_CHANGED           (SYSTEM_MESSAGE_BASE_ + 45)

/*!
  A fault was detected in flash-based PS. (No message content.)

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_PSFL_FAULT                (SYSTEM_MESSAGE_BASE_ + 46) /*!< A fault was detected in flash-based PS */

/*!
  BlueCore has been deconfigured as a USB device. To maintain symmetry
  with the MESSAGE_USB_ENUMERATED case, this message also contains a
  #MessageUsbConfigValue, although in this case the config_value will
  always be zero.

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_USB_DECONFIGURED          (SYSTEM_MESSAGE_BASE_ + 47)

/*!
  BlueCore has changed to alternate settings for a USB interface. The
  message content is a #MessageUsbAltInterface.

  In order to receive MESSAGE_USB_ALT_INTERFACE, an application must first explicitly register
  an interest in this message by using StreamConfigure(#VM_STREAM_USB_ALT_IF_MSG_ENABLED, 1).

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_USB_ALT_INTERFACE         (SYSTEM_MESSAGE_BASE_ + 48)

/*!
  USB Vbus transition from low to high has been detected,
  indicating the chip is now attached. (No message content.)

  In order to receive MESSAGE_USB_ATTACHED or MESSAGE_USB_DETACHED, an application must first
  explicitly register an interest in these messages by using StreamConfigure(#VM_STREAM_USB_ATTACH_MSG_ENABLED, 1).

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_USB_ATTACHED        (SYSTEM_MESSAGE_BASE_ + 49)

/*!
  USB Vbus transition from high to low has been detected,
  indicating the chip is now detached. (No message content.)

  In order to receive MESSAGE_USB_ATTACHED or MESSAGE_USB_DETACHED, an application must first
  explicitly register an interest in these messages by using StreamConfigure(#VM_STREAM_USB_ATTACH_MSG_ENABLED, 1).

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_USB_DETACHED        (SYSTEM_MESSAGE_BASE_ + 50)

/*!
  Bluecore has detected a Kalimba watchdog event indicating
  that the DSP must be restarted. (No message content.)

  This message is sent to the task registered with MessageKalimbaTask().
*/
#define MESSAGE_KALIMBA_WATCHDOG_EVENT     (SYSTEM_MESSAGE_BASE_ + 51)

/*!
  Bluecore has changed Bluetooth transmission power. (No message
  content).

  In order to receive a MESSAGE_TX_POWER_CHANGE_EVENT an
  application must first explicitly register an interest in this
  message by using VmTransmitPowerMessagesEnable(TRUE).  This must be
  done each time the message is received to re-register interest.

  This message can apply to any BR/EDR link; it is the responsibility
  of the application to determine which links, if any, have actually
  changed (the application should be prepared to handle the case where
  there is no net change to the power on any links). Applications
  concerned to catch all power change events should reenable this
  message using VmTransmitPowerMessagesEnable() before checking the
  power on the links.

  If this message is explicitly disabled with
  VmTransmitPowerMessagesEnable(FALSE), it is possible for one last
  event to occur if it was already queued.  Applications concerned by
  this should cancel pending messages.

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_TX_POWER_CHANGE_EVENT     (SYSTEM_MESSAGE_BASE_ + 52)

/*!
   A Capacitive Sensor pad has changed. The message content is a
   MessageCapacitiveSensorChanged.
   This message is sent to the task registered with
   MessageCapacitiveSensorTask().
*/
#define MESSAGE_CAPACITIVE_SENSOR_CHANGED   (SYSTEM_MESSAGE_BASE_ + 53)

/*!
   A voltage measurement has been made on an XIO.  The message content
   is a MessageXioVoltageMeasured.
   This message is sent to the task registered with XioVoltageTask()
*/
#define MESSAGE_XIO_VOLTAGE_MEASURED   (SYSTEM_MESSAGE_BASE_ + 54)


/*!
   Result of set message digest for serial flash partition.
   The message content is a MessageStreamSetDigest.
   This message is sent to the task registered with MessageSinkTask()
*/
#define MESSAGE_STREAM_SET_DIGEST (SYSTEM_MESSAGE_BASE_ + 55)

/*!
   Verification of a file system written to serial flash has completes.
   The message content is a MessageStreamPartitionVerify.
   This message is sent to the task registered with MessageSinkTask()
*/
#define MESSAGE_STREAM_PARTITION_VERIFY (SYSTEM_MESSAGE_BASE_ + 56)

/*!
   Verification of the reformatting of the serial flash has completed.
   The message content is a MessageStreamReformatVerify.
   This message is sent to the task registered with MessageSinkTask()
*/
#define MESSAGE_STREAM_REFORMAT_VERIFY (SYSTEM_MESSAGE_BASE_ + 57)

/*!
   An infrared button has been pressed or released. The message content
   is a MessageInfraRedEvent.
   This message is sent to the task registered with MessageInfraRedTask()
*/
#define MESSAGE_INFRARED_EVENT (SYSTEM_MESSAGE_BASE_ + 58)

/*!
   Reports the status of DFU from SQIF operation to the task registered with
   MessageSystemTask(). The message content is a #MessageDFUFromSQifStatus
   
*/
#define MESSAGE_DFU_SQIF_STATUS (SYSTEM_MESSAGE_BASE_ + 59)

/*!
  A message from Operator has arrived. The message content is a
  #MessageFromOperator.

  This message is sent to the task registered with specific operator ID.
 */
#define MESSAGE_FROM_OPERATOR              (SYSTEM_MESSAGE_BASE_ + 60)

/*!
  Report the validation result of an executable FS. The message content is a
  MessageExeFsValidationResult.

*/
#define MESSAGE_EXE_FS_VALIDATION_STATUS (SYSTEM_MESSAGE_BASE_ + 61)

/*!
A message from Operator framework has arrived. The message content is a
#MessageFromOperatorFramework.

This message is sent to the task registered with MessageOperatorFrameworkTask().
*/
#define MESSAGE_FROM_OPERATOR_FRAMEWORK (SYSTEM_MESSAGE_BASE_ + 62)

/*!
  Report that the other image bank is erased in all QSPI devices.
  The message content is a #MessageImageUpgradeEraseStatus.

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_IMAGE_UPGRADE_ERASE_STATUS (SYSTEM_MESSAGE_BASE_ + 63)

/*!
  Message to P1 about charger type detected.
  The message content is a #MessageChargerDetected.

  This message is sent to the task registered with MessageChargerTask().
*/
#define MESSAGE_CHARGER_DETECTED        (SYSTEM_MESSAGE_BASE_ + 64)

/*!
  Message to P1 about charger status changing.
  The message content is a #MessageChargerStatus.

  This message is sent to the task registered with MessageChargerTask().

  There is a 100ms debounce delay before the message is sent to the task.
*/
#define MESSAGE_CHARGER_STATUS          (SYSTEM_MESSAGE_BASE_ + 65)

/*
  Message to P1 informing about emergency mode of charger being removed when chip
  is running from it.
  The message content is a #MessageEmergencyMode.

  This message is sent to the task registered with MessageChargerTask().

  This message is deprecated and should not be used. Instead
  MESSAGE_CHARGER_CHANGED is sent indicating "charger_connected" = "False".
*/
/* #define MESSAGE_CHARGER_EMERGENCY       (SYSTEM_MESSAGE_BASE_ + 66) */

/*!
  Report that audio pwm outputs has been muted forever as the external audio
  amplifier became too hot. Chip must be reset before using the audio pwm again.
  This message is sent to the task registered with MessageSystemTask().
  (No message content.)
*/
#define MESSAGE_AUDIO_PWM_MUTED_FOREVER (SYSTEM_MESSAGE_BASE_ + 67)

/*!
  Exited from Dead Battery Provision mode.

  Report when dead battery provision mode is exited (i.e.either battery has
  sufficient voltage to operate or timeout in dead battery provision mode.)

  This message is sent to the task registered with MessageChargerTask().
*/
#define MESSAGE_CHARGER_DBP_EXITED (SYSTEM_MESSAGE_BASE_ + 68)

/*!
  Entered to Dead Battery Provision mode. This happens when
  - USB bus is suspended and
  - Battery has insufficient voltage to operate and
  - USB is plugged to standard downstream port.

  This message is sent to the task registered with MessageChargerTask().
*/
#define MESSAGE_CHARGER_DBP_ENTERED (SYSTEM_MESSAGE_BASE_ + 69)

/*!
 * A message to report chip subsystem version information. This
 * message is sent to the task registered with MessageSystemTask().
 * The message content is a #MessagesSubsystemVersionInfo
 */
#define MESSAGE_SUBSYSTEM_VERSION_INFO (SYSTEM_MESSAGE_BASE_ + 70)

/*!
 * A message to reports chip subsystem event information. This
 * message is sent to the task registered with MessageSystemTask().
 * The message content is a #MessageSubsystemEventReport
 */
#define MESSAGE_SUBSYSTEM_EVENT_REPORT (SYSTEM_MESSAGE_BASE_ + 71)

/*!
  Report that the missing sections in an image update are now copied from
  the boot image bank to the new image bank in all QSPI devices.
  The message content is a #MessageImageUpgradeCopyStatus.

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_IMAGE_UPGRADE_COPY_STATUS (SYSTEM_MESSAGE_BASE_ + 72)

/*!
 * A message to report Bitserial transfer events. This
 * message is sent to the task registered with MessageBitserialTask().
 * The message content is a #MessageBitserialEvent
 */
#define MESSAGE_BITSERIAL_EVENT (SYSTEM_MESSAGE_BASE_ + 73)

/*!
  Message to P1 informing about weak battery.
  The message content is a #MessageWeakBattery.
  This message is sent to the task registered with MessageChargerTask().
*/
#define MESSAGE_WEAK_BATTERY  (SYSTEM_MESSAGE_BASE_ + 74)

/*!
  Report that the audio image is now updated in the Audio QSPI device during
  an image upgrade.
  The message content is a #MessageImageUpgradeAudioStatus.

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_IMAGE_UPGRADE_AUDIO_STATUS (SYSTEM_MESSAGE_BASE_ + 75)

/*!
  Report that the adding all image sections to the hash context is completed.
  The message content is a #MessageImageUpgradeHashAllSectionsUpdateStatus.

  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_IMAGE_UPGRADE_HASH_ALL_SECTIONS_UPDATE_STATUS (SYSTEM_MESSAGE_BASE_ + 76)

/*!
  Message to P1 informing about SD card being inserted.
  The message content is a #MessageSdMmcEvent.
  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_SD_MMC_INSERTED  (SYSTEM_MESSAGE_BASE_ + 77)

/*!
  Message to P1 informing about SD card being removed.
  The message content is a #MessageSdMmcEvent.
  This message is sent to the task registered with MessageSystemTask().
*/
#define MESSAGE_SD_MMC_REMOVED  (SYSTEM_MESSAGE_BASE_ + 78)

/*!
  #MESSAGE_MORE_DATA: Message type received when more data has arrived at a Source.
 */
typedef struct
{
    /** The source which has more data. */
    Source source;
} MessageMoreData;

/*!
  #MESSAGE_MORE_SPACE: Message type received when a Sink has more space.
*/
typedef struct
{
    Sink sink;    /*!< The sink which has more space. */
} MessageMoreSpace;

/*!
  #MESSAGE_PIO_CHANGED: Message type received when the PIO state has changed.
*/
typedef struct
{
    uint16 state;        /*!< The state of PIO lines 0..15 when this message was sent. */
    uint32 time;         /*!< The time at which that state was valid. */
    uint16 state16to31;  /*!< The state of PIO lines 16..31 when this messsage was sent. */
    uint16 bank;         /*!< The bank index of the PIO lines. */
} MessagePioChanged;

/*!
  #MESSAGE_ADC_RESULT: Message type received when an ADC reading has been made.
*/
typedef struct
{
    vm_adc_source_type adc_source; /*!< The ADC source for which the reading was made. */
    uint16 reading; /*!< The reading derived from the hardware. See adc.h for how to interpret this reading. */
} MessageAdcResult;

/*!
  #MESSAGE_STREAM_DISCONNECT: Message type received when a Stream disconnects.
*/
typedef struct
{
    Source source; /*<! The source which was involved in the connection. */
    Sink sink;     /*<! The sink which was involved in the connection. */
} MessageStreamDisconnect;

/*!
  #MESSAGE_ENERGY_CHANGED: Message type received when a SCO energy level has passed the configured threshold.
*/
typedef struct
{
    Sink sco_sink; /*<! The SCO sink on which the estimated energy level has passed the configured threshold. */
    bool over_threshold; /*<! TRUE if the threshold was exceeded, else FALSE. */
} MessageEnergyChanged;

/*!
  #MESSAGE_SOURCE_EMPTY: Message type received when a Source is empty.
*/
typedef struct
{
    Source source; /*!< The source which is empty. */
} MessageSourceEmpty;

/*!
  #MESSAGE_FROM_KALIMBA: Message type received from Kalimba.
*/
typedef struct
{
    uint16 id;      /*!< The message id sent by Kalimba */
    uint16 data[4]; /*!< The message payload */
} MessageFromKalimba;

/*!
  #MESSAGE_FROM_KALIMBA_LONG: Long message type received from Kalimba.
*/
typedef struct
{
    uint16 id;      /*!< The message id sent by Kalimba */
    uint16 len;     /*!< The length of the message payload */
    uint16 data[1]; /*!< len words of message payload */
} MessageFromKalimbaLong;

/*!
  Message type received either when the host has set a configuration for the device,
  or when the device is deconfigured. In the latter case, config_value will always
  be zero.
*/
typedef struct
{
    uint16 config_value; /*!< bConfigurationValue for the selected configuration */
} MessageUsbConfigValue;

/*!
  Message type received when USB suspend state has changed.
*/
typedef struct
{
    bool has_suspended; /*!< TRUE if we have suspended, FALSE if we have unsuspended */
} MessageUsbSuspended;

/*!
  Message type received when an alternative setting is applied on a USB interface
*/
typedef struct
{
    uint16 interface;
    uint16 altsetting;
} MessageUsbAltInterface;

/*!
  Message type received when battery charger or VREG_EN pin states have changed.
*/
typedef struct
{
    bool charger_connected; /*!< TRUE if the charger is connected, FALSE if it is not */
    bool vreg_en_high;      /*!< TRUE if VREG_EN is high, FALSE if it is not */
} MessageChargerChanged;

/*!
  Message type received when charger is detected.
*/
typedef struct
{
    usb_attached_status attached_status; /*<! The charger type detected. */
    uint16 charger_dp_millivolts; /*<! The voltage (in mV) measured on D+ of a proprietary charger. */
    uint16 charger_dm_millivolts; /*<! The voltage (in mV) measured on D- of a proprietary charger. */
    usb_type_c_advertisement cc_status; /*<! USB Type-C current advertisements on CC line */
} MessageChargerDetected;

/*
  Message type received when charger
  disconnect emergency event i.e. a charger getting unplugged while the
  chip was running from the charger.

  This message is deprecated and should not be used. Instead
  MESSAGE_CHARGER_CHANGED is sent indicating "charger_connected" = "False".
*/

/*
typedef struct
{
    bool emergency_mode; / always TRUE since this message means we have emergency mode on /
} MessageEmergencyMode;
*/

/*!
  Message type received when charger status changes.
*/
typedef struct
{
    charger_status chg_status; /*<! Unswitched status from charger. */
} MessageChargerStatus;

/*!
  Message type received when an event is registered on the capacitive touch sensors
*/
typedef struct
{
    uint16  num_events;               /*!< number of events described in this message */
    capacitive_sensor_event event[1]; /*!< array of events (length of num_events) */
} MessageCapacitiveSensorChanged;

/*!
  #MESSAGE_ADC_RESULT: Message type received when an ADC reading has been made.
*/
typedef struct
{
    int16 reading; /*!< The reading derived from the hardware. */
} MessageXioVoltageMeasured;

typedef struct
{
    partition_filesystem_devices device; /*!< file system device written to */
    uint16 partition;                    /*!< partition written to */
    partition_set_digest_result set_digest_result;  /*!< result of set digest, see #partition_set_digest_result */
} MessageStreamSetDigest;

typedef struct
{
    partition_filesystem_devices device; /*!< file system device written to */
    uint16 partition;                    /*!< partition written to */
    partition_verify_result verify_result;  /*!< result of verification, see #partition_verify_result */
} MessageStreamPartitionVerify;

typedef struct
{
    partition_filesystem_devices device; /*!< file system device written to */
    partition_verify_result verify_result;  /*!< result of verification, see #partition_verify_result */
} MessageStreamReformatVerify;

/*!
  Message type received when DFU from SQIF process is complete.
*/
typedef struct
{
    vm_dfu_sqif_status status; /*!<  result of  DFU from SQIF process, see #vm_dfu_sqif_status*/
} MessageDFUFromSQifStatus;

/*!
  Message type received when a infra-red event has occurred
*/
typedef struct
{
    infrared_event    event;            /*!< Event type */
    infrared_protocol protocol;         /*!< Infra-red protocol */
    uint32 time;                        /*!< Time in milliseconds event occurred */
    uint16 address;                     /*!< Protocol specific device address */
    uint16 size_data;                   /*!< Size of data[] field */
    uint8 data[1];                      /*!< Protocol specific event data */
} MessageInfraRedEvent;

typedef struct
{
    uint16 len;                      /*!< The length of the message payload */
    uint16 message[1];                      /*!< Unsolicited message from operator */
} MessageFromOperator;

typedef struct {
    uint16 key; /*!< The key corresponding to the type of message */
    uint16 len; /*!< The length of the message payload */
    uint16 message[1];  /*!< Unsolicited message from operator framework */
} MessageFromOperatorFramework;

/*!
  Message type received when image upgrade erase is completed.
*/
typedef struct
{
    bool erase_status; /*!< TRUE if image erase is successful, else FALSE */
} MessageImageUpgradeEraseStatus;

/*!
  Message type received when image upgrade copy is completed.
*/
typedef struct
{
    bool copy_status; /*!< TRUE if image copy is successful, else FALSE */
} MessageImageUpgradeCopyStatus;

/*!
  Message type received when image upgrade audio is completed.
*/
typedef struct
{
    bool audio_status; /*!< TRUE if upgrade of audio is successful,else FALSE */
} MessageImageUpgradeAudioStatus;

/*!
  Message type received when all image sections are added to the hash context.
*/
typedef struct
{
    bool status; /*!< TRUE if hash update is successful,else FALSE */
} MessageImageUpgradeHashAllSectionsUpdateStatus;

typedef struct
{
    uint16          ss_id;                /*!< Subsystem id */
    uint16          hw_version;           /*!< Hardware version */
    uint32          fw_rom_version;       /*!< Firmware rom version */
    uint32          patched_fw_version;   /*!< Firmware patch version */
} MessageSubsystemVersionInfo;

typedef struct
{
    uint16                  ss_id;      /*!< Subsystem id */
    subsystem_event_level   level;      /*!< Event level */
    uint16                     id;      /*!< Event ID */
    uint16                    cpu;      /*!< cpu on which event occurred */
    uint16            occurrences;      /*!< number of occurrences of the event */
    uint32                   time;      /*!< System's microsecond clock time
                                             when the first event occurred */
    uint16                 arg[1];      /*!< Event specific data */
} MessageSubsystemEventReport;

/*!
  Message type received when Bitserial transfer is complete.
*/
typedef struct
{
    bitserial_result result; /*<! Result of Bitserial transfer. */
    bitserial_transfer_handle transfer_handle; /*<! Handle of Bitserial transfer. */
} MessageBitserialEvent;

/*!
  Message type received when weak battery is detected
*/
typedef struct
{
    pmu_power_source smps_source; /*<! Battery OR external supply for SMPS power source */
    pmu_power_source bypass_ldo_source; /*<! Battery OR external supply for BYPASS 
                                             LDO power source */
    bool charger_connected; /* <! Indicates whether a charger (VCHG supply) 
                                             is connected or not.*/
} MessageWeakBattery;

/*!
  Message type received when SD card insertion/removal event is detected
*/
typedef struct
{
    uint8 slot_id; /*<! Numerical ID for the SD slot. */
} MessageSdMmcEvent;

#endif
