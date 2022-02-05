#include "bomberman.h"
#include <stdio.h>

void move_player(level_t *level, movable_t *movable, const float delta_x, const float delta_y)
{
    printf("move %f", delta_x);
    movable->rect.x += delta_x;
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