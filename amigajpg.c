#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "gopt.h"
#include "log.h"
#include "pixel.h"
#include "jpeg.h"


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


SDL_Window *window;
SDL_Surface *screen;
SDL_Surface *surface;


int w = 640;
int h = 512;


//------------------------------------------------------------------------------
// Utils pour le log
//------------------------------------------------------------------------------
static FILE *f;
static void jpeg_log()
{
	f = fopen("amigajpg.log", "w");
	log_add_fp(f, LOG_DEBUG);
	log_set_quiet(1);
}
static void end_jpeg_log()
{
	fclose(f);
}


//-----------------------------------------------------------------------------
// Call back pour la progression
//-----------------------------------------------------------------------------
void display_in_title(char *text, int percent)
{
	char buf[50];

	if (percent > -1)
		sprintf(buf, "%s %i%%", text, percent);
	else
		sprintf(buf, "%s", text);
	printf("progression %d\n", percent);
}
void (*display_in_title_ptr)(char *, int) = &display_in_title;




int create_window(int is_ham)
{
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("",
				  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				  w, h,
				  0);

	screen = SDL_GetWindowSurface(window);

	if (!is_ham)
		surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
	else
		surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0, 0, 0, 0);


	return 1;
}



void draw_palette(IMAGE *image, PALETTE *palette)
{
	SDL_Color colors[255];

	for (int i = 0; i < palette->size; i++) {
		colors[i].r = palette->colors[i][0] * 16;
		colors[i].g = palette->colors[i][1] * 16;
		colors[i].b = palette->colors[i][2] * 16;
		colors[i].a = 255;
	}

	SDL_SetPaletteColors(surface->format->palette, colors, 0, palette->size);

	int block_x = w / 4;
	int block_y = h / 8;
	int color_index = 0;

	puts(SDL_GetPixelFormatName(surface->format->format));

	unsigned char *pixels = surface->pixels;
	for (int py = 0; py < 8; py++) {
		for (int px = 0; px < 4; px++) {
			for (int y = 0; y < block_y; y++)
				for (int x = 0; x < block_x; x++)
					pixels[(px * block_x + x) + (py * block_y + y) * w] = color_index;
			color_index++;
		}
	}
}





void draw_pixels(unsigned char *pixels, int width, int height, PALETTE *palette)
{
	SDL_Color colors[255];

	for (int i = 0; i < palette->size; i++) {
		colors[i].r = palette->colors[i][0] * 16;
		colors[i].g = palette->colors[i][1] * 16;
		colors[i].b = palette->colors[i][2] * 16;
		colors[i].a = 255;
	}

	SDL_SetPaletteColors(surface->format->palette, colors, 0, palette->size);

	puts(SDL_GetPixelFormatName(surface->format->format));

	unsigned char *sdl_pixels = surface->pixels;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			sdl_pixels[(x * 2) + (y * 2 * w)] = pixels[x + y * width];
			sdl_pixels[(x * 2 + 1) + (y * 2 * w)] = pixels[x + y * width];
			sdl_pixels[(x * 2) + ((y * 2 + 1) * w)] = pixels[x + y * width];
			sdl_pixels[(x * 2 + 1) + ((y * 2 + 1) * w)] = pixels[x + y * width];
		}
	}
}


void draw_pixels_ham(uint32_t *pixels, int width, int height, PALETTE *palette)
{
	puts(SDL_GetPixelFormatName(surface->format->format));

	uint32_t *sdl_pixels = surface->pixels;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			sdl_pixels[(x * 2) + (y * 2 * w)] = pixels[x + y * width];
			sdl_pixels[(x * 2 + 1) + (y * 2 * w)] = pixels[x + y * width];
			sdl_pixels[(x * 2) + ((y * 2 + 1) * w)] = pixels[x + y * width];
			sdl_pixels[(x * 2 + 1) + ((y * 2 + 1) * w)] = pixels[x + y * width];
		}
	}
}



int message_loop()
{
	while (1) {
		SDL_Event event;
		while (SDL_PollEvent(&event))
			if (event.type == SDL_QUIT) exit(0);

		//
		// Display the pixel buffer here.
		//
		SDL_BlitSurface(surface, NULL, screen, NULL);
		SDL_UpdateWindowSurface(window);
		SDL_Delay(100);
	}
}



int main(int argc, char *argv[])
{
	char image_filename[256];
	char palette_name[256];
	IMAGE *the_image;
	PALETTE the_palette;
	float *the_matrix;
	int the_matrix_size[2];
	int matrix_size = 0;
	int floyd = 0;

	jpeg_log();


	// Lecture des arguments
	enum { PICTURE_MODE, PALETTE_MODE, HAM_MODE } mode = PICTURE_MODE;
	int max_colors = 32;
	int iter_median_cut = 5;


	struct option options[7];

	options[0].long_name = "ham";
	options[0].short_name = 'h';
	options[0].flags = GOPT_ARGUMENT_FORBIDDEN;

	options[1].long_name = "colors";
	options[1].short_name = 'c';
	options[1].flags = GOPT_ARGUMENT_REQUIRED;

	options[2].long_name = "palette";
	options[2].short_name = 'p';
	options[2].flags = GOPT_ARGUMENT_FORBIDDEN;

	options[3].long_name = "input";
	options[3].short_name = 'i';
	options[3].flags = GOPT_ARGUMENT_REQUIRED;

	options[4].long_name = "matrix";
	options[4].short_name = 'm';
	options[4].flags = GOPT_ARGUMENT_REQUIRED;
	
	options[5].long_name = "floyd";
	options[5].short_name = 'f';
	options[5].flags = GOPT_ARGUMENT_FORBIDDEN;

	options[6].flags = GOPT_LAST;

	argc = gopt(argv, options);

	gopt_errors(argv[0], options);

	if (options[0].count) {
		printf("Utilisation du mode HAM\n");
		mode = HAM_MODE;
	}

	if (options[1].count) {
		max_colors = atoi(options[1].argument);
		if (max_colors == 0) {
			printf("Nombre de couleurs errone\n");
			exit(0);
		} else {
			float c = log(max_colors) / log(2);
			iter_median_cut = (int)c;
			max_colors = pow(2, (int)c);

			if (max_colors > 32) {
				printf("Nombre de couleurs > 32\n");
				exit(0);
			}

			printf("Utilisation d'une palette de %d couleurs\n", max_colors);
		}
	}

	if (options[4].count) {
		matrix_size = atoi(options[4].argument);
		floyd = 0;
	}
	
	if (options[5].count) {
		floyd = 1;
		matrix_size = 0;
	}

	if (options[2].count) {
		printf("Affichage de la palette optimale\n");
		mode = PALETTE_MODE;
	}

	if (options[3].count) {
		strcpy(image_filename, options[3].argument);
		printf("Chargement de %s\n", image_filename);
	} else {
		printf("Le nom de fichier jpg doit etre passe en parametre\n");
		exit(0);
	}



	the_image = load(image_filename, display_in_title);
	if (!the_image)
		return 0;


	printf("Verification de la taille de l'image\n");
	float ratio_x = 0, ratio_y = 0, ratio;
	int do_resize = 0;


	if (the_image->width > 320) {
		ratio_x = the_image->width / 320.0;
		printf("ratio x: %f\n", ratio_x);
		do_resize = 1;
	}
	if (the_image->height > 256) {
		ratio_y = the_image->height / 256.0;
		printf("ratio y: %f\n", ratio_y);
		do_resize = 1;
	}

	if (do_resize) {
		ratio = MAX(ratio_x, ratio_y);
		printf("Redimensionnement ratio %f\n", ratio);
		IMAGE *new_image = bilinear_resize(the_image,
						   (int)the_image->width / ratio,
						   (int)the_image->height / ratio);
		free_image(the_image);
		the_image = new_image;
	}


	// palette
	printf("Recherche d'une palette optimale\n");

	if (mode == HAM_MODE) {
		printf("HAM mode -> palette 16 couleurs\n");
		max_colors = 16;
		iter_median_cut = 4;
	}

	guess_palette_median_cut(the_image, &the_palette, iter_median_cut);
	// guess_palette(the_image, &the_palette);


	// dithering
	if (matrix_size == 0) {
		the_matrix = NULL;
	} else if (matrix_size == 2) {
		the_matrix = malloc(BAYER_2_2_SIZE[0] * BAYER_2_2_SIZE[1] * sizeof(float));
		the_matrix_size[0] = BAYER_2_2_SIZE[0];
		the_matrix_size[1] = BAYER_2_2_SIZE[1];
		get_precalculated_matrix(BAYER_2_2, BAYER_2_2_SIZE, the_matrix);
	} else if (matrix_size == 3) {
		the_matrix = malloc(BAYER_3_3_SIZE[0] * BAYER_3_3_SIZE[1] * sizeof(float));
		the_matrix_size[0] = BAYER_3_3_SIZE[0];
		the_matrix_size[1] = BAYER_3_3_SIZE[1];
		get_precalculated_matrix(BAYER_3_3, BAYER_3_3_SIZE, the_matrix);
	} else if (matrix_size == 4) {
		the_matrix = malloc(BAYER_4_4_SIZE[0] * BAYER_4_4_SIZE[1] * sizeof(float));
		the_matrix_size[0] = BAYER_4_4_SIZE[0];
		the_matrix_size[1] = BAYER_4_4_SIZE[1];
		get_precalculated_matrix(BAYER_4_4, BAYER_4_4_SIZE, the_matrix);
	} else if (matrix_size == 8) {
		the_matrix = malloc(BAYER_8_8_SIZE[0] * BAYER_8_8_SIZE[1] * sizeof(float));
		the_matrix_size[0] = BAYER_8_8_SIZE[0];
		the_matrix_size[1] = BAYER_8_8_SIZE[1];
		get_precalculated_matrix(BAYER_8_8, BAYER_8_8_SIZE, the_matrix);
	}


	if (mode != HAM_MODE) {
		unsigned char *pixels;
		if (!floyd) {
			pixels = ordered_dither(the_image, &the_palette, the_matrix, the_matrix_size, NULL);
		} else {
			pixels = floyd_steinberg(the_image, &the_palette, NULL);
		}

		if (!create_window(mode == HAM_MODE)) {
			printf("Impossible de creer l'ecran\n");
			return 0;
		}

		if (mode == PALETTE_MODE)
			draw_palette(the_image, &the_palette);
		else
			draw_pixels(pixels, the_image->width, the_image->height, &the_palette);

		free(pixels);
	} else {
		uint32_t *pixels;
		if (!floyd) {
			pixels = ordered_dither_ham(the_image, &the_palette, the_matrix, the_matrix_size, NULL);
		} else {
			pixels = floyd_steinberg_ham(the_image, &the_palette, NULL);
		}

		if (!create_window(mode == HAM_MODE)) {
			printf("Impossible de creer l'ecran\n");
			return 0;
		}
		draw_pixels_ham(pixels, the_image->width, the_image->height, &the_palette);

		free(pixels);
	}

	message_loop();

	free_image(the_image);

	free(the_matrix);
	end_jpeg_log();
}
