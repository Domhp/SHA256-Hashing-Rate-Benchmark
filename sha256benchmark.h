/*********************************************************************
* Filename:   sha256benchmark.h
* Author:     Dominic Howard-Parker
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for benchmarking sha256 using bitcoin
              style header.
*********************************************************************/
#include <stdint.h>
#include "sha256.h"

#define HEADER_SIZE 80

/************************* DATA TYPES **************************/
typedef struct{
    int32_t version;
    BYTE prevHeader[32];
    BYTE merkleRoot[32];
    uint32_t time;
    uint32_t nBits;
    uint32_t nonce;
}HEADER;