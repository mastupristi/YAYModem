/*
 * Copyright 2024 Massimiliano Cialdi
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
#ifndef YMODEM_PORT_H
#define YMODEM_PORT_H

#include <stdint.h>
#include <stddef.h>    /* for size_t */
#include <sys/types.h> /* for ssize_t */


/**
 * @brief log function
 *
 */
#define ymodem_log(...)

/**
 * @brief min macro
 *
 */
#define max2(a, b)                                                                                                     \
    ({                                                                                                                 \
        typeof(a) _a = (a);                                                                                            \
        typeof(b) _b = (b);                                                                                            \
        _a > _b ? _a : _b;                                                                                             \
    })

/**
 * @brief implementation of stpncpy
 *
 * is left to the user to choose whether to use the library function or use the version implemented here
 * if you wante the library function you can use #define or use something like this:
 *
 * #include <string.h>
 * static inline char *ymodem_port_stpncpy(char *dst, const char *src, size_t sz)
 * {
 *      return stpncpy(dst, src, sz);
 * }
 *
 */
char *ymodem_port_stpncpy(char *dst, const char *src, size_t sz) __attribute__((nonnull (1, 2)));

/**
 * @brief implementation of memchr
 *
 * is left to the user to choose whether to use the library function or use the version implemented here
 * if you wante the library function you can use #define or use something like this:
 *
 * #include <string.h>
 * static inline void *ymodem_port_memchr(const void *s, int c, size_t n)
 * {
 *      return memchr(s, c, n);
 * }
 */
void *ymodem_port_memchr(const void *s, int c, size_t n) __attribute__((nonnull (1)));


/**
 * @brief implementation of atoi
 *
 * is left to the user to choose whether to use the library function or use the version implemented here
 * if you wante the library function you can use #define or use something like this:
 *
 * #include <stdlib.h>
 * static inline int ymodem_port_atoi (const char *nptr)
 * {
 *      return atoi(nptr);
 * }
 */
int ymodem_port_atoi(const char *nptr) __attribute__((nonnull (1)));

/**
 * @brief callback to get maximum file size supported
 *
 * @retval maximum file size supported
 */
size_t ymodem_port_maxFileSize();

/**
 * @brief callback function called when start receiving bytes of a file
 *
 * it is supposed to intializa storage structures (eg. open file)
 *
 * @param filename null terminated file name string
 * @param filesize expected filesize. If negative expected filesize is unknown
 * @return 0 on success
 */
int32_t ymodem_port_ReceiveStart(const char *filename, ssize_t filesize);

/**
 * @brief callback function called every data block received
 *
 * it is supposed to store bytes into storage
 *
 * @param buffer array containing bytes to process (store)
 * @param buffSz array size
 * @return 0 on success
 */
int32_t ymodem_port_ProcessData(const uint8_t *buffer, size_t buffSz);

/**
 * @brief callback function called when end receiving bytes of a file
 *
 * it is supposed to finalize storage structures (eg. close file)
 *
 * @return 0 on success
 */
int32_t ymodem_port_ReceiveEnd(void);

/**
 * @brief function returning byte received whithin timeout
 *
 * @param tout timeout in ms
 * @return the byte received as an unsigned char cast to an int or -1 on timeout
 */
int ymodem_port_getByte(uint32_t tout);

/**
 * @brief output the byte c
 *
 * @param c byte to outout
 */
void ymodem_port_putByte(uint8_t c);

#endif /* YMODEM_PORT_H */
