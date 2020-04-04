/*
 *
 * Implementation SHA 256 / 384 / 512
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>


#include "library.h"

WORD_t* glob_K = NULL;

void InitK(WORD_t* K)
{
    // K est supposé alloué à la bonne taille
    K[0] = 0x428a2f98;  K[1] = 0x71374491;  K[2] = 0xb5c0fbcf;  K[3] = 0xe9b5dba5;
    K[4] = 0x3956c25b;  K[5] = 0x59f111f1;  K[6] = 0x923f82a4;  K[7] = 0xab1c5ed5;
    K[8]  = 0xd807aa98; K[9]  = 0x12835b01; K[10] = 0x243185be; K[11] = 0x550c7dc3;
    K[12] = 0x72be5d74; K[13] = 0x80deb1fe; K[14] = 0x9bdc06a7; K[15] = 0xc19bf174;
    K[16] = 0xe49b69c1; K[17] = 0xefbe4786; K[18] = 0x0fc19dc6; K[19] = 0x240ca1cc;
    K[20] = 0x2de92c6f; K[21] = 0x4a7484aa; K[22] = 0x5cb0a9dc; K[23] = 0x76f988da;
    K[24] = 0x983e5152; K[25] = 0xa831c66d; K[26] = 0xb00327c8; K[27] = 0xbf597fc7;
    K[28] = 0xc6e00bf3; K[29] = 0xd5a79147; K[30] = 0x06ca6351; K[31] = 0x14292967;
    K[32] = 0x27b70a85; K[33] = 0x2e1b2138; K[34] = 0x4d2c6dfc; K[35] = 0x53380d13;
    K[36] = 0x650a7354; K[37] = 0x766a0abb; K[38] = 0x81c2c92e; K[39] = 0x92722c85;
    K[40] = 0xa2bfe8a1; K[41] = 0xa81a664b; K[42] = 0xc24b8b70; K[43] = 0xc76c51a3;
    K[44] = 0xd192e819; K[45] = 0xd6990624; K[46] = 0xf40e3585; K[47] = 0x106aa070;
    K[48] = 0x19a4c116; K[49] = 0x1e376c08; K[50] = 0x2748774c; K[51] = 0x34b0bcb5;
    K[52] = 0x391c0cb3; K[53] = 0x4ed8aa4a; K[54] = 0x5b9cca4f; K[55] = 0x682e6ff3;
    K[56] = 0x748f82ee; K[57] = 0x78a5636f; K[58] = 0x84c87814; K[59] = 0x8cc70208;
    K[60] = 0x90befffa; K[61] = 0xa4506ceb; K[62] = 0xbef9a3f7; K[63] = 0xc67178f2;
}

WORD_t Ch(WORD_t x,
          WORD_t y,
          WORD_t z)
{
    return (  ( x & y ) ^ ( ~x  & z ) );
}

WORD_t Maj(WORD_t x,
           WORD_t y,
           WORD_t z)
{
    return (  ( x & y ) ^ ( x & z ) ^ (y & z)  );
}

WORD_t E0(WORD_t x)
{
    return (Sn(x, 2) ^ Sn(x, 13) ^ Sn(x, 22));
}

WORD_t E1(WORD_t x)
{
    return (Sn(x, 6) ^ Sn(x, 11) ^ Sn(x, 25));
}

WORD_t sig0(WORD_t x)
{
    return (Sn(x,7) ^ Sn(x, 18) ^ Rn(x, 3) );
}

WORD_t sig1(WORD_t x)
{
    return (Sn(x, 17) ^ Sn(x, 19) ^ Rn(x, 10));
}

WORD_t Sn(WORD_t x, u_int32 n)
{
    if( (n > 32) || (n < 0) )
    {
        perror("circulat shift, mauvaise valeur de n"); abort();
    }

    return ( (word_x >> n) | (word_x << (32 - n)) );
}

WORD_t Rn(WORD_t x, u_int32 n)
{
    return ( x << n);
}

void RemoveAddedBytes(FILE* fptr,
                      u_int64 size)
{
    int fd = fileno(fptr);
    if(fd == -1)
    {
        perror("error fileno()\n");
        exit(-1);
    }
    int code = ftruncate(fd, size);
    //ftruncate echoue et renvoie -1
    if(code == -1)
    {
        perror("error ftruncate\n");
        exit(-1);
    }
}

u_int32 MsgPadding(FILE* fptr,
                   u_int64 size)
{
        //fptr est deja ouvert ici
        u_int64* r = malloc(sizeof(u_int64));
        *r = file_size * 8;

        //reflechir a tous les cas possibles lors du padding
        if( (file_size * 8)%512 == 0 )
        {
            BYTE* buffer = malloc(sizeof(BYTE) * 448 / 8);
            buffer[0] = 1 << 7;
            for(int i = 1 ; i < 448/8; i++)
                buffer[i] = 0;

            fwrite(buffer, 1, 448/8,fptr);
            for(int i = 0 ; i < 8 ; i++)
                fwrite( (char*)r + 7 - i, 1, 1, fptr);
            fclose(fptr);
            return 448/8;

        }
        else if ( (file_size * 8 )%512 >= 448 ) //chevauchement de bloc
        {
            //on padd avec 1, puis des 0 afin de finir le bloc de 512. Ensuite on crée un nouveau bloc de 512 avec des 0, puis on padd avec file_size en binaire.
            // a ecrire
            BYTE* buffer = malloc(sizeof(BYTE) * (512/8 - (file_size * 8)%512 / 8) );
            buffer[0] = 1 << 7;
            for(int i = 1 ; i < (512/8 -  (file_size * 8)%512 / 8 ); i++ )
                buffer[i] = 0;

            fwrite(buffer, 1, ( (512/8 - (file_size * 8)%512 / 8 )),fptr);

            BYTE* buff = malloc(sizeof(BYTE) * 448 / 8);
            buff[0] = 0;
            for(int i = 1 ; i < 448/8; i++)
                buff[i] = 0;

            fwrite(buff, 1, 448/8,fptr);
            for(int i = 0 ; i < 8 ; i++)
                fwrite( (char*)r + 7 - i, 1, 1, fptr);
            fclose(fptr);
            return 448/8;

        }
            //ajouter un bit à 1
            //ajouter des zeros jusqu'a ce qu'il ne reste plus que 64 bits pour faire un bloc de 512 bits complet
            //ajouter file_size en tant que u64
        else if ( (file_size * 8 )%512 < 448 )
        {
            //on rempli le bloc avec 0x80 puis on ecrit file_size
            BYTE* buffer = malloc(sizeof(BYTE) * (512/8 - (file_size * 8)%512 / 8) - 8);
            buffer[0] = 1 << 7;
            for(int i = 1 ; i < (512/8 -  (file_size * 8)%512 / 8 ) - 8 ; i++ )
                buffer[i] = 0;

            fwrite(buffer, 1, ( (512/8 - (file_size * 8)%512 / 8 ) - 8 ),fptr);
            for(int i = 0 ; i < 8 ; i++)
                fwrite( (char*)r + 7 - i, 1, 1, fptr);
            fclose(fptr);
            return 0;
        }
        return 0;
}


u_int64 GetSize(char* filename)
{
    //lstat pour avoir la taille en octet et en u_int64 IMPORTANT
    struct stat* stat_buf = malloc(sizeof(struct stat));
    stat(filename, stat_buf);
    return  (stat_buf->st_size);
}

// alloue W, puis calcule et rempli ce tableau  à partir d'un bloc M, contenu dans buffer
void PreComputeW(BYTE* buffer,
                 WORD_t* W)
{
    W = malloc(sizeof(WORD_t) * 64);
    for(int i = 0,j = 0; i < 16 * 4 ; j++)
    {
        W[j] = (unsigned int) (buffer[i] << 24) ^ (unsigned int) (buffer[i + 1] << 16) ^
               (unsigned int) (buffer[i + 2] << 8) ^ (unsigned int) (buffer[i + 3]);
        i += 4;
    }
    for(int i = 16 ; i < 64 ; i++)
    {
        W[i] = sig1( W[i - 2] ) & (2^32 - 1) + W[i - 7] & (2^32 - 1) + sig0( W[i - 15] ) & (2^32 - 1) + W[i - 16] & (2^32 - 1);
    }
}

// Ne pas faire la même fonction que dans sha1_lib, essayer de trouver mieux, sans bug
void BinToHexString(WORD_t* res_word,
                    char* hash)
{
    return (void)0;
}

void InitRegisters(WORD_t* registers, // un tableau de 8 WORD_t ( taille de H, intermédiate hash value)
                   WORD_t* H,
                   int i)
{
    if( i == 1)
    {
        registers[0] = H1_0; registers[1] = H2_0; registers[2] = H3_0; registers[3] = H4_0;
        registers[4] = H5_0; registers[5] = H6_0; registers[6] = H7_0; registers[7] = H8_0;

        H[0] = H1_0; H[1] = H2_0; H[2] = H3_0; H[3] = H4_0;
        H[4] = H5_0; H[5] = H6_0; H[6] = H7_0; H[7] = H8_0;
    }
    else
    {
        for(int i = 0 ; i < 8 ; i++)
            registers[i] = H[i];
    }
}

void SHA256_CompressionFunction(WORD_t* registers,
                                BYTE* buffer,
                                WORD_t* K)
{
    WORD_t* W = NULL;
    PreComputeW(buffer, W);
    for(int j = 0 ; j < 64 ; j++)
    {
        WORD_t T_1 = registers[7] & (2 ^ 32 - 1) + sig1(registers[4]) & (2 ^ 32 - 1) + Ch(registers[4], registers[5], registers[6]) & (2 ^ 32 - 1) + K[j] & (2^32 - 1) + W[j];
        WORD_t T_2 = E0( registers[0] ) & (2^32 - 1) + Maj(registers[0],registers[1],registers[2]) & (2^32 - 1);
        registers[7] = registers[6];
        registers[6] = registers[5];
        registers[5] = registers[4];
        registers[4] = registers[3] & (2^32 - 1) + T_1 & (2^32 - 1);
        registers[3] = registers[2];
        registers[2] = registers[1];
        registers[1] = registers[0];
        registers[0] = T_1 & (2^32 - 1) + T_2 & (2^32 - 1);
    }
    free(W);
}

void ComputeIntermediateHash(WORD_t* H,
                             WORD_t* registers)
{
    for(int i = 0 ; i < 8 ; i++)
    {
        H[i] = registers[i] & (2^32 - 1) + H[i] & (2^32 - 1);
    }
}

void MainLoop(WORD_t* H,
              WORD_t* K)
{
    unsigned int nb = 0;
    int i = 0;
    char* buffer = malloc(sizeof(BYTE) * 512 / 8);
    FILE* fptr_in = fopen(filename, "rb");
    InitK(K);
    WORD_t* registers = malloc(sizeof(WORD_t) * 8);
    while( nb = fread(buffer, sizeof(BYTE),512/8,fptr_in) )
    {
        InitRegisters(registers, H, i);
        SHA256_CompressionFunction(registers, buffer, K);
        ComputeIntermediateHash(H, registers);
        ++i;
    }
    char* hash = malloc(sizeof(char) * 32 * 2);//verifier la taille
    BinToHexString(H, hash);

    fclose(fptr_in);

    free(hash);
    free(registers);
    free(buffer);
    return (void)0;
}