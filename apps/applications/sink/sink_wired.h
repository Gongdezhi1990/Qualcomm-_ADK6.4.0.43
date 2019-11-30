/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#ifndef _SINK_WIRED_H_
#define _SINK_WIRED_H_

#include <stdlib.h>
#include <audio_plugin_if.h>

#include "sink_volume.h"
#include "sink_audio_routing.h"

#define WIRED_RATE       (48000)

/* Indicates wired audio sampling rates */
typedef enum
{
    NON_DEFINED ,
    WIRED_RATE_44100 ,
    WIRED_RATE_48000 ,
    WIRED_RATE_88200 ,
    WIRED_RATE_96000  
}WIRED_SAMPLING_RATE_T;

/****************************************************************************
NAME 
    wiredAudioInit
    
DESCRIPTION
    Set up wired audio PIOs and configuration
    
RETURNS
    void
*/ 
#ifdef ENABLE_WIRED
void wiredAudioInit(void);
#else
#define wiredAudioInit() ((void)(0))
#endif
    
    


#ifdef ENABLE_WIRED
bool analoguePopulateConnectParameters(audio_connect_parameters *connect_parameters);
#else
#define analoguePopulateConnectParameters(x) (FALSE)
#endif

#ifdef ENABLE_WIRED
bool spdifPopulateConnectParameters(audio_connect_parameters *connect_parameters);
#else
#define spdifPopulateConnectParameters(x) (FALSE)
#endif
        
#ifdef ENABLE_WIRED
bool i2sPopulateConnectParameters(audio_connect_parameters *connect_parameters);
#else
#define i2sPopulateConnectParameters(x) (FALSE)
#endif

/****************************************************************************
NAME 
    analogAudioSinkMatch
    
DESCRIPTION
    Compare sink to analog audio sink.
    
RETURNS
    TRUE if Sink matches, FALSE otherwise
*/ 
#ifdef ENABLE_WIRED
bool analogAudioSinkMatch(Sink sink);
#else
#define analogAudioSinkMatch(x) (FALSE)
#endif

/****************************************************************************
NAME 
    spdifAudioSinkMatch
    
DESCRIPTION
    Compare sink to spdif audio sink.
    
RETURNS
    TRUE if Sink matches, FALSE otherwise
*/ 
#ifdef ENABLE_WIRED
bool spdifAudioSinkMatch(Sink sink);
#else
#define spdifAudioSinkMatch(x) (FALSE)
#endif

/****************************************************************************
NAME 
    i2sAudioSinkMatch
    
DESCRIPTION
    Compare sink to i2s audio sink.
    
RETURNS
    TRUE if Sink matches, FALSE otherwise
*/ 
#ifdef ENABLE_WIRED
bool i2sAudioSinkMatch(Sink sink);
#else
#define i2sAudioSinkMatch(x) (FALSE)
#endif

/****************************************************************************
NAME 
    analogAudioConnected
    
DESCRIPTION
    Determine if wired audio input is connected
    
RETURNS
    TRUE if connected, otherwise FALSE
*/ 
#ifdef ENABLE_WIRED
bool analogAudioConnected(void);
#else
#define analogAudioConnected()   (FALSE)
#endif

/****************************************************************************
NAME 
    analogGetAudioSink
    
DESCRIPTION
    Check analog state and return sink if available
    
RETURNS
    Sink if available, otherwise NULL
*/ 
#ifdef ENABLE_WIRED
Sink analogGetAudioSink(void);
#else
#define analogGetAudioSink() ((Sink)(NULL))
#endif

/****************************************************************************
NAME 
    analogGetAudioRate
    
DESCRIPTION
    Obtains the current defined sample rate for wired audio
    
RETURNS
    None
*/ 
#ifdef ENABLE_WIRED
void analogGetAudioRate (uint32 *rate);
#else
#define analogGetAudioRate(x) (*(x) = 0)
#endif

/****************************************************************************
NAME 
    spdifAudioConnected
    
DESCRIPTION
    Determine if spdif audio input is connected
    
RETURNS
    TRUE if connected, otherwise FALSE
*/ 
#ifdef ENABLE_WIRED
bool spdifAudioConnected(void);
#else
#define spdifAudioConnected()   (FALSE)
#endif

/****************************************************************************
NAME
    i2sAudioConnected
    
DESCRIPTION
    Determine if i2s audio input is connected
    
RETURNS
    TRUE if connected, otherwise FALSE
*/
#ifdef ENABLE_WIRED
bool i2sAudioConnected(void);
#else
#define i2sAudioConnected()   (FALSE)
#endif

/****************************************************************************
NAME 
    analogAudioCanPowerOff
    
DESCRIPTION
    Determine if analog audio blocks power off
    
RETURNS
    TRUE if device can power off, otherwise FALSE
*/ 
#define analogAudioCanPowerOff() ((!analogAudioConnected()) || powerManagerIsVbatCritical())

/****************************************************************************
NAME 
    spdifAudioCanPowerOff
    
DESCRIPTION
    Determine if spdif audio blocks power off
    
RETURNS
    TRUE if device can power off, otherwise FALSE
*/ 
#define spdifAudioCanPowerOff() ((!spdifAudioConnected()) || powerManagerIsVbatCritical())

/****************************************************************************
NAME 
     SinkWiredReadI2SAndSpdifConfiguration
    
    DESCRIPTION
    Gets I2S and spdif pskey config and any associated I2S data packets
 
RETURNS
     
*/

#ifdef ENABLE_WIRED
void SinkWiredReadI2SAndSpdifConfiguration(void);
#else
#define SinkWiredReadI2SAndSpdifConfiguration() ((void)(0))
#endif

/****************************************************************************
NAME 
     sinkWiredConfigureI2sInput
    
    DESCRIPTION
    Sets the Audio Instance for I2S from configuration
 
RETURNS
     
*/
#ifdef ENABLE_WIRED
void sinkWiredConfigureI2sInput(void);
#else
#define sinkWiredConfigureI2sInput() ((void)(0))
#endif

/****************************************************************************
NAME 
     sinkWiredConfigureSpdifInput
    
    DESCRIPTION
    Sets the Audio Instance for SPDIF from configuration
 
RETURNS

*/
#ifdef ENABLE_WIRED
void sinkWiredConfigureSpdifInput(void);
#else
#define sinkWiredConfigureSpdifInput() ((void)(0))
#endif

/****************************************************************************
NAME 
     SinkWiredGetAudioConnectedPowerOffTimeout
    
DESCRIPTION
    Gets the Sink Wired Audio Connected PowerOff Timeout
 
RETURNS
     uint16

*/

#ifdef ENABLE_WIRED
uint16 SinkWiredGetAudioConnectedPowerOffTimeout(void);
#else
#define SinkWiredGetAudioConnectedPowerOffTimeout() (0)
#endif

/****************************************************************************
NAME 
     SinkWiredSetAudioConnectedPowerOffTimeout
    
DESCRIPTION
    Sets the Sink Wired Audio Connected PowerOff Timeout
 
PARAMS
     uint16
*/

#ifdef ENABLE_WIRED
bool SinkWiredSetAudioConnectedPowerOffTimeout(uint16 timeout);
#else
#define SinkWiredSetAudioConnectedPowerOffTimeout(timeout) (FALSE)
#endif

/****************************************************************************
NAME 
    SinkWiredGetAnalogueDisconnectDebouncePeriod

DESCRIPTION
    Gets the Analogue Audio Disconnection debounce period, the time between
    physical disconection detection and destruction of the audio stream
    
RETURNS
    Debounce period in milliseconds

*/
#ifdef ENABLE_WIRED
uint16 SinkWiredGetAnalogueDisconnectDebouncePeriod(void);
#else
#define SinkWiredGetAnalogueDisconnectDebouncePeriod() (0)
#endif

/****************************************************************************
NAME 
     SinkWiredGetSpdifAC3Settings
    
    DESCRIPTION
    Gets the Sink Wired SpdifAC3Settings
 
RETURNS
     spdif_ac3_configuration*

*/  
spdif_ac3_configuration* SinkWiredGetSpdifAC3Settings(void);

/****************************************************************************
NAME 
     SinkWiredGetSpdifConfig
    
    DESCRIPTION
    Gets the Sink Wired SpdifConfig
 
RETURNS
     spdif_configuration*

*/  
spdif_configuration* SinkWiredGetSpdifConfig(void);

/****************************************************************************
NAME 
     SinkWiredIsPowerOffOnWiredAudioConnected()
    
    DESCRIPTION
    Gets the Sink Wired PowerOff On WiredAudio Connected
 
RETURNS
     bool

*/ 
#ifdef ENABLE_WIRED
bool SinkWiredIsPowerOffOnWiredAudioConnected(void);
#else
#define SinkWiredIsPowerOffOnWiredAudioConnected() (FALSE)
#endif

/****************************************************************************
NAME 
     SinkWiredPlayUsbAndWiredInLimbo()
    
    DESCRIPTION
    Gets the Sink Wired Play Usb And Wired In Limbo
 
RETURNS
     bool

*/  

#ifdef ENABLE_WIRED
bool SinkWiredPlayUsbAndWiredInLimbo(void);
#else
#define SinkWiredPlayUsbAndWiredInLimbo() (FALSE)
#endif

/****************************************************************************
NAME 
     SinkWiredGetAudioInputRate()
    
    DESCRIPTION
    Gets the Sink Wired Audio Input Rate
 
RETURNS
     WIRED_SAMPLING_RATE_T

*/
#ifdef ENABLE_WIRED
WIRED_SAMPLING_RATE_T SinkWiredGetAudioInputRate(void);
#else
#define SinkWiredGetAudioInputRate() (0)
#endif

/****************************************************************************
NAME 
     SinkWiredGetAudioOutputRate()
    
    DESCRIPTION
    Gets the Sink Wired Audio Outpur Rate
 
RETURNS
     WIRED_SAMPLING_RATE_T

*/  
WIRED_SAMPLING_RATE_T SinkWiredGetAudioOutputRate(void);

/****************************************************************************
NAME 
     SinkWiredGetSpdifDetect()
    
    DESCRIPTION
    Gets the Sink Wired Audio Spdif Detect
 
RETURNS
     uint8

*/  
uint8 SinkWiredGetSpdifDetect(void);

/****************************************************************************
NAME 
     SinkWiredGetAnalogInput()
    
    DESCRIPTION
    Gets the Sink Wired Audio Analog Input
 
RETURNS
     uint8

*/  
uint8 SinkWiredGetAnalogInput(void);

/****************************************************************************
NAME 
     SinkWiredGetSpdifInput()
    
    DESCRIPTION
    Gets the Sink Wired Audio Spdif Input
 
RETURNS
     uint8

*/  
uint8 SinkWiredGetSpdifInput(void);

/****************************************************************************
NAME 
     SinkWiredGetI2sDetect()
    
    DESCRIPTION
    Gets the Sink Wired Audio I2s Detect
 
RETURNS
     uint8

*/  
uint8 SinkWiredGetI2sDetect(void);

/****************************************************************************
NAME	
     SinkWiredGetI2sPluginType()
    
    DESCRIPTION
    Gets the Sink Wired Audio I2s Plugin type (pskey configured or SSM2518)
 
RETURNS
     uint8 i2s_plugin_type

*/
uint8 SinkWiredGetI2sPluginType(void);

/****************************************************************************
NAME	
	SinkWiredGetSessionData
    
DESCRIPTION
    Get Sink Wired Session data from PS Key and update the Sink Wired Global Data
*/
#ifdef ENABLE_WIRED
void SinkWiredGetSessionData(void);
#else
#define SinkWiredGetSessionData(x) ((void)(0))
#endif

/****************************************************************************
NAME	
	SinkWiredSetSessionData

DESCRIPTION
    Get Sink Wired Global Data and update Sink Wired Session data in PS Key
*/
#ifdef ENABLE_WIRED
void SinkWiredSetSessionData(void);
#else 
#define SinkWiredSetSessionData(x) ((void)(0))
#endif

/*************************************************************************
NAME    
    SinkWiredGetAnalogVolume
    
DESCRIPTION
    Get the Analog volume.

RETURNS
    void
    
**************************************************************************/
#ifdef ENABLE_WIRED
void SinkWiredGetAnalogVolume(volume_info *analog_volume);
#else 
#define SinkWiredGetAnalogVolume(x) ((void)(x))
#endif

/*************************************************************************
NAME    
    SinkWiredSetAnalogVolume
    
DESCRIPTION
    Set the Analog volume.

RETURNS
    void
    
**************************************************************************/
#ifdef ENABLE_WIRED
void SinkWiredSetAnalogVolume(const volume_info analog_volume);
#else
#define SinkWiredSetAnalogVolume(x) ((void)(x))
#endif


/*************************************************************************
NAME    
    SinkWiredGetSpdifVolume
    
DESCRIPTION
    Get the Spdif volume.

RETURNS
    void
    
**************************************************************************/
#ifdef ENABLE_WIRED
void SinkWiredGetSpdifVolume(volume_info *spdif_volume);
#else
#define SinkWiredGetSpdifVolume(x) ((void)(x))
#endif

/*************************************************************************
NAME    
    SinkWiredSetSpdifVolume
    
DESCRIPTION
    Set the Spdif volume.

RETURNS
    void
    
**************************************************************************/
#ifdef ENABLE_WIRED
void SinkWiredSetSpdifVolume(const volume_info spdif_volume);
#else
#define SinkWiredSetSpdifVolume(x)  ((void)(x))
#endif

/*************************************************************************
NAME    
    SinkWiredGetI2sVolume

DESCRIPTION
    Get the I2S volume.

RETURNS
    void

**************************************************************************/
#ifdef ENABLE_WIRED
void SinkWiredGetI2sVolume(volume_info *spdif_volume);
#else
#define SinkWiredGetI2sVolume(x) ((void)(x))
#endif

/*************************************************************************
NAME
    SinkWiredSetI2sVolume

DESCRIPTION
    Set the I2S volume.

RETURNS
    void

**************************************************************************/
#ifdef ENABLE_WIRED
void SinkWiredSetI2sVolume(const volume_info spdif_volume);
#else
#define SinkWiredSetI2sVolume(x)  ((void)(x))
#endif

/*************************************************************************
NAME
    SinkWiredGetAnalogMainVolume
    
DESCRIPTION
    Get the Analog Main volume.

RETURNS
    int16
    
**************************************************************************/
#ifdef ENABLE_WIRED
int16 SinkWiredGetAnalogMainVolume(void);
#else
#define SinkWiredGetAnalogMainVolume() (0)
#endif

/*************************************************************************
NAME    
    SinkWiredGetSpdifMainVolume
    
DESCRIPTION
    Get the Spdif Main volume.

RETURNS
    int16
    
**************************************************************************/
#ifdef ENABLE_WIRED
int16 SinkWiredGetSpdifMainVolume(void);
#else
#define SinkWiredGetSpdifMainVolume() (0)
#endif

/*************************************************************************
NAME
    SinkWiredGetI2sMainVolume

DESCRIPTION
    Get the I2S Main volume.

RETURNS
    int16

**************************************************************************/
#ifdef ENABLE_WIRED
int16 SinkWiredGetI2sMainVolume(void);
#else
#define SinkWiredGetI2sMainVolume() (0)
#endif

/****************************************************************************
NAME 
     SinkWiredGetSpdifOutput()
    
    DESCRIPTION
    Gets the Sink Wired Audio Spdif Output
 
RETURNS
     uint8

*/  
uint8 SinkWiredGetSpdifOutput(void);

#ifdef ENABLE_WIRED
bool wiredAnalogAudioIsRouted(void);
#else
#define wiredAnalogAudioIsRouted() (FALSE)
#endif

#ifdef ENABLE_WIRED
bool wiredSpdifAudioIsRouted(void);
#else
#define wiredSpdifAudioIsRouted() (FALSE)
#endif

#ifdef ENABLE_WIRED
bool wiredI2SAudioIsRouted(void);
#else
#define wiredI2SAudioIsRouted() (FALSE)
#endif

#endif /* _SINK_WIRED_H_ */
