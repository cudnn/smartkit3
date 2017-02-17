#include "light_util.h"

namespace util {

/**
 * [get_block_2D]
 * @param input             [data]
 * @param block2DY_rotation [2D blocks after rotation]
 * @param BLOCK_SIDE        [7]
 */
// void get_block_2D(const sv::Image input, float** block2DY_rotation, const int BLOCK_SIDE) {

// 	float block2DY[BLOCK_SIDE][BLOCK_SIDE];

// 	memset(block2DY, 0, sizeof(block2DY)); // initialize to 0
// 	memset(block2DYRotation, 0, sizeof(block2DY)); // initialize to 0

// 	int stepx = DownsampleCodeWidth / BLOCK_SIDE;
// 	int stepy = DownsampleCodeHeight / BLOCK_SIDE;

// 	for (int i = 0; i < BLOCK_SIDE * stepy; i++) {
// 		for (int j = 0; j < BLOCK_SIDE * stepx; j += 3) {
// 			int binx = j / stepx;
// 			int biny = i / stepy;
// 			block2DY[biny][binx] += code_data[i * DownsampleCodeWidth + j] & 0xff;
// 		}
// 	}

// 	int block2DY_len = sizeof(block2DY) / sizeof(float);
// 	int block2DY_cols = sizeof(block2DY[0]) / sizeof(float);
// 	int block2DY_rows = block2DY_len / block2DY_cols;

// 	for (int i = 0; i < BLOCK_SIDE * BLOCK_SIDE; i++) {
// 		block2DY[i / BLOCK_SIDE][i % BLOCK_SIDE] /= stepx * stepy * (256 - 1)
// 				/ 3;
// 		block2DYRotation[i % BLOCK_SIDE][block2DY_rows - i / BLOCK_SIDE - 1] = block2DY[i
// 				/ BLOCK_SIDE][i % BLOCK_SIDE];
// 	}
// }

/**
 * [get_block_1D] output 1D array blocks
 * @param input      [data]
 * @param block1D    [1D blocks]
 * @param BLOCK_SIDE [7]
 */
void get_block_1D(const smart::Image* input, float* block1D, const int BLOCK_SIDE) {
	int width = input->width;
	int height = input->height;
	unsigned char* data = new unsigned char[width * height];
	memcpy(data, input->data, width * height);


	float block2DY[BLOCK_SIDE][BLOCK_SIDE];
	float block2DYRotation[BLOCK_SIDE][BLOCK_SIDE];

	memset(block2DY, 0, sizeof(block2DY)); // initialize to 0
	memset(block2DYRotation, 0, sizeof(block2DY)); // initialize to 0

	int stepx = width / BLOCK_SIDE;
	int stepy = height / BLOCK_SIDE;

	for (int i = 0; i < BLOCK_SIDE * stepy; i++) {
		for (int j = 0; j < BLOCK_SIDE * stepx; j += 3) {
			int binx = j / stepx;
			int biny = i / stepy;
			block2DY[biny][binx] += data[i * width + j] & 0xff;
		}
	}

	int block2DY_len = sizeof(block2DY) / sizeof(float);
	int block2DY_cols = sizeof(block2DY[0]) / sizeof(float);
	int block2DY_rows = block2DY_len / block2DY_cols;
	for (int i = 0; i < BLOCK_SIDE * BLOCK_SIDE; i++) {
		block2DY[i / BLOCK_SIDE][i % BLOCK_SIDE] /= stepx * stepy * (256 - 1)
				/ 3;
		block2DYRotation[i % BLOCK_SIDE][block2DY_rows - i / BLOCK_SIDE - 1] = block2DY[i
				/ BLOCK_SIDE][i % BLOCK_SIDE];
	}

	//2D blocks -> 1D blocks
	for (int j = 0; j < BLOCK_SIDE; j++) {
		for (int i = 0; i < BLOCK_SIDE; i++) {
			block1D[j * BLOCK_SIDE + i] = block2DYRotation[j][i];
		}
	}
}

} //namespace util