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

char *ymodem_port_stpncpy(char *dst, const char *src, size_t sz)
{
    return NULL;
}


void *ymodem_port_memchr(const void *s, int c, size_t n)
{
    return NULL;
}

int ymodem_port_atoi (const char *nptr)
{
    return 0;
}


size_t ymodem_port_maxFileSize()
{
    return 0;
}

int32_t ymodem_port_ReceiveStart(const char * filename, ssize_t filesize)
{
    return 0;
}

int32_t ymodem_port_ProcessData(const uint8_t *buffer, size_t buffSz)
{
    return 0;
}

int32_t ymodem_port_ReceiveEnd(void)
{
    return 0;
}

int ymodem_port_getByte(uint32_t tout)
{
    return -1;
}

void ymodem_port_putByte(uint8_t c)
{

}