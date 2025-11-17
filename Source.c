#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SIZE 3
#define MAX_TILES (MAX_SIZE * MAX_SIZE)
#define MAX_STATES 400000
#define HASH_SIZE 1048576

typedef struct {
    int prev;
    int move;
} Node;

typedef struct {
    unsigned long long key;
    int value;
    bool used;
} HashEntry;

static unsigned char *states;
static Node *nodes;
static int *queue_indices;
static HashEntry *hash_table;

static unsigned long long encode_state(const unsigned char *state, int total)
{
    unsigned long long key = 0ULL;
    for (int i = 0; i < total; ++i)
    {
        key = (key << 5) + state[i];
    }
    return key;
}

static int hash_lookup(unsigned long long key)
{
    unsigned long long idx = key % HASH_SIZE;
    while (hash_table[idx].used)
    {
        if (hash_table[idx].key == key)
        {
            return hash_table[idx].value;
        }
        idx = (idx + 1) % HASH_SIZE;
    }
    return -1;
}

static void hash_insert(unsigned long long key, int value)
{
    unsigned long long idx = key % HASH_SIZE;
    while (hash_table[idx].used)
    {
        idx = (idx + 1) % HASH_SIZE;
    }
    hash_table[idx].used = true;
    hash_table[idx].key = key;
    hash_table[idx].value = value;
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

static void print_board(const int *state, int size)
{
    for (int r = 0; r < size; ++r)
    {
        printf("    ");
        for (int c = 0; c < size; ++c)
        {
            printf("%4d", state[r * size + c]);
        }
        printf("\n");
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

static int bfs(const unsigned char *start_state, unsigned long long goal_key, int size)
{
    int total = size * size;
    memcpy(states, start_state, total);
    nodes[0].prev = -1;
    nodes[0].move = -1;
    queue_indices[0] = 0;
    int front = 0;
    int back = 1;
    int state_count = 1;
    unsigned long long start_key = encode_state(start_state, total);
    hash_insert(start_key, 0);
    const int moves[4][2] = {
        {-1, 0}, // up
        {1, 0},  // down
        {0, -1}, // left
        {0, 1}   // right
    };
    while (front < back)
    {
        int current_index = queue_indices[front++];
        unsigned char *current_state = states + current_index * total;
        unsigned long long current_key = encode_state(current_state, total);
        if (current_key == goal_key)
        {
            return current_index;
        }
        int zero_index = 0;
        for (int i = 0; i < total; ++i)
        {
            if (current_state[i] == 0)
            {
                zero_index = i;
                break;
            }
        }
        int row = zero_index / size;
        int col = zero_index % size;
        for (int dir = 0; dir < 4; ++dir)
        {
            int new_row = row + moves[dir][0];
            int new_col = col + moves[dir][1];
            if (new_row < 0 || new_row >= size || new_col < 0 || new_col >= size)
            {
                continue;
            }
            unsigned char temp[ MAX_TILES ];
            memcpy(temp, current_state, total);
            int swap_index = new_row * size + new_col;
            unsigned char t = temp[zero_index];
            temp[zero_index] = temp[swap_index];
            temp[swap_index] = t;
            unsigned long long key = encode_state(temp, total);
            if (hash_lookup(key) != -1)
            {
                continue;
            }
            if (state_count >= MAX_STATES)
            {
                return -1;
            }
            unsigned char *dest = states + state_count * total;
            memcpy(dest, temp, total);
            nodes[state_count].prev = current_index;
            nodes[state_count].move = dir;
            hash_insert(key, state_count);
            queue_indices[back++] = state_count;
            if (key == goal_key)
            {
                return state_count;
            }
            state_count++;
        }
    }
    return -1;
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
    for (int i = 0; i < total; ++i)
    {
        int value;
        if (scanf("%d", &value) != 1)
        {
            fprintf(stderr, "Invalid input.\n");
            return 1;
        }
        initial[i] = (unsigned char)value;
    }
    unsigned char goal_state[MAX_TILES];
    for (int i = 0; i < total - 1; ++i)
    {
        goal_state[i] = (unsigned char)(i + 1);
    }
    goal_state[total - 1] = 0;
    unsigned long long goal_key = encode_state(goal_state, total);

    if (!is_solvable(initial, size))
    {
        printf("Initial state:\n");
        int printable[MAX_TILES];
        for (int i = 0; i < total; ++i)
        {
            printable[i] = initial[i];
        }
        print_board(printable, size);
        printf("\nThis puzzle is not solvable.\n");
        return 0;
    }

    states = calloc(MAX_STATES, total);
    nodes = calloc(MAX_STATES, sizeof(Node));
    queue_indices = calloc(MAX_STATES, sizeof(int));
    hash_table = calloc(HASH_SIZE, sizeof(HashEntry));
    if (!states || !nodes || !queue_indices || !hash_table)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    int solution_index = bfs(initial, goal_key, size);
    if (solution_index < 0)
    {
        fprintf(stderr, "Failed to find a solution.\n");
        return 1;
    }

    int *moves = malloc(MAX_STATES * sizeof(int));
    int move_count = 0;
    int current = solution_index;
    while (current != -1 && nodes[current].move != -1)
    {
        moves[move_count++] = nodes[current].move;
        current = nodes[current].prev;
    }

    int current_state[MAX_TILES];
    for (int i = 0; i < total; ++i)
    {
        current_state[i] = initial[i];
    }

    const char *direction_words[] = {"up", "down", "left", "right"};

    printf("Initial state:\n");
    print_board(current_state, size);
    printf("\n");

    for (int i = move_count - 1; i >= 0; --i)
    {
        int dir = moves[i];
        printf("Move a tile %s.\n", direction_words[dir]);
        apply_move(current_state, size, dir);
        print_board(current_state, size);
        printf("\n");
    }

    free(states);
    free(nodes);
    free(queue_indices);
    free(hash_table);
    free(moves);

    return 0;
}
