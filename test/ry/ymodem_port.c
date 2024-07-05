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
#include <ctype.h>
#include <stddef.h>


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

