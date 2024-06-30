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
 *  - Algorithm     = bit-by-bit
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
 */
#include "crc16-xmodem.h"     /* include the header file generated with pycrc */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



crc16_xmodem_t crc16_xmodem_update(crc16_xmodem_t crc, const void *data, size_t data_len)
{
    const unsigned char *d = (const unsigned char *)data;
    unsigned int i;
    bool bit;
    unsigned char c;

    while (data_len--) {
        c = *d++;
        for (i = 0; i < 8; i++) {
            bit = crc & 0x8000;
            crc = (crc << 1) | ((c >> (7 - i)) & 0x01);
            if (bit) {
                crc ^= 0x1021;
            }
        }
        crc &= 0xffff;
    }
    return crc & 0xffff;
}


crc16_xmodem_t crc16_xmodem_finalize(crc16_xmodem_t crc)
{
    unsigned int i;
    bool bit;

    for (i = 0; i < 16; i++) {
        bit = crc & 0x8000;
        crc <<= 1;
        if (bit) {
            crc ^= 0x1021;
        }
    }
    return crc & 0xffff;
}
