#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

#define MAX_SIZE 5
#define MAX_TILES (MAX_SIZE * MAX_SIZE)
#define FOUND_STATE (-1)
#define MAX_PATH_LEN 16384

static int goal_row[MAX_TILES];
static int goal_col[MAX_TILES];
static unsigned char manhattan_table[MAX_TILES][MAX_TILES];

static void write_separator(FILE *out, int size)
{
    int repeats = size * size;
    for (int i = 0; i < repeats; ++i)
    {
        fputs("= ", out);
    }
    fputc('\n', out);
}

static void write_board(FILE *out, const int *state, int size)
{
    write_separator(out, size);
    for (int r = 0; r < size; ++r)
    {
        for (int c = 0; c < size; ++c)
        {
            int value = state[r * size + c];
            if (value == 0)
            {
                fputs("     ", out);
            }
            else
            {
                fprintf(out, "%5d", value);
            }
        }
        fputc('\n', out);
    }
}

static bool is_solvable(const unsigned char *state, int size)
{
    int total = size * size;
    int inversions = 0;
    for (int i = 0; i < total; ++i)
    {
        if (state[i] == 0)
        {
            continue;
        }
        for (int j = i + 1; j < total; ++j)
        {
            if (state[j] != 0 && state[i] > state[j])
            {
                inversions++;
            }
        }
    }
    if (size % 2 == 1)
    {
        return inversions % 2 == 0;
    }
    int zero_row_from_bottom = 0;
    for (int i = 0; i < total; ++i)
    {
        if (state[i] == 0)
        {
            zero_row_from_bottom = size - (i / size);
            break;
        }
    }
    return (inversions + zero_row_from_bottom) % 2 == 0;
}

static int heuristic(const unsigned char *state, int size)
{
    int total = size * size;
    int sum = 0;
    for (int i = 0; i < total; ++i)
    {
        unsigned char tile = state[i];
        if (tile == 0)
        {
            continue;
        }
        sum += manhattan_table[tile][i];
    }

    for (int row = 0; row < size; ++row)
    {
        for (int i = 0; i < size; ++i)
        {
            int index_i = row * size + i;
            unsigned char tile_i = state[index_i];
            if (tile_i == 0 || goal_row[tile_i] != row)
            {
                continue;
            }
            int goal_col_i = goal_col[tile_i];
            for (int j = i + 1; j < size; ++j)
            {
                int index_j = row * size + j;
                unsigned char tile_j = state[index_j];
                if (tile_j == 0 || goal_row[tile_j] != row)
                {
                    continue;
                }
                if (goal_col_i > goal_col[tile_j])
                {
                    sum += 2;
                }
            }
        }
    }

    for (int col = 0; col < size; ++col)
    {
        for (int i = 0; i < size; ++i)
        {
            int index_i = i * size + col;
            unsigned char tile_i = state[index_i];
            if (tile_i == 0 || goal_col[tile_i] != col)
            {
                continue;
            }
            int goal_row_i = goal_row[tile_i];
            for (int j = i + 1; j < size; ++j)
            {
                int index_j = j * size + col;
                unsigned char tile_j = state[index_j];
                if (tile_j == 0 || goal_col[tile_j] != col)
                {
                    continue;
                }
                if (goal_row_i > goal_row[tile_j])
                {
                    sum += 2;
                }
            }
        }
    }

    return sum;
}

typedef struct
{
    int dir;
    int swap_index;
    unsigned char moved_tile;
    int heuristic_value;
} Candidate;

static void sort_candidates(Candidate *candidates, int count)
{
    for (int i = 1; i < count; ++i)
    {
        Candidate key = candidates[i];
        int j = i - 1;
        while (j >= 0 && candidates[j].heuristic_value > key.heuristic_value)
        {
            candidates[j + 1] = candidates[j];
            --j;
        }
        candidates[j + 1] = key;
    }
}

static int search(unsigned char *state, int size, int zero_index, int depth, int prev_move, int threshold, int current_h, int *path, int *solution_depth)
{
    int f = depth + current_h;
    if (f > threshold)
    {
        return f;
    }
    if (current_h == 0)
    {
        *solution_depth = depth;
        return FOUND_STATE;
    }

    int best = INT_MAX;
    int row = zero_index / size;
    int col = zero_index % size;

    static const int move_row[] = {-1, 1, 0, 0};
    static const int move_col[] = {0, 0, -1, 1};
    static const int opposite[] = {1, 0, 3, 2};

    Candidate candidates[4];
    int candidate_count = 0;

    for (int dir = 0; dir < 4; ++dir)
    {
        int new_row = row + move_row[dir];
        int new_col = col + move_col[dir];
        if (new_row < 0 || new_row >= size || new_col < 0 || new_col >= size)
        {
            continue;
        }
        if (prev_move != -1 && dir == opposite[prev_move])
        {
            continue;
        }

        int swap_index = new_row * size + new_col;
        unsigned char moved_tile = state[swap_index];
        state[swap_index] = 0;
        state[zero_index] = moved_tile;
        int next_h = heuristic(state, size);
        state[zero_index] = 0;
        state[swap_index] = moved_tile;

        candidates[candidate_count].dir = dir;
        candidates[candidate_count].swap_index = swap_index;
        candidates[candidate_count].moved_tile = moved_tile;
        candidates[candidate_count].heuristic_value = next_h;
        candidate_count++;
    }

    sort_candidates(candidates, candidate_count);

    for (int i = 0; i < candidate_count; ++i)
    {
        Candidate cand = candidates[i];
        state[cand.swap_index] = 0;
        state[zero_index] = cand.moved_tile;
        path[depth] = cand.dir;

        int t = search(state, size, cand.swap_index, depth + 1, cand.dir, threshold, cand.heuristic_value, path, solution_depth);

        state[zero_index] = 0;
        state[cand.swap_index] = cand.moved_tile;

        if (t == FOUND_STATE)
        {
            return FOUND_STATE;
        }
        if (t < best)
        {
            best = t;
        }
    }

    return best;
}

static int ida_star(unsigned char *state, int size, int zero_index, int *path)
{
    int start_h = heuristic(state, size);
    int threshold = start_h;
    int solution_depth = -1;

    while (1)
    {
        int t = search(state, size, zero_index, 0, -1, threshold, start_h, path, &solution_depth);
        if (t == FOUND_STATE)
        {
            return solution_depth;
        }
        if (t == INT_MAX || threshold >= MAX_PATH_LEN)
        {
            return -1;
        }
        threshold = t;
        if (threshold > MAX_PATH_LEN)
        {
            return -1;
        }
    }
}

static void apply_move(int *state, int size, int move)
{
    int total = size * size;
    int zero_index = 0;
    for (int i = 0; i < total; ++i)
    {
        if (state[i] == 0)
        {
            zero_index = i;
            break;
        }
    }
    int row = zero_index / size;
    int col = zero_index % size;
    int swap_index = zero_index;
    switch (move)
    {
        case 0: // up
            swap_index = (row - 1) * size + col;
            break;
        case 1: // down
            swap_index = (row + 1) * size + col;
            break;
        case 2: // left
            swap_index = row * size + (col - 1);
            break;
        case 3: // right
            swap_index = row * size + (col + 1);
            break;
    }
    int tmp = state[zero_index];
    state[zero_index] = state[swap_index];
    state[swap_index] = tmp;
}

int main(void)
{
    int size;
    if (scanf("%d", &size) != 1)
    {
        return 0;
    }
    if (size < 2 || size > MAX_SIZE)
    {
        fprintf(stderr, "Unsupported puzzle size.\n");
        return 1;
    }

    int total = size * size;
    unsigned char initial[MAX_TILES];
    int zero_index = -1;
    for (int i = 0; i < total; ++i)
    {
        int value;
        if (scanf("%d", &value) != 1)
        {
            fprintf(stderr, "Invalid input.\n");
            return 1;
        }
        initial[i] = (unsigned char)value;
        if (value == 0)
        {
            zero_index = i;
        }
    }

    for (int tile = 1; tile < total; ++tile)
    {
        goal_row[tile] = (tile - 1) / size;
        goal_col[tile] = (tile - 1) % size;
    }
    goal_row[0] = size - 1;
    goal_col[0] = size - 1;

    for (int tile = 1; tile < total; ++tile)
    {
        for (int pos = 0; pos < total; ++pos)
        {
            int row = pos / size;
            int col = pos % size;
            manhattan_table[tile][pos] = (unsigned char)(abs(row - goal_row[tile]) + abs(col - goal_col[tile]));
        }
    }

    if (!is_solvable(initial, size))
    {
        fprintf(stderr, "This puzzle is not solvable.\n");
        return 0;
    }

    unsigned char work_state[MAX_TILES];
    memcpy(work_state, initial, (size_t)total * sizeof(unsigned char));

    int path[MAX_PATH_LEN];
    int solution_depth = ida_star(work_state, size, zero_index, path);
    if (solution_depth < 0)
    {
        fprintf(stderr, "Failed to find a solution within limits.\n");
        return 1;
    }

    int current_state[MAX_TILES];
    for (int i = 0; i < total; ++i)
    {
        current_state[i] = initial[i];
    }

    FILE *log = fopen("log.txt", "w");
    if (!log)
    {
        fprintf(stderr, "Unable to open log file.\n");
        return 1;
    }

    write_board(log, current_state, size);
    for (int i = 0; i < solution_depth; ++i)
    {
        apply_move(current_state, size, path[i]);
        write_board(log, current_state, size);
    }

    fclose(log);
    printf("Solution written to log.txt\n");

    return 0;
}
