#include "bomberman.h"
#include <stdio.h>



int check_map_border(level_t *level,movable_t *movable, int x, int y)
{
    if(x < 0 || x + movable->rect.w > level->cols * level->cell_size ||
       y < 0 || y + movable->rect.h > level->rows * level->cell_size )
        return 1;
    else
        return 0;
}

int check_collisions(level_t *level, movable_t *movable, float x, float y)
{
    uint32_t cell_x;
    uint32_t cell_y;
    int half_h = movable->rect.h / 2;
    float size_reduction = 10;

    cell_x = (x + size_reduction) / level->cell_size;
    cell_y = (y + half_h + size_reduction) / level->cell_size;
    int32_t cell_1 = get_cell(level,cell_x,cell_y);

    cell_x = (x + movable->rect.w - size_reduction) / level->cell_size;
    cell_y = (y + half_h + size_reduction) /level->cell_size;
    int32_t cell_2 = get_cell(level, cell_x, cell_y);

    cell_x = (x + size_reduction) / level->cell_size;
    cell_y = (y + movable->rect.h - size_reduction)/level->cell_size;
    int32_t cell_3 = get_cell(level, cell_x, cell_y);

    cell_x = (x + movable->rect.w - size_reduction) / level->cell_size;
    cell_y = (y + movable->rect.h - size_reduction) /level->cell_size;
    int32_t cell_4 = get_cell(level, cell_x, cell_y);



    if (cell_1 & BLOCK_WALL || cell_2 & BLOCK_WALL || cell_3 & BLOCK_WALL || cell_4 & BLOCK_WALL)
    {
        return 1;
    }
    else
        return 0;
}

void move_player(level_t *level, movable_t *movable, const float delta_x, const float delta_y)
{
    if (delta_x == 0 && delta_y == 0)
    {
        return;
    }
    
    int new_x = (int)(movable->rect.x + delta_x);
    int new_y = (int)(movable->rect.y + delta_y);
    if (check_map_border(level,movable,new_x,new_y) || check_collisions(level,movable,new_x,new_y))
    {
        return;
    }

    movable->rect.x = new_x;
    movable->rect.y += delta_y;
}

int32_t move_on_level(level_t *level, movable_t *movable, const float delta_x, const float delta_y)
{
    float new_x = movable->rect.x + delta_x;
    float new_y = movable->rect.y + delta_y;

    if (new_x < 0)
        new_x = 0;
    else if (new_x + movable->rect.w >= level->cols * level->cell_size)
        new_x = (level->cols * level->cell_size) - movable->rect.w;

    if (new_y < 0)
        new_y = 0;
    else if (new_y + movable->rect.h >= level->rows * level->cell_size)
        new_y = (level->rows * level->cell_size) - movable->rect.h;

    int32_t cell = -1;
    // moving right ?
    if (new_x > movable->rect.x)
    {
        uint32_t cell_x = (new_x + movable->rect.w - 1) / level->cell_size; // cell on which the movable will end
        uint32_t cell_y = (movable->rect.y + movable->rect.h - 1) / level->cell_size; // test for feet !!!y increase going down
        cell = level_cell(level, cell_x, cell_y);
        if (cell & BLOCK_MASK_UNWALKABLE) // collision!
        {
            movable->rect.x = cell_x * level->cell_size - movable->rect.w; // bring back
        }
        else
        {
            cell_y = movable->rect.y / level->cell_size; // test for neck
            cell = level_cell(level, cell_x, cell_y);
            if (cell & BLOCK_MASK_UNWALKABLE) // collision!
            {
                movable->rect.x = cell_x * level->cell_size - movable->rect.w; // bring back
            }
            else
            {
                movable->rect.x = new_x;
            }
        }
    }else if(new_x<movable->rect.x){
        uint32_t cell_x = new_x / level->cell_size; // cell on which the movable will end
        uint32_t cell_y = (movable->rect.y + movable->rect.h - 1) / level->cell_size; // test for feet !!!y increase going down
        cell = level_cell(level, cell_x, cell_y);
        if (cell & BLOCK_MASK_UNWALKABLE) // collision!
        {
            movable->rect.x = cell_x * level->cell_size + level->cell_size +1; // bring back
        }
        else
        {
            cell_y = movable->rect.y / level->cell_size; // test for neck
            cell = level_cell(level, cell_x, cell_y);
            if (cell & BLOCK_MASK_UNWALKABLE) // collision!
            {
                movable->rect.x = cell_x * level->cell_size + level->cell_size +1; // bring back
            }
            else
            {
                movable->rect.x = new_x;
            }
        }
    }

    return cell;
}