
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>		// usleep
#include <GLES/gl.h>
#include "boing.h"
#include <math.h>

float COLOR_RED[]       = {0.9f,  0.0f,  0.0f,  1.0f};
float COLOR_WHITE[]     = {0.9f,  0.9f,  0.9f,  1.0f};
//float COLOR_RED2[]    = {0.5f,  0.0f,  0.0f,  1.0f};
float COLOR_RED2[]      = {0.9f,  0.0f,  0.0f,  1.0f};
//float COLOR_WHITE2[]  = {0.7f,  0.7f,  0.7f,  1.0f};
float COLOR_WHITE2[]    = {0.9f,  0.9f,  0.9f,  1.0f};
float COLOR_SHADOW[]    = {0.35f, 0.35f, 0.35f, 1.0f};
//float COLOR_SHADOW2[] = {0.25f, 0.25f, 0.25f, 1.0f};


static GLfloat colorBufferBoing  [BOING_COLOR_BUFFER_SIZE];
static GLfloat colorBufferShadow [BOING_COLOR_BUFFER_SIZE];
static GLfloat vertexBufferBoing [BOING_VERTEX_BUFFER_SIZE];
static int iColorBufferBoing  = 0;
static int iColorBufferShadow = 0;
static int iVertexBufferBoing = 0;
static float deg_rot_y        = 0.0f;
static float deg_rot_y_inc    = 2.0f;
static float ball_x           =   BOING_RADIUS;
static float ball_y           = - BOING_RADIUS;
static float ball_x_inc       = - 2.0f;
static float ball_y_inc       = 1.0f;
static bool colorToggle       = false;

bool PlaySound = false;

void DrawObjectCW(float * vertexBuffer, float * colorBuffer, int polygonCount)
{
    glEnableClientState(GL_COLOR_ARRAY);
    //glDisable(GL_CULL_FACE );
    glEnable(GL_CULL_FACE );
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnableClientState(GL_VERTEX_ARRAY);
    glColorPointer(4, GL_FLOAT, 0, colorBuffer);
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, polygonCount * 4);
    // Disable the vertices & color buffer.
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_CULL_FACE );
}

void DrawObjectCCW(float * vertexBuffer, float * colorBuffer, int polygonCount)

{
    glEnableClientState(GL_COLOR_ARRAY);
    //glDisable(GL_CULL_FACE );
    glEnable(GL_CULL_FACE );
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnableClientState(GL_VERTEX_ARRAY);
    glColorPointer(4, GL_FLOAT, 0, colorBuffer);
    glVertexPointer(3, GL_FLOAT, 0, vertexBuffer);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, polygonCount * 4);
    // Disable the vertices & color buffer.
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_CULL_FACE );
}

float TruncateDeg(float deg)
{
    int ideg = (int) deg;
    return (float) ideg;
}

float random_f()
{
    return rand() / RAND_MAX;
}

void BounceBall(bool reverseX, bool reverseY, float offsetX, float offsetY)
{
    float sign;
    float deg;
    if ( ball_x >  (BOUNCE_WIDTH/2 + WALL_R_OFFSET ) )
    {
        ball_x_inc = -0.5f - 0.75f * random_f();
        deg_rot_y_inc = -deg_rot_y_inc;
    }
    if ( ball_x < -(BOUNCE_HEIGHT/2 + WALL_L_OFFSET) )
    {
        ball_x_inc =  0.5f + 0.75f * random_f();
        deg_rot_y_inc = -deg_rot_y_inc;
    }
    ball_x += ball_x_inc;

    if(reverseX)
        glTranslatef(-1 * (ball_x + offsetX), 0.0f, 0.0f);
    else
        glTranslatef(ball_x + offsetX, 0.0f, 0.0f );

    //because of the random_f variable this could possibly hit twice.
    if ( ball_y >  BOUNCE_HEIGHT/2     )
    {
        if (ball_y_inc > 0)
        {
            PlaySound = true;
            ball_y_inc = -0.75f - 1.0f * random_f();
        }
    }
    if ( ball_y < -BOUNCE_HEIGHT/2 * 0.85 )
    {
        if (ball_y_inc < 0)
            ball_y_inc =  0.75f + 1.0f * random_f();
    }
    ball_y += ball_y_inc;
    if(reverseY)
        glTranslatef(0.0f, -1 * (ball_y + offsetY), 0.0f );
    else
        glTranslatef(0.0f, ball_y + offsetY, 0.0f );

    // Simulate the effects of gravity on Y movement.
    if ( ball_y_inc < 0 ) sign = -1.0f;
    else sign = 1.0f;
    deg = (ball_y + BOUNCE_HEIGHT/2) * 90 / BOUNCE_HEIGHT;
    if ( deg > 80 ) deg = 80;
    if ( deg < 10 ) deg = 10;

    //ball_y_inc = sign * 4.0f * sin_deg(deg);
    ball_y_inc = sign * 4.0f * sin(DEGREES_TO_RADIANS(deg));
}


void DrawBoingBall(bool DrawShadow)
{
    glPushMatrix();
    glMatrixMode( GL_MODELVIEW );
    glDisable(GL_TEXTURE_2D);
    glRotatef( deg_rot_y, 0.0f, 1.0f, 0.0f );
    glRotatef( -30.0f, 1.0f, 0.0f, 0.0f );
    // Continually rotate ball around Y axis.
    deg_rot_y = TruncateDeg(deg_rot_y + deg_rot_y_inc);

    if ( DrawShadow )
        DrawObjectCW(vertexBufferBoing, colorBufferShadow, BOING_POLY_COUNT);
    else
        DrawObjectCW(vertexBufferBoing, colorBufferBoing,  BOING_POLY_COUNT);

    glPopMatrix();
}

float sin_deg(float deg)
{
    return sin(DEGREES_TO_RADIANS(deg));
}


float cos_deg(float deg)
{
    return cos(DEGREES_TO_RADIANS(deg));
}


int SetColorBuffer(GLfloat * colorBuffer, int start, float * color)
{
    int index;
    for(index = 0; index < 4; index++)
        colorBuffer[start + index] = color[index];
    return start + 4;
}

int SetColorBuffer42(float * colorBuffer, int start , float * color1, float * color2)
{
    start = SetColorBuffer(colorBuffer, start, color1);
    start = SetColorBuffer(colorBuffer, start, color2);
    start = SetColorBuffer(colorBuffer, start, color1);
    start = SetColorBuffer(colorBuffer, start, color2);
    return start;
}

int SetColorBuffer4(float * colorBuffer, int start , float * color)
{
    start = SetColorBuffer(colorBuffer, start, color);
    start = SetColorBuffer(colorBuffer, start, color);
    start = SetColorBuffer(colorBuffer, start, color);
    start = SetColorBuffer(colorBuffer, start, color);
    return start;
}

int PutBuffer(float * buffer, int start, float value)
{
    buffer[start] = value;
    return ++start;
}

void InitBoingBallBand(float long_lo, float long_hi, float radius)
{

    float lat_deg;
    // Iterate thru the points of a latitude circle.
    // A latitude circle is a 2D set of X,Z points.
    for (lat_deg = 0; lat_deg <= (360 - STEP_LATITUDE); lat_deg += STEP_LATITUDE )
    {

        iColorBufferShadow = SetColorBuffer4(colorBufferShadow, iColorBufferShadow, COLOR_SHADOW);
        // Color this polygon with red or white.

        if ( colorToggle )
            iColorBufferBoing = SetColorBuffer42(colorBufferBoing, iColorBufferBoing, COLOR_RED,COLOR_RED2);
        else
            iColorBufferBoing = SetColorBuffer42(colorBufferBoing, iColorBufferBoing, COLOR_WHITE, COLOR_WHITE2);


        colorToggle = !colorToggle;
        // Change color if drawing shadow.
        float cos_deg_lat_deg               = cos_deg(lat_deg);
        float sin_deg_lat_deg               = sin_deg(lat_deg);
        float rad_sin_long_lo               = sin_deg(long_lo) * radius;
        float cos_deg_long_hi_rad           = cos_deg(long_hi) * radius;
        float cos_deg_long_low_rad          = cos_deg(long_lo) * radius;
        float cos_deg_lat_deg_step_lat      = cos_deg(lat_deg  + STEP_LATITUDE);
        float sin_deg_lat_deg_step_lat      = sin_deg(lat_deg  + STEP_LATITUDE);
        float rad_sin_deg_long_lo_step_long = sin_deg(long_lo  + STEP_LONGITUDE) * radius;

        //South-East Bottom-Left
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, cos_deg_lat_deg          * rad_sin_long_lo);
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, cos_deg_long_low_rad);
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, sin_deg_lat_deg          * rad_sin_long_lo);
        //North-East Top-Right
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, cos_deg_lat_deg          * rad_sin_deg_long_lo_step_long );
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, cos_deg_long_hi_rad);
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, sin_deg_lat_deg          * rad_sin_deg_long_lo_step_long );
        //South-West Bottom-Right
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, cos_deg_lat_deg_step_lat * rad_sin_long_lo);
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, cos_deg_long_low_rad);
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, sin_deg_lat_deg_step_lat * rad_sin_long_lo);
        //North-West Top-Left
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, cos_deg_lat_deg_step_lat * rad_sin_deg_long_lo_step_long);
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, cos_deg_long_hi_rad);
        iVertexBufferBoing = PutBuffer(vertexBufferBoing, iVertexBufferBoing, sin_deg_lat_deg_step_lat * rad_sin_deg_long_lo_step_long);

#ifdef DEBUG_BOING_COORDS
        printf("(%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f)\n",
               vertexBufferBoing[iVertexBufferBoing -12],
               vertexBufferBoing[iVertexBufferBoing -11],
               vertexBufferBoing[iVertexBufferBoing -10],
               vertexBufferBoing[iVertexBufferBoing -9],
               vertexBufferBoing[iVertexBufferBoing -8],
               vertexBufferBoing[iVertexBufferBoing -7],
               vertexBufferBoing[iVertexBufferBoing -6],
               vertexBufferBoing[iVertexBufferBoing -5],
               vertexBufferBoing[iVertexBufferBoing -4],
               vertexBufferBoing[iVertexBufferBoing -3],
               vertexBufferBoing[iVertexBufferBoing -2],
               vertexBufferBoing[iVertexBufferBoing -1]);
#endif

    }

    // Toggle color so that next band will opposite red/white colors than this one.
    colorToggle = !colorToggle;
    // This circular band is done.
}

void InitBoingBall(float radius)
{
    // Build a faceted latitude slice of the Boing ball,
    // stepping same-sized vertical bands of the sphere.

    iVertexBufferBoing = 0;
    iColorBufferBoing = 0;
    float lon_deg;
    for (lon_deg = 0; lon_deg < 180; lon_deg += STEP_LONGITUDE )
    {
        // Draw a latitude circle at this longitude.
        InitBoingBallBand(lon_deg,  lon_deg + STEP_LONGITUDE, radius);
    }
}

