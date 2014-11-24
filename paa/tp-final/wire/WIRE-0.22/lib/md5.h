
#ifndef MD5_H_INCLUDED
#define MD5_H_INCLUDED

#include <config.h>

// System libraries

#include <string>
#include <assert.h>

// Local libraries

#include "const.h"
#include "utils.h"

// Typedefs

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

typedef struct
{
    uint32 total[2];
    uint32 state[4];
    uint8 buffer[64];
}
md5_context;

// Functions

void md5_starts( md5_context *ctx );
void md5_update( md5_context *ctx, uint8 *input, uint32 length );
void md5_finish( md5_context *ctx, uint8 digest[16] );

void md5_string( char *input, unsigned int len, char *output );

#endif /* md5.h */
