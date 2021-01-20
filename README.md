# amigajpg
## description
Fed up with high resolutions and true colors pictures ?
Let's go back in the middle of the eighties and display jpg as an Amiga would do.
Constraints are:
* 4096 colors palette (16 tints per component).
* 320x256 pixels screen.
* 2, 4, 8, 16, 32 simultaneous colors over 4096,
* or HAM-6 mode: possibly display full palette with proximity constraints ([Hold-And-Modify](https://en.wikipedia.org/wiki/Hold-And-Modify)).

The program is able to :
* Display a jpg inside a 640x512 pixels SDL window (pixels are doubled).
* Find an optimal palette ([use of median-cut algorithm](https://en.wikipedia.org/wiki/Median_cut)).
* Render color tones with ordered or floyd steinberg dither.

## examples

### original
![Original](/images/original.jpg)

### HAM without dithering : ./amigajpg -h -i images/264750.jpg 
![HAM without dithering](/images/ham-no-dither.jpg)

### HAM with size 8 ordered matrix dithering : ./amigajpg -h -m8 -i images/264750.jpg
![HAM ordered dithering](/images/ham-no-dither.jpg)

### HAM with error diffusion floyd steinberg dithering : ./amigajpg -h -f -i images/264750.jpg
![HAM fs dithering](/images/ham-dither-floyd.jpg)

### 32 colors size 4 ordered matrix dithering : ./amigajpg -c 32 -m4 -i images/264750.jpg
![32 colors ordered dithering](/images/32-colors-4.jpg)

## command line arguments
* -i : input image filename
* -c [2,4,8,16,32] : number of simultaneous colors
* -h : HAM mode
* -m [2,3,4,8] : size of the ordered dither matrix
* -f : Floyd Steinber dithering

## compilation instruction
The code should easely compile under linux if:
* Package SDL2-devel is installed.
* Packages make and gcc are installed
The source of libjpeg is under jpeg-6b folder. The program is linked against jpeg-6b/libjpeg.a.
