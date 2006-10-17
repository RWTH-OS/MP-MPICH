#ifndef __DRAGGING_H__
#define __DRAGGING_H__


/* types of visuals for Get_drag_region */
/* Taken from mpe.h*/
#define MPE_NO_DRAG   -1
#define MPE_DRAG_NONE 0		     /* no visual */
#define MPE_DRAG_RECT 1		     /* rubber band box */
#define MPE_DRAG_LINE 2		     /* rubber band line */
#define MPE_DRAG_CIRCLE_RADIUS 3     /* rubber band circle, */
    /* one point is the center of the circle, other point is on the circle */
#define MPE_DRAG_CIRCLE_DIAMETER 4
    /* each point is on opposite sides of the circle */
#define MPE_DRAG_CIRCLE_BBOX 5
    /* the two points define a bounding box inside which is drawn a circle */
#define MPE_DRAG_OVAL_BBOX 6
    /* the two points define a bounding box inside which is drawn an oval */
#define MPE_DRAG_SQUARE 7
#define MPE_DRAG_FIXED_RECT 8

extern int ConvertCoords( int dragVisual, int x1, int y1, 
			  int x2, int y2, double ratio,
			  int *cx1, int *cy1, int *cwidth, 
			  int *cheight );

extern int DrawDragVisual( MPE_Context *Context);


#endif