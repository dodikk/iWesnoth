/*
 *  pango.hpp
 *	An simple, bare-bones iPhone replacement for the pango library
 */

typedef struct
{
	int x;
	int y;
	int width;
	int height;
} PangoRectangle;

typedef enum 
{
	PANGO_ELLIPSIZE_END,
	PANGO_ELLIPSIZE_NONE,
	PANGO_ELLIPSIZE_START
} PangoEllipsizeMode;
