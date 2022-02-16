#pragma comment(lib, "ws2_32.lib")
#include <zlib.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>



typedef struct chunk
{
    Uint32 length;
    char type[4];
    char *data;
    uLong crc;
} chunk_t;


void to_little(Uint32 *value)
{
    *value = ((*value >> 24) & 0xff) |
    ((*value << 8) & 0xff0000) |
    ((*value >> 8) & 0xff00) |
    ((*value << 24) & 0xff000000);
}

void rd_ch(SDL_RWops **file)
{

    chunk_t ch;
    SDL_RWread(*file, &(ch.length), 4, 1);

    SDL_RWread(*file, &(ch.type), 4, 1);
    ch.type[5] = '\0';

    ch.data = SDL_malloc(ch.length);
    SDL_RWread(*file, ch.data, ch.length, 1);

    ch.length = ntohl(ch.length);
    to_little(ch.type);
    uLong l_crc = crc32(0, (unsigned char *)&(ch.length), 4);
    uLong crc_sum = crc32(l_crc, (unsigned char *)&(ch.type), 4);

    //ch.crc = ntohl(ch.crc);

    printf("l = %d\n", ch.length);

    printf("%s\n", ch.type);

    SDL_RWread(*file, &ch.crc, 4, 1);


    printf("crc = %lu\n", ch.crc);
    printf("crc_sum = %lu\n", crc_sum);
    if (crc_sum == ch.crc)
    {
        printf("\n sum correct");
    }
    
}

int png_parse_file(SDL_RWops **file, Uint8 **pixel_data, SDL_Rect *rect)
{

    rd_ch(file);
    return 1;
}

int png_open_file(char *filename, Uint8 **img_data)
{

    SDL_RWops *file = SDL_RWFromFile(filename, "rb");
    if (!file)
    {
        return -1;
    }
    Sint64 file_size = SDL_RWsize(file);
    *img_data = SDL_malloc(file_size);

    if (!(*img_data))
    {
        SDL_RWclose(file);
        return -1;
    }

    char signature[8];
    char test[8];

    SDL_RWread(file, signature, 8, 1);

    
    char png_signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    char header[8];
    SDL_memcpy(img_data, header, 8);

    
    if (SDL_strcmp(signature,header)) //check if file is a png
    {

        png_parse_file(&file, NULL, NULL);
        SDL_RWclose(file);
        return 0;
    }else
    {
        return -1;
        SDL_RWclose(file);
    }
}






