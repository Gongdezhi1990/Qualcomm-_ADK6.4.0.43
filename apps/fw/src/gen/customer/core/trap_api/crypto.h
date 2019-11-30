#ifndef __CRYPTO_H__
#define __CRYPTO_H__

/*! Encryption operation control flags */
#define CRYPTO_FLAGS_AES_BIG_ENDIAN_COUNTER     1u
        
#if TRAPSET_CRYPTO

/**
 *  \brief Encrypt or decrypt data using the AES128-CTR algorithm. Encrypt and decrypt are
 *  identical operations in this scheme.
 *  \param key Pointer to the 128-bit (16 byte) cryptographic key
 *  \param nonce Pointer to the 128-bit (16 byte) initial vector. This value is XORed with the
 *  counter.
 *  \param flags Provides control over how the cryptographic algorithm is run e.g. whether
 *  parameters are big or little endian. Use a value of 0 for the standard
 *  operation or CRYPTO_FLAGS_AES_BIG_ENDIAN_COUNTER for a counter value that is
 *  applied to the upper bits of the initial vector.
 *  \param counter Counter value to use for the first block of data. It will be incremented and
 *  applied to each block in turn.
 *  \param source_data Pointer to the data to feed into the algorithm
 *  \param source_data_len_bytes Length of the source data. Normally this will match the dest_data_len_bytes
 *  value. If the dest_data_len_bytes value is greater then the source data will
 *  not be padded. Instead the dest_data pointer will only be written with
 *  source_data_len_bytes bytes.
 *  \param dest_data Pointer to the memory where the output data from the algorithm is to be
 *  written. This can be the same pointer as source_data in which case the source
 *  data will be over-written with the output data (in-place encrypt/decrypt).
 *  \param dest_data_len_bytes Length of the memory available at the dest_data pointer. If the
 *  source_data_len_bytes value is less than this then the dest_data pointer will
 *  only be written with source_data_len_bytes.
 *  \return TRUE - Operation completed successfully, FALSE - Some input parameters were
 *  invalid.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_crypto
 */
bool CryptoAes128Ctr(const uint8 * key, const uint8 * nonce, uint16 flags, uint32 counter, const uint8 * source_data, uint16 source_data_len_bytes, uint8 * dest_data, uint16 dest_data_len_bytes);

/**
 *  \brief Encrypt, Decrypt or hash data using the AES128-CBC algorithm.
 *  \param encrypt TRUE for an encryption or FALSE for decryption
 *  \param key Pointer to the 128-bit (16 byte) cryptographic key
 *  \param nonce Pointer to the 128-bit (16 byte) number to be used as the initial vector. This
 *  is over-written with the final vector from the operation. This can be used as
 *  a hash of the source data or as the initial vector for a further (chained)
 *  operation.
 *  \param flags Reserved for future use to control aspects of the cryptographic operation. Use
 *  a value of 0 for the standard operation.
 *  \param source_data Pointer to the data to feed into the algorithm
 *  \param source_data_len_bytes Length of the source data. If this is not a multiple of the block size (16
 *  bytes) then it will be padded with zeros up to the next block (16 byte)
 *  boundary before being fed into the crypto algorithm.
 *  \param dest_data Pointer to the memory where the output data from the algorithm is to be
 *  written. This may be NULL if the operation is being used as a hash. In that
 *  case the hash result is found in the nonce location. The dest_data parameter
 *  can take the same value as source_data in which case the source data will be
 *  over-written with the output data.
 *  \param dest_data_len_bytes Length of the memory available at the dest_data pointer. Normally this will be
 *  either zero (for a hash operation) or the source_data_len_bytes rounded up to
 *  the next block (16 byte) boundary. If dest_data_len_bytes is not a multiple of
 *  the block size (16 bytes) then the output will be truncated to fit.
 *  \return TRUE - Operation completed successfully, FALSE - Some input parameters were
 *  invalid.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_crypto
 */
bool CryptoAes128Cbc(bool encrypt, const uint8 * key, uint8 * nonce, uint16 flags, const uint8 * source_data, uint16 source_data_len_bytes, uint8 * dest_data, uint16 dest_data_len_bytes);
#endif /* TRAPSET_CRYPTO */
#endif
