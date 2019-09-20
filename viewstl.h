#ifndef VIEWSTL_VIEWSTL_H
#define VIEWSTL_VIEWSTL_H

#include <GL/gl.h>   // OpenGL itself.
#include <GL/glu.h>  // GLU support library.
#include <GL/glut.h> // GLUT support library.

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>     /* needed to sleep*/
#include <stdlib.h>    /* for malloc() and exit() */
#include <stdio.h>      /* needed for file & screen i/o */
#include <string.h>    /* for strcpy and relatives */
#include <stdint.h>    /*uintn_t and intn_t*/
#include <time.h>       /* For our FPS */
#include <math.h>       /* Gotta do some trig */
#include <errno.h>      /* Error checking */
#include <libgen.h>     /* basename() */
#ifdef __linux__
#include <sys/inotify.h> /* inotify interfaces */
#include <sys/poll.h> /* needed for non-blocking inotify checks */
#endif
#include "stl.h"

/* ASCII code for the various keys used */
#define ESCAPE 27     /* esc */
#define ROTyp  105    /* i   */
#define ROTym  109    /* m   */
#define ROTxp  107    /* k   */
#define ROTxm  106    /* j   */
#define SCAp   43     /* +   */
#define SCAm   45     /* -   */
/* The mouse buttons */
#define LMB 0	/* Left */
#define MMB 1   /* Middle */
#define RMB 2   /* Right */
/* Number of samples for frame rate */
#define FR_SAMPLES 10
/* ViewFlag arguments */
#define ORTHO 1
#define PERSPECTIVE 0
#define YES 1
#define NO 0
/* Other Constants */
#define PI 3.14159265358979323846
#define FOV 30   /* Field of view for perspective mode */
#define PCR 2  /* Helps pan adapt to varying model sizes */

/* Declarations ------------------------------------- */
char window_title[256];
char *filename;
int window; /* The number of our GLUT window */
int mem_size;
int MOUSEx = 0, MOUSEy = 0, BUTTON = 0;
/* Stuff for the frame rate calculation */
int FrameCount=0;
float FrameRate=0;
/* Settings for the light  */
float Light_Ambient[]=  { 0.1f, 0.1f, 0.1f, 1.0f };
float Light_Diffuse[]=  { 1.2f, 1.2f, 1.2f, 1.0f };
float Light_Position[]= { 2.0f, 2.0f, 0.0f, 1.0f };
int ViewFlag = 0; /* 0=perspective, 1=ortho */
int update = YES, idle_draw = YES;
int verbose = NO, reload = NO;

STL_data *model;

#ifdef __linux__
int reload_fd;
int reload_wd;
char reload_buffer[1024 * (sizeof(struct inotify_event) + 16)];
struct pollfd reload_pfd[1];
#endif

int main(int argc, char *argv[]);
void usage(int e);
int checkFileChanged();
void inotify_cleanup();

STL_data* loadStlFile(const char* filepath);
static void FindExtents(STL_data *stl);
static void TransformToOrigin(STL_data *stl);

void DrawGLScene();
static void GetFPS();

void InitGL(int Width, int Height);
void ReSizeGLScene(int Width, int Height);
static void SetView(int Width, int Height, STL_data *stl);

void keyPressed(unsigned char key, int x, int y);
void specialkeyPressed (int key, int x, int y);

void mouseButtonPress(int button, int state, int x, int y);
void mouseMotionPress(int x, int y);

#endif //VIEWSTL_VIEWSTL_H
