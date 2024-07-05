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
#ifndef YMODEM_SRC_YMODEM_H
#define YMODEM_SRC_YMODEM_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief callback to get maximum file size supported
 *
 * @param param user parameter
 * @retval maximum file size supported
 */
typedef size_t (*ymodem_maxFileSize_t)(void *param);

/**
 * @brief callback function called when start receiving bytes of a file
 *
 * it is supposed to intializa storage structures (eg. open file)
 *
 * @param param user parameter
 * @param filename null terminated file name string
 * @return 0 on success
 */
typedef int32_t (*ymodem_receiveStart_t)(void *param, const char *filename);

/**
 * @brief callback function called every data block received
 *
 * it is supposed to store bytes into storage
 *
 * @param param user parameter
 * @param buffer array containing bytes to process (store)
 * @param buffSz array size
 * @return 0 on success
 */
typedef int32_t (*ymodem_processData_t)(void *param, const uint8_t *buffer, size_t buffSz);

/**
 * @brief callback function called when end receiving bytes of a file
 *
 * it is supposed to finalize storage structures (eg. close file)
 *
 * @param param user parameter
 * @return 0 on success
 */
typedef int32_t (*ymodem_receiveEnd_t)(void *param);

/**
 * @brief function returning byte received whithin timeout
 *
 * @param param user parameter
 * @param tout timeout in ms
 * @return the byte received as an unsigned char cast to an int or -1 on timeout
 */
typedef int (*ymodem_getByte_t)(void *param, uint32_t tout);

/**
 * @brief output the byte c
 *
 * @param param user parameter
 * @param c byte to outout
 */
typedef void (*ymodem_putByte_t)(void *param, uint8_t c);


typedef struct ymodem_desc ymodem_desc_t;

#ifndef YM_FILE_NAME_LENGTH
#define YM_FILE_NAME_LENGTH        (256)
#endif

#define ROUND_UP_MULTIPLE_OF_4(x) (((x) + 3) & ~3)
#define ROUND_UP_MULTIPLE_OF_8(x) (((x) + 7) & ~7)

/* sed struct dimension depending on platform */
#if UINTPTR_MAX == 0xFFFFFFFF
#define STATICBUFF_SZ 1060 + ROUND_UP_MULTIPLE_OF_4(YM_FILE_NAME_LENGTH) /* for 32-bit platforms */
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
#define STATICBUFF_SZ 1096 + ROUND_UP_MULTIPLE_OF_8(YM_FILE_NAME_LENGTH) /* for 64-bit platforms */
#else
#error "Unknown platform"
#endif

typedef struct staticYmodem
{
    uint8_t dummy[STATICBUFF_SZ];
}staticYmodem_t;

/**
 * @brief initialization function
 * 
 * @param staticYmBuffer buffer used to store internal structures
 * @param cbParam user parameter to be passed to callbacks
 * @param maxFileSize callback
 * @param receiveStart callback
 * @param processData callback
 * @param receiveEnd callback
 * @param getByte callback
 * @param putByte callback
 * @return pointer to ymodem handle, or NULL on error 
 */
ymodem_desc_t *ymodem_init(staticYmodem_t *staticYmBuffer, void *cbParam, ymodem_maxFileSize_t maxFileSize, ymodem_receiveStart_t receiveStart, ymodem_processData_t processData, ymodem_receiveEnd_t receiveEnd, ymodem_getByte_t getByte, ymodem_putByte_t putByte);

int ymodem_receive(ymodem_desc_t *ymHdl);

#endif /* YMODEM_SRC_YMODEM_H */
