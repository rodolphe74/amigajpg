#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#ifndef PIXEL_H
#define PIXEL_H




struct PIXEL_STRUCT {
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
};
typedef struct PIXEL_STRUCT PIXEL;



struct IMAGE_STRUCT {
	unsigned short	width;
	unsigned short	height;
	PIXEL *		pixels;
};
typedef struct IMAGE_STRUCT IMAGE;


struct PIXEL_STRUCT_short {
	short	r;
	short	g;
	short	b;
};
typedef struct PIXEL_STRUCT_short PIXEL_short;

struct PIXEL_STRUCT_WORD {
	short	r;
	short	g;
	short	b;
};
typedef struct PIXEL_STRUCT_WORD PIXEL_WORD;


struct PALETTE_STRUCT {
	unsigned char	colors[32][3];
	unsigned char	size;
	char		name[50];
};
typedef struct PALETTE_STRUCT PALETTE;



struct BUCKET_STRUCT {
	unsigned char **colors;
	short		size;
	unsigned char	mean[3];
};
typedef struct BUCKET_STRUCT BUCKET;


static short BAYER_2_2[2 * 2] = {
	0, 2,
	3, 1
};
static int BAYER_2_2_SIZE[2] = { 2, 2 };



static short BAYER_3_3[3 * 3] = {
	6, 8, 4,
	1, 0, 3,
	5, 2, 7
};
static int BAYER_3_3_SIZE[2] = { 3, 3 };



static short BAYER_4_4[4 * 4] = {
	0,  8,	2,  10,
	12, 4,	14, 6,
	3,  11, 1,  9,
	15, 7,	13, 5
};
static int BAYER_4_4_SIZE[2] = { 4, 4 };



static short BAYER_8_8[8 * 8] = {
	0,  48, 12, 60, 3,  51, 15, 63,
	32, 16, 44, 28, 35, 19, 47, 31,
	8,  56, 4,  52, 11, 59, 7,  55,
	40, 24, 36, 20, 43, 27, 39, 23,
	2,  50, 14, 62, 1,  49, 13, 61,
	34, 18, 46, 30, 33, 17, 45, 29,
	10, 58, 6,  54, 9,  57, 5,  53,
	42, 26, 38, 22, 41, 25, 37, 21
};
static int BAYER_8_8_SIZE[2] = { 8, 8 };



//------------------------------------------------------------------------------
// Trouve une palette adaptee a l'image
//------------------------------------------------------------------------------
void guess_palette(IMAGE *image, PALETTE *palette);


//------------------------------------------------------------------------------
// Trouve une palette adaptee a l'image avec median cut
//------------------------------------------------------------------------------
void guess_palette_median_cut(IMAGE *image, PALETTE *palette, int iter);


//------------------------------------------------------------------------------
// Retrouve la couleur la plus proche par rapport la palette
//------------------------------------------------------------------------------
unsigned char find_closest_color(unsigned char c_current[3],
				 unsigned char palette[32][3],
				 unsigned char palette_size);


//------------------------------------------------------------------------------
// Retrouve la couleur la plus proche par rapport la palette en ham
// Retourne un index de couleur entre 0 et 4095
//------------------------------------------------------------------------------
short find_closest_color_ham(unsigned char c_current[3],
			     int is_c_prec,
			     unsigned char c_prec[3],
			     unsigned char palette[32][3],
			     unsigned char palette_size);


//------------------------------------------------------------------------------
// Redimensionnement de l'image
//------------------------------------------------------------------------------
IMAGE *bilinear_resize(IMAGE *image, unsigned short width, unsigned short height);




//------------------------------------------------------------------------------
// Dither ordonne
// Retourne la matrice precalcule par rapport a la matrice seuil/bayer
//------------------------------------------------------------------------------
void get_precalculated_matrix(short *matrix, int *matrix_size, float *pre_calc_matrix);


//------------------------------------------------------------------------------
// Trouve un seuil optimal par rapport à la palette
//------------------------------------------------------------------------------
void find_threshold(PALETTE * palette, unsigned char threshold[3]);


//------------------------------------------------------------------------------
// Dither ordonne
// Image retournee doit etre liberee par l'appelant
//------------------------------------------------------------------------------
unsigned char *ordered_dither(IMAGE *source, PALETTE *palette, float *matrix, int *matrix_size, void (*f)(char *, int));

//------------------------------------------------------------------------------
// Floyd Steinberg
// Image retournee doit etre liberee par l'appelant
//------------------------------------------------------------------------------
unsigned char *floyd_steinberg(IMAGE *source, PALETTE *palette, void (*f)(char *, int));


//------------------------------------------------------------------------------
// Dither ordonne HAM
// Image retournee doit etre liberee par l'appelant
//------------------------------------------------------------------------------
uint32_t *ordered_dither_ham(IMAGE *source, PALETTE *palette, float *matrix, int *matrix_size, void (*f)(char *, int));

//------------------------------------------------------------------------------
// Floyd Steinberg HAM
// Image retournee doit etre liberee par l'appelant
//------------------------------------------------------------------------------
uint32_t *floyd_steinberg_ham(IMAGE *source, PALETTE *palette, void (*f)(char *, int));


#endif
