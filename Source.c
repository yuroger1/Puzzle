#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#define puzzle_maximum_size 3//puzzle size
int stack[puzzle_maximum_size][puzzle_maximum_size];//puzzle size
int outtype = 0;
FILE* save_data;
void fbr(FILE* br)
{
	fscanf(br, "%*[^\n]");
	fscanf(br, "%*c");
}
void get_puzzle(FILE* data, int size)
{
	for (int i = 0; i != size; i++)
	{
		for (int j = 0; j != size; j++)
		{
			int tmp;
			fscanf(data, "%d", &tmp);
			fscanf(data, "%*c");
			stack[i][j] = tmp;
		}
	}
}
void push_puzzle(FILE* data, int size)
{
	if (outtype == 0)
	{
		fprintf(data, "= = = = = = = = = = = = = = = = = = = = = = = = = = =\n");
		for (int i = 0; i != size; i++)
		{
			for (int j = 0; j != size; j++)
			{
				int tmp = stack[i][j];
				!tmp ? fprintf(data, "     ") : fprintf(data, "%5d", stack[i][j]);
			}
			fprintf(data, "\n");
		}
	}
	if (outtype == 1)
	{
		fprintf(save_data, "= = = = = = = = = = = = = = = = = = = = = = = = = = =\n");
		for (int i = 0; i != size; i++)
		{
			for (int j = 0; j != size; j++)
			{
				int tmp = stack[i][j];
				!tmp ? fprintf(save_data, "     ") : fprintf(save_data, "%5d", stack[i][j]);
			}
			fprintf(save_data, "\n");
		}
	}
}
void targect_e(int find, int size, int distance[2])
{
	distance[0] = find / size;
	distance[1] = find % size;
	distance[1] --;
}
void find(int var, int size, int zero[2])
{
	for (int i = 0; i != size; i++)
	{
		for (int j = 0; j != size; j++)
		{
			if (stack[i][j] == var)
			{
				zero[0] = i;
				zero[1] = j;
			}
		}
	}
}
void swap(int action, int size)
{
	int zero[2];
	find(0, size, zero);
	int act = 0;
	int tmp;
	if (action == 56 || action == 1)//8
	{
		if (zero[0] == 0)
		{
			return 1;
		}
		tmp = stack[zero[0]][zero[1]];
		stack[zero[0]][zero[1]] = stack[zero[0] - 1][zero[1]];
		stack[zero[0] - 1][zero[1]] = tmp;
	}
	if (action == 53 || action == 2)//5
	{
		if (zero[0] == size - 1)
		{
			return 1;
		}
		tmp = stack[zero[0]][zero[1]];
		stack[zero[0]][zero[1]] = stack[zero[0] + 1][zero[1]];
		stack[zero[0] + 1][zero[1]] = tmp;
	}
	if (action == 52 || action == 3)//4
	{
		if (zero[1] == 0)
		{
			return 1;
		}
		tmp = stack[zero[0]][zero[1]];
		stack[zero[0]][zero[1]] = stack[zero[0]][zero[1] - 1];
		stack[zero[0]][zero[1] - 1] = tmp;
	}
	if (action == 54 || action == 4)//6
	{
		if (zero[1] == size - 1)
		{
			return 1;
		}
		find(0, size, zero);
		tmp = stack[zero[0]][zero[1]];
		stack[zero[0]][zero[1]] = stack[zero[0]][zero[1] + 1];
		stack[zero[0]][zero[1] + 1] = tmp;
	}
	push_puzzle(stdout, size);
}
void go_to_r(int size, int column, int row)
{
	for (; row != 0;)
	{
		if (row > 0)
		{
			swap(4, size);
			row--;
		}
		if (row < 0)
		{
			swap(3, size);
			row++;
		}
	}
	for (; column != 0;)
	{
		if (column < 0)
		{
			swap(2, size);
			column++;
		}
		if (column > 0)
		{
			swap(1, size);
			column--;
		}
	}
}
void go_to_c(int size, int column, int row)
{
	for (; column != 0;)
	{
		if (column < 0)
		{
			swap(2, size);
			column++;
		}
		if (column > 0)
		{
			swap(1, size);
			column--;
		}
	}
	for (; row != 0;)
	{
		if (row > 0)
		{
			swap(4, size);
			row--;
		}
		if (row < 0)
		{
			swap(3, size);
			row++;
		}
	}
}
void swap_l(int size)
{
	int zero[2];
	find(0, size, zero);
	if (zero[0] == size - 1)
	{
		swap(1, size);
		swap(3, size);
		swap(3, size);
		swap(2, size);
		swap(4, size);
	}
	else
	{
		swap(2, size);
		swap(3, size);
		swap(3, size);
		swap(1, size);
		swap(4, size);
	}
}
void swap_r(int size)
{
	int zero[2];
	find(0, size, zero);
	if (zero[0] == size - 1)
	{
		swap(1, size);
		swap(4, size);
		swap(4, size);
		swap(2, size);
		swap(3, size);
	}
	else
	{
		swap(2, size);
		swap(4, size);
		swap(4, size);
		swap(1, size);
		swap(3, size);
	}
}
void swap_u(int size)
{
	swap(4, size);
	swap(1, size);
	swap(1, size);
	swap(3, size);
	swap(2, size);
}
void swap_d(int size)
{
	swap(4, size);
	swap(2, size);
	swap(2, size);
	swap(3, size);
	swap(1, size);
}
void move_r(int var, int size)
{
	int distance[2];
	int zero[2];
	int source[2];
	int moving;
	//debug
	targect_e(var, size, distance);
	find(0, size, zero);
	find(var, size, source);
	moving = distance[1] - source[1];
	if (moving > 0)
	{
		if (zero[0] == source[0] && zero[1] <= source[1] && moving != 0)
		{
			swap(2, size);
			targect_e(var, size, distance);
			find(0, size, zero);
			find(var, size, source);
		}
		int a = zero[0] - source[0];
		int b = source[1] - zero[1] + 1;
		go_to_r(size, a, b);
		swap(3, size);
		moving--;
	}
	if (moving < 0)
	{
		if (zero[0] == source[0] && zero[1] >= source[1] && moving != 0)
		{
			swap(2, size);
			targect_e(var, size, distance);
			find(0, size, zero);
			find(var, size, source);
		}
		go_to_r(size, zero[0] - source[0], (source[1] - 1) - zero[1]);
		swap(4, size);
		moving++;
	}
	for (; moving != 0;)
	{
		if (moving > 0)
		{
			swap_r(size);
			moving--;
		}
		if (moving < 0)
		{
			swap_l(size);
			moving++;
		}
	}
}
void move_c(int var, int size)
{
	int distance[2];
	int zero[2];
	int source[2];
	int moving;
	targect_e(var, size, distance);
	find(0, size, zero);
	find(var, size, source);
	moving = distance[0] - source[0];
	if (moving == 0 && !(distance[0] == source[0]))
	{
		for (; size - 1 == zero[1];)
		{
			swap(4, size);
			targect_e(var, size, distance);
			find(0, size, zero);
			find(var, size, source);
		}
		return 1;
	}
	if (moving == 0)
	{
		swap(2, size);
	}
	if (moving != 0)
	{
		if (zero[0] == source[0])
		{
			if (source[0] != size - 1)
			{
				swap(2, size);
				swap(4, size);
				swap(4, size);
				swap(1, size);
				targect_e(var, size, distance);
				find(0, size, zero);
				find(var, size, source);
				moving = distance[0] - source[0];
			}
			else
			{
				swap(1, size);
				targect_e(var, size, distance);
				find(0, size, zero);
				find(var, size, source);
				moving = distance[0] - source[0];
			}
		}
	}
	if (moving != 0)
	{
		if (zero[1] <= source[1] && zero[0] >= source[0])
		{
			for (; zero[1] != size - 1;)
			{
				swap(4, size);
				find(0, size, zero);
			}
			targect_e(var, size, distance);
			find(var, size, source);
		}
	}
	if (moving > 0)
	{
		go_to_c(size, zero[0] - source[0] - 1, zero[1] - source[1]);
		swap(1, size);
		moving--;
	}
	if (moving < 0)
	{
		go_to_c(size, zero[0] - source[0] + 1, source[1] - zero[1]);
		swap(2, size);
		moving++;
	}
	for (; moving != 0;)
	{
		if (moving > 0)
		{
			swap_d(size);
			moving--;
		}
		if (moving < 0)
		{
			swap_u(size);
			moving++;
		}
	}
}
void push_in_line(int var, int size, int zero[2])
{
	find(0, size, zero);
	int goal[2];
	goal[0] = var / size;
	goal[1] = 0;
	find(0, size, zero);
	go_to_r(size, zero[0] - goal[0], -(zero[1]));
	swap(1, size);
	for (int i = 0; i != size - 2; i++)
	{
		swap(4, size);
	}
	swap(2, size);
	swap(4, size);
	swap(1, size);
	for (int i = 0; i != size - 1; i++)
	{
		swap(3, size);
	}
	swap(2, size);
}
void move_r_size(int var, int size)
{
	swap(2, size);
	int distance[2];
	int zero[2];
	int source[2];
	int moving;
	distance[0] = var / size;
	distance[1] = size - 2;
	find(0, size, zero);
	find(var, size, source);
	moving = distance[1] - source[1];
	if (moving > 0)
	{
		if (zero[0] == source[0] && zero[1] < source[1])
		{
			if (zero[0] == size - 1)
			{
				swap(1, size);
			}
			else
			{
				swap(2, size);
				swap(4, size);
				swap(4, size);
				swap(1, size);
			}
			distance[0] = var / size;
			distance[1] = size - 2;
			find(0, size, zero);
			find(var, size, source);
		}
		int a = zero[0] - source[0];
		int b = -(zero[1] - source[1] - 1);
		go_to_r(size, a, b);
		swap(3, size);
		moving--;
	}
	if (moving < 0)
	{
		if (zero[0] == source[0] && zero[1] > source[1])
		{
			if (zero[0] == size - 1)
			{
				swap(1, size);
			}
			else
			{
				swap(2, size);
				swap(3, size);
				swap(3, size);
				swap(1, size);
			}
			distance[0] = var / size;
			distance[1] = size - 2;
			find(0, size, zero);
			find(var, size, source);
		}
		go_to_r(size, zero[0] - source[0], (source[1] - 1) - zero[1]);
		swap(4, size);
		moving++;
	}
	for (; moving != 0;)
	{
		if (moving > 0)
		{
			swap_r(size);
			moving--;
		}
		if (moving < 0)
		{
			swap_l(size);
			moving++;
		}
	}
}
void move_c_size(int var, int size)
{
	int distance[2];
	int zero[2];
	int source[2];
	int moving;
	distance[0] = var / size;
	distance[1] = size - 2;
	find(0, size, zero);
	find(var, size, source);
	moving = distance[0] - source[0];
	if (zero[0] != size - 1 && source[0] == zero[0])
	{
		swap(2, size);
		swap(4, size);
		swap(4, size);
		swap(1, size);
		targect_e(var, size, distance);
		find(0, size, zero);
		find(var, size, source);
		moving = distance[0] - source[0];
	}
	if (moving > 0)
	{

		go_to_c(size, zero[0] - source[0] - 1, zero[1] - source[1]);
		swap(1, size);
		moving--;
	}
	if (moving < 0)
	{
		if (source[0] < zero[0] && source[1] == zero[1])
		{
			swap(4, size);
			targect_e(var, size, distance);
			find(0, size, zero);
			find(var, size, source);
		}
		go_to_c(size, zero[0] - source[0] + 1, source[1] - zero[1]);
		swap(2, size);
		moving++;
	}
	for (; moving != 0;)
	{
		if (moving > 0)
		{
			swap_d(size);
			moving--;
		}
		if (moving < 0)
		{
			swap_u(size);
			moving++;
		}
	}
	swap(2, size);
}
void norval(int var, int size)
{
	move_r(var, size);
	move_c(var, size);
}
void special(int var, int size)
{
	int distance[2], zero[2], source[2];
	intf("special\n");
	find(0, size, zero);
	find(var, size, source);
	distance[0] = (var / size) - 1;
	distance[1] = size - 1;
	if (distance[0] != source[0] || source[1] != distance[1])
	{
		move_r_size(var, size);
		move_c_size(var, size);
		int zero[2];
		find(0, size, zero);
		push_in_line(var, size, zero);
	}

}
void circle(int size, int n)
{
	swap(1, size);
	for (int i = 0; i != n; i++)
	{
		swap(3, size);
	}
	swap(2, size);
	for (int i = 0; i != n; i++)
	{
		swap(4, size);
	}
}
void circle_rev(int size, int n)
{
	for (int i = 0; i != n; i++)
	{
		swap(3, size);
	}
	swap(1, size);
	for (int i = 0; i != n; i++)
	{
		swap(4, size);
	}
	swap(2, size);
}
void swup_up(int size)
{
	swap(3, size);
	swap(1, size);
	swap(4, size);
	swap(2, size);
	swap(4, size);
	swap(1, size);
	swap(3, size);
	swap(2, size);
	swap(3, size);
	swap(1, size);
	swap(4, size);
	swap(2, size);
	swap(4, size);
	swap(1, size);
	swap(3, size);
	swap(2, size);
	swap(4, size);
	swap(1, size);
	swap(3, size);
	swap(3, size);
	swap(2, size);
	swap(4, size);
	swap(4, size);
	swap(1, size);
	swap(3, size);
	swap(3, size);
	swap(2, size);
	swap(4, size);
}
int main()
{
	printf("Choose output location\n(0: screen 1: log.txt else Do not show)\n?");
	scanf("%d", &outtype);
	//outtype = 0;
	if (outtype == 1)
	{
		save_data = fopen("log.txt", "w");
	}
	FILE* data;
	data = fopen("puzzle.csv", "r");
	int size = 0;
	fscanf(data, "%d", &size);
	if (size > puzzle_maximum_size)
	{
		printf("size is too big");
		return 1;
	}
	fbr(data);
	get_puzzle(data, size);
	fclose(data);
	push_puzzle(stdout, size);
	//define
	for (int var = 1; var != (size * (size - 2)) + 1; var++)
	{
		/*if (var == 15)
		{
			puts(" ");
		}*/
		var% size == 0 ? special(var, size) : norval(var, size);
	}
	int target[3];
	target[0] = (size * (size - 2)) + 1;
	target[1] = target[0] + size;
	target[2] = 0;
	int zero[2], goal[2];
	for (; target[2] != size - 2; target[2]++)
	{
		find(0, size, zero);
		go_to_c(size, 0 - (size - zero[0] - 1), size - zero[1] - 1);
		targect_e(target[0], size, goal);
		for (; target[1] != stack[goal[0]][goal[1]];)
		{
			circle(size, (size - target[2]) - 1);
		}
		if (target[0] != stack[goal[0] + 1][goal[1]])
		{
			for (; target[0] != stack[goal[0]][goal[1] + 1];)
			{
				circle(size, size - target[2] - 2);
			}
			circle_rev(size, size - target[2] - 1);
		}
		else
		{
			for (int i = 0; i != size - target[2] - 2; i++)
			{
				swap(3, size);
			}
			swup_up(size);
			for (int i = 0; i != size - target[2] - 2; i++)
			{
				swap(4, size);
			}
		}

		target[0]++;
		target[1]++;
	}

	for (; stack[size - 2][size - 2] != target[0];)
	{
		swap(1, size);
		swap(3, size);
		swap(2, size);
		swap(4, size);
	}
	if (outtype == 1)
	{
		fclose(save_data);
	}
	system("pause");
	return 0;
}
