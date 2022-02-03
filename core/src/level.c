#include "bomberman.h"

int level_init(level_t *level, const uint32_t cols, const uint32_t rows, const uint32_t cell_size, int32_t *cells)
{
    if (!level || !cols || !rows || !cell_size)
        return -1;

    level->cols = cols;
    level->rows = rows;
    level->cell_size = cell_size;
    level->cells = cells;
    return 0;
}

int32_t level_cell(level_t *level, const uint32_t col, const uint32_t row)
{
    if (!level->cells)
        return -1;
    if (col >= level->cols)
        return -1;
    if (row >= level->rows)
        return -1;

    return level->cells[row * level->cols + col];
}