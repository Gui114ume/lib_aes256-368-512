#ifndef UNTITLED2_LIBRARY_H
#define UNTITLED2_LIBRARY_H

/*
 * Pense bête !! ( x % 2^n )  ==  x & ( 2^n - 1 )
 * 2^32 - 1 = 0xffffffff sur un unsigned int
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

typedef unsigned long long u_int64;
typedef unsigned int       u_int32;
typedef unsigned char      BYTE;
typedef unsigned int WORD_t;

typedef struct BLOCK
{
    WORD_t word[16];
} BLOCK_t;

static const WORD_t H1_0 = 0x6a09e667;
static const WORD_t H2_0 = 0xbb67ae85;
static const WORD_t H3_0 = 0x3c6ef372;
static const WORD_t H4_0 = 0xa54ff53a;
static const WORD_t H5_0 = 0x510e527f;
static const WORD_t H6_0 = 0x9b05688c;
static const WORD_t H7_0 = 0x1f83d9ab;
static const WORD_t H8_0 = 0x5be0cd19;

u_int32 MsgPadding(FILE* fptr,
                   u_int64 size);

u_int64 GetSize(char* filename);

//void Divide_M_InWord(BYTE* buffer,
//                     WORD_t* W);

// alloue W, puis calcule et rempli ce tableau W
void PreComputeW(BYTE* buffer,
                 WORD_t* W);

//
void BinToHexString(WORD_t* res_word,
                    char* hash);

void InitRegisters(WORD_t* registers, // un tableau de 8 WORD_t ( taille de H, intermédiate hash value)
                   WORD_t* H,
                   int i);

void SHA256_CompressionFunction(WORD_t* registers,
                                BYTE* buffer,
                                WORD_t* K);

void ComputeIntermediateHash(WORD_t* H,
                             WORD_t* registers);

void InitK(WORD_t* K);

WORD_t Sn(WORD_t x,
          u_int32 n);

WORD_t Rn(WORD_t x,
          u_int32 n);

WORD_t Ch(WORD_t x,
          WORD_t y,
          WORD_t z);

WORD_t Maj(WORD_t x,
           WORD_t y,
           WORD_t z);

WORD_t E0(WORD_t x);

WORD_t E1(WORD_t x);

WORD_t sig0(WORD_t x);

WORD_t sig1(WORD_t x);

void RemoveAddedBytes(FILE* fptr,
                      u_int64 size);

void MainLoop(WORD_t* H,
              WORD_t* K,
              char* filename);

#endif //UNTITLED2_LIBRARY_H
