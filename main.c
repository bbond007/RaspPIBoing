#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "bcm_host.h"
#include "GLES/gl.h"
#include "EGL/egl.h"
#include <EGL/eglext_brcm.h>

//#include "EGL/eglext.h"
#include "boing.h"
#include "audio.h"

typedef struct
{
    uint32_t screen_width;
    uint32_t screen_height;
    Window win;
    Display * x_display;
// OpenGL|ES objects
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
} BOING_STATE_T;


//function prototypes:
static void init_ogl(BOING_STATE_T *state);
static void init_model_proj(BOING_STATE_T *state);
static void exit_func(BOING_STATE_T *state);


//------------------------------------------------------------------------------

static void init_ogl(BOING_STATE_T *state)
{
    int32_t success = 0;
    EGLBoolean result;
    EGLint num_config;

    static EGL_DISPMANX_WINDOW_T nativewindow;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;
    EGLConfig config;

    static const EGLint attribute_list[] =
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_SURFACE_TYPE,
//        EGL_WINDOW_BIT,
        EGL_PIXMAP_BIT | EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    // get an EGL display connection
    state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(state->display!= EGL_NO_DISPLAY);

    // initialize the EGL display connection
    result = eglInitialize(state->display, NULL, NULL);
    assert(EGL_FALSE != result);

    // get an appropriate EGL frame buffer configuration
    result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
    assert(EGL_FALSE != result);

    // create an EGL rendering context
    state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, NULL);
    assert(state->context!= EGL_NO_CONTEXT);

    // create an EGL window surface
    if(state->x_display == NULL)
    {
        success = graphics_get_display_size(0 /* LCD */, &state->screen_width, &state->screen_height);
        assert( success >= 0 );


        dst_rect.x = 0;
        dst_rect.y = 0;
        dst_rect.width = state->screen_width;
        dst_rect.height = state->screen_height;

        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.width = state->screen_width << 16;
        src_rect.height = state->screen_height << 16;

        dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
        dispman_update = vc_dispmanx_update_start( 0 );

        dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
                          0/*layer*/, &dst_rect, 0/*src*/, &src_rect, DISPMANX_PROTECTION_NONE,
                          0 /*alpha*/, 0/*clamp*/, 0/*transform*/);

        nativewindow.element = dispman_element;
        nativewindow.width = state->screen_width;
        nativewindow.height = state->screen_height;
        vc_dispmanx_update_submit_sync( dispman_update );

        state->surface = eglCreateWindowSurface( state->display, config, &nativewindow, NULL );
        assert(state->surface != EGL_NO_SURFACE);
    }
    else
    {
        EGLint pixel_format = EGL_PIXEL_FORMAT_ARGB_8888_BRCM;
        //EGLint pixel_format = EGL_PIXEL_FORMAT_RGB_565_BRCM;
        EGLint rt;
        eglGetConfigAttrib(state->display, config, EGL_RENDERABLE_TYPE, &rt);

        if (rt & EGL_OPENGL_ES_BIT)
        {
            pixel_format |= EGL_PIXEL_FORMAT_RENDER_GLES_BRCM;
            pixel_format |= EGL_PIXEL_FORMAT_GLES_TEXTURE_BRCM;
        }

        if (rt & EGL_OPENGL_ES2_BIT)
        {
            pixel_format |= EGL_PIXEL_FORMAT_RENDER_GLES2_BRCM;
            pixel_format |= EGL_PIXEL_FORMAT_GLES2_TEXTURE_BRCM;
        }

        if (rt & EGL_OPENVG_BIT)
        {
            pixel_format |= EGL_PIXEL_FORMAT_RENDER_VG_BRCM;
            pixel_format |= EGL_PIXEL_FORMAT_VG_IMAGE_BRCM;
        }

        if (rt & EGL_OPENGL_BIT)
        {
            pixel_format |= EGL_PIXEL_FORMAT_RENDER_GL_BRCM;
        }

        EGLint pixmap[5];
        pixmap[0] = 0;
        pixmap[1] = 0;
        pixmap[2] = state->screen_width;
        pixmap[3] = state->screen_height;
        pixmap[4] = pixel_format;

        eglCreateGlobalImageBRCM(state->screen_width, state->screen_height, pixmap[4], 0, state->screen_width*4, pixmap);
        state->surface = eglCreatePixmapSurface(state->display, config, pixmap, 0);
        assert(state->surface != EGL_NO_SURFACE );

    }

    // connect the context to the surface
    result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
    assert(EGL_FALSE != result);
    // Set background color and clear buffers
    glClearColor(0.55f, 0.55f, 0.55f, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT );
    glClear( GL_DEPTH_BUFFER_BIT );
    glShadeModel(GL_FLAT);
    printf("init_ogl: screen size = %d * %d\n", state->screen_width, state->screen_height);

}

//------------------------------------------------------------------------------

static void init_model_proj(BOING_STATE_T *state)
{
    float nearp = 0.01f;
    float farp =  1000.0f;
    float hht;
    float hwd;

    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    glViewport(0, 0, (GLsizei)state->screen_width, (GLsizei)state->screen_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    hht = nearp * (float)tan(45.0 / 2.0 / 180.0 * M_PI);

    hwd = hht * (float)state->screen_width / (float)state->screen_height;

    if (state->x_display == NULL)
        glFrustumf(-hwd, hwd, -hht, hht, nearp, farp);
    else
        glFrustumf(hwd, -hwd, hht, -hht, nearp, farp);

}

//------------------------------------------------------------------------------

static void redraw_scene_common(BOING_STATE_T *state)
{
    // Start with a clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //glRotatef(180, 0.0f, 0.0f, 0.0f);
    glTranslatef(0.f, 0.f, -BOING_RADIUS * 6.5);
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    DrawGrid(0,0, -BOING_RADIUS);
    BounceBall(state->x_display==NULL?false:true, true, 10.0f, 15.0f);
    DrawBoingBall(true);
    if(state->x_display==NULL)
        glTranslatef(SHADOW_OFFSET_X, SHADOW_OFFSET_Y, 0.0f);
    else
        glTranslatef(-1 * (SHADOW_OFFSET_X), SHADOW_OFFSET_Y, 0.0f);

    DrawBoingBall(false);

}
//------------------------------------------------------------------------------

static void redraw_scene_fullscreen(BOING_STATE_T *state)
{
    redraw_scene_common(state);
    eglSwapBuffers(state->display, state->surface);
}

//------------------------------------------------------------------------------

static void redraw_scene_xwindow(BOING_STATE_T *state)
{
    static bool donesetup = false;
    static XWindowAttributes gwa;
    static XImage * image = NULL;
    static GC gc;
    int count;

    if (!donesetup)
    {
        donesetup = true;
        XGetWindowAttributes(state->x_display, state->win ,&gwa);
        //glViewport(0 ,0 , gwa.width, gwa.height);
#ifdef NATEDOGG_DEBUG
        printf("DEBUG: gwa.height = %d : gwa.height = %d : state->x_display = 0x%08x : state->win = 0x%x08\n",
               gwa.width, gwa.height, (unsigned int) state->x_display, (unsigned int) state->win);
#endif
        image = XGetImage(state->x_display, state->win, 0, 0, gwa.width, gwa.height, AllPlanes, ZPixmap);
        gc = DefaultGC(state->x_display, 0);
    }

    redraw_scene_common(state);

    glFinish();
    unsigned int *buffer = (unsigned int *)malloc(gwa.height * gwa.width * 4);
    glReadPixels(0, 0, gwa.width, gwa.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    for (count = 0; count < gwa.width * gwa.height / 2; count++)
    {
        unsigned int *dest = (unsigned int*)(&(image->data[0]));
        unsigned int src0 = buffer[count * 2];
        unsigned int src1 = buffer[count * 2 + 1];

        unsigned char r0, g0, b0;
        unsigned char r1, g1, b1;

        r0 = src0 & 0xff;
        g0 = (src0 >> 8) & 0xff;
        b0 = (src0 >> 16) & 0xff;
        r1 = src1 & 0xff;
        g1 = (src1 >> 8) & 0xff;
        b1 = (src1 >> 16) & 0xff;
        dest[count] = ((r0 >> 3) << 27) | ((g0 >> 2) << 21) | ((b0 >> 3) << 16)
                      | ((r1 >> 3) << 11) | ((g1 >> 2) << 5) | ((b1 >> 3) << 0);
    }

    free(buffer);
    XPutImage(state->x_display, state->win, gc, image, 0, 0, 0, 0, gwa.width, gwa.height);
}

//------------------------------------------------------------------------------

static void exit_func(BOING_STATE_T *state)
// Function to be passed to atexit().
{
    // clear screen;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    eglSwapBuffers(state->display , state->surface);
    // Release OpenGL resources
    eglMakeCurrent( state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    eglDestroySurface( state->display, state->surface );
    eglDestroyContext( state->display, state->context );
    eglTerminate( state->display );

    if(state->x_display != NULL)
        XDestroyWindow(state->x_display, state->win);

    printf("\nAmiga Boing Ball for Raspberry PI - bbond007\n\n");
} // exit_func()


//------------------------------------------------------------------------------

bool kbHit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------

bool create_x_window(BOING_STATE_T *state, int width, int height)
{
    state->x_display = XOpenDisplay(NULL); // open the standard display (the primary screen)
    if (state->x_display == NULL)
    {
        fputs("cannot connect to X server\n", stderr);
        return false;
    }

    Window root = DefaultRootWindow(state->x_display); // get the root window (usually the whole screen)

    XSetWindowAttributes swa;
    swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;

    state->screen_width = width;
    state->screen_height = height;
    // create a window with the provided parameters
    state->win = XCreateWindow (state->x_display, root, 0, 0, state->screen_width,
                                state->screen_height, 0, CopyFromParent, InputOutput,
                                CopyFromParent, CWEventMask, &swa );


    XMapWindow (state->x_display, state->win ); // make the window visible on the screen
    XStoreName (state->x_display, state->win, "RaspPIBoing.BIN" ); // give the window a name
    return true;
}

//==============================================================================
// globals and externals
//------------------------------------------------------------------------------
extern const signed char soundraw_data[];
extern const unsigned int soundraw_size;
extern bool PlaySound;
static volatile int terminate;
static BOING_STATE_T _state, *state=&_state;
//------------------------------------------------------------------------------

int main (int argc, char **argv)
{

    InitBoingBall(BOING_RADIUS);
    InitGrid(GRID_SIZE, 2.0f);
    bcm_host_init();
    // Clear application state
    memset( state, 0, sizeof( *state ) );
    // Start OGLES

    int iSoundDeviceNo = 0;
    bool useXwin = false;
    bool goLarge = false;

    XEvent xev;
    int i;
    if(argc > 1)
    {
        for (i=1; i < argc; i++)
        {
            if(strcmp(argv[i], "hdmi") == 0 || strcmp(argv[i], "HDMI") == 0)
                iSoundDeviceNo = 1;

            if(strcmp(argv[i], "none") == 0 || strcmp(argv[i], "NONE") == 0)
                iSoundDeviceNo = -1;

            if(strcmp(argv[i], "x") == 0 || strcmp(argv[i], "X") == 0)
                useXwin = true;

            if(strcmp(argv[i], "X") == 0)
                goLarge = true;
        }
    }

    if(useXwin)
    {
        printf("video output = X11:PixmapSurface");
        if(goLarge)
        {
            create_x_window(state, 640, 480);
            printf(" - large\n");
        }
        else
        {
            create_x_window(state, 320, 240);
            printf(" - small\n");
        }
    }
    else
    {
        printf("video output = fullscreen\n");
        state->x_display = NULL;
    }

    init_ogl(state);
    init_model_proj(state);
    AudioSampleInfo asiBoing;
    asiBoing.st = NULL;

    switch (iSoundDeviceNo)
    {
    case -1 :
        printf("sound = none\n");
        break;
    case  0 :
        printf("sound = headphone\n");
        break;
    case  1 :
        printf("sound = HDMI\n");
        break;
    }

    if (iSoundDeviceNo != -1)
        load_sample(&asiBoing, (uint8_t *) soundraw_data, soundraw_size, 8000, 16, 1, iSoundDeviceNo);

    /* GRRRRRRRRRRRRRRRR! XGetImage
    Error of failed request:  BadMatch (invalid parameter attributes)
    Major opcode of failed request:  73 (X_GetImage)
    Serial number of failed request:  12
    Current serial number in output stream:  12
    this is probably based on side-effects and not a good fix.
    but for now i'm happy this works... PRIME THE MSG LOOP..
    */

    if(useXwin)
    {
        for(i=0; i< 100; i++)
        {
            if(XPending(state->x_display))
                XNextEvent(state->x_display, &xev);

            usleep(10*1000);
        }
    }

    while (!kbHit() && !terminate)
    {
        usleep(5*1000);
        if(!useXwin)
        {
            redraw_scene_fullscreen(state);
        }
        else
        {
            while (XPending(state->x_display))
            {
                // check for events from the x-server
                XNextEvent(state->x_display, &xev);
                if (xev.type == KeyPress)
                    terminate = true;
            }
            redraw_scene_xwindow(state);
        }

        if(PlaySound && iSoundDeviceNo != -1)
        {
            PlaySound = false;
            play_sample(&asiBoing, false);
        }
    }

    if(asiBoing.st != NULL)
        delete_sample(&asiBoing);

    exit_func(state);

    return 0;
}
