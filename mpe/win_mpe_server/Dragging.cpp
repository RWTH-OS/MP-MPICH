/*
    $Id: Dragging.cpp,v 1.3 2000/11/14 08:35:36 karsten Exp $
*/


#include <windows.h>
#include <math.h>
#include "r_mpe.h"
#include "internal.h"
#include "Dragging.h"

/* convert start and end coordinates to x,y,width,height coords. */
/* different transformations required for each dragVisual. */

int ConvertCoords( int dragVisual, int x1, int y1, 
		   int x2, int y2, double ratio,
		   int *cx1, int *cy1, int *cwidth, 
		   int *cheight )
{
  int width, height, left, top, longestSide;
  double dist;

  if (x1<x2) {
    left = x1; width = x2-x1;
  } else {
    left = x2; width = x1-x2;
  }
  if (y1<y2) {
    top = y1; height = y2-y1;
  } else {
    top = y2; height = y1-y2;
  }
  dist = (double)sqrt( (double)((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)) );
  longestSide = (width>height)?width:height;

  switch (dragVisual) {
  case MPE_DRAG_LINE:
    *cx1 = x1;
    *cy1 = y1;
    *cwidth = x2;
    *cheight = y2;
    break;

  case MPE_DRAG_FIXED_RECT:
    if (width*ratio > height) {
      /* width is limiting factor */
      height = (int) (width * ratio);
    } else {
      width = (int) (height / ratio);
    }
    *cx1 = (x1>x2) ? (x1-width)  : x1;
    *cy1 = (y1>y2) ? (y1-height) : y1;
    *cwidth = width;
    *cheight = height;
    break;

  case MPE_DRAG_SQUARE:
    *cx1 = (x1>x2) ? (x1-longestSide) : x1;
    *cy1 = (y1>y2) ? (y1-longestSide) : y1;
    *cwidth = longestSide;
    *cheight = longestSide;
    break;

  case MPE_DRAG_CIRCLE_RADIUS:
    *cx1 = (int) (x1-dist);
    *cy1 = (int) (y1-dist);
    *cwidth = (int) (dist*2);
    *cheight = (int) (dist*2);
    break;

  case MPE_DRAG_CIRCLE_DIAMETER:
    *cx1 = (int) ((x1+x2)/2 - dist/2);
    *cy1 = (int) ((y1+y2)/2 - dist/2);
    *cwidth = (int) dist;
    *cheight = (int) dist;
    break;

  case MPE_DRAG_CIRCLE_BBOX:
    *cx1 = (x1>x2) ? (x1-longestSide) : x1;
    *cy1 = (y1>y2) ? (y1-longestSide) : y1;
    *cwidth = longestSide;
    *cheight = longestSide;
    break;

  default:
  case MPE_DRAG_OVAL_BBOX:
  case MPE_DRAG_RECT:

    *cx1 = left;
    *cy1 = top;
    *cwidth = width;
    *cheight = height;
    break;
  }

  return 0;
}


/* ratio is in terms of height/width */

int DrawDragVisual(MPE_Context *Context)

{
  int width, height, top, left;
   int x1,x2,y1,y2,dragVisual;
   

  if(Context->CurrentVisual == MPE_DRAG_NONE) return 0;
  x1 = Context->DragCoords.left;
  y1 = Context->DragCoords.top;
  x2 = Context->DragCoords.right;
  y2 = Context->DragCoords.bottom;
  dragVisual = Context->CurrentVisual;

  ConvertCoords( dragVisual, x1, y1, x2, y2, Context->Ratio, &left, &top,
		 &width, &height );


  switch (dragVisual) {
  case MPE_DRAG_NONE: break;
  case MPE_DRAG_FIXED_RECT:
  case MPE_DRAG_SQUARE:
  case MPE_DRAG_RECT:
      Rectangle(Context->dragDC,left,top,left+width,top+height);
    break;

  case MPE_DRAG_CIRCLE_RADIUS:
  case MPE_DRAG_CIRCLE_DIAMETER:
  case MPE_DRAG_CIRCLE_BBOX:
  case MPE_DRAG_OVAL_BBOX:
      Ellipse(Context->dragDC,left,top,left+width,top+height);
      break;

  default:
  case MPE_DRAG_LINE:
    MoveToEx(Context->dragDC,left,top,0);
    LineTo(Context->dragDC,width,height);
    
    break;

  }
  return 0;
}
