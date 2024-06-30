/**
 * \file
 * Functions and types for CRC checks.
 *
 * Generated on Tue May  7 19:26:44 2024
 * by pycrc v0.10.0, https://pycrc.org
 * using the configuration:
 *  - Width         = 16
 *  - Poly          = 0x1021
 *  - XorIn         = 0x0000
 *  - ReflectIn     = False
 *  - XorOut        = 0x0000
 *  - ReflectOut    = False
 *  - Algorithm     = table-driven
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file defines the functions crc16_xmodem_init(), crc16_xmodem_update() and crc16_xmodem_finalize().
 *
 * The crc16_xmodem_init() function returns the initial \c crc value and must be called
 * before the first call to crc16_xmodem_update().
 * Similarly, the crc16_xmodem_finalize() function must be called after the last call
 * to crc16_xmodem_update(), before the \c crc is being used.
 * is being used.
 *
 * The crc16_xmodem_update() function can be called any number of times (including zero
 * times) in between the crc16_xmodem_init() and crc16_xmodem_finalize() calls.
 *
 * This pseudo-code shows an example usage of the API:
 * \code{.c}
 * crc16_xmodem_t crc;
 * unsigned char data[MAX_DATA_LEN];
 * size_t data_len;
 *
 * crc = crc16_xmodem_init();
 * while ((data_len = read_data(data, MAX_DATA_LEN)) > 0) {
 *     crc = crc16_xmodem_update(crc, data, data_len);
 * }
 * crc = crc16_xmodem_finalize(crc);
 * \endcode
 */
#ifndef CRC16_XMODEM_H
#define CRC16_XMODEM_H

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * The definition of the used algorithm.
 *
 * This is not used anywhere in the generated code, but it may be used by the
 * application code to call algorithm-specific code, if desired.
 */
#define CRC_ALGO_TABLE_DRIVEN 1


/**
 * The type of the CRC values.
 *
 * This type must be big enough to contain at least 16 bits.
 */
typedef uint16_t crc16_xmodem_t;


/**
 * Calculate the initial crc value.
 *
 * \return     The initial crc value.
 */
static inline crc16_xmodem_t crc16_xmodem_init(void)
{
    return 0x0000;
}


/**
 * Update the crc value with new data.
 *
 * \param[in] crc      The current crc value.
 * \param[in] data     Pointer to a buffer of \a data_len bytes.
 * \param[in] data_len Number of bytes in the \a data buffer.
 * \return             The updated crc value.
 */
crc16_xmodem_t crc16_xmodem_update(crc16_xmodem_t crc, const void *data, size_t data_len);


/**
 * Calculate the final crc value.
 *
 * \param[in] crc  The current crc value.
 * \return     The final crc value.
 */
static inline crc16_xmodem_t crc16_xmodem_finalize(crc16_xmodem_t crc)
{
    return crc;
}


#ifdef __cplusplus
}           /* closing brace for extern "C" */
#endif

#endif      /* CRC16_XMODEM_H */
