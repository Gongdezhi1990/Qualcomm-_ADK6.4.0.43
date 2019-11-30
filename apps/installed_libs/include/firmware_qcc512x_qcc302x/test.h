#ifndef __TEST_H__
#define __TEST_H__

/*! file  @brief Functions to enter BlueCore test modes */
#define FIRST_PCM_IF 0

#if TRAPSET_TEST

/**
 *  \brief Internal function. Don't call this function directly. Don't rely on this being
 *  present.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_test
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool TestPerform_(uint16 len, const uint16 * args);
#endif /* TRAPSET_TEST */

/**
 *  \brief Radio test pause.
 *  \return TRUE on success, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
bool TestPause(void );

/**
 *  \brief Radiotest TX start. 
 *  \param lo_freq The transmission frequency in MHz. 
 *  \param level The transmit power level to use. 
 *  \param mod_freq The modulation offset. Not used. 
 *  \return TRUE on success, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
bool TestTxStart(uint16 lo_freq, uint16 level, uint16 mod_freq);

/**
 *  \brief Radiotest TX data 1. 
 *  \param lo_freq The transmission frequency in MHz. 
 *  \param level The transmit power level to use. See the documentation for TestTxStart(). 
 *  \return TRUE on success, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
bool TestTxData1(uint16 lo_freq, uint16 level);

/**
 *  \brief Radiotest TX data 2. 
 *  \param cc The country code to use. Must be 0. 
 *  \param level The transmit power level to use. See the documentation for TestTxStart(). 
 *  \return TRUE on success, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
bool TestTxData2(uint16 cc, uint16 level);

/**
 *  \brief Radiotest RX start. 
 *  \param lo_freq Receive frequency in MHz. 
 *  \param highside Set to 0 for low-side modulation, 1 for high-side modulation. 
 *  \param attn The receiver attenuation to use. 
 *  \return TRUE on success, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
bool TestRxStart(uint16 lo_freq, uint16 highside, uint16 attn);

/**
 *  \brief Configure the radiotest packet type. 
 *  \param packet_type The packet type to use. 
 *  \param packet_size The packet size to use (in bytes), excluding FEC or CRC. 
 *  \return TRUE on success, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
bool TestCfgPkt(uint16 packet_type, uint16 packet_size);

/**
 *  \brief Set radio test transmit power. 
 *  \param power_dbm The power in dBm corresponding to an entry in the BT radio power table to use
 *  for subsequent tests. 
 *  \return TRUE on success, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
bool TestTxPower(uint16 power_dbm);

/**
 *  \brief Radio test deep sleep.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool TestDeepSleep(void );

/**
 *  \brief Configure hardware loopback for PCM port. 
 *  \param pcm_mode  Chosen loopback mode. Valid values: 0, 1, 2
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool TestPcmLb(uint16 pcm_mode);

/**
 *  \brief Radiotest loop back. 
 *  \param lo_freq The TX/RX frequency in MHz. 
 *  \param level The transmit level to use. See TestTxStart() for details. 
 *  \return TRUE on success, else FALSE. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 */
bool TestLoopback(uint16 lo_freq, uint16 level);

/**
 *  \brief Configure external hardware loopback for PCM port. A block of random data is
 *  written to the PCM output port and is read back again on the PCM input port. 
 *  \param pcm_mode  Chosen loopback mode. Valid values: 0, 1, 2
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool TestPcmExtLb(uint16 pcm_mode);

/**
 *  \brief Radio test for configuring the crystal trim value. 
 *  \param xtal_ftrim Selected crystal trim value.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool TestCfgXtalFtrim(uint16 xtal_ftrim);

/**
 *  \brief Play a constant tone on the PCM port (or the codec for BC02 with
 *  PSKEY_HOSTIO_MAP_SCO_CODEC set). 
 *  \param freq  Chosen frequency. 
 *  \param ampl  Chosen amplitude. 
 *  \param dc  Specifies a constant offset to add to the audio data.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool TestPcmTone(uint16 freq, uint16 ampl, uint16 dc);

/**
 *  \brief Turn on codec hardware for stereo loopback 
 *  \param samp_rate  Sampling rate. Valid values: 8000, 11025, 16000, 22050, 24000, 32000 and 44100 
 *  \param reroute_optn  Routing option. Valid values: 0, 1, 2, 3
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool TestCodecStereoLb(uint16 samp_rate, uint16 reroute_optn);

/**
 *  \brief Play a constant tone on the PCM port (or the codec for BC02 with
 *  PSKEY_HOSTIO_MAP_SCO_CODEC set). 
 *  \param freq  Chosen frequency. Valid range: 0 (low) to 3 (high). 
 *  \param ampl  Chosen amplitude. Valid range: 0 (minimum) to 8 (maximum). 
 *  \param dc  Specifies a constant offset to add to the audio data. 
 *  \param interface Chosen PCM interface. A value from the \#audio_instance enumeration. 
 *  \return TRUE if successful, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool TestPcmToneIf(uint16 freq, uint16 ampl, uint16 dc, uint16 interface);

/**
 *  \brief Configure hardware loopback for PCM port. 
 *  \param pcm_mode  Chosen mode. Valid values: 0 (slave), 1 (master), 2 (Manchester slave) 
 *  \param interface Chosen PCM interface. A value from the \#audio_instance enumeration. 
 *  \return TRUE if successful, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool TestPcmLbIf(uint16 pcm_mode, uint16 interface);

/**
 *  \brief Configure external hardware loopback for PCM port. A block of random data is
 *  written to the PCM output port and is read back again on the PCM input port. 
 *  \param pcm_mode  Chosen mode. Valid values: 0 (slave), 1 (master), 2 (Manchester slave) 
 *  \param interface Chosen PCM interface. A value from the \#audio_instance enumeration. 
 *  \return TRUE if successful, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool TestPcmExtLbIf(uint16 pcm_mode, uint16 interface);
#endif
