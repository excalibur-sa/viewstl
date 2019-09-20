/*
 * ViewStl 1.0
 *      Copyright (C) 2004 Doug Dingus (ViewStl 0.35)
 *      Copyright (C) 2019 Andrew Hall (ViewStl 1.0 Modifications and later)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


/* Basic Includes for OGL, GLUT, and other */
/* WIN32 needs some additional ones  */

/* Integrated patches from Hans  hhof@users.sourceforge.net 0.35 */

// TODO: Replace sscanf with slightly safer alternatives. They can overrun their dest arrays currently.

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
#include <sys/inotify.h> /* inotify interfaces */
#include <sys/poll.h> /* needed for non-blocking inotify checks */
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

int checkFileChanged() {
    if (poll(reload_pfd, 1, 0) < 1) return 0;

    int length = read(reload_fd, reload_buffer, 1024 * (sizeof(struct inotify_event) + 16));
    if (length < 0) return 0;

    int ptr = 0;
    while (ptr < length) {
        struct inotify_event *e = (struct inotify_event*) &reload_buffer[ptr];
        if (!e->len) continue;
        if (e->wd == reload_wd && strcmp(e->name, filename) == 0) return 1;
        ptr += (sizeof(struct inotify_event) + e->len);
    }
    return 0;
}

void inotify_cleanup() {
    inotify_rm_watch(reload_fd, reload_wd);
    close(reload_fd);
}
#endif

/* This function reads through the array of polygons (poly_list) to find the */
/* largest and smallest vertices in the model.  This data will be used by the */
/* transform_model function to center the part at the origin for rotation */
/* and easy autoscale */
static void FindExtents(STL_data *stl)
{
    STL_extents *extents = &(stl->extents);

    for (unsigned int poly_idx = 0; poly_idx < stl->tris_size; poly_idx++)
    {
        float *vertex_a = stl->tris[poly_idx].vertex_a;
        float *vertex_b = stl->tris[poly_idx].vertex_b;
        float *vertex_c = stl->tris[poly_idx].vertex_c;

        /* Positive Extents */

        extents->x_max = (vertex_a[0] > extents->x_max) ? vertex_a[0] : extents->x_max;
        extents->y_max = (vertex_a[1] > extents->y_max) ? vertex_a[1] : extents->y_max;
        extents->z_max = (vertex_a[2] > extents->z_max) ? vertex_a[2] : extents->z_max;

        extents->x_max = (vertex_b[0] > extents->x_max) ? vertex_b[0] : extents->x_max;
        extents->y_max = (vertex_b[1] > extents->y_max) ? vertex_b[1] : extents->y_max;
        extents->z_max = (vertex_b[2] > extents->z_max) ? vertex_b[2] : extents->z_max;

        extents->x_max = (vertex_c[0] > extents->x_max) ? vertex_c[0] : extents->x_max;
        extents->y_max = (vertex_c[1] > extents->y_max) ? vertex_c[1] : extents->y_max;
        extents->z_max = (vertex_c[2] > extents->z_max) ? vertex_c[2] : extents->z_max;

        /* Negative Extents */

        extents->x_min = (vertex_a[0] < extents->x_min) ? vertex_a[0] : extents->x_min;
        extents->y_min = (vertex_a[1] < extents->y_min) ? vertex_a[1] : extents->y_min;
        extents->z_min = (vertex_a[2] < extents->z_min) ? vertex_a[2] : extents->z_min;

        extents->x_min = (vertex_b[0] < extents->x_min) ? vertex_b[0] : extents->x_min;
        extents->y_min = (vertex_b[1] < extents->y_min) ? vertex_b[1] : extents->y_min;
        extents->z_min = (vertex_b[2] < extents->z_min) ? vertex_b[2] : extents->z_min;

        extents->x_min = (vertex_c[0] < extents->x_min) ? vertex_c[0] : extents->x_min;
        extents->y_min = (vertex_c[1] < extents->y_min) ? vertex_c[1] : extents->y_min;
        extents->z_min = (vertex_c[2] < extents->z_min) ? vertex_c[2] : extents->z_min;
    }
}

/* This translates the center of the rectangular bounding box surrounding */
/* the model to the origin.  Makes for good rotation.  Also it does a quick */
/* Z depth calculation that will be used bring the model into view (mostly) */
static void TransformToOrigin(STL_data *stl) {
    float LongerSide, ViewAngle;
    STL_extents *extents = &(stl->extents);

    /* first transform into positive quadrant */
    for (unsigned int poly_idx = 0; poly_idx < stl->tris_size; poly_idx++) {
        stl->tris[poly_idx].vertex_a[0] += (0 - extents->x_min);
        stl->tris[poly_idx].vertex_a[1] += (0 - extents->y_min);
        stl->tris[poly_idx].vertex_a[2] += (0 - extents->z_min);


        stl->tris[poly_idx].vertex_b[0] += (0 - extents->x_min);
        stl->tris[poly_idx].vertex_b[1] += (0 - extents->y_min);
        stl->tris[poly_idx].vertex_b[2] += (0 - extents->z_min);

        stl->tris[poly_idx].vertex_c[0] += (0 - extents->x_min);
        stl->tris[poly_idx].vertex_c[1] += (0 - extents->y_min);
        stl->tris[poly_idx].vertex_c[2] += (0 - extents->z_min);
    }
    FindExtents(stl);
    /* Do quick Z_Depth calculation while part resides in ++ quadrant */
    /* Convert Field of view to radians */
    ViewAngle = ((FOV / 2) * (PI / 180));
    LongerSide = (extents->x_max > extents->y_max) ? extents->x_max : extents->y_max;

    /* Put the result where the main drawing function can see it */
    stl->transform.z_depth = ((LongerSide / 2) / tanf(ViewAngle));
    stl->transform.z_depth *= -1;

    /* Do another calculation for clip planes */
    /* Take biggest part dimension and use it to size the planes */
    if ((extents->x_max > extents->y_max) && (extents->x_max > extents->z_max))
        extents->ext_max = extents->x_max;
    if ((extents->y_max > extents->x_max) && (extents->y_max > extents->z_max))
        extents->ext_max = extents->y_max;
    if ((extents->z_max > extents->y_max) && (extents->z_max > extents->x_max))
        extents->ext_max = extents->z_max;

    /* Then calculate center and put it back to origin */
    for (unsigned int poly_idx = 0; poly_idx < stl->tris_size; poly_idx++) {
        stl->tris[poly_idx].vertex_a[0] -= (extents->x_max / 2);
        stl->tris[poly_idx].vertex_a[1] -= (extents->y_max / 2);
        stl->tris[poly_idx].vertex_a[2] -= (extents->z_max / 2);

        stl->tris[poly_idx].vertex_b[0] -= (extents->x_max / 2);
        stl->tris[poly_idx].vertex_b[1] -= (extents->y_max / 2);
        stl->tris[poly_idx].vertex_b[2] -= (extents->z_max / 2);

        stl->tris[poly_idx].vertex_c[0] -= (extents->x_max / 2);
        stl->tris[poly_idx].vertex_c[1] -= (extents->y_max / 2);
        stl->tris[poly_idx].vertex_c[2] -= (extents->z_max / 2);
    }
}

/* Sets up Projection matrix according to command switch -o or -p */
/* called from initgl and the window resize function */
static void SetView(int Width, int Height, STL_data *stl)
{
    float aspect = (float)Width / (float)Height;
    if (verbose)
        printf("Window Aspect is: %f\n", aspect);

    if (ViewFlag == PERSPECTIVE)
        gluPerspective(FOV,(GLfloat)Width/(GLfloat)Height,0.1f,(stl->transform.z_depth + stl->extents.ext_max));
    else if (ViewFlag == ORTHO)
        glOrtho((stl->extents.x_min*1.2f), (stl->extents.x_max*1.2f), (stl->extents.y_min*aspect), (stl->extents.y_max*aspect), -1.0f, 10.0f);

}


/* Frame rate counter.  Based off of the Oreilly OGL demo !  */
/* updates the global variables FrameCount & FrameRate each time it is called. */
/* called from the main drawing function */
static void GetFPS() 
{
    static clock_t last=0;
    clock_t now;
    float delta;

    if (++FrameCount >= FR_SAMPLES) {
        now  = clock();
        delta= (now - last) / (float) CLOCKS_PER_SEC;
        last = now;

        FrameRate = FR_SAMPLES / delta;
        FrameCount = 0;
    }
}


/* A general OpenGL initialization function. */
/* Called once from main() */
void InitGL(int Width, int Height)	        /* We call this right after our OpenGL window is created.*/
{
    glClearColor(0.1f, 0.0f, 0.0f, 0.0f);		/* This Will Clear The Background Color To Dark Red*/
    glClearDepth(1.0);				/* Enables Clearing Of The Depth Buffer*/
    glDepthFunc(GL_LESS);			        /* The Type Of Depth Test To Do*/
    glEnable(GL_DEPTH_TEST);		        /* Enables Depth Testing*/
    glShadeModel(GL_SMOOTH);			/* Enables Smooth Color Shading*/
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();				/* Reset The Projection Matrix*/

    SetView(Width, Height, model);  /* Setup the View Matrix */

    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
    /* Set up some lights and turn them on. */
    glLightfv(GL_LIGHT1, GL_POSITION, Light_Position);
    glLightfv(GL_LIGHT1, GL_AMBIENT,  Light_Ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  Light_Diffuse); 
    glEnable (GL_LIGHT1); 
}


/* The function called when our window is resized  */
void ReSizeGLScene(int Width, int Height)
{
    if (Height==0)	/* Prevent A Divide By Zero If The Window Is Too Small*/
        Height=1;

    glViewport(0, 0, Width, Height);    /* Reset The Current Viewport And Perspective Transformation*/
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    SetView(Width, Height, model);

    glMatrixMode(GL_MODELVIEW);
    update = YES;
}


/* The main drawing function. */
void DrawGLScene()
{
    if ((!update) && (!idle_draw))
        return;
    update = NO;
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);	/* Clear The Screen And The Depth Buffer*/
    /* for now use two different methods of scaling depending on ortho or perspective */
    if (ViewFlag == PERSPECTIVE)
    {
        glLoadIdentity();
        glTranslatef(model->transform.pan_x, model->transform.pan_y, (model->transform.z_depth + model->transform.scale));
        glRotatef(model->transform.rot_x, 1.0f, 0.0f, 0.0f);
        glRotatef(model->transform.rot_y, 0.0f, 1.0f, 0.0f);
    }

    /* ortho scaling is broken */
    if (ViewFlag == ORTHO)
    {
        glLoadIdentity();
        glTranslatef(model->transform.pan_x, model->transform.pan_y, -5.0);
        glRotatef(model->transform.rot_x, 1.0f, 0.0f, 0.0f);
        glRotatef(model->transform.rot_y, 0.0f, 1.0f, 0.0f);

    }
    for(unsigned int idx = 0 ; idx < model->tris_size ; idx++)
    {
        STL_triangle *tri = &model->tris[idx];
        glBegin(GL_POLYGON);
        glNormal3f(tri->normal[0], tri->normal[1], tri->normal[2]);
        glVertex3f(tri->vertex_a[0], tri->vertex_a[1], tri->vertex_a[2]);
        glVertex3f(tri->vertex_b[0], tri->vertex_b[1], tri->vertex_b[2]);
        glVertex3f(tri->vertex_c[0], tri->vertex_c[1], tri->vertex_c[2]);
        glEnd();
    }
    /* swap the buffers to display, since double buffering is used.*/
    glutSwapBuffers();
    GetFPS();  /* Get frame rate stats */

    /* Copy saved window name into temp string arg1 so that we can add stats */

    /* cut down on the number of redraws on window title.  Only draw once per sample*/
    if (FrameCount == 0) {
        char window_title_fps[sizeof(window_title)+32];
        snprintf(window_title_fps, sizeof(window_title_fps), "%s - %.2f FPS", window_title, FrameRate);
        glutSetWindowTitle(window_title_fps);
    }

    if (reload && checkFileChanged()) {
        printf("File change detected. Reloading model... (unimplemented)\n");
    }
}


/* The function called whenever a mouse button event occurs */
void mouseButtonPress(int button, int state, int x, int y)
{
    if (verbose)
        printf(" mouse--> %i %i %i %i\n", button, state, x, y); 
    BUTTON = button;
    MOUSEx = x;
    MOUSEy = y;
    update = YES;
}


/* The function called whenever a mouse motion event occurs */
void mouseMotionPress(int x, int y)
{
    if (verbose)
        printf("You did this with the mouse--> %i %i\n", x, y); 
    if (BUTTON == LMB)
    {
        model->transform.pan_x += ((MOUSEx - x)*(tanf(0.26179939)*(model->transform.z_depth+model->transform.scale)))*.005;
        model->transform.pan_y -= ((MOUSEy - y)*(tanf(0.26179939)*(model->transform.z_depth+model->transform.scale)))*.005;
        MOUSEx = x;
        MOUSEy = y;
    } 
    if (BUTTON == MMB)
    {
        model->transform.rot_x -= ((MOUSEy - y)*0.5);
        model->transform.rot_y -= ((MOUSEx - x)*0.5);
        MOUSEx = x;
        MOUSEy = y;
    } 
    if (BUTTON == RMB)
    {
        model->transform.scale += ((MOUSEy - y)*(tanf(0.26179939)*(model->transform.z_depth+model->transform.scale)))*.01;
        model->transform.orth_scale += ((MOUSEy - y)*(tanf(0.26179939)*(model->transform.z_depth+model->transform.orth_scale)))*.01;
        MOUSEx = x;
        MOUSEy = y;
    } 
    update = YES;
}


/* The function called whenever a key is pressed. */
void keyPressed(unsigned char key, int x, int y) 
{
    /* Keyboard debounce */
    /* I don't know which lib has this in win32 */
    sleep(0.1);

    /* Pressing escape kills everything --Have a nice day! */
    if (key == ESCAPE) 
    { 
        /* shut down our window */
        glutDestroyWindow(window); 

        /* exit the program...normal termination. */
        exit(0);                   
    }
    if (verbose)
        printf("You pressed key--> %i at %i, %i screen location\n", key, x, y);
    update = YES;
}


/* This function is for the special keys.  */
/* The dynamic viewing keys need to be time based */
void specialkeyPressed (int key, int x, int y)
{
    /* keep track of time between calls, if it exceeds a certian value, then */
    /* assume the user has released the key and wants to start fresh */
    static int first = YES;
    static clock_t last=0;
    clock_t now;
    float delta;

    /* Properly initialize the MOUSE vars on first time through this function */
    if (first)
    {
        first = NO;
        MOUSEx = x;
        MOUSEy = y;
    }

    /* If the clock exceeds a reasonable value, assume user has released F key */      now  = clock();
    delta= (now - last) / (float) CLOCKS_PER_SEC;
    last = now;
    if (delta > 0.1)
    {
        MOUSEx = x;
        MOUSEy = y;
    }

    switch(key) {
        case 1: /* Pan is assigned the F1 key */
            model->transform.pan_x += ((MOUSEx - x)*(tanf(0.26179939)*(model->transform.z_depth+model->transform.scale)))*.005;
            model->transform.pan_y -= ((MOUSEy - y)*(tanf(0.26179939)*(model->transform.z_depth+model->transform.scale)))*.005;
            MOUSEx = x; MOUSEy = y; break;
        case 2: /* Zoom or Scale is the F2 key */
            model->transform.scale += ((MOUSEy - y)*(tanf(0.26179939)*(model->transform.z_depth+model->transform.scale)))*.01;
            model->transform.orth_scale += ((MOUSEy - y)*(tanf(0.26179939)*(model->transform.z_depth+model->transform.orth_scale)))*.01;
            /* scale = scale - ((MOUSEy - y)*0.05);
               oScale = oScale - ((MOUSEy - y)*0.05);  */
            MOUSEx = x; MOUSEy = y; break;
        case 3: /* Rotate assigned the F3 key */
            model->transform.rot_y -= ((MOUSEx - x)*0.5);
            model->transform.rot_x -= ((MOUSEy - y)*0.5);
            MOUSEx = x; MOUSEy = y; break;
        case 4: /* Cool Display Stuff... */
            glPolygonMode(GL_FRONT, GL_FILL); break;
        case 5:
            glPolygonMode(GL_FRONT, GL_LINE);
            glPolygonMode(GL_BACK, GL_FILL); break;
        case 6:
            glPolygonMode(GL_FRONT, GL_LINE);
            glPolygonMode(GL_BACK, GL_POINT); break;
        case 7:
            glDisable(GL_CULL_FACE); break;
        case 8:
            glEnable(GL_CULL_FACE); break;
        default:
            break;
    }
    if (verbose)   
        printf("Special Key--> %i at %i, %i screen location\n", key, x, y);
    update = YES;
}

void usage(int e) {
    printf("Usage: viewstl [OPTIONS]... [FILE]\n");
    printf("View stereolithographic (.stl) 3D models.\n\n");
    printf("  -o (Ortho View EXPEREMENTAL)\n");
    printf("  -p (Perspective View [default])\n");
    printf("  -f (Redraw only on view change)\n");
    printf("  -v (Report debug info to STDOUT)\n");
#ifdef __linux__
    printf("  -r (Reload model on file change. (Linux only)\n");
#endif
    if (e) exit(1);
}

STL_data* loadStlFile(const char* filepath) {
    FILE *tmp_file = fopen(filepath, "rb");
    if (tmp_file == NULL) {
        int e = errno;
        printf("viewstl: %s: %s\n\n", filepath, strerror(e));
        usage(1);
    }
    struct stat path_stat;
    fstat(fileno(tmp_file), &path_stat);
    if (S_ISDIR(path_stat.st_mode)) {
        fclose(tmp_file);
        printf("viewstl: %s: %s\n\n", filepath, strerror(EISDIR));
        usage(1);
    }

    STL_data *tmp_stl = malloc(sizeof(STL_data));
    tmp_stl->extents.x_max = 0; tmp_stl->extents.x_min = 0;
    tmp_stl->extents.y_max = 0; tmp_stl->extents.y_min = 0;
    tmp_stl->extents.z_max = 0; tmp_stl->extents.z_min = 0;
    tmp_stl->transform.pan_x = 0;
    tmp_stl->transform.pan_y = 0;
    tmp_stl->transform.rot_x = 0;
    tmp_stl->transform.rot_y = 0;
    tmp_stl->transform.scale = 1;
    tmp_stl->transform.z_depth = -5;

    char buf[80]; char *chk_p;
    fread(buf, 1, sizeof(buf), tmp_file);
    chk_p = strstr(buf, "solid");
    if (!chk_p) { // STL is binary if chk_p is false
        readStlBinary(tmp_file, tmp_stl);
        tmp_stl->type = STL_TYPE_BINARY;
    } else {
        readStlAscii(tmp_file, tmp_stl);
        tmp_stl->type = STL_TYPE_ASCII;
    }

    fclose(tmp_file);
    return tmp_stl;
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0)
            ViewFlag = ORTHO;
        else if (strcmp(argv[i], "-p") == 0)
            ViewFlag = PERSPECTIVE;

        if (strcmp(argv[i], "-f") == 0)
            idle_draw = NO;

        if (strcmp(argv[i], "-v") == 0)
            verbose = YES;
#ifdef __linux__
        if (strcmp(argv[i], "-r") == 0)
            reload = YES;
#endif

        if (filename == NULL)
            filename = argv[i];
    }

    if (!filename) {
        printf("%s: No file was specified.\n", basename(argv[0]));
        usage(1);
    }

    if (verbose) printf("Loading file %s ...\n", filename);
    model = loadStlFile(filename);
    if (verbose) printf("%i bytes allocated!\n", model->_tris_malloc_size);

    FindExtents(model);
    TransformToOrigin(model);

    if (verbose) {
        printf("File Processed\n");
        //printf("Poly Count = %i\n", old_poly_count);
        printf("Poly Count = %u\n", model->tris_size);
        printf("Part extents:\nx: %f/%f y: %f/%f z: %f/%f\n",
               model->extents.x_min, model->extents.x_max, model->extents.y_min,
               model->extents.y_max, model->extents.z_min, model->extents.z_max);
    }

    printf("Running in %s mode.\n", (ViewFlag == PERSPECTIVE ? "perspective" : "orthographic"));

    if (!idle_draw)
        printf("Automatic redraw disabled.\n");

#ifdef __linux__
    if (reload) {
        reload_fd = inotify_init();

        if (reload_fd < 0) {
            printf("Something went wrong while enabling inotify. Disabling file watcher.\n");
            reload = 0;
        } else {
            reload_wd = inotify_add_watch(reload_fd, dirname(filename), IN_CREATE | IN_MODIFY | IN_DELETE);
            reload_pfd[0].fd = reload_fd;
            reload_pfd[0].events = POLLIN;
            printf("Watching for file changes.\n");
        }
    }
#endif

    /* Initialize GLUT state - glut will take any command line arguments that pertain to it or 
       X Windows - look at its documentation at http://reality.sgi.com/mjk/spec3/spec3.html */  
    glutInit(&argc, argv);  

    /* Select type of Display mode:   
       Double buffer 
       RGBA color
       Depth buffered for automatic clipping */  
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);  

    /* get a 640 x 480 window */
    glutInitWindowSize(640, 480);  

    /* the window starts at the upper left corner of the screen */
    glutInitWindowPosition(0, 0);  

    snprintf(window_title, sizeof(window_title), "ViewStl 1.0 viewing: %s (%s) - %i polys - %liKB (%iKB alloc)",
            filename, (model->type == STL_TYPE_ASCII ? "ascii" : "binary"),
            model->tris_size, (model->tris_size * sizeof(STL_triangle))/1024, model->_tris_malloc_size/1024);

    window = glutCreateWindow(window_title);

    /* Register the event callback functions since we are using GLUT */
    glutDisplayFunc(&DrawGLScene); /* Register the function to do all our OpenGL drawing. */
    glutIdleFunc(&DrawGLScene); /* Even if there are no events, redraw our gl scene. */
    glutReshapeFunc(&ReSizeGLScene); /* Register the function called when our window is resized. */
    glutKeyboardFunc(&keyPressed); /* Register the function called when the keyboard is pressed. */
    glutSpecialFunc(&specialkeyPressed); /* Register the special key function */ 
    glutMouseFunc(&mouseButtonPress); /* Register the function called when the mouse buttons are pressed */
    glutMotionFunc(&mouseMotionPress); /*Register the mouse motion function */

    /* Initialize our window. */
    InitGL(640, 480);

    /* Start Event Processing Engine */
#ifdef __linux__
    atexit(inotify_cleanup);
#endif
    glutMainLoop();  

    return 1;
}


