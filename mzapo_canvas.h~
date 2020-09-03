/*  Everything display related  */

#include <stdbool.h>

//MZAPO_DISPLAY_DIMENSIONS 480x320
#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320

typedef struct RGB{
	int R; // Red
	int G; // Green
	int B; // Blue
}RGB;

typedef struct Canvas{
	unsigned int h; // Height
	unsigned int w; // Width
	unsigned char* img; // Image buffer
}Canvas;

/*  This function will from X and Y coordinates extract color from Canvas.  */
RGB GetPixelValue(int x, int y, Canvas* img_canvas);

/*  Color specifix piexel of canvas buffer  */
void ColorCanvasPixel(unsigned int x, unsigned int y, RGB* color, Canvas* img_canvas);

/*  Create full circle in canvas
	'x,y' - row and column of pixel which will be circle center
	'radius' - radius of our circle in pixels
	'RGB' - color of the circle
	'canvas' - our SDL image/canvas  */
void CanvasCircle(unsigned int x, unsigned int y, int radius, RGB* color, Canvas* img_canvas);

// See comments in mzapo_canvas.c
void Canvas2CanvasFull(Canvas* A, Canvas* B, unsigned int B_x, unsigned int B_y, bool transparent, RGB* transparent_color);

// See comments in mzapo_canvas.c
void Canvas2CanvasChopped( Canvas* A, unsigned int A_start_x, unsigned int A_start_y, unsigned int A_x_w, unsigned int A_y_h, Canvas* B, unsigned int B_x, unsigned int B_y, bool transparent, RGB* transparent_color);
						
/* Save PNG from struct canvas to path 'filename' */
int SavePNGFromCanvas(const char *filename, Canvas *img_canvas);

/* Load PNG to struct canvas from path 'filename' */
int LoadPNG2Canvas(const char *filename, Canvas *img_canvas);

/*  This may seem unneccesarry but we will need this function with the board.
	This will ahve to change once we wont relly on SDL.
	Backgrounds images will have to be of DISPLAY_WIDTHxDISPLAY_HEIGHT*/
void CanvasRefresh(Canvas* canvas);

/*  Initialize canvas. This will ahve to change once we wont relly on SDL  */
void CanvasInnit();

/*  Draws text to the canvas where left up position is (start_x,start_y) and 
	separation is the number of pixels between letters (>=0), color with color */
void DrawText(Canvas* canvas, char* text, int start_x, int start_y, unsigned int separation, RGB* color);
