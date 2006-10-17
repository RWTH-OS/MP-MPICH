#define BLACK	0		/* 0x000000 */
#define BLUE	255		/* 0x0000FF */	
#define CYAN	65535		/* 0x00FFFF */
#define GREEN	65280		/* 0x00FF00 */
#define MAGENTA	16711935	/* 0xFF00FF */
#define PINK	16756655	/* 0xFFAFAF */
#define PURPLE	3342387		/* 0x330033 */
#define ORANGE	16762880	/* 0xFFC800 */
#define RED	16711680	/* 0xFF0000 */
#define YELLOW	16776960	/* 0xFFFF00 */
#define WHITE	16777215	/* 0xFFFFFF */

void perfometer();
void fperfometer(char* filename);
void mark_perfometer(int color, char *label);
