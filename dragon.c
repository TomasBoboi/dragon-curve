#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"
#include "bmptools.h"
#include "dragon.h"

static void dragon_CheckArgs(int argc, char *argv[]);
static void dragon_InitIterations(int argc, char *argv[]);
static void dragon_InitPosition(void);
static void dragon_InitImageDimensions(void);
static void dragon_AllocatePixelData(void);
static void dragon_InitPixelData(void);
static void dragon_OpenOutputFile(void);

static uint8_t dragon_ComputeNextDirection(uint8_t currentDir_u8, uint8_t turn_u8);
static dragon_Coordinates_t dragon_ApplyDirection(dragon_Coordinates_t currentCoords_t, uint8_t direction_u8);
static void dragon_colorPixel(dragon_Coordinates_t currentCoords_t);

struct dragon_Coordinates_st {
	int32_t x_s32;
	int32_t y_s32;
};

const int32_t dragon_Step_s32 = 2;

const int32_t dragon_ImageDimensionLookup_ms32[DRAGON_MAX_ITERATIONS + 1][2] = {
	/*       {width, height} */
	/* x  */ {0, 0},
	/* 1  */ {3, 3},
	/* 2  */ {3, 5},
	/* 3  */ {7, 5},
	/* 4  */ {11, 7},
	/* 5  */ {13, 15},
	/* 6  */ {15, 23},
	/* 7  */ {31, 25},
	/* 8  */ {47, 31},
	/* 9  */ {53, 63},
	/* 10 */ {63, 95},
	/* 11 */ {127, 105},
	/* 12 */ {191, 127},
	/* 13 */ {213, 255},
	/* 14 */ {255, 383},
	/* 15 */ {511, 425},
	/* 16 */ {767, 511},
	/* 17 */ {853, 1023},
	/* 18 */ {1023, 1535},
	/* 19 */ {2047, 1705},
	/* 20 */ {3071, 2047},
	/* 21 */ {3413, 4095},
	/* 22 */ {4095, 6143},
	/* 23 */ {8191, 6825},
	/* 24 */ {12287, 8191},
	/* 25 */ {13653, 16383},
	/* 26 */ {16383, 24575},
	/* 27 */ {32767, 27305},
	/* 28 */ {49151, 32767},
	/* 29 */ {54613, 65535},
	/* 30 */ {65535, 98303}
};

const int32_t dragon_ImageStartingPointLookup_ms32[DRAGON_MAX_ITERATIONS + 1][2] = {
	/*       {x, y} */
	/* x  */ {0, 0},
	/* 1  */ {2, 2},
	/* 2  */ {4, 2},
	/* 3  */ {4, 2},
	/* 4  */ {4, 2},
	/* 5  */ {4, 2},
	/* 6  */ {4, 4},
	/* 7  */ {4, 20},
	/* 8  */ {10, 36},
	/* 9  */ {42, 42},
	/* 10 */ {74, 42},
	/* 11 */ {84, 42},
	/* 12 */ {84, 42},
	/* 13 */ {84, 42},
	/* 14 */ {84, 84},
	/* 15 */ {84, 340},
	/* 16 */ {170, 596},
	/* 17 */ {682, 682},
	/* 18 */ {1194, 682},
	/* 19 */ {1364, 682},
	/* 20 */ {1364, 682},
	/* 21 */ {1364, 682},
	/* 22 */ {1364, 1364},
	/* 23 */ {1364, 5460},
	/* 24 */ {2730, 9556},
	/* 25 */ {10922, 10922},
	/* 26 */ {19114, 10922},
	/* 27 */ {21844, 10922},
	/* 28 */ {21844, 10922},
	/* 29 */ {21844, 10922},
	/* 30 */ {21844, 21844}
};

int dragon_OutputFileDescriptor_fd;

int32_t dragon_ImageWidth_s32;
int32_t dragon_ImageHeight_s32;
uint8_t **dragon_PixelData_ppu8;

int32_t dragon_NumberOfIterations_s32;
int32_t dragon_Iterations2N_s32;
int32_t dragon_CurrentIterationNumber_s32;

dragon_Coordinates_t dragon_CurrentCoordinate_t;
uint8_t dragon_CurrentDirection_u8;

int main(int argc, char *argv[])
{
	dragon_Initialize(argc, argv);

	dragon_GenerateCurve();

	bmp_WriteImage(dragon_OutputFileDescriptor_fd, dragon_PixelData_ppu8, dragon_ImageWidth_s32, dragon_ImageHeight_s32);

	dragon_Terminate();

	return 0;
}

void dragon_Initialize(int argc, char *argv[])
{
	printf("Initializing...\n");

	printf("\t");
	dragon_CheckArgs(argc, argv);

	printf("\t");
	dragon_InitIterations(argc, argv);

	printf("\t");
	dragon_InitPosition();

	printf("\t");
	dragon_InitImageDimensions();

	printf("\t");
	dragon_AllocatePixelData();

	printf("\t");
	dragon_InitPixelData();

	printf("\t");
	dragon_OpenOutputFile();
}

static void dragon_CheckArgs(int argc, char *argv[])
{
	printf("Checking arguments... ");

	if(DRAGON_ARGNO != argc)
		utils_ErrorMessage("not enough arguments");
	
	printf("done\n");
}

static void dragon_InitIterations(int argc, char *argv[])
{
	printf("Initializing iterations... ");

	dragon_NumberOfIterations_s32 = atoi(argv[1]);
	dragon_Iterations2N_s32 = 1 << dragon_NumberOfIterations_s32;

	dragon_CurrentIterationNumber_s32 = 1;

	printf("done\n");
}

static void dragon_InitPosition(void)
{
	printf("Initializing the position... ");

	dragon_CurrentCoordinate_t.x_s32 = dragon_ImageStartingPointLookup_ms32[dragon_NumberOfIterations_s32][0];
	dragon_CurrentCoordinate_t.y_s32 = dragon_ImageStartingPointLookup_ms32[dragon_NumberOfIterations_s32][1];

	dragon_CurrentDirection_u8 = DRAGON_DIRECTION_UP;

	printf("done\n");
}

static void dragon_InitImageDimensions(void)
{
	printf("Initializing the image dimensions... ");

	dragon_ImageWidth_s32 = dragon_ImageDimensionLookup_ms32[dragon_NumberOfIterations_s32][0];
	dragon_ImageHeight_s32 = dragon_ImageDimensionLookup_ms32[dragon_NumberOfIterations_s32][1];

	printf("done\n");
}

static void dragon_AllocatePixelData(void)
{
	printf("Allocating the pixel data...");

	dragon_PixelData_ppu8 = (uint8_t **)malloc(dragon_ImageHeight_s32 * sizeof(uint8_t *));

	for(int32_t count_s32 = 0; count_s32 < dragon_ImageHeight_s32; count_s32++)
		dragon_PixelData_ppu8[count_s32] = (uint8_t *)malloc(dragon_ImageWidth_s32 * sizeof(uint8_t));

	printf("done\n");
}

static void dragon_InitPixelData(void)
{
	printf("Initializing the pixel data... ");

	for(int32_t row_s32 = 0; row_s32 < dragon_ImageHeight_s32; row_s32++)
		for(int32_t column_s32 = 0; column_s32 < dragon_ImageWidth_s32; column_s32++)
			dragon_PixelData_ppu8[row_s32][column_s32] = BMP_COLOR_WHITE;
	
	printf("done\n");
}

static void dragon_OpenOutputFile(void)
{
	printf("Opening the output file... ");

	dragon_OutputFileDescriptor_fd = open("output.bmp", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

	if(dragon_OutputFileDescriptor_fd == -1)
		utils_ErrorMessage("opening the output file");
	
	printf("done\n");
}

void dragon_GenerateCurve(void)
{
	printf("Generating the curve... \n");

	int32_t iteration_s32;
	uint8_t turn_u8;
	
	dragon_CurrentCoordinate_t = dragon_ApplyDirection(dragon_CurrentCoordinate_t, dragon_CurrentDirection_u8);

	for(iteration_s32 = 1; iteration_s32 < dragon_Iterations2N_s32; iteration_s32++)
	{
		turn_u8 = (((iteration_s32 & -iteration_s32) << 1) & iteration_s32) != 0;
		dragon_CurrentDirection_u8 = dragon_ComputeNextDirection(dragon_CurrentDirection_u8, turn_u8);
		dragon_CurrentCoordinate_t = dragon_ApplyDirection(dragon_CurrentCoordinate_t, dragon_CurrentDirection_u8);
		if(iteration_s32 == dragon_Iterations2N_s32 / 4)
			printf("\t25%%\n");
		else if(iteration_s32 == dragon_Iterations2N_s32 / 2)
			printf("\t50%%\n");
		else if(iteration_s32 == 3 * dragon_Iterations2N_s32 / 4)
			printf("\t75%%\n");
	}
	dragon_colorPixel(dragon_CurrentCoordinate_t);
	printf("\t100%%\n");
}

static uint8_t dragon_ComputeNextDirection(uint8_t currentDir_u8, uint8_t turn_u8)
{
	uint8_t result_u8;

	if(DRAGON_TURN_LEFT == turn_u8)
	{
		if(DRAGON_DIRECTION_UP == currentDir_u8)
			result_u8 = DRAGON_DIRECTION_LEFT;
		else if(DRAGON_DIRECTION_DOWN == currentDir_u8)
			result_u8 = DRAGON_DIRECTION_RIGHT;
		else if(DRAGON_DIRECTION_LEFT == currentDir_u8)
			result_u8 = DRAGON_DIRECTION_DOWN;
		else if(DRAGON_DIRECTION_RIGHT == currentDir_u8)
			result_u8 = DRAGON_DIRECTION_UP;
		else
		{
			/* do nothing */
		}
	}
	else if(DRAGON_TURN_RIGHT == turn_u8)
	{
		if(DRAGON_DIRECTION_UP == currentDir_u8)
			result_u8 = DRAGON_DIRECTION_RIGHT;
		else if(DRAGON_DIRECTION_DOWN == currentDir_u8)
			result_u8 = DRAGON_DIRECTION_LEFT;
		else if(DRAGON_DIRECTION_LEFT == currentDir_u8)
			result_u8 = DRAGON_DIRECTION_UP;
		else if(DRAGON_DIRECTION_RIGHT == currentDir_u8)
			result_u8 = DRAGON_DIRECTION_DOWN;
		else
		{
			/* do nothing */
		}
	}
	else
	{
		/* do nothing */
	}

	return result_u8;
}

static dragon_Coordinates_t dragon_ApplyDirection(dragon_Coordinates_t currentCoords_t, uint8_t direction_u8)
{
	dragon_Coordinates_t result_t;
	int32_t count_s32 = dragon_Step_s32;

	result_t.x_s32 = currentCoords_t.x_s32;
	result_t.y_s32 = currentCoords_t.y_s32;
	
	while(count_s32 > 0)
	{
		dragon_colorPixel(result_t);

		switch(direction_u8)
		{
			case DRAGON_DIRECTION_UP:
				result_t.y_s32--;
				break;
			case DRAGON_DIRECTION_DOWN:
				result_t.y_s32++;
				break;
			case DRAGON_DIRECTION_LEFT:
				result_t.x_s32--;
				break;
			case DRAGON_DIRECTION_RIGHT:
				result_t.x_s32++;
				break;
			
			default:
				utils_ErrorMessage("invalid direction");
				break;
		}

		count_s32--;
	}

	return result_t;
}

static void dragon_colorPixel(dragon_Coordinates_t currentCoords_t)
{
	dragon_PixelData_ppu8[currentCoords_t.x_s32][currentCoords_t.y_s32] = BMP_COLOR_BLACK;
}

void dragon_Terminate(void)
{
	printf("Terminating program... ");

	for(int32_t row_s32 = 0; row_s32 < dragon_ImageHeight_s32; row_s32++)
		free(dragon_PixelData_ppu8[row_s32]);
	free(dragon_PixelData_ppu8);

	printf("done\n");
}