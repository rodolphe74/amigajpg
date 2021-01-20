#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "powf.h"
#include "pixel.h"
#include "log.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
// #define ABS(a) ((a) < 0 ? -(a) : (a))



int my_strlen(char *s)
{
	int i = 0;

	while (s[++i]);
	return i;
}


int my_abs(int v)
{
	if (v < 0)
		return -v;
	else
		return v;
}

unsigned short flr(double x)
{
	return (unsigned short)x - (x < (unsigned short)x);
}



unsigned short cl(double x)
{
	return (unsigned short)x + (x > (unsigned short)x);
}





// Function to convert hexadecimal to decimal
int hex_to_dec(char hexVal[])
{
	int len = my_strlen(hexVal);

	// Initializing base value to 1, i.e 16^0
	int base = 1;

	int dec_val = 0;

	// Extracting characters as digits from last character
	for (int i = len - 1; i >= 0; i--) {
		// if character lies in '0'-'9', converting
		// it to integral 0-9 by subtracting 48 from
		// ASCII value.
		if (hexVal[i] >= '0' && hexVal[i] <= '9') {
			dec_val += (hexVal[i] - 48) * base;

			// incrementing base by power
			base = base * 16;
		}
		// if character lies in 'A'-'F' , converting
		// it to integral 10 - 15 by subtracting 55
		// from ASCII value
		else if (hexVal[i] >= 'A' && hexVal[i] <= 'F') {
			dec_val += (hexVal[i] - 55) * base;

			// incrementing base by power
			base = base * 16;
		}
	}


	return dec_val;
}






void guess_palette(IMAGE *image, PALETTE *palette)
{
	int block_x = image->width / 4;
	int block_y = image->height / 8;
	int color_index = 0;
	unsigned long sum[3], count, mid[3];

	for (int py = 0; py < 8; py++) {
		for (int px = 0; px < 4; px++) {
			// Recherche de la couleur moyenne pour le bloc
			sum[0] = 0; sum[1] = 0; sum[2] = 0;
			count = 0;
			for (int y = 0; y < block_y; y++) {
				for (int x = 0; x < block_x; x++) {
					PIXEL p = image->pixels[(block_y * py * image->width) + y * image->width + (block_x * px) + x];
					sum[0] += p.r;
					sum[1] += p.g;
					sum[2] += p.b;
					count++;
				}
			}

			mid[0] = sum[0] / count;
			mid[1] = sum[1] / count;
			mid[2] = sum[2] / count;

			palette->colors[color_index][0] = mid[0] / 16;
			palette->colors[color_index][1] = mid[1] / 16;
			palette->colors[color_index][2] = mid[2] / 16;

			color_index++;
		}
	}
	palette->size = 32;
	strcpy(palette->name, "guess");
}



void init_bucket(BUCKET *bucket, short size)
{
	bucket->colors = malloc(size * sizeof(char *));
	for (int i = 0; i < size; i++)
		bucket->colors[i] = malloc(3);
	bucket->size = size;
}


void free_bucket(BUCKET *bucket)
{
	if (bucket->size > 0) {
		for (int i = 0; i < bucket->size; i++)
			free(bucket->colors[i]);
		free(bucket->colors);
	}
}

void swap_colors(BUCKET *bucket, int i, int j)
{
	unsigned char tmp[3];

	tmp[0] = bucket->colors[j][0];
	tmp[1] = bucket->colors[j][1];
	tmp[2] = bucket->colors[j][2];

	bucket->colors[j][0] = bucket->colors[i][0];
	bucket->colors[j][1] = bucket->colors[i][1];
	bucket->colors[j][2] = bucket->colors[i][2];

	bucket->colors[i][0] = tmp[0];
	bucket->colors[i][1] = tmp[1];
	bucket->colors[i][2] = tmp[2];
}

int partition_bucket(BUCKET *bucket, int range, int low, int high)
{
	int pivot = bucket->colors[high][range];        // pivot
	int i = (low - 1);                              // Index of smaller element

	for (int j = low; j <= high - 1; j++) {
		// If current element is smaller than the pivot
		if (bucket->colors[j][range] < pivot) {
			i++; // increment index of smaller element
			swap_colors(bucket, i, j);
		}
	}
	swap_colors(bucket, i + 1, high);
	return i + 1;
}

// Tri d'un bucket en quickSort
void sort_bucket(BUCKET *bucket, int range, int low, int high)
{
	if (low < high) {
		/* pi is partitioning index, arr[p] is now
		 * at right place */
		int pi = partition_bucket(bucket, range, low, high);

		// Separately sort elements before
		// partition and after partition
		sort_bucket(bucket, range, low, pi - 1);
		sort_bucket(bucket, range, pi + 1, high);
	}
}

int find_greatest_range(BUCKET *bucket)
{
	unsigned char r = 0, g = 0, b = 0;

	for (int i = 0; i < bucket->size; i++) {
		if (bucket->colors[i][0] > r)
			r = bucket->colors[i][0];
		if (bucket->colors[i][1] > g)
			g = bucket->colors[i][1];
		if (bucket->colors[i][2] > b)
			b = bucket->colors[i][2];
	}

	// printf("gr %d   gg %d   gb %d\n", r, g, b);

	if (r >= g && r >= b)
		return 0;
	else if (g >= r && g >= b)
		return 1;
	else
		return 2;
}

void split_bucket(BUCKET *bucket, BUCKET *new_bucket)
{
	int mid = bucket->size / 2;

	init_bucket(new_bucket, bucket->size - mid);
	for (int i = mid; i < bucket->size; i++) {
		new_bucket->colors[i - mid][0] = bucket->colors[i][0];
		new_bucket->colors[i - mid][1] = bucket->colors[i][1];
		new_bucket->colors[i - mid][2] = bucket->colors[i][2];

		free(bucket->colors[i]);
	}
	bucket->size = mid;
}

void mean_bucket(BUCKET *bucket)
{
	short sum_r = 0, sum_g = 0, sum_b = 0;

	for (int i = 0; i < bucket->size; i++) {
		sum_r += bucket->colors[i][0];
		sum_g += bucket->colors[i][1];
		sum_b += bucket->colors[i][2];
	}
	bucket->mean[0] = sum_r / bucket->size;
	bucket->mean[1] = sum_g / bucket->size;
	bucket->mean[2] = sum_b / bucket->size;
}

void print_bucket(BUCKET *bucket)
{
	printf("Taille : %d\n", bucket->size);
	for (int i = 0; i < bucket->size; i++)
		printf("(%u %u %u)\n", bucket->colors[i][0], bucket->colors[i][1], bucket->colors[i][2]);
}

void guess_palette_median_cut(IMAGE *image, PALETTE *palette, int iter)
{
	short max_colors = 1;

	for (int i = 0; i < iter; i++)
		max_colors *= 2;

	printf("max_colors %d\n", max_colors);

	// BUCKET bucket[32];
	BUCKET bucket[max_colors];

	short full_palette[4096];
	short amiga_color;
	short colors_count = 0;
	short color_index = 0;
	unsigned char r, g, b;

	for (int i = 0; i < 4096; i++)
		full_palette[i] = 0;

	// Recherche du nombre de couleurs
	for (int y = 0; y < image->height; y++) {
		for (int x = 0; x < image->width; x++) {
			amiga_color = 16 * 16 * (image->pixels[x + y * image->width].r / 16)
				      + 16 * (image->pixels[x + y * image->width].g / 16)
				      + (image->pixels[x + y * image->width].b / 16);

			if (full_palette[amiga_color] == 0) {
				full_palette[amiga_color] = 1;
				colors_count++;
			}
		}
	}
	printf("%d couleurs differentes\n", colors_count);

	// init
	init_bucket(&bucket[0], colors_count);
	for (int i = 0; i < 4096; i++) {
		if (full_palette[i] != 0) {
			r = i / 256;
			g = (i % 256) / 16;
			b = (i % 256) % 16;
			// printf("color_index %d ->  %d = %u %u %u\n", color_index, i, r, g, b);
			bucket[0].colors[color_index][0] = r;
			bucket[0].colors[color_index][1] = g;
			bucket[0].colors[color_index][2] = b;
			color_index++;
		}
	}





	int last_bucket = 1;
	int bucket_count = 0;
	int new_idx = 0;
	int greatest_range = 0;

	for (int k = 0; k < iter; k++) {
		new_idx = last_bucket;
		for (int i = 0; i < last_bucket; i++) {
			greatest_range = find_greatest_range(&bucket[i]);
			sort_bucket(&bucket[i], greatest_range, 0, bucket[i].size - 1);
			split_bucket(&bucket[i], &bucket[new_idx]);
			new_idx++;
		}
		last_bucket *= 2;
	}


	for (int i = 0; i < max_colors; i++)
		mean_bucket(&bucket[i]);


	// palette
	for (int i = 0; i < max_colors; i++) {
		palette->colors[i][0] = bucket[i].mean[0];
		palette->colors[i][1] = bucket[i].mean[1];
		palette->colors[i][2] = bucket[i].mean[2];
		free_bucket(&bucket[i]);
	}
	palette->size = max_colors;
}



int color_delta(unsigned char c1[3], unsigned char c2[3])
{
	int dr = abs((c1[0]) - (c2[0]));
	int dg = abs((c1[1]) - (c2[1]));
	int db = abs((c1[2]) - (c2[2]));

	enum { DISTANCE, WEIGHTED, SIMPLE } errorFunc = DISTANCE;

	switch (errorFunc) {
	case SIMPLE:
		return dr + dg + db;
	case DISTANCE:
		return sqrt((dr * dr) + (dg * dg) + (db * db));
	case WEIGHTED:
	default:
		return dr * 3 + dg * 4 + db * 2;
	}
}




unsigned char find_closest_color(unsigned char	c_current[3],
				 unsigned char	palette[32][3],
				 unsigned char	palette_size)
{
	unsigned long d_plt;

	unsigned char amiga_palette[32][3];
	unsigned char amiga_c_prec[3];
	unsigned char amiga_c_current[3];
	unsigned char amiga_c_ham[3];

	unsigned long diff = ULONG_MAX;

	unsigned char ham_plt = 0;


	// conversion du pixel au format amiga
	amiga_c_current[0] = c_current[0] / 16;
	amiga_c_current[1] = c_current[1] / 16;
	amiga_c_current[2] = c_current[2] / 16;


	// Recherche dans la palette
	for (int i = 0; i < palette_size; i++) {
		d_plt = color_delta(palette[i], amiga_c_current);
		if (d_plt < diff) {
			ham_plt = i;
			diff = d_plt;
		}
	}

	// Prise en compte de la distance la plus courte par rapport au
	// pixel demande
	// log_debug("meilleur approx %u", ham_plt);
	return ham_plt;
}



short find_closest_color_ham(unsigned char	c_current[3],
			     int		is_c_prec,
			     unsigned char	c_prec[3],
			     unsigned char	palette[32][3],
			     unsigned char	palette_size)
{
	unsigned long d_plt;
	unsigned long d_ham_r;
	unsigned long d_ham_g;
	unsigned long d_ham_b;

	unsigned char amiga_palette[16][3];
	unsigned char amiga_c_prec[3];
	unsigned char amiga_c_current[3];
	unsigned char amiga_c_ham[3];

	unsigned long diff = ULONG_MAX;

	short ham_plt = 0;

	// conversion du pixel prec au format amiga
	amiga_c_prec[0] = c_prec[0] / 16;
	amiga_c_prec[1] = c_prec[1] / 16;
	amiga_c_prec[2] = c_prec[2] / 16;

	// conversion du pixel au format amiga
	amiga_c_current[0] = c_current[0] / 16;
	amiga_c_current[1] = c_current[1] / 16;
	amiga_c_current[2] = c_current[2] / 16;


	// Recherche dans la palette

	for (int i = 0; i < 16; i++) {
		d_plt = color_delta(palette[i], amiga_c_current);
		if (d_plt <= diff) {
			ham_plt = 256 * palette[i][0] + 16 * palette[i][1] + palette[i][2];
			diff = d_plt;
		}
	}



	if (is_c_prec) {
		// rouge
		for (int i = 0; i < 16; i++) {
			amiga_c_ham[0] = i;
			amiga_c_ham[1] = amiga_c_prec[1];
			amiga_c_ham[2] = amiga_c_prec[2];
			d_ham_r = color_delta(amiga_c_current, amiga_c_ham);
			if (d_ham_r < diff) {
				ham_plt = 256 * amiga_c_ham[0] + 16 * amiga_c_ham[1] + amiga_c_ham[2];
				diff = d_ham_r;
			}
		}

		// vert

		for (int i = 0; i < 16; i++) {
			amiga_c_ham[0] = amiga_c_prec[0];
			amiga_c_ham[1] = i;
			amiga_c_ham[2] = amiga_c_prec[2];
			d_ham_g = color_delta(amiga_c_current, amiga_c_ham);
			if (d_ham_g < diff) {
				ham_plt = 256 * amiga_c_ham[0] + 16 * amiga_c_ham[1] + amiga_c_ham[2];
				diff = d_ham_g;
			}
		}

		// bleu
		for (int i = 0; i < 16; i++) {
			amiga_c_ham[0] = amiga_c_prec[0];
			amiga_c_ham[1] = amiga_c_prec[1];
			amiga_c_ham[2] = i;
			d_ham_b = color_delta(amiga_c_current, amiga_c_ham);
			if (d_ham_b < diff) {
				ham_plt = 256 * amiga_c_ham[0] + 16 * amiga_c_ham[1] + amiga_c_ham[2];
				diff = d_ham_b;
			}
		}
	}

	// Prise en compte de la distance la plus courte par rapport au
	// pixel demande
	return ham_plt;
}





//------------------------------------------------------------------------------

// Retourne un pointeur sur une nouvelle image redimensionnee width*height

//------------------------------------------------------------------------------

IMAGE *bilinear_resize(IMAGE *image, unsigned short width, unsigned short height)
{
	unsigned short x_l, y_l, x_h, y_h;
	float x_weight, y_weight;
	/* unsigned short x_weight, y_weight; */
	// char x_weight, y_weight;

	PIXEL pixel, a, b, c, d;

	IMAGE *resized = (IMAGE *)malloc(sizeof(IMAGE));
	PIXEL *pixels = (PIXEL *)malloc(sizeof(PIXEL) * width * height);

	resized->pixels = pixels;
	resized->width = width;
	resized->height = height;


	float x_ratio = width > 1 ? ((float)(image->width - 1) / (width - 1)) : 0;
	float y_ratio = height > 1 ? ((float)(image->height - 1) / (height - 1)) : 0;
	// short percent = 0, old_percent = 0;

	for (unsigned short i = 0; i < height; i++) {
		/*
		 * percent = (int)((i / (float)height) * 100);
		 * if (percent != old_percent)
		 *      printf("Progression %d\n", percent);
		 * old_percent = percent;
		 */

		for (unsigned short j = 0; j < width; j++) {
			x_l = flr(x_ratio * j);
			y_l = flr(y_ratio * i);
			x_h = cl(x_ratio * j);
			y_h = cl(y_ratio * i);

			x_weight = (x_ratio * j) - x_l;
			y_weight = (y_ratio * i) - y_l;

			a = image->pixels[y_l * image->width + x_l];
			b = image->pixels[y_l * image->width + x_h];
			c = image->pixels[y_h * image->width + x_l];
			d = image->pixels[y_h * image->width + x_h];

			pixel.r = a.r * (1 - x_weight) * (1 - y_weight)
				  + b.r * x_weight * (1 - y_weight)
				  + c.r * y_weight * (1 - x_weight)
				  + d.r * x_weight * y_weight;

			pixel.g = a.g * (1 - x_weight) * (1 - y_weight)
				  + b.g * x_weight * (1 - y_weight)
				  + c.g * y_weight * (1 - x_weight)
				  + d.g * x_weight * y_weight;

			pixel.b = a.b * (1 - x_weight) * (1 - y_weight)
				  + b.b * x_weight * (1 - y_weight)
				  + c.b * y_weight * (1 - x_weight)
				  + d.b * x_weight * y_weight;

			resized->pixels[i * width + j] = pixel;
		}
	}

	return resized;
}





void get_precalculated_matrix(short *matrix, int *matrix_size,
			      float *pre_calc_matrix)
{
	for (int y = 0; y < matrix_size[1]; y++) {
		for (int x = 0; x < matrix_size[0]; x++) {
			pre_calc_matrix[y * matrix_size[1] + x] = (matrix[y * matrix_size[1] + x] + 1.0f) / (matrix_size[0] * matrix_size[1]);

			// log_debug("(%d %d)  >%f ", x, y, pre_calc_matrix[y * matrix_size[1] + x]);
		}
	}
}



short linear_space(short x)
{
	float xf = x / 255.0f;
	float yf;

	if (xf <= 0.04045)
		yf = xf / 12.92;
	else
		yf = slow_powf(((xf + 0.055) / 1.055), 2.4);
	return (int)(yf * 255);
}



void find_threshold(PALETTE *palette, unsigned char threshold[3])
{
	BUCKET bucket;
	unsigned char diff;

	init_bucket(&bucket, palette->size);
	for (int i = 0; i < palette->size; i++) {
		bucket.colors[i][0] = palette->colors[i][0] * 16;
		bucket.colors[i][1] = palette->colors[i][1] * 16;
		bucket.colors[i][2] = palette->colors[i][2] * 16;
	}
	for (int k = 0; k < 3; k++) {
		threshold[k] = 0;
		sort_bucket(&bucket, k, 0, bucket.size - 1);
		for (int i = 0; i < palette->size - 1; i++) {
			diff = my_abs(bucket.colors[i][k] - bucket.colors[i + 1][k]);
			if (diff >= threshold[k])
				threshold[k] = diff;
		}
	}
}


unsigned char *ordered_dither(IMAGE *source, PALETTE *palette, float *matrix,
			      int *matrix_size, void (*f)(char *, int))
{
	unsigned char d[3], d_prec[3];
	unsigned char threshold[3];
	short d2[3];
	short index_color;
	float map_value;
	short percent, last_percent;
	int use_gamma = 1;
	float gamma = 2.0, gamma_correction;


	// Seuil optimal par rapport a la palette
	find_threshold(palette, threshold);

	printf("Seuil optimal %u %u %u\n", threshold[0], threshold[1], threshold[2]);


	unsigned char *pixels = malloc(sizeof(unsigned char) * source->height * source->width);

	if (!pixels) {
		printf("Pas assez de memoire\n");
		return NULL;
	}

	if (matrix)
		use_gamma = 1;
	else
		use_gamma = 0;

	for (short y = 0; y < source->height; y++) {
		for (short x = 0; x < source->width; x++) {
			PIXEL p = source->pixels[y * source->width + x];


			if (use_gamma) {
				gamma_correction = 1 / gamma;
				p.r = 255 * slow_powf((p.r / 255.0), gamma_correction);
				p.g = 255 * slow_powf((p.g / 255.0), gamma_correction);
				p.b = 255 * slow_powf((p.b / 255.0), gamma_correction);
			}

			if (matrix) {
				map_value = matrix[((y % matrix_size[1]) * matrix_size[1]) + (x % matrix_size[0])];

				d2[0] = linear_space(p.r) + map_value * threshold[0];
				d2[1] = linear_space(p.g) + map_value * threshold[1];
				d2[2] = linear_space(p.b) + map_value * threshold[2];

				d[0] = (unsigned char)(d2[0] > 255 ? 255 : d2[0]);
				d[1] = (unsigned char)(d2[1] > 255 ? 255 : d2[1]);
				d[2] = (unsigned char)(d2[2] > 255 ? 255 : d2[2]);
			} else {
				d[0] = p.r;
				d[1] = p.g;
				d[2] = p.b;
			}

			pixels[y * source->width + x] = find_closest_color(d, palette->colors, palette->size);
		}
	}

	return pixels;
}



unsigned char *floyd_steinberg(IMAGE *source, PALETTE *palette, void (*f)(char *, int))
{
	unsigned char threshold[3];
	short old_pixel[3], new_pixel[3];
	unsigned char p[3];
	short quant_error[3];
	short index_color;

	// Seuil optimal par rapport a la palette
	find_threshold(palette, threshold);

	printf("Seuil optimal %u %u %u\n", threshold[0], threshold[1], threshold[2]);

	unsigned char *pixels = malloc(sizeof(unsigned char) * source->height * source->width);

	if (!pixels) {
		printf("Pas assez de memoire\n");
		return NULL;
	}


	// Init buffer
	PIXEL_WORD *screen_buffer = malloc((sizeof(PIXEL_WORD)) * (source->width * source->height));

	if (!screen_buffer) {
		printf("Pas assez de memoire");
		return NULL;
	}

	for (short y = 0; y < source->height; y++) {
		for (short x = 0; x < source->width; x++) {
			screen_buffer[y * source->width + x].r = source->pixels[y * source->width + x].r;
			screen_buffer[y * source->width + x].g = source->pixels[y * source->width + x].g;
			screen_buffer[y * source->width + x].b = source->pixels[y * source->width + x].b;
		}
	}

	// Calcul et resultat dans buffer short
	for (short y = 0; y < source->height; y++) {
		/*
		 * percent = (100 * y) / source->height;
		 * if (last_percent != percent) {
		 *      (*f)("Dithering ", percent);
		 *      last_percent = percent;
		 * }
		 */

		for (short x = 0; x < source->width; x++) {
			old_pixel[0] = max(0, min(255, screen_buffer[y * source->width + x].r));
			old_pixel[1] = max(0, min(255, screen_buffer[y * source->width + x].g));
			old_pixel[2] = max(0, min(255, screen_buffer[y * source->width + x].b));

			// pixel centre
			p[0] = (unsigned char) old_pixel[0];
			p[1] = (unsigned char) old_pixel[1];
			p[2] = (unsigned char) old_pixel[2];
			
			
			index_color = find_closest_color(p, palette->colors, palette->size);
			pixels[x + y * source->width] = index_color;

			new_pixel[0] = palette->colors[index_color][0] * 16;
			new_pixel[1] = palette->colors[index_color][1] * 16;
			new_pixel[2] = palette->colors[index_color][2] * 16;

			// erreur
			quant_error[0] = old_pixel[0] - new_pixel[0];
			quant_error[1] = old_pixel[1] - new_pixel[1];
			quant_error[2] = old_pixel[2] - new_pixel[2];
			
			// pixels autour
			if (x + 1 < source->width) {
				screen_buffer[y * source->width + x + 1].r = screen_buffer[y * source->width + x + 1].r + (int)(quant_error[0] * (7.0f / 16.0f));
				screen_buffer[y * source->width + x + 1].g = screen_buffer[y * source->width + x + 1].g + (int)(quant_error[1] * (7.0f / 16.0f));
				screen_buffer[y * source->width + x + 1].b = screen_buffer[y * source->width + x + 1].b + (int)(quant_error[2] * (7.0f / 16.0f));
			}
			if (x - 1 >= 0 && y + 1 < source->height) {
				screen_buffer[(y + 1) * source->width + x - 1].r = screen_buffer[(y + 1) * source->width + x - 1].r + (int)(quant_error[0] * (3.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x - 1].g = screen_buffer[(y + 1) * source->width + x - 1].g + (int)(quant_error[1] * (3.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x - 1].b = screen_buffer[(y + 1) * source->width + x - 1].b + (int)(quant_error[2] * (3.0f / 16.0f));
			}
			if (y + 1 < source->height) {
				screen_buffer[(y + 1) * source->width + x].r = screen_buffer[(y + 1) * source->width + x].r + (int)(quant_error[0] * (5.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x].g = screen_buffer[(y + 1) * source->width + x].g + (int)(quant_error[1] * (5.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x].b = screen_buffer[(y + 1) * source->width + x].b + (int)(quant_error[2] * (5.0f / 16.0f));
			}
			if (x + 1 < source->width && y + 1 < source->height) {
				screen_buffer[(y + 1) * source->width + x + 1].r = screen_buffer[(y + 1) * source->width + x + 1].r + (int)(quant_error[0] * (1.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x + 1].g = screen_buffer[(y + 1) * source->width + x + 1].g + (int)(quant_error[1] * (1.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x + 1].b = screen_buffer[(y + 1) * source->width + x + 1].b + (int)(quant_error[2] * (1.0f / 16.0f));
			}
		}
	}
	free(screen_buffer);
	return pixels;
}





uint32_t *floyd_steinberg_ham(IMAGE *source, PALETTE *palette, void (*f)(char *, int))
{
	unsigned char threshold[3];
	short old_pixel[3], new_pixel[3];
	unsigned char p[3];
	short quant_error[3];
	short index_color;
	unsigned char d[3], d_prec[3];

	// Seuil optimal par rapport a la palette
	find_threshold(palette, threshold);

	printf("Seuil optimal %u %u %u\n", threshold[0], threshold[1], threshold[2]);

	uint32_t *pixels = malloc(sizeof(uint32_t) * source->height * source->width);

	if (!pixels) {
		printf("Pas assez de memoire\n");
		return NULL;
	}


	// Init buffer
	PIXEL_WORD *screen_buffer = malloc((sizeof(PIXEL_WORD)) * (source->width * source->height));

	if (!screen_buffer) {
		printf("Pas assez de memoire");
		return NULL;
	}

	for (short y = 0; y < source->height; y++) {
		for (short x = 0; x < source->width; x++) {
			screen_buffer[y * source->width + x].r = source->pixels[y * source->width + x].r;
			screen_buffer[y * source->width + x].g = source->pixels[y * source->width + x].g;
			screen_buffer[y * source->width + x].b = source->pixels[y * source->width + x].b;
		}
	}

	// Calcul et resultat dans buffer short
	for (short y = 0; y < source->height; y++) {
		/*
		 * percent = (100 * y) / source->height;
		 * if (last_percent != percent) {
		 *      (*f)("Dithering ", percent);
		 *      last_percent = percent;
		 * }
		 */

		for (short x = 0; x < source->width; x++) {
			old_pixel[0] = max(0, min(255, screen_buffer[y * source->width + x].r));
			old_pixel[1] = max(0, min(255, screen_buffer[y * source->width + x].g));
			old_pixel[2] = max(0, min(255, screen_buffer[y * source->width + x].b));

			// pixel centre
			p[0] = (unsigned char) old_pixel[0];
			p[1] = (unsigned char) old_pixel[1];
			p[2] = (unsigned char) old_pixel[2];
			
			int is_p_prec = 0;
			if (x > 0) {
				is_p_prec = 1;
				PIXEL p_prec = source->pixels[y * source->width + x - 1];
				d_prec[0] = p_prec.r;
				d_prec[1] = p_prec.g;
				d_prec[2] = p_prec.b;
			}
			
			
			short amiga_color = find_closest_color_ham(p, is_p_prec, d_prec, palette->colors, palette->size);
			unsigned char r_amiga = amiga_color / 256;
			unsigned char g_amiga = (amiga_color % 256) / 16;
			unsigned char b_amiga = (amiga_color % 256) % 16;
			pixels[y * source->width + x] = (r_amiga * 16) << 16 | (g_amiga * 16) << 8 | (b_amiga * 16);

			new_pixel[0] = r_amiga * 16;
			new_pixel[1] = g_amiga * 16;
			new_pixel[2] = b_amiga * 16;

			// erreur
			quant_error[0] = old_pixel[0] - new_pixel[0];
			quant_error[1] = old_pixel[1] - new_pixel[1];
			quant_error[2] = old_pixel[2] - new_pixel[2];
			
			// pixels autour
			if (x + 1 < source->width) {
				screen_buffer[y * source->width + x + 1].r = screen_buffer[y * source->width + x + 1].r + (int)(quant_error[0] * (7.0f / 16.0f));
				screen_buffer[y * source->width + x + 1].g = screen_buffer[y * source->width + x + 1].g + (int)(quant_error[1] * (7.0f / 16.0f));
				screen_buffer[y * source->width + x + 1].b = screen_buffer[y * source->width + x + 1].b + (int)(quant_error[2] * (7.0f / 16.0f));
			}
			if (x - 1 >= 0 && y + 1 < source->height) {
				screen_buffer[(y + 1) * source->width + x - 1].r = screen_buffer[(y + 1) * source->width + x - 1].r + (int)(quant_error[0] * (3.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x - 1].g = screen_buffer[(y + 1) * source->width + x - 1].g + (int)(quant_error[1] * (3.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x - 1].b = screen_buffer[(y + 1) * source->width + x - 1].b + (int)(quant_error[2] * (3.0f / 16.0f));
			}
			if (y + 1 < source->height) {
				screen_buffer[(y + 1) * source->width + x].r = screen_buffer[(y + 1) * source->width + x].r + (int)(quant_error[0] * (5.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x].g = screen_buffer[(y + 1) * source->width + x].g + (int)(quant_error[1] * (5.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x].b = screen_buffer[(y + 1) * source->width + x].b + (int)(quant_error[2] * (5.0f / 16.0f));
			}
			if (x + 1 < source->width && y + 1 < source->height) {
				screen_buffer[(y + 1) * source->width + x + 1].r = screen_buffer[(y + 1) * source->width + x + 1].r + (int)(quant_error[0] * (1.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x + 1].g = screen_buffer[(y + 1) * source->width + x + 1].g + (int)(quant_error[1] * (1.0f / 16.0f));
				screen_buffer[(y + 1) * source->width + x + 1].b = screen_buffer[(y + 1) * source->width + x + 1].b + (int)(quant_error[2] * (1.0f / 16.0f));
			}
		}
	}
	free(screen_buffer);
	return pixels;
}






uint32_t *ordered_dither_ham(IMAGE *source, PALETTE *palette, float *matrix,
			     int *matrix_size, void (*f)(char *, int))
{
	unsigned char d[3], d_prec[3];
	unsigned char threshold[3];
	short d2[3];
	short index_color;
	float map_value;
	short percent, last_percent;
	int use_gamma = 1;
	float gamma = 2.0, gamma_correction;


	// Seuil optimal par rapport a la palette
	find_threshold(palette, threshold);

	printf("Seuil optimal %u %u %u\n", threshold[0], threshold[1], threshold[2]);

	uint32_t *pixels = malloc(sizeof(unsigned int) * source->height * source->width);

	if (!pixels) {
		printf("Pas assez de memoire\n");
		return NULL;
	}

	if (matrix)
		use_gamma = 1;
	else
		use_gamma = 0;



	for (short y = 0; y < source->height; y++) {
		for (short x = 0; x < source->width; x++) {
			PIXEL p = source->pixels[y * source->width + x];


			if (use_gamma) {
				gamma_correction = 1 / gamma;
				p.r = 255 * slow_powf((p.r / 255.0), gamma_correction);
				p.g = 255 * slow_powf((p.g / 255.0), gamma_correction);
				p.b = 255 * slow_powf((p.b / 255.0), gamma_correction);
			}

			int is_p_prec = 0;
			if (x > 0) {
				is_p_prec = 1;
				PIXEL p_prec = source->pixels[y * source->width + x - 1];
				d_prec[0] = p_prec.r;
				d_prec[1] = p_prec.g;
				d_prec[2] = p_prec.b;
			}

			if (matrix) {
				map_value = matrix[((y % matrix_size[1]) * matrix_size[1]) + (x % matrix_size[0])];

				d2[0] = linear_space(p.r) + map_value * threshold[0];
				d2[1] = linear_space(p.g) + map_value * threshold[1];
				d2[2] = linear_space(p.b) + map_value * threshold[2];

				d[0] = (unsigned char)(d2[0] > 255 ? 255 : d2[0]);
				d[1] = (unsigned char)(d2[1] > 255 ? 255 : d2[1]);
				d[2] = (unsigned char)(d2[2] > 255 ? 255 : d2[2]);
			} else {
				d[0] = p.r;
				d[1] = p.g;
				d[2] = p.b;
			}


			// retourne la couleur de la palette amiga sur 4096
			short amiga_color = find_closest_color_ham(d, is_p_prec, d_prec, palette->colors, palette->size);
			unsigned char r_amiga = amiga_color / 256;
			unsigned char g_amiga = (amiga_color % 256) / 16;
			unsigned char b_amiga = (amiga_color % 256) % 16;
			pixels[y * source->width + x] = (r_amiga * 16) << 16 | (g_amiga * 16) << 8 | (b_amiga * 16);
		}
	}

	return pixels;
}
