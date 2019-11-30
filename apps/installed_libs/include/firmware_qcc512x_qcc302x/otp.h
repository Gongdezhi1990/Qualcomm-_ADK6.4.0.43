#ifndef __OTP_H__
#define __OTP_H__

/*! file  @brief Provides access to One Time Programmable (OTP) Memory.
** 
** 
Some chips have One Time Programmable (OTP) memory. OTP may 
contain some data which might be needed to read by VM Application.
**
** 
*/

#if TRAPSET_OTP

/**
 *  \brief Allows to read the contents of OTP memory. 
 *    
 *   
 *   Note that this trap supports reading only 128 bits of OTP memory accessible
 *  to customers in ADK6 firmware. 
 *  \param read_value buffer for storing the data read from OTP. 
 *  \param start_word offset in words where to start reading from. 
 *  \param length_words length of data to be read in words. 
 *  \return TRUE if successful, else FALSE. This function allows the VM Application to read
 *  the content of the OTP memory. This function assumes the first word written on
 *  the OTP memory starts from eFuse 0 (zero). Application should provide the
 *  starting location in words starting from word 0 in OTP memory and should
 *  provide the number of words to be read from the starting offset. After the
 *  completion of the API call, the supplied buffer will contain the read data in
 *  following order as shown in example. If the word offset is 12 and 3 words need
 *  to be read then the buffer[0] will contain data from offset 12, and buffer[2]
 *  will contain data from offset 14. Note: It will return FALSE if the total
 *  number of words to be read from starting location exceeds the size of OTP or
 *  the supplied location is more than the size of OTP. OTP size can be different
 *  on different chips. The maximum possible offset will be OTP size -1 as the 1st
 *  word is marked as offset zero. So for a 16 word OTP, the maximum offset will
 *  be 15 and the first word will be at offset 0.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_otp
 */
bool OtpReadData(uint16 * read_value, uint16 start_word, uint16 length_words);

/**
 *  \brief Allows to write into OTP memory. 
 *    
 *   This function allows the VM Application to write contents into 128 bits of
 *  customer accessible OTP memory. 
 *   This function assumes the first word written on the OTP memory starts from
 *  eFuse 0 (zero). 
 *   Application should provide the starting location in words starting from word
 *  0 in OTP memory 
 *   and should provide the number of words to be written from the starting
 *  offset. After the 
 *   completion of the API call, return value will specify if the write worked or
 *  not. 
 *   If the word offset is 2 and 3 words need to be written then the buffer[0]
 *  will be 
 *   written to offset 2 and buffer[2] at offset 4. 
 *   
 *   Note: It will return FALSE if the total number of words to write from
 *  starting 
 *   location exceeds the size of OTP or the supplied location is more than the
 *  size of OTP. 
 *   OTP size can be different on different chips. The maximum possible offset
 *  will be 
 *   OTP size -1 as the 1st word is marked as offset zero. So for a 8 word OTP, 
 *   the maximum offset will be 7 and the first word will be at offset 0.
 *   Note: If write is successful then chip needs to be reset before reading to
 *  confirm those values.
 *  \param write_value buffer for data to write to OTP. 
 *  \param start_word offset in words where to start write from. 
 *  \param length_words length of data to be written in words. 
 *  \return TRUE if successful, else FALSE.  
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_otp
 */
bool OtpWriteData(const uint16 * write_value, uint16 start_word, uint16 length_words);
#endif /* TRAPSET_OTP */
#endif
