#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>

/* dimensions of the image we want to write */
int width = 640;                        //L'image à charger est bien en 480*640
int height = 480;
int bytes_per_pixel = 3;                /* or 1 for GRACYSCALE images */
J_COLOR_SPACE color_space = JCS_RGB;    /* or JCS_GRAYSCALE for grayscale images */

int read_jpeg_file(char *filename)
{
	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1];

	FILE *infile = fopen(filename, "rb");
	unsigned long location = 0;
	int i = 0;

	if (!infile) {
		printf("Error opening jpeg file %s\n!", filename);
		return -1;
	}
	/* here we set up the standard libjpeg error handler */
	cinfo.err = jpeg_std_error(&jerr);
	/* setup decompression process and source, then read JPEG header */
	jpeg_create_decompress(&cinfo);
	/* this makes the library read from infile */
	jpeg_stdio_src(&cinfo, infile);
	/* reading the image header which contains image information */
	jpeg_read_header(&cinfo, TRUE);

	printf("JPEG File Information: \n");
	printf("Image width and height: %d pixels and %d pixels.\n", cinfo.image_width, cinfo.image_height);
	printf("Color components per pixel: %d.\n", cinfo.num_components);
	printf("Color space: %d.\n", cinfo.jpeg_color_space);

	/* Start decompression jpeg here */
	jpeg_start_decompress(&cinfo);

	/* now actually read the jpeg into the raw buffer */
	row_pointer[0] = (unsigned char *)malloc(cinfo.output_width * cinfo.num_components);

	/* read one scan line at a time */
	while (cinfo.output_scanline < cinfo.image_height) {
		jpeg_read_scanlines(&cinfo, row_pointer, 1);

		if (cinfo.output_scanline % 10 == 0)
			printf("lignes lues %u\n", cinfo.output_scanline);
	}

	/* wrap up decompression, destroy objects, free pointers and close open files */
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(row_pointer[0]);
	fclose(infile);
	return 1;
}



int main(int argc, char *argv[])
{
	if (argc < 1) {
		printf("jpg en argument\n");
		exit(0);
	}

	if (read_jpeg_file(argv[1]) > 0)
		printf("lu\n");
	else return -1;

	return 0;
}
