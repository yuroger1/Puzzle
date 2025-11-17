#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define MAX_SIZE 11
#define MAX_TILES (MAX_SIZE * MAX_SIZE)

static const int dir_row[4] = {-1, 1, 0, 0};
static const int dir_col[4] = {0, 0, -1, 1};

typedef struct
{
    int size;
    int total;
    int state[MAX_TILES];
    bool locked[MAX_TILES];
    int zero_index;
} Puzzle;

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

static bool in_bounds(int size, int row, int col)
{
    return row >= 0 && row < size && col >= 0 && col < size;
}

static int direction_between(int from, int to, int size)
{
    int from_row = from / size;
    int from_col = from % size;
    int to_row = to / size;
    int to_col = to % size;
    if (to_row == from_row - 1 && to_col == from_col)
    {
        return 0;
    }
    if (to_row == from_row + 1 && to_col == from_col)
    {
        return 1;
    }
    if (to_row == from_row && to_col == from_col - 1)
    {
        return 2;
    }
    if (to_row == from_row && to_col == from_col + 1)
    {
        return 3;
    }
    return -1;
}

static void perform_move(Puzzle *p, int dir, FILE *log)
{
    int size = p->size;
    int row = p->zero_index / size;
    int col = p->zero_index % size;
    int new_row = row + dir_row[dir];
    int new_col = col + dir_col[dir];
    if (!in_bounds(size, new_row, new_col))
    {
        return;
    }
    int new_index = new_row * size + new_col;
    int tmp = p->state[p->zero_index];
    p->state[p->zero_index] = p->state[new_index];
    p->state[new_index] = tmp;
    p->zero_index = new_index;
    write_board(log, p->state, size);
}

static bool move_blank_to(Puzzle *p, int target, const bool *extra_block, FILE *log)
{
    if (p->zero_index == target)
    {
        return true;
    }
    int size = p->size;
    bool visited[MAX_TILES];
    int prev[MAX_TILES];
    int queue[MAX_TILES];
    memset(visited, 0, sizeof(visited));
    int head = 0;
    int tail = 0;
    queue[tail++] = p->zero_index;
    visited[p->zero_index] = true;
    prev[p->zero_index] = -1;

    while (head < tail)
    {
        int pos = queue[head++];
        if (pos == target)
        {
            break;
        }
        int row = pos / size;
        int col = pos % size;
        for (int dir = 0; dir < 4; ++dir)
        {
            int new_row = row + dir_row[dir];
            int new_col = col + dir_col[dir];
            if (!in_bounds(size, new_row, new_col))
            {
                continue;
            }
            int next = new_row * size + new_col;
            if (visited[next])
            {
                continue;
            }
            bool blocked = p->locked[next];
            if (extra_block && extra_block[next])
            {
                blocked = true;
            }
            if (blocked && next != target)
            {
                continue;
            }
            visited[next] = true;
            prev[next] = pos;
            queue[tail++] = next;
        }
    }

    if (!visited[target])
    {
        return false;
    }

    int path[MAX_TILES];
    int path_len = 0;
    for (int at = target; at != -1; at = prev[at])
    {
        path[path_len++] = at;
    }

    for (int i = path_len - 2; i >= 0; --i)
    {
        int next = path[i];
        int dir = direction_between(p->zero_index, next, size);
        if (dir == -1)
        {
            return false;
        }
        perform_move(p, dir, log);
    }

    return true;
}

static int find_tile(const Puzzle *p, int value)
{
    for (int i = 0; i < p->total; ++i)
    {
        if (p->state[i] == value)
        {
            return i;
        }
    }
    return -1;
}

static bool find_tile_path(const Puzzle *p, int start, int target, int *path, int *path_len)
{
    if (start == target)
    {
        path[0] = start;
        *path_len = 1;
        return true;
    }
    int size = p->size;
    bool visited[MAX_TILES];
    int prev[MAX_TILES];
    int queue[MAX_TILES];
    memset(visited, 0, sizeof(visited));
    int head = 0;
    int tail = 0;
    queue[tail++] = start;
    visited[start] = true;
    prev[start] = -1;

    while (head < tail)
    {
        int pos = queue[head++];
        if (pos == target)
        {
            break;
        }
        int row = pos / size;
        int col = pos % size;
        for (int dir = 0; dir < 4; ++dir)
        {
            int new_row = row + dir_row[dir];
            int new_col = col + dir_col[dir];
            if (!in_bounds(size, new_row, new_col))
            {
                continue;
            }
            int next = new_row * size + new_col;
            if (visited[next])
            {
                continue;
            }
            if (p->locked[next] && next != target)
            {
                continue;
            }
            visited[next] = true;
            prev[next] = pos;
            queue[tail++] = next;
        }
    }

    if (!visited[target])
    {
        return false;
    }

    int len = 0;
    for (int at = target; at != -1; at = prev[at])
    {
        path[len++] = at;
    }
    for (int i = 0; i < len / 2; ++i)
    {
        int tmp = path[i];
        path[i] = path[len - 1 - i];
        path[len - 1 - i] = tmp;
    }
    *path_len = len;
    return true;
}

static bool move_tile_along_path(Puzzle *p, int *path, int path_len, FILE *log)
{
    for (int i = 0; i < path_len - 1; ++i)
    {
        int current = path[i];
        int next = path[i + 1];
        bool extra[MAX_TILES];
        memset(extra, 0, sizeof(extra));
        extra[current] = true;
        if (!move_blank_to(p, next, extra, log))
        {
            return false;
        }
        int dir = direction_between(p->zero_index, current, p->size);
        if (dir == -1)
        {
            return false;
        }
        perform_move(p, dir, log);
    }
    return true;
}

static bool place_tile(Puzzle *p, int value, int target_index, FILE *log)
{
    if (value == 0)
    {
        return move_blank_to(p, target_index, NULL, log);
    }
    int start = find_tile(p, value);
    if (start == -1)
    {
        return false;
    }
    if (start == target_index)
    {
        return true;
    }
    int path[MAX_TILES];
    int path_len = 0;
    if (!find_tile_path(p, start, target_index, path, &path_len))
    {
        return false;
    }
    return move_tile_along_path(p, path, path_len, log);
}

static bool solve_last_block(Puzzle *p, FILE *log)
{
    int size = p->size;
    int cells[4];
    cells[0] = (size - 2) * size + (size - 2);
    cells[1] = (size - 2) * size + (size - 1);
    cells[2] = (size - 1) * size + (size - 2);
    cells[3] = (size - 1) * size + (size - 1);

    unsigned char target_tiles[4];
    for (int i = 0; i < 3; ++i)
    {
        target_tiles[i] = (unsigned char)(cells[i] + 1);
    }
    target_tiles[3] = 0;

    unsigned char start_tiles[4];
    int start_zero = -1;
    for (int i = 0; i < 4; ++i)
    {
        start_tiles[i] = (unsigned char)p->state[cells[i]];
        if (start_tiles[i] == 0)
        {
            start_zero = i;
        }
    }

    bool already_solved = true;
    for (int i = 0; i < 4; ++i)
    {
        if (start_tiles[i] != target_tiles[i])
        {
            already_solved = false;
            break;
        }
    }
    if (already_solved)
    {
        return true;
    }

    typedef struct
    {
        unsigned char tiles[4];
        unsigned char zero;
        int parent;
        int move_dir;
    } MiniNode;

    static const int neighbor_counts[4] = {2, 2, 2, 2};
    static const int neighbor_positions[4][2] = {
        {1, 2},
        {0, 3},
        {0, 3},
        {1, 2}
    };
    static const int neighbor_dirs[4][2] = {
        {3, 1},
        {2, 1},
        {0, 3},
        {0, 2}
    };

    MiniNode nodes[128];
    uint64_t seen[128];
    int seen_count = 0;
    int head = 0;
    int tail = 0;

    nodes[tail].zero = (unsigned char)start_zero;
    memcpy(nodes[tail].tiles, start_tiles, sizeof(start_tiles));
    nodes[tail].parent = -1;
    nodes[tail].move_dir = -1;

    uint64_t code = ((uint64_t)start_tiles[0]) | ((uint64_t)start_tiles[1] << 16) |
                    ((uint64_t)start_tiles[2] << 32) | ((uint64_t)start_tiles[3] << 48);
    seen[seen_count++] = code;
    tail++;

    int solution_index = -1;

    while (head < tail)
    {
        MiniNode current = nodes[head];
        bool match = true;
        for (int i = 0; i < 4; ++i)
        {
            if (current.tiles[i] != target_tiles[i])
            {
                match = false;
                break;
            }
        }
        if (match)
        {
            solution_index = head;
            break;
        }

        for (int k = 0; k < neighbor_counts[current.zero]; ++k)
        {
            int next_zero = neighbor_positions[current.zero][k];
            int dir = neighbor_dirs[current.zero][k];
            MiniNode next = current;
            unsigned char temp = next.tiles[next_zero];
            next.tiles[next_zero] = next.tiles[current.zero];
            next.tiles[current.zero] = temp;
            next.zero = (unsigned char)next_zero;
            next.parent = head;
            next.move_dir = dir;

            uint64_t next_code = ((uint64_t)next.tiles[0]) | ((uint64_t)next.tiles[1] << 16) |
                                 ((uint64_t)next.tiles[2] << 32) | ((uint64_t)next.tiles[3] << 48);
            bool already_seen = false;
            for (int i = 0; i < seen_count; ++i)
            {
                if (seen[i] == next_code)
                {
                    already_seen = true;
                    break;
                }
            }
            if (already_seen)
            {
                continue;
            }
            seen[seen_count++] = next_code;
            nodes[tail++] = next;
        }
        head++;
    }

    if (solution_index == -1)
    {
        return false;
    }

    int moves[64];
    int move_count = 0;
    for (int idx = solution_index; idx != -1; idx = nodes[idx].parent)
    {
        if (nodes[idx].move_dir != -1)
        {
            moves[move_count++] = nodes[idx].move_dir;
        }
    }

    for (int i = move_count - 1; i >= 0; --i)
    {
        perform_move(p, moves[i], log);
    }

    return true;
}

static bool solve_puzzle(Puzzle *p, FILE *log)
{
    int size = p->size;
    for (int row = 0; row < size - 2; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            int target_index = row * size + col;
            int expected = target_index + 1;
            if (!place_tile(p, expected, target_index, log))
            {
                return false;
            }
            p->locked[target_index] = true;
        }
    }

    for (int col = 0; col < size - 2; ++col)
    {
        for (int row = size - 2; row < size; ++row)
        {
            int target_index = row * size + col;
            int expected = target_index + 1;
            if (!place_tile(p, expected, target_index, log))
            {
                return false;
            }
            p->locked[target_index] = true;
        }
    }

    return solve_last_block(p, log);
}

static bool is_solvable(const int *state, int size)
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
        return (inversions % 2) == 0;
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

    if ((zero_row_from_bottom % 2) == 0)
    {
        return (inversions % 2) == 1;
    }
    return (inversions % 2) == 0;
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
        fprintf(stderr, "Supported puzzle sizes: 2 to %d.\n", MAX_SIZE);
        return 1;
    }

    Puzzle puzzle;
    puzzle.size = size;
    puzzle.total = size * size;
    memset(puzzle.locked, 0, sizeof(puzzle.locked));

    for (int i = 0; i < puzzle.total; ++i)
    {
        if (scanf("%d", &puzzle.state[i]) != 1)
        {
            fprintf(stderr, "Invalid input.\n");
            return 1;
        }
        if (puzzle.state[i] == 0)
        {
            puzzle.zero_index = i;
        }
    }

    if (!is_solvable(puzzle.state, size))
    {
        fprintf(stderr, "This puzzle is not solvable.\n");
        return 0;
    }

    FILE *log = fopen("log.txt", "w");
    if (!log)
    {
        fprintf(stderr, "Unable to open log.txt for writing.\n");
        return 1;
    }

    write_board(log, puzzle.state, size);

    if (!solve_puzzle(&puzzle, log))
    {
        fclose(log);
        fprintf(stderr, "Failed to construct a solution.\n");
        return 1;
    }

    fclose(log);
    printf("Solution written to log.txt\n");
    return 0;
}
