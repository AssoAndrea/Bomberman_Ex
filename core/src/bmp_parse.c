#include <stdio.h>
#include "bmp_parser.h"

#define SIZE_OFF 2
#define PIXEL_OFF 10
#define IMG_WIDTH_OFF 18
#define IMG_HEIGHT_OFF 22
#define PIXEL_DATA_SIZE_OFF 34
#define ALIGNMENT 4



int bmp_create_texture(Uint8 *data,const Uint8 channels,SDL_Renderer *renderer,SDL_Texture **texture,SDL_Rect *img_rect)
{
    BMP_image_t bmp_img;
    
    Uint8 *pixel_start_off = (*(data + PIXEL_OFF)) + data;
    //SDL_memcpy(pixel_start_off, data + PIXEL_OFF, 4); //offset da cui iniziano i pixel


    SDL_memcpy(&bmp_img.width, data + IMG_WIDTH_OFF, 4);
    SDL_memcpy(&bmp_img.height, data + IMG_HEIGHT_OFF, 4);
    if(img_rect)
    {
        img_rect->w = bmp_img.width;
        img_rect->h = bmp_img.height;
    }
    *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STATIC, bmp_img.width, bmp_img.height);
    if (!(*texture))
    {
        perror("error on create texture: ");
        return -1;
    }
    Uint32 row_size = bmp_img.width * channels;
    Uint32 padded_row_size = (row_size / ALIGNMENT) * ALIGNMENT;
    if (row_size % ALIGNMENT != 0)
    {
        padded_row_size += ALIGNMENT;
    }

    Uint8 *pixels = SDL_malloc(row_size * bmp_img.height);
    if (!pixels)
    {
        SDL_free(pixels);
        perror("error on create pixels array: ");
        return -1;
    }

    for (Uint32 i = 0; i < bmp_img.height; i++)
    {
        Uint8 *texture_row = pixels + (i * row_size);
        Uint8 *bmp_row = pixel_start_off + ((bmp_img.height - 1) - i) * padded_row_size;
        SDL_memcpy(texture_row, bmp_row, row_size);
    }
    SDL_UpdateTexture(*texture, NULL, pixels, row_size);
    SDL_free(pixels);
    return 0;
}

int bmp_open_file(const char *fileName,const char *mode, Uint8 **content)
{
     
    SDL_RWops *rw = SDL_RWFromFile(fileName,mode);
    if(!rw) 
        return -1;

    Sint64 file_size = SDL_RWsize(rw);
    *content = SDL_malloc(file_size);

    if(!(*content)){
        SDL_RWclose(rw);
        return -1;
    }

    SDL_RWread(rw,*content, file_size, 1);
    SDL_RWclose(rw);
    return 0;
}