#ifndef DRAGON_H
#define DRAGON_H

#define TRUE	1u
#define FALSE	0u

#define DRAGON_ARGNO	2u

#define DRAGON_MAX_ITERATIONS	30u

#define DRAGON_TURN_LEFT	0u
#define DRAGON_TURN_RIGHT	1u

#define DRAGON_DIRECTION_UP		1u
#define DRAGON_DIRECTION_DOWN	2u
#define DRAGON_DIRECTION_LEFT	3u
#define DRAGON_DIRECTION_RIGHT	4u

typedef struct dragon_Coordinates_st dragon_Coordinates_t;

void dragon_Initialize(int argc, char *argv[]);
void dragon_GenerateCurve(void);
void dragon_Terminate(void);

#endif