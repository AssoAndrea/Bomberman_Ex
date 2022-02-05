#include "SDL.h"
#include "bomberman.h"
#include "level_1.h"
#include "bmp_parser.h"
#include "stdio.h"

SDL_Rect ZeroRect()
{
    SDL_Rect rect = {0, 0, 0, 0};
    return rect;
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    level_t level_1;
    level_init(&level_1, 8, 8, 64, level_1_cells);


 
    SDL_Window *window = SDL_CreateWindow("Bomberman",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,level_1.cols * level_1.cell_size,level_1.rows * level_1.cell_size, 0);
 
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    //parser test
    SDL_Texture *bmp_parser_texture;
    Uint8 *bmp_parser_img_data;
    bmp_open_file("Capture.bmp","rb",&bmp_parser_img_data);
    SDL_Rect bmp_parser_rect;
    bmp_parser_rect.x = 20;
    bmp_parser_rect.y = 20;
    bmp_create_texture(bmp_parser_img_data, 3, renderer, &bmp_parser_texture, &bmp_parser_rect);
    SDL_free(bmp_parser_img_data);

    bomberman_t player0;
    player0.movable.speed = 48;
    player0.movable.rect = ZeroRect();
    player0.movable.texture = NULL;
    Uint8 *img_data;
    bmp_open_file("assets//Bman.bmp", "rb", &img_data);
    bmp_create_texture(img_data, 3, renderer, &player0.movable.texture, &player0.movable.rect);

    player0.movable.rect.x = 20;
    player0.movable.rect.y = 20;
    SDL_free(img_data);

    SDL_Rect cell_rect = {0, 0, level_1.cell_size, level_1.cell_size};
    SDL_Texture *wall_texture;
    SDL_Texture *ground_texture;
    SDL_Texture *destroyable_texture;
    bmp_open_file("assets//wall.bmp", "rb", &img_data); //wall
    bmp_create_texture(img_data, 3, renderer, &wall_texture,NULL);
    SDL_free(img_data);

    bmp_open_file("assets//floor.bmp", "rb", &img_data); //floor
    bmp_create_texture(img_data, 3, renderer, &ground_texture,NULL);
    SDL_free(img_data);
    
    bmp_open_file("assets//explodable.bmp", "rb", &img_data); //explodable
    bmp_create_texture(img_data, 3, renderer, &destroyable_texture,NULL);
    SDL_free(img_data);

    float delta_right = 0;
    float delta_left = 0;
    float delta_down = 0;
    float delta_up = 0;
 
    int running = 1;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    delta_right = player0.movable.speed * (1.0 / 60);
                    printf("right %f\n",delta_right);
                }
                if (event.key.keysym.sym == SDLK_LEFT)
                {
                    delta_left = -player0.movable.speed * (1.0 / 60);
                    printf("left %f\n", delta_left);
                }
                if (event.key.keysym.sym == SDLK_DOWN)
                {
                    delta_down = player0.movable.speed * (1.0 / 60);
                }
                if (event.key.keysym.sym == SDLK_UP)
                {
                    delta_up = -player0.movable.speed * (1.0 / 60);
                }
            }
            if (event.type == SDL_KEYUP)
            {
                if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    delta_right = 0;
                }
                if (event.key.keysym.sym == SDLK_LEFT)
                {
                    delta_left = 0;
                }
                if (event.key.keysym.sym == SDLK_DOWN)
                {
                    delta_down = 0;
                }
                if (event.key.keysym.sym == SDLK_UP)
                {
                    delta_up = 0;
                }
            }
        }

        // SDL_RenderCopy(renderer, bmp_parser_texture, NULL, &bmp_parser_rect);
        // SDL_RenderPresent(renderer);

        for (uint32_t row = 0; row < level_1.rows; row++)  //draw level
        {
            for (uint32_t col = 0; col < level_1.cols; col++)
            {
                int32_t cell = level_cell(&level_1, col, row);
                int32_t cell_texture = cell & 0xff;
                cell_rect.x = col * level_1.cell_size;
                cell_rect.y = row * level_1.cell_size;
                SDL_Texture *textureToDraw = NULL;

                if (cell_texture == BLOCK_GROUND)
                {
                    textureToDraw = ground_texture;
                }
                else if (cell_texture == BLOCK_WALL)
                {
                    textureToDraw = wall_texture;
                }
                else if (cell_texture == BLOCK_DESTROYABLE)
                {
                    textureToDraw = destroyable_texture;
                }
                SDL_RenderCopy(renderer,textureToDraw,NULL,&cell_rect);

            }
        }
 
        move_player(&level_1, &player0.movable, delta_right + delta_left, delta_down + delta_up);

        SDL_RenderCopy(renderer,player0.movable.texture,NULL,&player0.movable.rect);
        SDL_RenderPresent(renderer);
    }
 
	SDL_Quit();
    return 0;
}