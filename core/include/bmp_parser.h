#include <SDL.h>

typedef struct bmp_image
{
    Uint32 width;
    Uint32 height;
} BMP_image_t;

int bmp_open_file(const char *fileName, const char *mode, Uint8 **content);

int bmp_create_texture(Uint8 *data, const Uint8 channels, SDL_Renderer *renderer, SDL_Texture **texture,SDL_Rect *rect);
