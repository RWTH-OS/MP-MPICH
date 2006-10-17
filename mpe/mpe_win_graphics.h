#ifndef _MPE_GRAPHICS_
#define _MPE_GRAPHICS_

#ifdef MPE_NOMPI
typedef int MPI_Comm;
#else
#include "mpi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifndef ANSI_ARGS
#if defined(__STDC__) || defined(__cplusplus) || defined(HAVE_PROTOTYPES)
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif
#endif



typedef enum { MPE_WHITE = 0, MPE_BLACK = 1, MPE_RED = 2, MPE_YELLOW = 3, 
	       MPE_GREEN = 4, MPE_CYAN = 5, MPE_BLUE = 6, MPE_MAGENTA = 7,
               MPE_AQUAMARINE = 8, MPE_FORESTGREEN = 9, MPE_ORANGE = 10, 
	       MPE_MAROON = 11, MPE_BROWN = 12, MPE_PINK = 13, MPE_CORAL = 14, 
               MPE_GRAY = 15} MPE_Color;


  /* given existing pixel 'dst' and new, overlapping pixel 'src' */
#define MPE_LOGIC_CLEAR        1
#define MPE_LOGIC_AND          9
#define MPE_LOGIC_ANDREVERSE   5
#define MPE_LOGIC_COPY         13
#define MPE_LOGIC_ANDINVERTED  3
#define MPE_LOGIC_NOOP         11
#define MPE_LOGIC_XOR          7
#define MPE_LOGIC_OR           15
#define MPE_LOGIC_NOR          2
#define MPE_LOGIC_EQUIV        10
#define MPE_LOGIC_INVERT       6
#define MPE_LOGIC_ORREVERSE    14
#define MPE_LOGIC_COPYINVERTED 4
#define MPE_LOGIC_ORINVERTED   12
#define MPE_LOGIC_NAND         8
#define MPE_LOGIC_SET          16

#define MPE_BUTTON1 1
#define MPE_BUTTON2 2
#define MPE_BUTTON3 3
#define MPE_BUTTON4 4
#define MPE_BUTTON5 5


/* types of visuals for Get_drag_region */
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




typedef void *MPE_XGraph;

typedef struct MPE_Point_ {
  int x, y;
  MPE_Color c;
} MPE_Point;

#define MPE_GRAPH_INDEPDENT  0
#define MPE_GRAPH_INDEPENDENT MPE_GRAPH_INDEPDENT
#define MPE_GRAPH_COLLECTIVE 1

IMPORT_MPI_API int MPE_Open_graphics ANSI_ARGS(( MPE_XGraph *handle, MPI_Comm comm, 
	   char *display, int x, int y, int w, int h, int is_collective ));

IMPORT_MPI_API int MPE_Draw_point ANSI_ARGS(( MPE_XGraph handle, int x, int y,
	   MPE_Color color ));

IMPORT_MPI_API int MPE_Draw_line ANSI_ARGS(( MPE_XGraph handle, int x1, int y1,
	   int x2, int y2, MPE_Color color ));

IMPORT_MPI_API int MPE_Draw_circle ANSI_ARGS(( MPE_XGraph, int, int, int, MPE_Color ));

IMPORT_MPI_API int MPE_Draw_string ANSI_ARGS(( MPE_XGraph, int, int, MPE_Color,
				       char * ));

IMPORT_MPI_API int MPE_Fill_rectangle ANSI_ARGS(( MPE_XGraph handle, int x, int y,
	   int w, int h, MPE_Color color ));

IMPORT_MPI_API int MPE_Draw_rectangle ANSI_ARGS(( MPE_XGraph handle, int x, int y,
	   int w, int h, MPE_Color color ));


IMPORT_MPI_API int MPE_Update ANSI_ARGS(( MPE_XGraph handle ));

IMPORT_MPI_API int MPE_Num_colors ANSI_ARGS(( MPE_XGraph handle, int *nc ));

IMPORT_MPI_API int MPE_Make_color_array ANSI_ARGS(( MPE_XGraph handle, int ncolors, 
	   MPE_Color array[] ));

IMPORT_MPI_API int MPE_Close_graphics ANSI_ARGS(( MPE_XGraph *handle ));

IMPORT_MPI_API int MPE_CaptureFile ANSI_ARGS(( MPE_XGraph, char *, int ));

IMPORT_MPI_API int MPE_Draw_points ANSI_ARGS(( MPE_XGraph, MPE_Point *, int ));

IMPORT_MPI_API int MPE_Fill_circle ANSI_ARGS(( MPE_XGraph, int, int, int, MPE_Color ));

IMPORT_MPI_API int MPE_Draw_logic  ANSI_ARGS(( MPE_XGraph, int ));

IMPORT_MPI_API int MPE_Line_thickness ANSI_ARGS(( MPE_XGraph, int ));

IMPORT_MPI_API int MPE_Draw_dashes ANSI_ARGS(( MPE_XGraph, int ));

IMPORT_MPI_API int MPE_Dash_offset ANSI_ARGS(( MPE_XGraph, int ));

IMPORT_MPI_API int MPE_Add_RGB_color ANSI_ARGS(( MPE_XGraph, int, int, int, 
					 MPE_Color * ));

IMPORT_MPI_API int MPE_Xerror ANSI_ARGS(( int, char * ));

/* xmouse */
IMPORT_MPI_API int MPE_Get_mouse_press ANSI_ARGS(( MPE_XGraph, int *, int *, int * ));
IMPORT_MPI_API int MPE_Iget_mouse_press ANSI_ARGS(( MPE_XGraph, int *, int *, int *,
					    int * ));
IMPORT_MPI_API int MPE_Get_drag_region ANSI_ARGS(( MPE_XGraph, int, int, 
					   int *, int *, int *, int * ));
IMPORT_MPI_API int MPE_Get_drag_region_fixratio ANSI_ARGS(( MPE_XGraph, int, double, 
						    int *, int *, 
						    int *, int * ));

#ifdef __cplusplus
}
#endif


#endif

