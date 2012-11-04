
#include <stdbool.h>
//#define DEBUG_BOING_COORDS
 
#define DEGREES_TO_RADIANS(__ANGLE) ((__ANGLE) / 180.0 * M_PI)
#define BOING_RADIUS     (50)
#define STEP_LONGITUDE   (22.5)
#define STEP_LATITUDE    (22.5)
#define BOING_POLY_COUNT (128)
#define GRID_POLY_COUNT  (13 * 13 * 13)
#define GRID_SIZE        (BOING_RADIUS * 5.5)		
#define BOING_VERTEX_BUFFER_SIZE (BOING_POLY_COUNT * 4 * 3 * 4)
#define BOING_COLOR_BUFFER_SIZE  (BOING_POLY_COUNT * 4 * 4 * 4)
#define GRID_VERTEX_BUFFER_SIZE  (GRID_POLY_COUNT  * 4 * 3 * 4)
#define GRID_COLOR_BUFFER_SIZE   (GRID_POLY_COUNT  * 4 * 4 * 4)
#define BOUNCE_HEIGHT   (BOING_RADIUS * 2.1f)
#define BOUNCE_WIDTH    (BOING_RADIUS * 2.1f)
#define SHADOW_OFFSET_X (-15.0f)
#define SHADOW_OFFSET_Y (5.0f)
#define WALL_L_OFFSET   (5.0f)
#define WALL_R_OFFSET   (5.0f)
#define DIST_BALL       (BOING_RADIUS * 2.0f + BOING_RADIUS * 0.1f)
     
void DrawBoingBall(bool DrawShadow);
void InitBoingBall(float radius);
void InitGrid(float size,  float widthLine);
void DrawGrid(float x, float y, float z);
int SetColorBuffer(GLfloat * colorBuffer, int start, float * color);
int SetColorBuffer4(float * colorBuffer, int start , float * color);
int SetColorBuffer42(float * colorBuffer, int start , float * color1, float * color2);
int PutBuffer(float * buffer, int start, float value);
void DrawObjectCCW(float * vertexBuffer, float * colorBuffer, int polygonCount); 
void DrawObjectCW(float * vertexBuffer, float * colorBuffer, int polygonCount); 
void BounceBall(bool reverseX, bool reverseY, float offsetX, float offsetY);




