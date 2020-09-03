/*
	This code will be our library for canvas (frame/display) functions like
	definition of pixel, color structures.
	Definition of canvas buffer.

	Coloring specific pixel with color.
	Loading pictures into buffer and
	functions to write one buffer to our main canvas buffer where we specify
	boudraries of smaller buffer as well as bigger buffer -> So we can f.e.
	place one image to another. In order to place images onto canvas we have to
	have defined some transparent color f.e. (1,1,1) -> while coloring this color
	will be ignored.
*/
#include <stdio.h>
#include <stdlib.h> // Malloc
#include <stdbool.h>
#include <string.h> // Strlen

#include "font_types.h"
#include "lodepng.h"      // Loading PNG files
#include "mzapo_canvas.h" // Display, cavnas functions
#include "xwin_sdl.h" // Temporary library till we get rid of SDL

/////////////// Canvas operations ///////////////

/*  This function will from X and Y coordinates extract color from canvas.  */
RGB GetPixelValue(int x, int y, Canvas* img_canvas){
	RGB rgb;
	
	int index = (y*img_canvas->w)*3 + x*3;
	rgb.R = img_canvas->img[index];
	rgb.G = img_canvas->img[index + 1];
	rgb.B = img_canvas->img[index + 2];
	
	return rgb;
}

/*  
	Color specific pixel with just coordinates and 'RGB struct'
	'x,y' - row and column of pixel we want to change
	'RGB' - color of the pixel
	'canvas' - our SDL image/canvas  
*/
void ColorCanvasPixel(unsigned int x, unsigned int y, RGB* color, Canvas* img_canvas){	
	// Access specific buffer place and color it with argument 'RGB' values
	int index = (int)((y*img_canvas->w*3) + x*3);
	img_canvas->img[index] = color->R;
	index++;
	img_canvas->img[index] = color->G;
	index++;
	img_canvas->img[index] = color->B;
	return;
}

/*  
	Create full circle in canvas
	'x,y' - row and column of pixel which will be circle center
	'radius' - radius of our circle in pixels
	'RGB' - color of the circle
	'canvas' - our SDL image/canvas  
*/
void CanvasCircle(unsigned int offset_x, unsigned int offset_y, int radius, RGB* color, Canvas* img_canvas){	
	// Access specific buffer place and color it with argument 'RGB' values
	int mid_offset = radius;
	int radius_sq = radius*radius;
	for(int y = 0; y < (2*radius); y++){
		for(int x = 0; x < (2*radius); x++){
			int y_dist = abs(mid_offset-y);
			int x_dist = abs(mid_offset-x);
			if(((y_dist*y_dist)+(x_dist*x_dist)) <= radius_sq){
				if((offset_x+x) >= 0 && (offset_x+x) <= DISPLAY_WIDTH && (offset_y+y) <= DISPLAY_HEIGHT && (offset_y+y) >= 0){
					ColorCanvasPixel(offset_x+x, offset_y+y, color, img_canvas);
				}
			}
		}
	}

	return;
}

/*  This functions takes one canvas A and copies is to another cavnas B.
	To coordiantions specified by B_x and B_y. B_x and B_y are basically offsets in B.
	No need to worry about overflow. It's taken care of.  */
void Canvas2CanvasFull(Canvas* A, Canvas* B, unsigned int B_x, unsigned int B_y, bool transparent, RGB* transparent_color){

	// Iterate image a and copy pixel by pixel values from canvas A to B
	for(int A_y = 0; A_y < A->h; A_y++){
		for(int A_x = 0; A_x < A->w; A_x++){
			// Check boundraries
			unsigned int Target_x = A_x + B_x; // Origin from A + offset
			unsigned int Target_y = A_y + B_y; // Origin from A + offset
			
			if((Target_x < B->w) && (Target_y < B->h)){ 
				// Grab pixel from canvas A and copy it to canvas B
				RGB color = GetPixelValue(A_x, A_y, A); // From A
				
				if((color.R == transparent_color->R) && (color.G == transparent_color->G) && (color.B == transparent_color->B) && (transparent == true)){
					continue; // Transparent pixel
				}else{
					ColorCanvasPixel(Target_x, Target_y, &color, B); // Color B
				}
				
			}else{
				continue; // Out of bounds
			}
		}
	}
	return;
}

/*  This functions takes one canvas A and copies is to another cavnas B.
	A_start_x, A_start_y are the coordinates we will be choping from copied canvas and 
	B_x, B_y are the coordinates where we will place our A canvas.
	Size of the copped piece is specified in A_x_w
	No need to worry about overflow. It's taken care of.  */
void Canvas2CanvasChopped(	Canvas* A, // Original canvas
						unsigned int A_start_x, // x,y coord of starting chopping pixel
						unsigned int A_start_y, 
						unsigned int A_x_w, // width and height of chopped part 
						unsigned int A_y_h, 
						Canvas* B, // Target canvas
						unsigned int B_x, // Target coord in B canvas
						unsigned int B_y,
						bool transparent, 
						RGB* transparent_color){
						
	// Iterate image a and copy pixel by pixel values from canvas A to B
	for(int A_y = A_start_y; A_y < (A_y_h+A_start_y); A_y++){
		for(int A_x = A_start_x; A_x < (A_x_w+A_start_x); A_x++){
			// Check boundraries
			unsigned int Target_x = A_x + B_x; // Origin from A + offset
			unsigned int Target_y = A_y + B_y; // Origin from A + offset
			
			if((Target_x < B->w) && (Target_y < B->h) && (A_x < A->w) && (A_y < A->h) 
					 && (Target_x > 0) && (Target_y > 0) && (A_x > 0) && (A_y > 0)){ 
				// Grab pixel from canvas A and copy it to canvas B
				RGB color = GetPixelValue(A_x, A_y, A); // From A
				
				// Should this color be ignored ?
				
				if((color.R == transparent_color->R) && (color.G == transparent_color->G) && (color.B == transparent_color->B) && (transparent == true)){
					continue; // Transparent pixel
				}else{
					ColorCanvasPixel(Target_x, Target_y, &color, B); // Color B
				}
				
				
			}else{
				continue; // Out of bounds
			}
		}
	}
	return;
}


/*  Refresh canvas display image  */
void CanvasRefresh(Canvas* canvas){ // THIS WILL HAVE TO BE CHANGED ONCE WE WONT RELLY ON SDL
	xwin_redraw(DISPLAY_WIDTH, DISPLAY_HEIGHT, canvas->img); // Redraw canvas
	return;
}

void CanvasInnit(){
	xwin_init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
}

/*  Draws text to the canvas where left up position is (start_x,start_y) and 
	separation is the number of pixels between letters (>=0)  
	This function basically iterates through every character in string,
	each character is represented in the font_winFreeSystem14x16 with its,
	width, height(all have the same height), and binary (uint_16),
	for each line in specific character we mask the bits with 1000...,0100...,0010...
	and when bitwise operation with this iterative binary and line of a character is
	not zero (meaning there is number 1 which represents color in character) then
	color the pixel with argument 'color'. We also have arguments for moving like
	'c_main_x','c_main_y' so we dont draw on one place. And add semerator at the end */
void DrawText(Canvas* canvas, char* text, int start_x, int start_y, unsigned int separation, RGB* color){
	int c_main_x = 0;
	int c_main_y = 0;
	
	int first_c_offset = font_winFreeSystem14x16.firstchar;

	unsigned int c_height = font_winFreeSystem14x16.height;
	
	
	
	for(int c = 0; c < strlen(text); c++){
		if(text[c] == 10){ // End of line when working with array strings
			break;}
		
		uint32_t c_offset = font_winFreeSystem14x16.offset[(int)(text[c])-first_c_offset];
		unsigned char c_width = font_winFreeSystem14x16.width[(int)(text[c])-first_c_offset];

		for(int c_temp_y = 0; c_temp_y < c_height; c_temp_y++){
			uint16_t c_line_data = font_winFreeSystem14x16.bits[c_offset+c_temp_y];

			uint16_t iterator = 32768; // 1000000000000000 mask bits
			for(int c_temp_x = 0; c_temp_x < c_width; c_temp_x++){
				if((iterator & c_line_data) != 0){
					// color pixel if and mask is not zero
					ColorCanvasPixel((start_x+c_main_x+c_temp_x), (start_y+c_main_y+c_temp_y), color, canvas);
				}
				iterator = iterator >> 1; // 0100000000000000 ...
			}
		}
		c_main_x += (c_width + separation);
	}
	return;
}
/////////////////////////////////////////////////


/////////////// PNG operations //////////////////
/* 
	Save PNG from struct canvas to path 'filename' 
*/
int SavePNGFromCanvas(const char *filename, Canvas *img_canvas){
	/*Encode the image*/
	unsigned error = lodepng_encode24_file(filename, img_canvas->img, img_canvas->w, img_canvas->h);

	if (error) { // If there's an error, display it
		printf("error %u: %s\n", error, lodepng_error_text(error));
		return -1; // Error
	} else {
		return 1; // Success
	}
}

/* 
	Load PNG to struct canvas from path 'filename' 
*/
int LoadPNG2Canvas(const char *filename, Canvas *img_canvas){
	unsigned error;

	error = lodepng_decode24_file(&(img_canvas->img), &(img_canvas->w), &(img_canvas->h), filename);
	if (error) {
		printf("ERROR: %u: %s\n", error, lodepng_error_text(error));
		return -1; // Error
	} else {
		return 1; // Success
	} // free(&(img_canvas->img));
}
/////////////////////////////////////////////////
