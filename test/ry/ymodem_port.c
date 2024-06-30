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
#include "ymodem_port.h"
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>

/* max file size supported in byte */
#define MAX_FILE_SIZE (1*1024*1024)

static int fd = -1;
static ssize_t remainingBytes;

char *ymodem_port_stpncpy(char *dst, const char *src, size_t sz)
{
    char * const dstGuard = dst + sz;
    char * ret;

    /* copy characters */
    while (dst < dstGuard && *src != '\0')
    {
        *dst++ = *src++;
    }
    ret = dst; /* save return value */
    /* fille with 0 any remaining part of the buffer */
    while (dst < dstGuard)
    {
        *dst++ = '\0';
    }
    return ret;
}


void *ymodem_port_memchr(const void *s, int c, size_t n)
{
    const uint8_t *p = s;
    const uint8_t *pGuard = p + n;

    for (; p < pGuard; p++)
    {
        if(((uint8_t)c) == *p)
        {
            return (void *)p;
        }
    }
    return NULL;
}

int ymodem_port_atoi (const char *nptr)
{
    int result = 0;    /* init the resuklt to 0 */
    int sign = 1;      /* sign handling */

    /* skip any initial space */
    while (isspace((int)*nptr))
    {
        nptr++;
    }

    /* check the sign */
    switch(*nptr)
    {
    case '-':
        sign = -1; /* set negative and Fall-through */
    case '+':
        nptr++; /* advance the pointer and Fall-through */
    default:
        break;
    }

    /* Processes each character until they are digits */
    while (isdigit((int)*nptr))
    {
        result = result * 10 + (*nptr - '0');  /* conversion */
        nptr++;
    }

    /* apply sign to result */
    return result * sign;
}


size_t ymodem_port_maxFileSize()
{
    return MAX_FILE_SIZE;
}

int32_t ymodem_port_ReceiveStart(const char * filename, ssize_t filesize)
{
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(-1 != fd)
    {
        remainingBytes = filesize;
        return 0;
    }
    remainingBytes = 0;
    return -1;
}

int32_t ymodem_port_ProcessData(const uint8_t *buffer, size_t buffSz)
{
    int written;
    uint32_t chunkDim;
    if(remainingBytes >= 0)
    {
        chunkDim = min2(remainingBytes, buffSz);
        if(0 == chunkDim)
        {
            return -1;
        }
    }
    else
    {
        chunkDim = buffSz;
    }
    written = write(fd, buffer,chunkDim);
    if(written == chunkDim)
    {
        if(remainingBytes >= 0)
        {
            remainingBytes -= chunkDim;
        }
        return 0;
    }
    return -1;
}

int32_t ymodem_port_ReceiveEnd(void)
{
    close(fd);
    fd = -1;
    remainingBytes = 0;
    return 0;
}

int ymodem_port_getByte(uint32_t tout)
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

void ymodem_port_putByte(uint8_t c)
{
    write(STDOUT_FILENO, &c, 1);
}