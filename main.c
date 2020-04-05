#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "library.h"

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("usage : %s [filename]\nabort...\n",argv[0]);
        abort();
    }
    WORD_t* K = malloc(sizeof(WORD_t) * 64);
    WORD_t* H = malloc(sizeof(WORD_t) * 8 );

    MainLoop(H, K, argv[1]);
    free(K);
    free(H);
    return 1;
}