#include "SDL.h"
#include "bomberman.h"
#include "level_1.h"
#include "bmp_parser.h"
#include "stdio.h"
#include "client.h"
#include <winsock.h>
#include <time.h>



SDL_Rect ZeroRect()
{
    SDL_Rect rect = {0, 0, 0, 0};
    return rect;
}

bomberman_t player0;
bomberman_t player1;
bomberman_t local_player;
bomberman_t remote_player;

level_t level_1;

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    level_init(&level_1, 8, 8, 64, level_1_cells);

    Uint8 *pngData;

    //png_open_file("python//Bman_F_f01.png", &pngData);

    SDL_Window *window = SDL_CreateWindow("Bomberman",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,level_1.cols * level_1.cell_size,level_1.rows * level_1.cell_size, 0);
 
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Color_t white = {255, 255, 255};
    Color_t red = {0, 255, 0};

    #pragma region player0
    
    player0.movable.speed = 60;
    player0.movable.rect = ZeroRect();
    player0.movable.rect.w = 64;
    player0.movable.rect.h = 128;
    player0.movable.texture = NULL;
    Uint8 *img_data;
    bmp_open_file("assets//bman_ab.bmp", "rb", &img_data);
    bmp_create_texture(img_data, 4, renderer, &player0.movable.texture, &player0.movable.rect,white);
    player0.movable.rect.x = 20;
    player0.movable.rect.y = 20;
    SDL_free(img_data);
    #pragma endregion

    #pragma region player1
    
    player1.movable.speed = 60;
    player1.movable.rect = ZeroRect();
    player1.movable.rect.w = 64;
    player1.movable.rect.h = 128;
    player1.movable.texture = NULL;
    bmp_open_file("assets//bman_ab.bmp", "rb", &img_data);
    bmp_create_texture(img_data, 4, renderer, &player1.movable.texture, &player1.movable.rect,white);
    player1.movable.rect.x = 100;
    player1.movable.rect.y = 20;
    SDL_free(img_data);
    #pragma endregion

    #pragma region wall
    SDL_Rect cell_rect = {0, 0, level_1.cell_size, level_1.cell_size};
    SDL_Texture *wall_texture;
    SDL_Texture *ground_texture;
    SDL_Texture *destroyable_texture;
    bmp_open_file("assets//wall.bmp", "rb", &img_data); //wall
    bmp_create_texture(img_data, 3, renderer, &wall_texture,NULL,white);
    SDL_free(img_data);
    #pragma endregion

    #pragma region floor
    bmp_open_file("assets//floor.bmp", "rb", &img_data); //floor
    bmp_create_texture(img_data, 3, renderer, &ground_texture,NULL,white);
    SDL_free(img_data);
    #pragma endregion
    
    #pragma region explodable
    bmp_open_file("assets//explodable.bmp", "rb", &img_data); //explodable
    bmp_create_texture(img_data, 3, renderer, &destroyable_texture,NULL,white);
    SDL_free(img_data);
    #pragma endregion

    
    float delta_right = 0;
    float delta_left = 0;
    float delta_down = 0;
    float delta_up = 0;

    

    client_init();
    send_auth_req();

    //send_position(10.0, 10.0);

    clock_t start = clock();  //time
    clock_t start_send_timer = clock();  //time
    float send_data_timer = (float)1 / (float)10;

    int running = 1;
    while (running)
    {
        SDL_Event event;
        #pragma region POLL_EVENT
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    delta_right = player0.movable.speed * (1.0 / 60);
                }
                else if (event.key.keysym.sym == SDLK_LEFT)
                {
                    delta_left = -player0.movable.speed * (1.0 / 60);
                }
                else if (event.key.keysym.sym == SDLK_DOWN)
                {
                    delta_down = player0.movable.speed * (1.0 / 60);
                }
                else if (event.key.keysym.sym == SDLK_UP)
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
        #pragma endregion



        // SDL_RenderCopy(renderer, bmp_parser_texture, NULL, &bmp_parser_rect);
        // SDL_RenderPresent(renderer);
        #pragma region draw_level
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
        #pragma endregion
 
        move_player(&level_1, &local_player.movable, delta_right + delta_left, delta_down + delta_up);

        //server
        if (client_is_authorized()==0)
        {
            keep_alive(&start);
            clock_t cur_time = clock();
            float elapsed_time = (float)(cur_time - start_send_timer) / CLOCKS_PER_SEC;
            if (elapsed_time > send_data_timer)
            {
                //printf("elapsed time = %f\n", elapsed_time);
                start_send_timer = clock();
                send_position(local_player.movable.rect.x,local_player.movable.rect.y);

            }
        }
        
        receive_data();


        SDL_RenderCopy(renderer,local_player.movable.texture,NULL,&local_player.movable.rect);
        SDL_RenderCopy(renderer,remote_player.movable.texture,NULL,&remote_player.movable.rect);
        SDL_RenderPresent(renderer);

    }
 
	SDL_Quit();
    return 0;
}