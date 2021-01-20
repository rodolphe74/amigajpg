#include <gem.h>
#include <tos.h>
#include <stdio.h>
#include <stdlib.h>
#include "picojpeg.h"
#include "log.h"
#include "pixel.h"
#include "jpeg.h"

extern void set_pixel_low(UWORD *screen, UWORD x, UWORD y, UBYTE color);
extern UBYTE find_closest_color(UBYTE c[3], UBYTE palette[16][3], UBYTE palette_size);
extern void find_palette_16(IMAGE *image, UBYTE palette[16][3]);
extern IMAGE *bilinear_resize(IMAGE *image, UWORD width, UWORD height);
extern IMAGE *convert_to_internal_format(ULONG x, ULONG y, ULONG comp, void *data, LONG write_alpha, LONG scanline_pad);
extern UBYTE *pjpeg_load_from_file(const char *pFilename, ULONG *x, ULONG *y, ULONG *comps, pjpeg_scan_type_t *pScan_type);



//------------------------------------------------------------------------------
// Utils pour le log
//------------------------------------------------------------------------------
static FILE *f;
static void jpeg_log()
{
	f = fopen("picotos.log", "w");
	log_add_fp(f, LOG_DEBUG);
	log_set_quiet(1);
}
static void end_jpeg_log()
{
	fclose(f);
}



int main()
{
	jpeg_log();

	UBYTE *pImage;
	ULONG width, height, comps;
	pjpeg_scan_type_t scan_type;
	const char *p = "?";

	WORD old_mode = 0;
	UWORD *log_screen = (UWORD *)Logbase();
	UWORD *phys_screen = (UWORD *)Physbase();

	old_mode = Getrez();
	Setscreen((void *)-1, (void *)-1, ST_LOW);
	Cursconf(0, 0);

	printf("jpeg load\n");

	pImage = pjpeg_load_from_file("K2.JPG", &width, &height, &comps, &scan_type);

	printf("jpeg loaded\n");
	printf("components: %lu\n", comps);

	switch (scan_type) {
	case PJPG_GRAYSCALE:
		p = "GRAYSCALE";
		break;
	case PJPG_YH1V1:
		p = "H1V1";
		break;
	case PJPG_YH2V1:
		p = "H2V1";
		break;
	case PJPG_YH1V2:
		p = "H1V2";
		break;
	case PJPG_YH2V2:
		p = "H2V2";
		break;
	}
	printf("scan type: %s\n", p);

	LONG has_alpha = !(comps & 1);
	LONG pad = (-width * 3) & 3;
	IMAGE *image = convert_to_internal_format(width, height, comps, pImage, has_alpha, pad);

	free(pImage);
	UBYTE the_palette[16][3];

	printf("image size: %u*%u\n", image->width, image->height);

	if (image->width > 320 || image->height > 200) {
		IMAGE *resized = bilinear_resize(image, 320, 200);
		free_image(image);
		image = resized;
	}

	find_palette_16(image, the_palette);


	draw_image_low(phys_screen, image, 0, 0, the_palette);


	printf("K");
	getchar();

	if (old_mode != ST_LOW)
		Setscreen((void *)-1, (void *)-1, old_mode);

	free_image(image);

	end_jpeg_log();

	return 0;
}
