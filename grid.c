#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>		// usleep
#include <GLES/gl.h>
#include "boing.h"
#include <math.h>

static GLfloat colorBufferGrid   [GRID_COLOR_BUFFER_SIZE];
static GLfloat vertexBufferGrid  [GRID_VERTEX_BUFFER_SIZE];
static int iColorBufferGrid;
static int iVertexBufferGrid;

static float COLOR_PURPLE[]     = {0.5f,  0.1f,  0.5f,  1.0f};
static float COLOR_PURPLE2[]    = {0.4f,  0.0f,  0.4f,  1.0f};

void InitGrid(float size,  float widthLine)
{
    int 	row, col;
    int		cellTotal	= 12;
    float	cellSize	= size / cellTotal;
    float	z_offset	= 0;
    float	x_left, x_right;
    float	y_top, y_bottom;

    iVertexBufferGrid = 0;
    iColorBufferGrid  = 0;
    y_bottom = - size / 2;
    float z_front = z_offset + size / 2;
    float z_back  = z_offset;

    for ( col = cellTotal + 1; col > 0; col--)
    {
        // Compute co-ords of line.
        x_left   = - size / 2 + col * cellSize;
        x_right  = x_left + widthLine;

        //iColorBufferGrid = SetColorBuffer42(colorBufferGrid, iColorBufferGrid, COLOR_PURPLE, COLOR_PURPLE2);
        iColorBufferGrid = SetColorBuffer4(colorBufferGrid, iColorBufferGrid, COLOR_PURPLE);

        //** LB, RB, LF, RF
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_left);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_bottom);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_back);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_right);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_bottom);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_back);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_left);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_bottom);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_front);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_right);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_bottom);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_front);
    }

    y_top    =  size / 2;
    y_bottom = -size / 2 - widthLine;

    // Draw vertical lines (as skinny 3D rectangles).
    for ( col = 0; col <= cellTotal; col++ )
    {
        // Compute co-ords of line.
        x_left   = -size / 2 + col * cellSize;
        x_right  = x_left + widthLine;

        // iColorBufferGrid = SetColorBuffer42(colorBufferGrid, iColorBufferGrid, COLOR_PURPLE, COLOR_PURPLE2);
        iColorBufferGrid = SetColorBuffer4(colorBufferGrid, iColorBufferGrid, COLOR_PURPLE);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_left);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_bottom);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_offset);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_right);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_bottom);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_offset);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_left);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_top);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_offset);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_right);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_top);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_offset);
    }

    x_left   = -size / 2;
    x_right  =  size / 2 + widthLine;

    for ( row = 0; row <= cellTotal; row++ )
    {
        // Compute co-ords of line.
        y_top = size / 2 - row * cellSize;
        y_bottom = y_top - widthLine;
        //iColorBufferGrid = SetColorBuffer42(colorBufferGrid, iColorBufferGrid, COLOR_PURPLE, COLOR_PURPLE2);
        iColorBufferGrid = SetColorBuffer4(colorBufferGrid, iColorBufferGrid, COLOR_PURPLE);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_left);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_top);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_offset);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_left);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_bottom);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_offset);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_right);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_top);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_offset);

        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, x_right);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, y_bottom);
        iVertexBufferGrid = PutBuffer(vertexBufferGrid, iVertexBufferGrid, z_offset);
    }
}

void DrawGrid(float x, float y, float z)
{
    glPushMatrix();
    glMatrixMode( GL_MODELVIEW );
    glDisable(GL_TEXTURE_2D);
    glTranslatef(x, y, z);
    //glRotatef( 10.0f, 1.0f, 0.0f, 0.0f );
    DrawObjectCCW(vertexBufferGrid, colorBufferGrid, GRID_POLY_COUNT);
    glPopMatrix();
}
