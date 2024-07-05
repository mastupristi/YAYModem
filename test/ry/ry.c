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
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ymodem.h"

/* max file size supported in byte */
#define MAX_FILE_SIZE (1*1024*1024)

typedef struct userParam
{
    int fd;
}userParam_t;


static staticYmodem_t staticYmBuff;

static userParam_t usrParam;

static size_t usr_maxFileSize()
{
    return MAX_FILE_SIZE;
}

static int32_t usr_ReceiveStart(userParam_t *param, const char * filename)
{
    param->fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(-1 != param->fd)
    {
        return 0;
    }
    return -1;
}

static int32_t usr_ProcessData(userParam_t *param, const uint8_t *buffer, size_t buffSz)
{
    int written;
    written = write(param->fd, buffer,buffSz);
    if(written == buffSz)
    {
        return 0;
    }
    return -1;
}

static int32_t usr_ReceiveEnd(userParam_t *param)
{
    close(param->fd);
    param->fd = -1;
    return 0;
}

static int usr_getByte(userParam_t *param, uint32_t tout)
{
    struct timeval tv;
    fd_set readfds;
    uint8_t charBuf;

    // Imposta il timeout
    tv.tv_sec = tout/1000;
    tv.tv_usec = (tout%1000)*1000;

    // Pulisci l'insieme dei file descriptors e aggiungi stdin
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    // Attendere fino a quando l'input non diventa disponibile o scade il timeout
    int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

    switch(ret)
    {
    case -1:
        perror("select()");
        return -1;
    case 0:
        // fprintf(stderr, "timeout\n");
        return -1;
    default:
        if (read(STDIN_FILENO, &charBuf, 1) < 0)
        {
            perror("read()");
            return -1;
        }
        // fprintf(stderr, "%c", charBuf);
        return charBuf;
    }
}

static void usr_putByte(userParam_t *param, uint8_t c)
{
    write(STDOUT_FILENO, &c, 1);
}


int main()
{
    int ret = 0;
    ymodem_desc_t *ymHdl;

    ymHdl = ymodem_init(&staticYmBuff, &usrParam,
            (ymodem_maxFileSize_t)usr_maxFileSize,
            (ymodem_receiveStart_t)usr_ReceiveStart,
            (ymodem_processData_t)usr_ProcessData,
            (ymodem_receiveEnd_t)usr_ReceiveEnd,
            (ymodem_getByte_t)usr_getByte,
            (ymodem_putByte_t)usr_putByte);
    ret = ymodem_receive(ymHdl);
    fprintf(stderr, "ret %d\n", ret);
    return 0;
}
