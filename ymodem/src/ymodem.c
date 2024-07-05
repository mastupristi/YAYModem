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
#include "ymodem.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "crc16-xmodem.h"
#include "ymodem_port.h"
#include <sys/types.h> /* for ssize_t */

#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)
#define PACKET_TRAILER          (2)
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)

/* length of the file size field in the block 0 */
#ifndef YM_FILE_SIZE_LENGTH
#define YM_FILE_SIZE_LENGTH        (16)
#endif

#define SOH                     (0x01)  /* start of 128-byte data packet */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* end of transmission */
#define ACK                     (0x06)  /* acknowledge */
#define NAK                     (0x15)  /* negative acknowledge */
#define CAN                     (0x18)  /* two of these in succession aborts transfer */
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */



#define PKT_TIMEOUT_ms          (10000)
#define CHAR_TIMEOUT_ms         (1000)
#define MAX_RETRY               (5)

#define min(a, b)                                                                                                     \
    ({                                                                                                                 \
        typeof(a) _a = (a);                                                                                            \
        typeof(b) _b = (b);                                                                                            \
        _a < _b ? _a : _b;                                                                                             \
    })

typedef enum
{
    pktTYPE_timeout = -2,
    pktTYPE_brokenPkt = -1, /* this include crc errors, unknown characters, etc. */
    pktTYPE_data,
    pktTYPE_EOT,
    pktTYPE_ACK,
    pktTYPE_NAK,
    pktTYPE_CAN,
}pktTYPE_t;


struct ymodem_desc
{
    uint8_t data[PACKET_1K_SIZE]; /* buffer for blocks */
    char filename[YM_FILE_NAME_LENGTH]; /* buffer for filenames */
    ssize_t filesize; /* filesize */
    ssize_t bytesRecved; /* file bytes received */

    void *cbParam; /* parameter to pass to the callbacks */

    /* callbacks */
    ymodem_maxFileSize_t maxFileSize;
    ymodem_receiveStart_t receiveStart;
    ymodem_processData_t processData;
    ymodem_receiveEnd_t receiveEnd;
    ymodem_getByte_t getByte;
    ymodem_putByte_t putByte;
};

_Static_assert(sizeof(struct ymodem_desc) == sizeof(staticYmodem_t), "sizes of public and private structures must match");

static pktTYPE_t ymodem_receive_packet(ymodem_desc_t *ymHdl, size_t *pktLen, u_int8_t *seqNum)
{
    int c;

    /* wait first char */
    c = ymHdl->getByte(ymHdl->cbParam, PKT_TIMEOUT_ms);
    switch(c)
    {
    case -1: /* timeout or error */
        ymodem_log("timeout\n");
        return pktTYPE_timeout;
    case CAN:
        c = ymHdl->getByte(ymHdl->cbParam, CHAR_TIMEOUT_ms);
        if (CAN == c)
        {
            ymodem_log("Abort trom other\n");
            return pktTYPE_CAN;
        }
        return pktTYPE_brokenPkt;
    case SOH:
        *pktLen = PACKET_SIZE;
        break;
    case STX:
        *pktLen = PACKET_1K_SIZE;
        break;
    case EOT:
        ymodem_log("EOT\n");
        return pktTYPE_EOT;
    case ACK:
        ymodem_log("ACK\n");
        return pktTYPE_ACK;
    case NAK:
        ymodem_log("NAK\n");
        return pktTYPE_NAK;
    }

    /* get block number and its complement */
    uint8_t blk_n, blk_n_compl;
    c = ymHdl->getByte(ymHdl->cbParam, CHAR_TIMEOUT_ms);
    if(c < 0)
    {
        ymodem_log("broken 1\n");
        return pktTYPE_brokenPkt;
    }
    blk_n = c & 0xff;
    c = ymHdl->getByte(ymHdl->cbParam, CHAR_TIMEOUT_ms);
    if(c < 0)
    {
        ymodem_log("broken 2\n");
        return pktTYPE_brokenPkt;
    }
    blk_n_compl = c & 0xff;
    /* get data bytes and compute crc */
    crc16_xmodem_t computedCrc;
    computedCrc = crc16_xmodem_init();
    for(int i=0;i<*pktLen;i++)
    {
        c = ymHdl->getByte(ymHdl->cbParam, CHAR_TIMEOUT_ms);
        if(c < 0)
        {
            ymodem_log("broken 3\n");
            return pktTYPE_brokenPkt;
        }
        ymHdl->data[i] = (uint8_t)c;
        computedCrc = crc16_xmodem_update(computedCrc, &ymHdl->data[i], 1);
    }
    computedCrc = crc16_xmodem_finalize(computedCrc);

    /* get crc */
    uint16_t crc;
    c = ymHdl->getByte(ymHdl->cbParam, CHAR_TIMEOUT_ms);
    if(c < 0)
    {
        ymodem_log("broken 4\n");
        return pktTYPE_brokenPkt;
    }
    crc = ((uint8_t)c)<<8;
    c = ymHdl->getByte(ymHdl->cbParam, CHAR_TIMEOUT_ms);
    if(c < 0)
    {
        ymodem_log("broken 5\n");
        return pktTYPE_brokenPkt;
    }
    crc |= ((uint8_t)c)<<0;

    /* check block number with its complement */
    if( blk_n != (uint8_t)(~blk_n_compl))
    {
        ymodem_log("block number\n");
        return pktTYPE_brokenPkt;
    }
    *seqNum = blk_n;

    /* chaeck crc */
    if( crc != computedCrc)
    {
        ymodem_log("crc\n");
        return pktTYPE_brokenPkt;
    }
    ymodem_log("data (blk n. %hhu)\n", blk_n);
    return pktTYPE_data;
}

typedef enum
{
    blk0TYPE_Error = -1,
    blk0TYPE_OK,
    blk0TYPE_Empty,
}blk0TYPE_t;

static blk0TYPE_t ymodem_parse_block0(const uint8_t *data, size_t pktLen, char *filename, ssize_t *filesize)
{
    if(0 == data[0]) /* a null pathname should terminate trasmission */
    {
        return blk0TYPE_Empty;
    }
    char *dstPtr  = ymodem_port_stpncpy(filename, (const char *)data, YM_FILE_NAME_LENGTH);
    filename[YM_FILE_NAME_LENGTH-1] = 0; /* null termination, just in case */
    int idx = dstPtr -filename;
    uint8_t *fileSzPtr = ymodem_port_memchr(&data[idx], 0, pktLen-idx); /* at the moment fileSzPtr actually point to null termination char of the filename */
    if(NULL == fileSzPtr) /* it seems that filename is endless */
    {
        return blk0TYPE_Error;
    }
    fileSzPtr++; /* now fileSzPtr point to the first char of filesize */
    if(' ' == *fileSzPtr) /* in this case filesize is omitted */
    {
        *filesize = -1; /* file size is unknown */
        return blk0TYPE_OK;
    }
    if(!isdigit(*fileSzPtr)) /* the filesize field has to be decimal */
    {
        return blk0TYPE_Error;
    }
    *filesize = ymodem_port_atoi((const char *)fileSzPtr);
    return blk0TYPE_OK;
}

typedef enum
{
    fileRecv_Error = -1, /* this include timeout, crc errors, unknown characters, etc. */
    fileRecv_OK,
    fileRecv_EOT,
    fileRecv_Abort,
}fileRecv_t;

static fileRecv_t ymodem_receive_file(ymodem_desc_t *ymHdl)
{
    pktTYPE_t pktType;
    uint8_t blkNum;
    size_t pktLen;
    int retryCount = 0;
    size_t maxFileSize;

    /* request to start transmission */
    ymHdl->putByte(ymHdl->cbParam, CRC16);

    retryCount = 0;
    do
    {
        /* wait packet */
        pktType = ymodem_receive_packet(ymHdl, &pktLen, &blkNum);
        /* check packet */
        switch (pktType)
        {
        case pktTYPE_timeout: /* when timeout we have to resend 'C' */
            ymHdl->putByte(ymHdl->cbParam, CRC16);
            continue;
        case pktTYPE_brokenPkt:
        case pktTYPE_EOT:
        case pktTYPE_ACK:
        case pktTYPE_NAK: /* for unexpected char or broken packet we send NAK */
            ymHdl->putByte(ymHdl->cbParam, NAK);
            continue;
        case pktTYPE_CAN: /* If sender ask to stop transer we ACK and exit */
            ymHdl->putByte(ymHdl->cbParam, ACK);
            return fileRecv_Abort;
        case pktTYPE_data:
            break;
        }

        if(0 != blkNum) /* at this point we are waiting only packet 0 */
        {
            ymHdl->putByte(ymHdl->cbParam, NAK);
            continue;
        }
        break; /* when we are here we are sure that packet is valideted */
    }while(++retryCount < MAX_RETRY);
    if(retryCount >= MAX_RETRY) /* we hav retryed enough, we give up asking sender to abort transfer */
    {
        ymHdl->putByte(ymHdl->cbParam, CAN);
        ymHdl->putByte(ymHdl->cbParam, CAN);
        return fileRecv_Error;
    }
    blk0TYPE_t blk0Type;

    /* parse block 0 */
    blk0Type = ymodem_parse_block0(ymHdl->data, pktLen, ymHdl->filename, &ymHdl->filesize);
    ymHdl->bytesRecved = 0;

    switch(blk0Type)
    {
    case blk0TYPE_Error: /* we give up */
        ymHdl->putByte(ymHdl->cbParam, CAN);
        ymHdl->putByte(ymHdl->cbParam, CAN);
        return fileRecv_Error;
    case blk0TYPE_OK:
        ymHdl->putByte(ymHdl->cbParam, ACK);
        break;
    case blk0TYPE_Empty: /* empty block means end of transfer */
        ymHdl->putByte(ymHdl->cbParam, ACK);
        return fileRecv_EOT;
    }

    maxFileSize = ymHdl->maxFileSize(ymHdl->cbParam);

    if (ymHdl->filesize > maxFileSize) /* if the file if too long we give up */
    {
        ymHdl->putByte(ymHdl->cbParam, CAN);
        ymHdl->putByte(ymHdl->cbParam, CAN);
        return fileRecv_Error;
    }
    int32_t resStart;
    resStart = ymHdl->receiveStart(ymHdl->cbParam, ymHdl->filename);
    if (0 != resStart) /* error initialing transfer */
    {
        ymHdl->putByte(ymHdl->cbParam, CAN);
        ymHdl->putByte(ymHdl->cbParam, CAN);
        return fileRecv_Error;
    }
    fileRecv_t ret = fileRecv_Error;

    uint8_t expectedPacket = 1;
    /* request to continue transmission */
    ymHdl->putByte(ymHdl->cbParam, CRC16);
    while(1)
    {
        retryCount = 0;
        do
        {
            /* wait packet */
            pktType = ymodem_receive_packet(ymHdl, &pktLen, &blkNum);
            /* check packet */
            switch (pktType)
            {
            case pktTYPE_timeout:
            case pktTYPE_brokenPkt:
            case pktTYPE_ACK:
            case pktTYPE_NAK: /* for timeout or unexpected char or broken packet we send NAK */
                ymodem_log("send NAK due to pkType %d\n", pktType);
                ymHdl->putByte(ymHdl->cbParam, NAK);
                continue;
            case pktTYPE_EOT:
                ymHdl->putByte(ymHdl->cbParam, ACK);
                ret = fileRecv_OK;
                goto ymodem_receive_file_end;
            case pktTYPE_CAN: /* If sender ask to stop transer we ACK and exit */
                ymHdl->putByte(ymHdl->cbParam, ACK);
                ret = fileRecv_Abort;
                 goto ymodem_receive_file_end;
           case pktTYPE_data:
                break;
            }

            if(expectedPacket != blkNum) /* an out-of-sequence packet */
            {
                ymodem_log("out of sequence [exp %hhu, recv %hhu]\n", expectedPacket, blkNum);
                ymHdl->putByte(ymHdl->cbParam, NAK);
                continue;
            }
            break; /* when we are here we are sure that packet is valideted */
        }while(++retryCount < MAX_RETRY);

        if(retryCount >= MAX_RETRY)
        {
            ymHdl->putByte(ymHdl->cbParam, CAN);
            ymHdl->putByte(ymHdl->cbParam, CAN);
            ret = fileRecv_Error;
            goto ymodem_receive_file_end;
        }

        size_t actualDataSz;
        if(ymHdl->filesize < 0)
        {
            actualDataSz = pktLen;
        }
        else
        {
            actualDataSz = min(ymHdl->filesize - ymHdl->bytesRecved, pktLen);
        }

        int32_t resProcess;
        resProcess = ymHdl->processData(ymHdl->cbParam, ymHdl->data, actualDataSz);
        ymHdl->bytesRecved += actualDataSz;
        if (0 != resProcess) /* error initialing transfer */
        {
            ymHdl->putByte(ymHdl->cbParam, CAN);
            ymHdl->putByte(ymHdl->cbParam, CAN);
            ret = fileRecv_Error;
            goto ymodem_receive_file_end;
        }
        ymHdl->putByte(ymHdl->cbParam, ACK);
        expectedPacket++;
    }
ymodem_receive_file_end:
    ymHdl->receiveEnd(ymHdl->cbParam);
    return ret;
}

ymodem_desc_t *ymodem_init(staticYmodem_t *staticYmBuffer, void *cbParam, ymodem_maxFileSize_t maxFileSize, 
                            ymodem_receiveStart_t receiveStart, ymodem_processData_t processData,
                            ymodem_receiveEnd_t receiveEnd, ymodem_getByte_t getByte, ymodem_putByte_t putByte)
{
    if(NULL == staticYmBuffer)
    {
        return NULL;
    }

    ymodem_desc_t *ymHdl = (ymodem_desc_t *)staticYmBuffer;
    ymHdl->cbParam = cbParam;
    ymHdl->maxFileSize = maxFileSize;
    ymHdl->receiveStart = receiveStart;
    ymHdl->processData = processData;
    ymHdl->receiveEnd = receiveEnd;
    ymHdl->getByte = getByte;
    ymHdl->putByte = putByte;
    return ymHdl;
}

int ymodem_receive(ymodem_desc_t *ymHdl)
{
    fileRecv_t fileRes;
    do
    {
        fileRes = ymodem_receive_file(ymHdl);
    }while(fileRecv_OK == fileRes);

    return fileRecv_EOT == fileRes ? 0: 1;
}
