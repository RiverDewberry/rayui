#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "./mkimg.h"

/*SOURCES:
 * https://www.libpng.org/pub/png/spec/1.2/PNG-CRCAppendix.html
 * https://www.rfc-editor.org/rfc/rfc1950
 */

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
    uint32_t c;
    int n, k;

    for (n = 0; n < 256; n++) {
        c = (uint32_t) n;
        for (k = 0; k < 8; k++) {
            if (c & 1)
                c = 0xEDB88320 ^ (c >> 1);
            else
                c = c >> 1;
            }
        crc_table[n] = c;
    }
    crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
should be initialized to all 1's, and the transmitted value
is the 1's complement of the final running CRC (see the
crc() routine below)). */

unsigned long update_crc(
    unsigned long crc, unsigned char *buf, int len
)
{
    unsigned long c = crc;
    int n;

    if (!crc_table_computed)
        make_crc_table();
    for (n = 0; n < len; n++) {
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len)
{
    uint8_t temp[4];
    *(uint32_t*)temp = update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
    return *((uint32_t*)((uint8_t[]){temp[3], temp[2], temp[1], temp[0]}));
}

uint32_t adler32(uint8_t *buf, int len)
{
        uint32_t adler = 1;
        uint32_t s1 = adler & 0xffff;
        uint32_t s2 = (adler >> 16) & 0xffff;
        int n;

        for (n = 0; n < len; n++) {
          s1 = (s1 + buf[n]) % 65521;
          s2 = (s2 + s1)     % 65521;
        }
        return (s2 << 16) + s1;
}

/*END OF EXTERNAL CODE*/

uint8_t png_signature[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

void writeBigEndian(void *src, void *dest, size_t size)
{
    for(int i = 0; i < size; i++)
        ((char*) dest)[i] = ((char*) src)[size - i - 1];
}

void writeChunk(
    uint32_t len,
    uint8_t *type,
    uint8_t *data,
    FILE *fptr
)
{
    uint32_t bigEndianLen;

    writeBigEndian(&len, &bigEndianLen, 4);

    fwrite(&bigEndianLen, 4, 1, fptr);
    fwrite(type, 1, 4, fptr); 
    fwrite(data, 1, len, fptr);

    uint8_t *crc_input = (uint8_t*) malloc(4 + len);

    *((uint32_t*)crc_input) = *((uint32_t*)(type));

    for(int i = 0; i < len; i++)
        crc_input[i + 4] = data[i];

    unsigned long chunk_crc = crc(crc_input, len + 4);

    fwrite(&chunk_crc, 4, 1, fptr);

    free(crc_input);
}

void findRepeat(
    uint16_t *offset,
    uint16_t *len,
    uint8_t *window,
    uint8_t *input,
    uint32_t windowLen,
    uint32_t inputLen
)
{
    int bestIndex = 0;
    int bestLen = 0;
    *len = 1;
    *offset = 0;

    return;

    if(windowLen == 0)return;

    for(int i = 0; i < windowLen; i++)
    {
        int len = 0;

        for(int j = i; j < inputLen; j++)
        {
            if(input[j - i] == window[j])
                len++;
            else break;
        }

        if(len > bestLen)
        {
            bestIndex = i;
            bestLen = len;
        }
    }

    *len = bestLen + (bestLen == 0);
    *offset = (windowLen - bestIndex) * (bestLen != 0);

    if(*len < 3)*len = 1, *offset = 0;

    //printf("REPEAT %d\n", *len);
}

void getLenCode(
    uint16_t len,
    uint16_t *code,
    uint8_t *bits,
    uint16_t *bitVals
)
{
    uint16_t minLen[] = {
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51,
        59, 67, 83, 99, 115, 131, 163, 195, 227, 258
    };

    uint16_t maxLen[] = {
        3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 22, 26, 30, 34, 42, 50, 58,
        66, 82, 98, 114, 130, 162, 194, 226, 257, 258
    };

    uint8_t bitLen[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4,
        5, 5, 5, 5, 0
    };

    for(int i = 0; i < 29; i++)
    {
        if((len <= maxLen[i]) && (len >= minLen[i]))
        {
            *code = i + 257;
            *bitVals = len - minLen[i];
            *bits = bitLen[i];
            //printf("len: %d\n", len);
            //printf("code: %d\n", *code);
            return;
        }
    }
}

void getDistCode(
    uint32_t dist,
    uint16_t *code,
    uint8_t *bits,
    uint16_t *bitVals
)
{
    uint32_t minDist[] = {
        1, 2, 3, 4, 5, 6, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385,
        513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
    };

    uint32_t maxDist[] = {
        1, 2, 3, 4, 5, 6, 12, 17, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512,
        768, 1024, 1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384, 24576,
        32768
    };

    uint8_t bitLen[] = {
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10,
        10, 11, 11, 12, 12, 13, 13
    };

    for(int i = 0; i < 29; i++)
    {
        if((dist <= maxDist[i]) && (dist >= minDist[i]))
        {
            *code = i;
            *bitVals = dist - minDist[i];
            *bits = bitLen[i];
            return;
        }
    }
}

void writeBits(
    uint32_t *bitCounter,
    uint8_t *buffer,
    uint8_t bits,
    uint32_t bitVals,
    uint8_t reverseOrder
)
{
    if(reverseOrder == 0)
    {
        for(int i = 0; i < bits; i++)
        {
            if((bitVals & (1 << i)) != 0)
            {
                buffer[(*bitCounter) >> 3] |= 1 << ((*bitCounter) & 7);
            }
            (*bitCounter)++;
        }
    } else
    {
        for(int i = bits; i-- > 0;)
        {
            if((bitVals & (1 << i)) != 0)
            {
                buffer[(*bitCounter) >> 3] |= 1 << ((*bitCounter) & 7);
            }
            (*bitCounter)++;
        }
    }

    //if(bits > 0)
    //{
        //printf("%d, %d\n", bitVals, bits);
        //for(int j = 0; j < (*bitCounter >> 3) + ((*bitCounter & 7) != 0); j++)
        //{
            //for(int k = 8; k-- > 0;)
            //{
                //if((k + (j * 8)) < *bitCounter)
                    //printf("%d", (buffer[j] >> k) & 1);
                //else //printf("-");

                //if(k == 4)//printf(" ");
            //}
            //printf("\n");
        //}
        //printf("\n");
    //}
}

void findPrefix(
    uint16_t in,
    uint8_t* retBits,
    uint16_t* retVal
)
{
    if(in < 144)
    {
        *retVal = 0b00110000 + in;
        *retBits = 8;

    } else if(in < 256)
    {
        *retVal = 0b110010000 + in - 144;
        *retBits = 9;
    } else if(in < 280)
    {
        *retVal = in - 256;
        *retBits = 7;
    } else {
        *retVal = 0b11000000 + in - 280;
        *retBits = 8;
    }

    //printf("prefix info %d %d %d\n", *retVal, *retBits, in);

    return;
}

//takes a buffer of uint8_t and returns a deflated buffer
void deflateBuffer(
    uint8_t *inBuf,
    uint8_t **outBuf,
    uint32_t len,
    uint32_t *outLen
)
{
    /*HEADER INFO*/
    //gets the checksum for the deflated buffer
    uint32_t checksum = adler32(inBuf, len);

    //CM = 8, CINFO = 7, FDICT = 0, FLEVEL = 0
    uint16_t zlibHeader = 0x7800;

    //finds FCHECK
    if((zlibHeader % 31) != 0) zlibHeader += 31 - (zlibHeader % 31);

    *outLen = 0;
    /*ENDO OF HEADER INFO*/

    /*DEFLATION OF BUFFER*/

    int32_t windowStart = 2047;
    uint8_t* window = inBuf - windowStart;
    int32_t windowLen = 0;
    int32_t input = 0;

    uint16_t *offsets = (uint16_t*) malloc(len << 1);
    uint16_t *lengths = (uint16_t*) malloc(len << 1);
    uint8_t *chars = (uint8_t*) malloc(len);

    *offsets = 0;
    *lengths = 0;
    *chars = *inBuf;

    //printf("LEN -> %d\n", len);
    //for(int i = 0; i < len; i++)
    //{
    //    if((inBuf[i] >> 4) == 0)//printf("0");
        //printf("%x", inBuf[i]);
    //}
    //printf("\n");

    uint32_t pos = 0;

    for(int32_t i = 0; i < len;)
    {
        findRepeat(
            offsets + pos,
            lengths + pos,
            window + windowStart,
            inBuf + i,
            windowLen,
            ((len - i) > 31) ? 31 : (len - i)
        );

        //printf("%d\n", ((len - i) > 31) ? 31 : (len - i));

        windowStart -= lengths[pos];
        windowLen += lengths[pos];

        if(windowLen >= 2047)windowLen = 2047;

        if(windowStart < 0)windowStart = 0;

        window += lengths[pos];
        chars[pos] = inBuf[i];
        //printf("char %d\n", chars[i]);
        i += lengths[pos];
        pos++;
    }

    *outBuf = (uint8_t*) malloc(pos * 2 + 10);

    for(int i = 0; i < pos * 2 + 10; i++)
        (*outBuf)[i] = 0;

    uint32_t bitPos = 0;

    writeBits(&bitPos, *outBuf, 1, 1, 0);
    writeBits(&bitPos, *outBuf, 2, 1, 0);

    for(int i = 0; i < pos; i++)
    {

        uint8_t bitNum;
        uint16_t bitVal;

        if(lengths[i] == 1)
        {
            findPrefix(
                chars[i],
                &bitNum,
                &bitVal
            );

            writeBits(&bitPos, *outBuf, bitNum, bitVal, 1);
        } else
        {

            uint8_t extraBits;
            uint16_t extraVal;
            uint16_t code;

            getLenCode(
                lengths[i],
                &code,
                &extraBits,
                &extraVal
            );

            findPrefix(
                code,
                &bitNum,
                &bitVal
            );

            writeBits(&bitPos, *outBuf, bitNum, bitVal, 1);
        writeBits(&bitPos, *outBuf, extraBits, extraVal, 0);

            getDistCode(
                offsets[i],
                &code,
                &extraBits,
                &extraVal
            );

            writeBits(&bitPos, *outBuf, 5, code, 0);
            writeBits(&bitPos, *outBuf, extraBits, extraVal, 0);
        }
    }

    writeBits(&bitPos, *outBuf, 7, 0, 1);

    *outLen = (bitPos >> 3) + ((bitPos & 7) != 0);

    free(offsets);
    free(lengths);
    free(chars);

    /*END OF DEFLATION OF BUFFER*/

    //adds 2 bits for the header and 4 for the checksum   
    *outLen += 6;

    uint8_t *temp = *outBuf;

    (*outBuf) = malloc(*outLen);

    writeBigEndian(&zlibHeader, *outBuf, 2);
    writeBigEndian(&checksum, (*outBuf) + (*outLen) - 4, 4);

    for(int i = 0; i < ((*outLen) - 6); i++)
    {
        (*outBuf)[2 + i] = temp[i];
    }

    free(temp);
}

void makePngFromBitmap(
    uint8_t *red,
    uint8_t *green,
    uint8_t *blue,
    uint32_t w,
    uint32_t h,
    char *path
)
{
    //writes the signature
    FILE *pngfile = fopen(path, "wb+");
    fwrite(png_signature, 1, 8, pngfile);
    fseek(pngfile, 8, SEEK_SET);

    uint8_t IHDR[13];

    writeBigEndian(&w, IHDR, 4);
    writeBigEndian(&h, IHDR + 4, 4);

    IHDR[8] = 8;
    IHDR[9] = 2;
    IHDR[10] = 0;
    IHDR[11] = 0;
    IHDR[12] = 0;

    writeChunk(
        13,
        (uint8_t[]) {'I', 'H', 'D', 'R'},
        IHDR,
        pngfile
    );

    uint8_t *outBuf;
    uint32_t outBufLen;

    uint8_t *imgBuf = (uint8_t*) malloc(w * h * 3 + h);

    for(int i = 0; i < h; i++)
    {
        imgBuf[i * w * 3 + i] = 0;
        for(int j = 0; j < w; j++)
        {
            imgBuf[3 * (i * w + j) + 1 + i] = red[i * w + j];
            imgBuf[3 * (i * w + j) + 2 + i] = green[i * w + j];
            imgBuf[3 * (i * w + j) + 3 + i] = blue[i * w + j];
        }
    }

    deflateBuffer(
        imgBuf,
        &outBuf,
        3 * w * h + h,
        &outBufLen
    );

    //for(int i = 0; i < outBufLen; i++)
    //{
        //if((outBuf[i] >> 4) == 0)//printf("0");
        //printf("%x", outBuf[i]);
    //}

    //printf("\n");

    free(imgBuf);

    writeChunk(
        outBufLen,
        (uint8_t[]) {'I', 'D', 'A', 'T'},
        outBuf,
        pngfile
    );

    free(outBuf);

    writeChunk(
        0,
        (uint8_t[]) {'I', 'E', 'N', 'D'},
        (uint8_t[]){0},
        pngfile
    );
}

int main(void)
{

    srand(time(NULL));

    uint8_t *red = malloc(100 * 100);
    uint8_t *green = malloc(100 * 100);
    uint8_t *blue = malloc(100 * 100);

    for(int i = 0; i < 100 * 100; i++)
    {
        red[i] = 0;//rand();
        green[i] = i & 0xff;//rand();
        blue[i] = rand();
    }

    makePngFromBitmap(
        red,
        green,
        blue,
        100,
        100,
        "test.png"
    );

    free(red);
    free(green);
    free(blue);

    return 0;
}
