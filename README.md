# meanstack
meanstack is a utility for aligning and stacking astrophotography images to reduce noise. This utility is optimized for use with untracked telescopes and allows for alignment of images where star detection techniques can be unreliable. Supported features are image debayering, rough alignment using keyframe offsets, fine alignment by tracking around a specified object and mean (average) stacking with support for dark frames.

# Dependencies
- libjpeg-turbo
- libpng
- libtiff

# Usage
Firstly, the input image files must be prepared in the correct format. The utility accepts 8/16/32-bit TIFFs, 8/16-bit PNGs and JPEGs. The files must be placed in a directory and will be loaded in natural (numerical) order.

Next, if image alignment is required, a rough alignment offsets file must be provided. This file may look like this:
```
0 1163 0
1193 0 30
```
Each line is an offset keyframe - the first digit is the index of the image file, second and third are X and Y coordinates where the image should be placed in the output. The indices always start from 0 even if input filenames are named differently.

For fine alignment, the align utility may be used to correct the rough offsets around some tracked object:
```sh
$ meanstack align -i input-image-dir -O input-offsets-file.txt -p 437,448 -s 64 -m 24 -o final-offsets-file.txt
```
The -p argument specifies the center position of the tracked object, -s argument specifies the search radius where pixel comparison will be performed, and the -m argument specifies the maximum offset radius. It is important that the entire search radius around the object is present in all images.

Finally, the image can be aligned and stacked using the stack utility:
```sh
$ meanstack stack -i input-image-dir -O final-offsets-file.txt -o output-stack.tiff
```

The output image is a completed mean (average) stack that can be processed in any image editor.
