#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#define BMP_HEADER_OFFSET 54
#define FILE_SIZE_OFFSET 2
#define FILE_WIDTH_OFFSET 18
#define FILE_HEIGHT_OFFSET 22
#define RGB_TRIPLE 3
#define CONST_POWER 100

char **colors;

double *bs_power;

typedef struct point {
	int x;
	int y;
} point;

char *create_header(const char *dummy_filename, int height, int width) {
	char *header_bytes = (char*) malloc(BMP_HEADER_OFFSET);
	FILE *file = fopen(dummy_filename, "r");
	fread(header_bytes, 1, BMP_HEADER_OFFSET, file);
	fclose(file);
	int filesize = height * width * RGB_TRIPLE + BMP_HEADER_OFFSET;
	memcpy(header_bytes + FILE_SIZE_OFFSET, &filesize, sizeof(int32_t));
	memcpy(header_bytes + FILE_WIDTH_OFFSET, &width, sizeof(int32_t));
	memcpy(header_bytes + FILE_HEIGHT_OFFSET, &height, sizeof(int32_t));
	return header_bytes;
}

void init_equal_bs_power(double *bs_power, int bs_count) {
	for (int i = 0; i < bs_count; ++i)
		bs_power[i] = CONST_POWER;
}

point *get_rand_bs_coords(int height, int width, int bs_count) {
	point *bs_coords = (point*) malloc(bs_count * sizeof(point));
	for (int i = 0; i < bs_count; ++i) {
		(bs_coords[i]).x = rand() % width;
		(bs_coords[i]).y = rand() % height;
	}
	return bs_coords;
}

point *read_bs_coords_and_power(FILE *file, int bs_count,
			   					double *bs_power) {
	point *bs_coords = (point*) malloc(bs_count * sizeof(point));
	for (int i = 0; i < bs_count; ++i) {
		fscanf(file, "(%d ; %d) %lf\n", &((bs_coords[i]).x),
					   				&((bs_coords[i]).y),
									bs_power + i);
	}
	return bs_coords;
}

void paint_bs(FILE *board, point *bs_coords, int height, int width,
			  int bs_count) {
	int offset;
	for (int i = 0; i < bs_count; ++i) {
		offset = ((bs_coords[i]).y * width + (bs_coords[i]).x) * RGB_TRIPLE;
		fseek(board, BMP_HEADER_OFFSET + offset, SEEK_SET);
		fputc(0, board);
		fputc(0, board);
		fputc(0, board);
	}
}
/* returns index of nearest bs */
int get_nearest_bs(point *cur_point, point *bs_coords, int bs_count) {
	int argmax = 0;
	int dist, max_power = 0;
	double power;
	for (int i = 0; i < bs_count; ++i) {
		dist = (cur_point->x - (bs_coords[i]).x) * 
				(cur_point->x - (bs_coords[i]).x) +
				(cur_point->y - (bs_coords[i]).y) * 
				(cur_point->y - (bs_coords[i]).y);
		power = bs_power[i] / (double) dist;
		if (power - max_power > 0.1) {
			max_power = power;
			argmax = i;
		}
	}
	return argmax;
}

void paint_pixel(FILE *board, int offset, int color_idx) {
	fseek(board, offset + BMP_HEADER_OFFSET, SEEK_SET);
	fwrite(colors[color_idx], sizeof(char), RGB_TRIPLE, board);
}

void paint_zones(FILE *board, int height, int width, point *bs_coords,
				int bs_count) {
	point cur_point;
	int color_idx, offset;
	for (int i = 0; i < height; ++i) {
		cur_point.y = i;
		for (int j = 0; j < width; ++j) {
			offset = (i * width + j) * RGB_TRIPLE;
			cur_point.x = j;
			color_idx = get_nearest_bs(&cur_point, bs_coords, bs_count);
			paint_pixel(board, offset, color_idx);
		}
	}
}

int main(int argc, char **argv) {
	srand(time(NULL));
	char *dummy_filename = *(argv + 1);
	int height = atoi(*(argv + 2)),
		width = atoi(*(argv + 3)),
		bs_count = atoi(*(argv + 4));
	bs_power = (double*) malloc(bs_count * sizeof(double));
	FILE *bs_param = NULL;
	point *bs_coords;
	if (strcmp(*(argv + 5), "-rand") == 0) {
		bs_coords = get_rand_bs_coords(height, width, bs_count);
		init_equal_bs_power(bs_power, bs_count);
	}
	else {
		bs_param = fopen(*(argv + 5), "r");
		bs_coords = read_bs_coords_and_power(bs_param, bs_count, bs_power);
		fclose(bs_param);
	}
	char *header_bytes = create_header(dummy_filename, height, width);
	FILE *board = fopen("board.bmp", "w");
	fwrite(header_bytes, BMP_HEADER_OFFSET, 1, board);
	for (int i = 0; i < height * width * RGB_TRIPLE; ++i)
		fputc(255, board);
	free(header_bytes);
	for (int i = 0; i < bs_count; ++i)
		printf("%d %d %lf\n", (bs_coords[i]).x, (bs_coords[i]).y, 
												bs_power[i]);
	colors = (char**) malloc(bs_count * sizeof(char*));
	for (int i = 0; i < bs_count; ++i) {
		colors[i] = (char*) malloc(RGB_TRIPLE * sizeof(char));
		for (int j = 0; j < RGB_TRIPLE; ++j)
			colors[i][j] = rand() % 256;
	}
	paint_zones(board, height, width, bs_coords, bs_count);
	paint_bs(board, bs_coords, height, width, bs_count);
	fclose(board);
	free(bs_power);
	free(bs_coords);
	for (int i = 0; i < bs_count; ++i)
		free(colors[i]);
	free(colors);
	return 0;
}
