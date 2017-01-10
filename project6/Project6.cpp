#include <stdio.h>
// yes, I know stdio.h is not good C++, but I like the *printf( )
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include "glui.h"

//
//
//	This is a sample OpenGL / GLUT / GLUI program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with radio buttons
//
//	The left mouse button allows rotation
//	The middle mouse button allows scaling
//	The glui window allows:
//		1. The 3d object to be transformed
//		2. The projection to be changed
//		3. The color of the axes to be changed
//		4. The axes to be turned on and off
//		5. The transformations to be reset
//		6. The program to quit
//
//	Author:			Haoxiang Wang
//
//  Latest update:	April 9, 2015
//


//
// constants:
//
// NOTE: There are a bunch of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.
//
//

// title of these windows:

const char *WINDOWTITLE = { "Project #6 - Haoxiang Wang" };
const char *GLUITITLE = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE = { true };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// able to use the left mouse for either rotation or scaling,
// in case have only a 2-button mouse:

enum LeftButton
{
	ROTATE,
	SCALE
};


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT = { 4 };
const int MIDDLE = { 2 };
const int RIGHT = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


// window background color (rgba):

const float BACKCOLOR[] = { 0., 0., 0., 0. };


// line width for the axes:

const GLfloat AXES_WIDTH = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};


// the color definitions:
// this order must match the radio button order

const GLfloat Colors[][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE = { GL_LINEAR };
const GLfloat FOGDENSITY = { 0.30f };
const GLfloat FOGSTART = { 1.5 };
const GLfloat FOGEND = { 4. };



//
// non-constant global variables:
//

int	ActiveButton;			// current button that is down
GLuint	AxesList;			// list to hold the axes
int	AxesOn;					// != 0 means to draw the axes
int	DebugOn;				// != 0 means to print debugging info
int	DepthCueOn;				// != 0 means to use intensity depth cueing
GLUI *	Glui;				// instance of glui window
int	GluiWindow;				// the glut id for the glui window
int	LeftButton;				// either ROTATE or SCALE
//GLuint	BoxList;			// object display list
GLuint	PointList;
int	MainWindow;				// window id for main graphics window
GLfloat	RotMatrix[4][4];	// set by glui rotation widget
float	Scale, Scale2;		// scaling factors
int	WhichColor;				// index into Colors[ ]
int	WhichProjection;		// ORTHO or PERSP
int	Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;			// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets


//
// function prototypes:
//

void	Animate(void);
void	Buttons(int);
void	Display(void);
void	DoRasterString(float, float, float, char *);
void	DoStrokeString(float, float, float, float, char *);
float	ElapsedSeconds(void);
void	InitGlui(void);
void	InitGraphics(void);
void	InitLists(void);
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset(void);
void	Resize(int, int);
void	Visibility(int);

void	Arrow(float[3], float[3]);
void	Cross(float[3], float[3], float[3]);
float	Dot(float[3], float[3]);
float	Unit(float[3], float[3]);
void	Axes(float);
void	HsvRgb(float[3], float[3]);

//added into program
inline float SQR(float);
void Sliders(int);
void SetTimeStep(int);
void Vector(float, float, float, float*, float*, float*);
void streamlines(float, float, float);
void Advect(float*, float*, float*);
void DrawObject(float, float, float);

//Define for Project6
#define NX 10
#define NY 10
#define NZ 10
#define VECTORSCALE 0
#define FSRS 1
#define MAX_ITERATIONS 300
#define SOME_TOLERANCE 0.15

int BoundOn;
int VectorOn;
int StreamlinesOn;
int WhichWayDoProbe;
float TimeStep;

GLUI_HSlider * VectorScaleSlider;
float VectorScaleValue[1];
GLUI_StaticText * VectorScaleLable;
const float VECTORSCALEMIN = { 0.01f };
const float VECTORSCALEMAX = { 0.4f };
const char* VECTORSCALEFORMAT = { "Vector Scale: %5.3f" };

GLUI_HSlider * FSRSSlider;
float FSRSLowHigh[2];
GLUI_StaticText * FSRSLable;
const float FSRSMIN = { 0. };
const float FSRSMAX = { sqrt(12.f) };
const char* FSRSFORMAT = { "Flow Speed Range Slider: %5.3f - %5.3f" };

float ProbeXY[2];
float ProbeYZ[2];
float ProbeXZ[2];

GLboolean redraw_streamline = false;
int iterations = 0;

enum WhichProb
{
	OFF,
	SINGLE,
	MULTI,
	RIBBON,
	LINETRACE,
	BLOBTRACE
};

struct node
{
	float x, y, z;           // location
	float vx, vy, vz;        // vector tail
	float rgb[3];		     // the assigned color (to be used later)
	float s;				 // speed
};

struct node  Nodes[NX][NY][NZ], PlaneXY[NX][NY], PlaneXZ[NX][NZ], PlaneYZ[NY][NZ];

inline float SQR(float x)
{
	return x * x;
}

void
Sliders(int id)
{
	char str[128];

	switch (id)
	{
	case VECTORSCALE:
		sprintf(str, VECTORSCALEFORMAT, VectorScaleValue[0]);
		VectorScaleLable->set_text(str);
		break;
	case FSRS:
		sprintf(str, FSRSFORMAT, FSRSLowHigh[0], FSRSLowHigh[1]);
		FSRSLable->set_text(str);
		break;
	}
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void SetTimeStep(int id)
{

}

void
Vector(float x, float y, float z, float *vxp, float *vyp, float *vzp)
{
	*vxp = y * z * (y*y + z*z);
	*vyp = x * z * (x*x + z*z);
	*vzp = x * y * (x*x + y*y);
}

void streamlines(float x, float y, float z)
{
	float vx, vy, vz;
	//glLineWidth(1.);
	//glColor3f(0.0, 255., 255.);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < MAX_ITERATIONS; i++)
	{
		if (x < -1 || x > 1)
			break;
		if (y < -1 || y > 1)
			break;
		if (z < -1 || z > 1)
			break;
		glVertex3f(x, y, z);
		Vector(x, y, z, &vx, &vy, &vz);
		if (sqrt(SQR(vx) + SQR(vy) + SQR(vz)) < SOME_TOLERANCE)
			break;
		Advect(&x, &y, &z);
	}
	glEnd();
}

void Advect(float *x, float *y, float *z)
{
	float xa, ya, za;
	float xb, yb, zb;
	float vxa, vya, vza;
	float vxb, vyb, vzb;
	float xc, yc, zc;
	float vx, vy, vz;

	xa = *x;
	ya = *y;
	za = *z;
	Vector(xa, ya, za, &vxa, &vya, &vza);
	xb = xa + TimeStep * vxa;
	yb = ya + TimeStep * vya;
	zb = za + TimeStep * vza;
	Vector(xb, yb, zb, &vxb, &vyb, &vzb);
	vx = (vxa + vxb) / 2.;
	vy = (vya + vyb) / 2.;
	vz = (vza + vzb) / 2.;
	xc = xa + TimeStep * vx;
	yc = ya + TimeStep * vy;
	zc = za + TimeStep * vz;

	*x = xc;
	*y = yc;
	*z = zc;
}

void DrawObject(float dx, float dy, float dz)
{
	glColor3f(0., 255., 0.);
	glBegin(GL_LINE_STRIP);
	glVertex3f(0.0 + dx, 0.05 + dy, 0.0 + dz);
	glVertex3f(0.05 + dx, 0.0 + dy, 0.05 + dz);
	glVertex3f(-0.05 + dx, 0.0 + dy, 0.05 + dz);
	glVertex3f(0.0 + dx, 0.05 + dy, 0.0 + dz);
	glVertex3f(0.05 + dx, 0.0 + dy, -0.05 + dz);
	glVertex3f(-0.05 + dx, 0.0 + dy, -0.05 + dz);
	glVertex3f(0.0 + dx, 0.05 + dy, 0.0 + dz);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0.0 + dx, -0.05 + dy, 0.0 + dz);
	glVertex3f(0.05 + dx, 0.0 + dy, 0.05 + dz);
	glVertex3f(0.05 + dx, 0.0 + dy, -0.05 + dz);
	glVertex3f(0.0 + dx, -0.05 + dy, 0.0 + dz);
	glVertex3f(-0.05 + dx, 0.0 + dy, 0.05 + dz);
	glVertex3f(-0.05 + dx, 0.0 + dy, -0.05 + dz);
	glVertex3f(0.0 + dx, -0.05 + dy, 0.0 + dz);
	glEnd();
}

//
// main program:
//

int
main(int argc, char *argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit(&argc, argv);


	// setup all the graphics stuff:

	InitGraphics();


	// create the display structures that will not change:

	InitLists();


	// init all the global variables used by Display( ):
	// this will also post a redisplay
	// it is important to call this before InitGlui( )
	// so that the variables that glui will control are correct
	// when each glui widget is created

	Reset();


	// setup all the user interface stuff:

	InitGlui();


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutMainLoop();


	// this is here to make the compiler happy:

	return 0;
}



//
// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it
//

void
Animate(void)
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:
	redraw_streamline = true;
	iterations += 8;
	if (iterations >= MAX_ITERATIONS)
	{
		iterations = 0;
	}

	// force a call to Display( ) next time it is convenient:

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}




//
// glui buttons callback:
//

void
Buttons(int id)
{
	char str[256];

	switch (id)
	{
	case RESET:
		Reset();

		sprintf(str, VECTORSCALEFORMAT, VectorScaleValue[0]);
		VectorScaleLable->set_text(str);

		sprintf(str, FSRSFORMAT, FSRSLowHigh[0], FSRSLowHigh[1]);
		FSRSLable->set_text(str);
		FSRSSlider->set_float_limits(FSRSLowHigh[0], FSRSLowHigh[1]);


		Glui->sync_live();
		glutSetWindow(MainWindow);
		glutPostRedisplay();
		break;

	case QUIT:
		// gracefully close the glui window:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:

		Glui->close();
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Button ID %d\n", id);
	}

}



//
// draw the complete scene:
//

void
Display(void)
{
	if (DebugOn != 0)
	{
		fprintf(stderr, "Display\n");
	}


	// set which window we want to do the graphics into:

	glutSetWindow(MainWindow);


	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);


	// specify shading to be flat:

	glShadeModel(GL_FLAT);


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (WhichProjection == ORTHO)
		glOrtho(-3., 3., -3., 3., 0.1, 1000.);
	else
		gluPerspective(90., 1., 0.1, 1000.);


	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !

	gluLookAt(0., 0., 3., 0., 0., 0., 0., 1., 0.);


	// translate the objects in the scene:
	// note the minus sign on the z value
	// this is to make the appearance of the glui z translate
	// widget more intuitively match the translate behavior
	// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !

	glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);


	// rotate the scene:
	// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !

	glRotatef((GLfloat)Yrot, 0., 1., 0.);
	glRotatef((GLfloat)Xrot, 1., 0., 0.);
	glMultMatrixf((const GLfloat *)RotMatrix);


	// uniformly scale the scene:

	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
	GLfloat scale2 = 1. + Scale2;		// because glui translation starts at 0.
	if (scale2 < MINSCALE)
		scale2 = MINSCALE;
	glScalef((GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2);


	// set the fog parameters:
	// DON'T NEED THIS IF DOING 2D !

	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}


	// possibly draw the axes:

	if (AxesOn != 0)
	{
		glColor3fv(&Colors[WhichColor][0]);
		glCallList(AxesList);
	}


	// set the color of the object:

	glColor3fv(Colors[WhichColor]);


	// draw the current object:

	//project #6
	//bounding box
	if (BoundOn == GLUITRUE)
	{
		glLineWidth(1.);
		InitLists();
	}
	//vector cloud
	float head[3], tail[3];

	if (VectorOn == GLUITRUE)
	{
		for (int i = 0; i < NX; i++)
		{
			for (int j = 0; j < NY; j++)
			{
				for (int k = 0; k < NZ; k++)
				{
					//calculate head coordinates
					head[0] = (Nodes[i][j][k].x + (Nodes[i][j][k].vx / 2) * VectorScaleValue[0]);
					head[1] = (Nodes[i][j][k].y + (Nodes[i][j][k].vy / 2) * VectorScaleValue[0]);
					head[2] = (Nodes[i][j][k].z + (Nodes[i][j][k].vz / 2) * VectorScaleValue[0]);
					//calculate tail coordinates
					tail[0] = (Nodes[i][j][k].x - (Nodes[i][j][k].vx / 2) * VectorScaleValue[0]);
					tail[1] = (Nodes[i][j][k].y - (Nodes[i][j][k].vy / 2) * VectorScaleValue[0]);
					tail[2] = (Nodes[i][j][k].z - (Nodes[i][j][k].vz / 2) * VectorScaleValue[0]);
					//check speed range
					if (Nodes[i][j][k].s < FSRSLowHigh[0] || Nodes[i][j][k].s > FSRSLowHigh[1])
						continue;
					//draw arrows
					glLineWidth(1.);
					glColor3f(Nodes[i][j][k].rgb[0], Nodes[i][j][k].rgb[1], Nodes[i][j][k].rgb[2]);
					Arrow(tail, head);
				}
			}
		}
	}
	//streamlines
	if (StreamlinesOn == GLUITRUE)
	{
		for (int i = 0; i < NX; i += 2)
		{
			for (int j = 0; j < NY; j += 2)
			{
				for (int k = 0; k < NZ; k += 2)
				{
					glLineWidth(1.);
					glColor3f(0.0, 255., 255.);
					streamlines(Nodes[i][j][k].x, Nodes[i][j][k].y, Nodes[i][j][k].z);
					streamlines(Nodes[9][j][k].x, Nodes[9][j][k].y, Nodes[9][j][k].z);
					streamlines(Nodes[i][9][k].x, Nodes[i][9][k].y, Nodes[i][9][k].z);
					streamlines(Nodes[i][j][9].x, Nodes[i][j][9].y, Nodes[i][j][9].z);
					streamlines(Nodes[9][9][k].x, Nodes[9][9][k].y, Nodes[9][9][k].z);
					streamlines(Nodes[i][9][9].x, Nodes[i][9][9].y, Nodes[i][9][9].z);
					streamlines(Nodes[9][j][9].x, Nodes[9][j][9].y, Nodes[9][j][9].z);
					streamlines(Nodes[9][9][9].x, Nodes[9][9][9].y, Nodes[9][9][9].z);
				}
			}
		}

	}
	//probe
	float ProbeXYZ[3];
	ProbeXYZ[0] = (GLfloat)ProbeXY[0] + (GLfloat)ProbeXZ[0];
	ProbeXYZ[1] = (GLfloat)ProbeXY[1] + (GLfloat)ProbeYZ[1];
	ProbeXYZ[2] = -(GLfloat)ProbeYZ[0] - (GLfloat)ProbeXZ[1];

	if (ProbeXYZ[0] > 1)
		ProbeXYZ[0] = 1;
	else if (ProbeXYZ[0] < -1)
		ProbeXYZ[0] = -1;
	if (ProbeXYZ[1] > 1)
		ProbeXYZ[1] = 1;
	else if (ProbeXYZ[1] < -1)
		ProbeXYZ[1] = -1;
	if (ProbeXYZ[2] > 1)
		ProbeXYZ[2] = 1;
	else if (ProbeXYZ[2] < -1)
		ProbeXYZ[2] = -1;

	switch (WhichWayDoProbe)
	{
	case OFF:
		break;
	case SINGLE:
		//draw single
		iterations = MAX_ITERATIONS;
		DrawObject(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
		glLineWidth(2.);
		streamlines(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
		break;
	case MULTI:
		//draw multi
		iterations = MAX_ITERATIONS;
		DrawObject(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
		glLineWidth(2.);
		streamlines(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
		streamlines(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2] + 0.2);
		streamlines(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2] - 0.2);
		streamlines(ProbeXYZ[0], ProbeXYZ[1] + 0.2, ProbeXYZ[2]);
		streamlines(ProbeXYZ[0], ProbeXYZ[1] - 0.2, ProbeXYZ[2]);
		streamlines(ProbeXYZ[0] + 0.2, ProbeXYZ[1], ProbeXYZ[2]);
		streamlines(ProbeXYZ[0] - 0.2, ProbeXYZ[1], ProbeXYZ[2]);
		break;
	case RIBBON:
		//draw ribbon
		float x, y, z;
		float cx, cy, cz;
		float cxp, cyp, czp;
		float nx, ny, nz;
		float nxp, nyp, nzp;

		DrawObject(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);

		cx = x = ProbeXYZ[0] + 4 * 0.05;
		cy = y = ProbeXYZ[1];
		cz = z = ProbeXYZ[2];

		glBegin(GL_QUADS);
		for (int i = 0; i < 10; i++)
		{
			cxp = cx - 0.05;
			cyp = cy;
			czp = cz;

			for (int j = 0; j < 50; j++)
			{
				if (cx < -1 || cx > 1 || cxp < -1 || cxp > 1)
					break;
				if (cy < -1 || cy > 1 || cyp < -1 || cyp > 1)
					break;
				if (cz < -1 || cz > 1 || czp < -1 || czp > 1)
					break;

				nx = cx;
				ny = cy;
				nz = cz;
				Advect(&nx, &ny, &nz);

				nxp = cxp;
				nyp = cyp;
				nzp = czp;
				Advect(&nxp, &nyp, &nzp);

				glVertex3f(cx, cy, cz);
				glVertex3f(cxp, cyp, czp);
				glVertex3f(nxp, nyp, nzp);
				glVertex3f(nx, ny, nz);

				cx = nx;
				cy = ny;
				cz = nz;

				cxp = nxp;
				cyp = nyp;
				czp = nzp;
			}
			cx = x - i * 0.05;
			cy = y;
			cz = z;
		}
		glEnd();
		break;
	case LINETRACE:
		//draw line trace
		float lcx[7], lcy[7], lcz[7];

		DrawObject(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
		glLineWidth(2.);

		if (redraw_streamline == true)
		{
			for (int n = 0; n < 7; n++)
			{
				lcx[n] = ProbeXYZ[0] + 3 * 0.05 - n*0.05;
				lcy[n] = ProbeXYZ[1] + 3 * 0.05 - n*0.05;
				lcz[n] = ProbeXYZ[2] + 3 * 0.05 - n*0.05;
			}

			for (int i = 0; i < iterations; i++)
			{
				for (int j = 0; j < 7; j++)
				{
					if (lcx[j] < -1 || lcx[j] > 1)
						break;
					if (lcy[j] < -1 || lcy[j] > 1)
						break;
					if (lcz[j] < -1 || lcz[j] > 1)
						break;
					Advect(&lcx[j], &lcy[j], &lcz[j]);
				}
			}

			glBegin(GL_LINE_STRIP);
			glVertex3f(ProbeXYZ[0] + 4 * 0.05, ProbeXYZ[1] + 4 * 0.05, ProbeXYZ[2] + 4 * 0.05);
			for (int n = 0; n < 7; n++)
			{
				glVertex3f(lcx[n], lcy[n], lcz[n]);
			}
			glVertex3f(ProbeXYZ[0] - 4 * 0.05, ProbeXYZ[1] - 4 * 0.05, ProbeXYZ[2] - 4 * 0.05);
			glEnd();
		}
		break;
	case BLOBTRACE:
		//draw blob
		//create little cube
		struct node littleCube[8];
		littleCube[0].x = ProbeXYZ[0] - 0.05;
		littleCube[0].y = ProbeXYZ[1] - 0.05;
		littleCube[0].z = ProbeXYZ[2] + 0.05;
		littleCube[1].x = ProbeXYZ[0] + 0.05;
		littleCube[1].y = ProbeXYZ[1] - 0.05;
		littleCube[1].z = ProbeXYZ[2] + 0.05;
		littleCube[2].x = ProbeXYZ[0] - 0.05;
		littleCube[2].y = ProbeXYZ[1] + 0.05;
		littleCube[2].z = ProbeXYZ[2] + 0.05;
		littleCube[3].x = ProbeXYZ[0] + 0.05;
		littleCube[3].y = ProbeXYZ[1] + 0.05;
		littleCube[3].z = ProbeXYZ[2] + 0.05;
		littleCube[4].x = ProbeXYZ[0] - 0.05;
		littleCube[4].y = ProbeXYZ[1] - 0.05;
		littleCube[4].z = ProbeXYZ[2] - 0.05;
		littleCube[5].x = ProbeXYZ[0] + 0.05;
		littleCube[5].y = ProbeXYZ[1] - 0.05;
		littleCube[5].z = ProbeXYZ[2] - 0.05;
		littleCube[6].x = ProbeXYZ[0] - 0.05;
		littleCube[6].y = ProbeXYZ[1] + 0.05;
		littleCube[6].z = ProbeXYZ[2] - 0.05;
		littleCube[7].x = ProbeXYZ[0] + 0.05;
		littleCube[7].y = ProbeXYZ[1] + 0.05;
		littleCube[7].z = ProbeXYZ[2] - 0.05;
		//tracing
		if (redraw_streamline == true)
		{
			for (int i = 0; i < iterations; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					if (littleCube[j].x < -1 || littleCube[j].x > 1)
						break;
					if (littleCube[j].y < -1 || littleCube[j].y > 1)
						break;
					if (littleCube[j].z < -1 || littleCube[j].z > 1)
						break;
					Advect(&littleCube[j].x, &littleCube[j].y, &littleCube[j].z);
				}
			}
				//redraw object
			DrawObject(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
			glLineWidth(2.);
			//draw little cube
				glBegin(GL_QUADS);
				glVertex3f(littleCube[0].x, littleCube[0].y, littleCube[0].z);
				glVertex3f(littleCube[1].x, littleCube[1].y, littleCube[1].z);
				glVertex3f(littleCube[3].x, littleCube[3].y, littleCube[3].z);
				glVertex3f(littleCube[2].x, littleCube[2].y, littleCube[2].z);

				glVertex3f(littleCube[1].x, littleCube[1].y, littleCube[1].z);
				glVertex3f(littleCube[5].x, littleCube[5].y, littleCube[5].z);
				glVertex3f(littleCube[7].x, littleCube[7].y, littleCube[7].z);
				glVertex3f(littleCube[3].x, littleCube[3].y, littleCube[3].z);

				glVertex3f(littleCube[5].x, littleCube[5].y, littleCube[5].z);
				glVertex3f(littleCube[4].x, littleCube[4].y, littleCube[4].z);
				glVertex3f(littleCube[6].x, littleCube[6].y, littleCube[6].z);
				glVertex3f(littleCube[7].x, littleCube[7].y, littleCube[7].z);

				glVertex3f(littleCube[4].x, littleCube[4].y, littleCube[4].z);
				glVertex3f(littleCube[0].x, littleCube[0].y, littleCube[0].z);
				glVertex3f(littleCube[2].x, littleCube[2].y, littleCube[2].z);
				glVertex3f(littleCube[6].x, littleCube[6].y, littleCube[6].z);

				glVertex3f(littleCube[0].x, littleCube[0].y, littleCube[0].z);
				glVertex3f(littleCube[1].x, littleCube[1].y, littleCube[1].z);
				glVertex3f(littleCube[5].x, littleCube[5].y, littleCube[5].z);
				glVertex3f(littleCube[4].x, littleCube[4].y, littleCube[4].z);

				glVertex3f(littleCube[2].x, littleCube[2].y, littleCube[2].z);
				glVertex3f(littleCube[3].x, littleCube[3].y, littleCube[3].z);
				glVertex3f(littleCube[7].x, littleCube[7].y, littleCube[7].z);
				glVertex3f(littleCube[6].x, littleCube[6].y, littleCube[6].z);
				glEnd();
		}
		break;
	}

	// draw some gratuitous text that just rotates on top of the scene:

	//glDisable( GL_DEPTH_TEST );
	//glColor3f( 0., 1., 1. );
	//DoRasterString( 0., 1., 0., "Point Cloud in Beaver Orange" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0., 100., 0., 100.);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1., 1., 1.);
	DoRasterString(5., 5., 0., "Project #6");


	// swap the double-buffered framebuffers:

	glutSwapBuffers();


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
}

//
// use glut to display a string of characters using a raster font:
//

void
DoRasterString(float x, float y, float z, char *s)
{
	char c;			// one character to print

	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}



//
// use glut to display a string of characters using a stroke font:
//

void
DoStrokeString(float x, float y, float z, float ht, char *s)
{
	char c;			// one character to print

	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05 + 33.33);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}



//
// return the number of seconds since the start of the program:
//

float
ElapsedSeconds(void)
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.;
}



//
// initialize the glui window:
//

void
InitGlui(void)
{
	GLUI_Panel *panel;
	GLUI_RadioGroup *group;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;

	GLUI_Panel *bigpanel;
	GLUI_Spinner *spinner;
	char str[128];
	// setup the glui window:

	glutInitWindowPosition(INIT_WINDOW_SIZE + 50, 0);
	Glui = GLUI_Master.create_glui((char *)GLUITITLE);


	Glui->add_statictext((char *)GLUITITLE);
	Glui->add_separator();

	Glui->add_checkbox("Axes", &AxesOn);

	Glui->add_checkbox("Bounding Box", &BoundOn);

	Glui->add_checkbox("Perspective", &WhichProjection);

	Glui->add_checkbox("Intensity Depth Cue", &DepthCueOn);

	Glui->add_checkbox("Vector Cloud", &VectorOn);

	Glui->add_checkbox("Streamlines", &StreamlinesOn);

	//added for project6

	bigpanel = Glui->add_panel("Probe");
	group = Glui->add_radiogroup_to_panel(bigpanel, &WhichWayDoProbe);
	Glui->add_radiobutton_to_group(group, "Off");
	Glui->add_radiobutton_to_group(group, "Single");
	Glui->add_radiobutton_to_group(group, "Multi");
	Glui->add_radiobutton_to_group(group, "Ribbon");
	Glui->add_radiobutton_to_group(group, "Line Trace");
	Glui->add_radiobutton_to_group(group, "Blob");
	panel = Glui->add_panel_to_panel(bigpanel, "Move the probe");
	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Probe XY", GLUI_TRANSLATION_XY, &ProbeXY[0]);
	trans->set_speed(0.05f);
	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Probe YZ", GLUI_TRANSLATION_XY, &ProbeYZ[0]);
	trans->set_speed(0.05f);
	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Probe ZX", GLUI_TRANSLATION_XY, &ProbeXZ[0]);
	trans->set_speed(0.05f);

	Glui->add_separator();

	spinner = Glui->add_spinner("Time Step", GLUI_SPINNER_FLOAT, &TimeStep, 0, (GLUI_Update_CB)SetTimeStep);
	spinner->set_float_limits(0.02, 1.);

	Glui->add_separator();

	VectorScaleSlider = Glui->add_slider(false, GLUI_HSLIDER_FLOAT, VectorScaleValue, VECTORSCALE, (GLUI_Update_CB)Sliders);
	VectorScaleSlider->set_float_limits(VECTORSCALEMIN, VECTORSCALEMAX);
	VectorScaleSlider->set_w(200);		// good slider width
	sprintf(str, VECTORSCALEFORMAT, VectorScaleValue[0]);
	VectorScaleLable = Glui->add_statictext(str);

	Glui->add_separator();

	FSRSSlider = Glui->add_slider(true, GLUI_HSLIDER_FLOAT, FSRSLowHigh, FSRS, (GLUI_Update_CB)Sliders);
	FSRSSlider->set_float_limits(FSRSMIN, FSRSMAX);
	FSRSSlider->set_w(200);		// good slider width
	sprintf(str, FSRSFORMAT, FSRSLowHigh[0], FSRSLowHigh[1]);
	FSRSLable = Glui->add_statictext(str);

	Glui->add_separator();

	//panel = Glui->add_panel(  "Axes Color" );
	//	group = Glui->add_radiogroup_to_panel( panel, &WhichColor );
	//		Glui->add_radiobutton_to_group( group, "Red" );
	//		Glui->add_radiobutton_to_group( group, "Yellow" );
	//		Glui->add_radiobutton_to_group( group, "Green" );
	//		Glui->add_radiobutton_to_group( group, "Cyan" );
	//		Glui->add_radiobutton_to_group( group, "Blue" );
	//		Glui->add_radiobutton_to_group( group, "Magenta" );
	//		Glui->add_radiobutton_to_group( group, "White" );
	//		Glui->add_radiobutton_to_group( group, "Black" );

	panel = Glui->add_panel("Object Transformation");

	rot = Glui->add_rotation_to_panel(panel, "Rotation", (float *)RotMatrix);

	// allow the object to be spun via the glui rotation widget:

	rot->set_spin(1.0);


	Glui->add_column_to_panel(panel, GLUIFALSE);
	scale = Glui->add_translation_to_panel(panel, "Scale", GLUI_TRANSLATION_Y, &Scale2);
	scale->set_speed(0.005f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0]);
	trans->set_speed(0.05f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans Z", GLUI_TRANSLATION_Z, &TransXYZ[2]);
	trans->set_speed(0.05f);

	Glui->add_checkbox("Debug", &DebugOn);


	panel = Glui->add_panel("", GLUIFALSE);

	Glui->add_button_to_panel(panel, "Reset", RESET, (GLUI_Update_CB)Buttons);

	Glui->add_column_to_panel(panel, GLUIFALSE);

	Glui->add_button_to_panel(panel, "Quit", QUIT, (GLUI_Update_CB)Buttons);


	// tell glui what graphics window it needs to post a redisplay to:

	Glui->set_main_gfx_window(MainWindow);


	// set the graphics window's idle function if needed:

	GLUI_Master.set_glutIdleFunc(Animate);
}



//
// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
//

void
InitGraphics(void)
{
	float hsv[3], rgb[3];
	float vx, vy, vz;

	for (int i = 0; i < NX; i++)
	{
		float x = -1. + 2. * (float)i / (float)(NX - 1);
		for (int j = 0; j < NY; j++)
		{
			float y = -1. + 2. * (float)j / (float)(NY - 1);
			for (int k = 0; k < NZ; k++)
			{
				float z = -1. + 2. * (float)k / (float)(NZ - 1);
				//calculate x, y, z
				Nodes[i][j][k].x = x;
				Nodes[i][j][k].y = y;
				Nodes[i][j][k].z = z;
				//calculate vx, vy, vz
				Vector(x, y, z, &vx, &vy, &vz);
				Nodes[i][j][k].vx = vx;
				Nodes[i][j][k].vy = vy;
				Nodes[i][j][k].vz = vz;
				//calculate spped
				Nodes[i][j][k].s = sqrt(SQR(vx) + SQR(vy) + SQR(vz));
				//calculate hsv and translate to rgb
				hsv[0] = 240. - 240. * ((Nodes[i][j][k].s - 0.) / (2 * sqrt(3.) - 0.));
				hsv[1] = 1.;
				hsv[2] = 1.;
				HsvRgb(hsv, rgb);
				//store rgb
				Nodes[i][j][k].rgb[0] = rgb[0];
				Nodes[i][j][k].rgb[1] = rgb[1];
				Nodes[i][j][k].rgb[2] = rgb[2];
			}
		}
	}

	// setup the display mode:
	// ( *must* be done before call to glutCreateWindow( ) )
	// ask for color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);


	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);


	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);


	// setup the clear values:

	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);


	// setup the callback routines:


	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(NULL);
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(0, NULL, 0);

	// DO NOT SET THE GLUT IDLE FUNCTION HERE !!
	// glutIdleFunc( NULL );
	// let glui take care of it in InitGlui( )
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void InitLists(void)
{
	//Draw Bounding Box
	glBegin(GL_LINES);
	glColor3f(255., 255., 255.);
	glVertex3f(-1., -1., -1.);
	glVertex3f(-1., -1., 1);
	glVertex3f(-1., -1., -1.);
	glVertex3f(-1., 1., -1.);
	glVertex3f(-1., -1., -1.);
	glVertex3f(1., -1., -1.);
	glVertex3f(1., 1., 1.);
	glVertex3f(-1., 1., 1.);
	glVertex3f(1., 1., 1.);
	glVertex3f(1., -1., 1.);
	glVertex3f(1., 1., 1.);
	glVertex3f(1., 1., -1.);
	glVertex3f(-1., 1., -1.);
	glVertex3f(-1., 1., 1.);
	glVertex3f(-1., 1., -1.);
	glVertex3f(1., 1., -1.);
	glVertex3f(1., -1., 1.);
	glVertex3f(-1., -1., 1.);
	glVertex3f(1., -1., 1.);
	glVertex3f(1., -1., -1.);
	glVertex3f(-1., 1., 1.);
	glVertex3f(-1., -1., 1.);
	glVertex3f(1., 1., -1.);
	glVertex3f(1., -1., -1.);
	glEnd();

	// create the axes:

	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();
}



//
// the keyboard callback:
//

void
Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		WhichProjection = PERSP;
		break;

	case 'q':
	case 'Q':
	case ESCAPE:
		Buttons(QUIT);	// will not return here
		break;			// happy compiler

	case 'r':
	case 'R':
		LeftButton = ROTATE;
		break;

	case 's':
	case 'S':
		LeftButton = SCALE;
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}


	// synchronize the GLUI display with the variables:

	Glui->sync_live();


	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}



//
// called when the mouse button transitions down or up:
//

void
MouseButton(int button, int state, int x, int y)
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}


	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}



//
// called when the mouse moves while a button is down:
//

void
MouseMotion(int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "MouseMotion: %d, %d\n", x, y);


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		switch (LeftButton)
		{
		case ROTATE:
			Xrot += (ANGFACT*dy);
			Yrot += (ANGFACT*dx);
			break;

		case SCALE:
			Scale += SCLFACT * (float)(dx - dy);
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;
		}
	}


	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}



// reset the transformations and the colors:
//
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset(void)
{
	ActiveButton = 0;
	AxesOn = GLUITRUE;
	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	LeftButton = ROTATE;
	Scale = 1.0;
	Scale2 = 0.0;		// because we add 1. to it in Display( )
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;

	RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0] = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1] = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3] = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;

	//added into program
	BoundOn = GLUITRUE;
	VectorOn = GLUIFALSE;
	VectorScaleValue[0] = 0.01;
	StreamlinesOn = GLUIFALSE;
	FSRSLowHigh[0] = FSRSMIN;
	FSRSLowHigh[1] = FSRSMAX;
	TimeStep = 0.02;
	ProbeXY[0] = ProbeXY[1] = ProbeYZ[0] = ProbeYZ[1] = ProbeXZ[0] = ProbeXZ[1] = 0.;
	WhichWayDoProbe = OFF;
}



//
// called when user resizes the window:
//

void
Resize(int width, int height)
{
	if (DebugOn != 0)
		fprintf(stderr, "ReSize: %d, %d\n", width, height);

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


//
// handle a change to the window's visibility:
//

void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}




///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////

// size of wings as fraction of length:

#define WINGS	0.10


// axes:

#define X	1
#define Y	2
#define Z	3


// x, y, z, axes:

static float axx[3] = { 1., 0., 0. };
static float ayy[3] = { 0., 1., 0. };
static float azz[3] = { 0., 0., 1. };


void
Arrow(float tail[3], float head[3])
{
	float u[3], v[3], w[3];		// arrow coordinate system

	// set w direction in u-v-w coordinate system:

	w[0] = head[0] - tail[0];
	w[1] = head[1] - tail[1];
	w[2] = head[2] - tail[2];


	// determine major direction:

	int axis = X;
	float mag = fabs(w[0]);
	if (fabs(w[1])  > mag)
	{
		axis = Y;
		mag = fabs(w[1]);
	}
	if (fabs(w[2])  > mag)
	{
		axis = Z;
		mag = fabs(w[2]);
	}


	// set size of wings and turn w into a Unit vector:

	float d = WINGS * Unit(w, w);


	// draw the shaft of the arrow:

	glBegin(GL_LINE_STRIP);
	glVertex3fv(tail);
	glVertex3fv(head);
	glEnd();

	// draw two sets of wings in the non-major directions:

	float x, y, z;

	if (axis != X)
	{
		Cross(w, axx, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}


	if (axis != Y)
	{
		Cross(w, ayy, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}



	if (axis != Z)
	{
		Cross(w, azz, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}
}



float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}



void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];

	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];

	if (dist > 0.0)
	{
		dist = sqrt(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}

	return dist;
}



// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
	0.f, 1.f, 0.f, 1.f
};

static float xy[] = {
	-.5f, .5f, .5f, -.5f
};

static int xorder[] = {
	1, 2, -3, 4
};


static float yx[] = {
	0.f, 0.f, -.5f, .5f
};

static float yy[] = {
	0.f, .6f, 1.f, 1.f
};

static int yorder[] = {
	1, 2, 3, -2, 4
};


static float zx[] = {
	1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[] = {
	.5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[] = {
	1, 2, 3, 4, -5, 6
};


// fraction of the length to use as height of the characters:

const float LENFRAC = 0.10f;


// fraction of length to use as start location of the characters:

const float BASEFRAC = 1.10f;


//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes(float length)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++)
	{
		int j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact*xx[j], fact*xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++)
	{
		int j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact*yx[j], base + fact*yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++)
	{
		int j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact*zy[j], base + fact*zx[j]);
	}
	glEnd();

}




// routine to convert HSV to RGB
//
// Reference:  Foley, van Dam, Feiner, Hughes,
//		"Computer Graphics Principles and Practices,"


void
HsvRgb(float hsv[3], float rgb[3])
{
	float r, g, b;			// red, green, blue

	// guarantee valid input:

	float h = hsv[0] / 60.;
	while (h >= 6.)	h -= 6.;
	while (h <  0.) 	h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;


	// if sat==0, then is a gray:

	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}


	// get an rgb from the hue itself:

	float i = floor(h);
	float f = h - i;
	float p = v * (1. - s);
	float q = v * (1. - s*f);
	float t = v * (1. - (s * (1. - f)));

	switch ((int)i)
	{
	case 0:
		r = v;	g = t;	b = p;
		break;

	case 1:
		r = q;	g = v;	b = p;
		break;

	case 2:
		r = p;	g = v;	b = t;
		break;

	case 3:
		r = p;	g = q;	b = v;
		break;

	case 4:
		r = t;	g = p;	b = v;
		break;

	case 5:
		r = v;	g = p;	b = q;
		break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}
