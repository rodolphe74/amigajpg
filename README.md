# amigajpg
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
