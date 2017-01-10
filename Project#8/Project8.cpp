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
//	Author:			Joe Graphics
//
//  Latest update:	March 27, 2013
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

const char *WINDOWTITLE = { "Project #8 - Haoxiang Wang" };
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

//added for project8, which tecture
enum Textures
{
	NO_TEXTURE,
	GL_REPLACE_TEXTURE,
	GL_MODULATE_TEXTURE
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
GLuint	BoxList;			// object display list
int	MainWindow;				// window id for main graphics window
GLfloat	RotMatrix[4][4];	// set by glui rotation widget
float	Scale, Scale2;		// scaling factors
int	WhichColor;				// index into Colors[ ]
int	WhichProjection;		// ORTHO or PERSP
int	Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;			// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets

//added for project 8
#define NUMLNGS		201
#define NUMLATS		105

const int PERIOD_MS = 10 * 1000;
int CloudOn;
int WhichTexture;
int SmoothLightingOn;
float HeightExa;
float LHFactor;
bool Paused;
float Time;

const float LNGMIN = { -289.6f };
const float LNGMAX = { 289.6f };
const float LATMIN = { -197.5f };
const float LATMAX = { 211.2f };
const float HGTMIN = { 0.0f };
const float HGTMAX = { 2.891f };

int GraphWindow;
int RideWindow;

struct LngLatHgt
{
	float lng, lat, hgt;
	float s, t;
};

struct LngLatHgt Points[NUMLATS][NUMLNGS];

GLuint TerrainTex;
GLuint TerrainTexRide;

struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

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
//add for project8
void	SetSpinner(void);
unsigned char *BmpToTexture(char *, int *, int *);
int	ReadInt(FILE *);
short	ReadShort(FILE *);
void DrawCloud(double, double);
void	DisplayG(void);
void	DisplayR(void);

//add for project8
void SetSpinner(void)
{

}

void DisplayG()
{
	if (DebugOn != 0)
	{
		fprintf(stderr, "Display\n");
	}


	// set which window we want to do the graphics into:

	glutSetWindow(GraphWindow);


	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);


	// specify shading to be flat:

	glShadeModel(GL_SMOOTH);


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
	gluOrtho2D(-1., 1., -1., 1.);


	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// draw the current object:
	glColor3f(255., 255., 0.);
	glLineWidth(2.);
	glBegin(GL_LINE_STRIP);
	glVertex2f(-0.85, 0.85);
	glVertex2f(-0.85, 0.15);
	glVertex2f(0.85, 0.15);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex2f(-0.85, -0.15);
	glVertex2f(-0.85, -0.85);
	glVertex2f(0.85, -0.85);
	glEnd();
	glBegin(GL_LINES);
	glVertex2f(-0.85, 0.5);
	glVertex2f(0.85, 0.5);
	glVertex2f(-0.85, -0.5);
	glVertex2f(0.85, -0.5);
	glEnd();
	glColor3f(0., 255., 0.);
	glLineWidth(3.);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i <= 100; i++)
	{
		glVertex2f((-0.85 + 1.7 / 100. * i), cos(2 * 3.1415926535 * ((1.7 / 100. * i) / 1.7))*0.35+0.5);
	}
	glEnd();
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 100; i++)
	{
		glVertex2f((-0.85 + 1.7 / 100. * i), -sin(2 * 3.1415926535 * ((1.7 / 100. * i) / 1.7))*0.35 - 0.5);
	}
	glEnd();
	if (CloudOn == GLUITRUE)
	{
		glColor3f(255., 0., 0.);
		glBegin(GL_QUADS);
		glVertex2f(-0.875 + Time*1.7, 0.875 + cos(2 * 3.1415926535 * Time)*0.35 - 0.35);
		glVertex2f(-0.825 + Time*1.7, 0.875 + cos(2 * 3.1415926535 * Time)*0.35 - 0.35);
		glVertex2f(-0.825 + Time*1.7, 0.825 + cos(2 * 3.1415926535 * Time)*0.35 - 0.35);
		glVertex2f(-0.875 + Time*1.7, 0.825 + cos(2 * 3.1415926535 * Time)*0.35 - 0.35);

		glVertex2f(-0.875 + Time*1.7, -0.475 - sin(2 * 3.1415926535 * Time)*0.35);
		glVertex2f(-0.825 + Time*1.7, -0.475 - sin(2 * 3.1415926535 * Time)*0.35);
		glVertex2f(-0.825 + Time*1.7, -0.525 - sin(2 * 3.1415926535 * Time)*0.35);
		glVertex2f(-0.875 + Time*1.7, -0.525 - sin(2 * 3.1415926535 * Time)*0.35);
		glEnd();
	}
	else
	{
		glColor3f(255., 0., 0.);
		glBegin(GL_QUADS);
		glVertex2f(-0.875, 0.875);
		glVertex2f(-0.825, 0.875);
		glVertex2f(-0.825, 0.825);
		glVertex2f(-0.875, 0.825);

		glVertex2f(-0.875, -0.475);
		glVertex2f(-0.825, -0.475);
		glVertex2f(-0.825, -0.525);
		glVertex2f(-0.875, -0.525);
		glEnd();
	}
	// draw some gratuitous text that just rotates on top of the scene:

	/*glDisable(GL_DEPTH_TEST);
	glColor3f(0., 1., 1.);
	DoRasterString(0., 1., 0., "Text That Moves");*/


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


	// swap the double-buffered framebuffers:

	glutSwapBuffers();


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
}

void DisplayR()
{
	float gradLong[3], gradLat[3];
	float inten;
	float v01[3], v02[3];
	float norm[3];

	if (DebugOn != 0)
	{
		fprintf(stderr, "Display\n");
	}


	// set which window we want to do the graphics into:

	glutSetWindow(RideWindow);


	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);


	// specify shading to be flat:

	//glShadeModel(GL_SMOOTH);


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
	//glMatrixMode(GL_TEXTURE);
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !
	if (CloudOn == GLUITRUE)
	{
		gluLookAt(150. * cos(2 * 3.1415926535 * Time), 90., 150. * sin(2 * 3.1415926535 * Time), 150. / cos(3.1415926535 / 5) * cos(2 * 3.1415926535 * Time + 3.1415926535 / 5), 75., 150. / cos(3.1415926535 / 5) * sin(2 * 3.1415926535 * Time + 3.1415926535 / 5), 0., 1., 0.);
	}
	else
	{
		gluLookAt(150., 90., 0., 150. / cos(3.1415926535 / 5) * cos(3.1415926535 / 5), 75., 150. / cos(3.1415926535 / 5) * sin(3.1415926535 / 5), 0., 1., 0.);
	}
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

	//glCallList(BoxList);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	switch (WhichTexture)
	{
	case NO_TEXTURE:
		glDisable(GL_TEXTURE_2D);
		break;
	case GL_REPLACE_TEXTURE:
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		break;
	case GL_MODULATE_TEXTURE:
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		break;
	}

	glBindTexture(GL_TEXTURE_2D, TerrainTexRide);
	glBegin(GL_TRIANGLES);
	if (SmoothLightingOn == GLUITRUE)
	{
		glShadeModel(GL_SMOOTH);
		for (int z = 1; z < NUMLATS - 2; z++)
		{
			for (int x = 1; x < NUMLNGS - 2; x++)
			{
				//[z][x]
				gradLong[0] = Points[z][x + 1].lng - Points[z][x - 1].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z][x + 1].hgt - Points[z][x - 1].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 1][x].hgt - Points[z - 1][x].hgt);
				gradLat[2] = Points[z + 1][x].lat - Points[z - 1][x].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x].s, Points[z][x].t);
				glVertex3f(Points[z][x].lng, Points[z][x].hgt*HeightExa, Points[z][x].lat);

				//[z+1][x]
				gradLong[0] = Points[z + 1][x + 1].lng - Points[z + 1][x - 1].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z + 1][x + 1].hgt - Points[z + 1][x - 1].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 2][x].hgt - Points[z][x].hgt);
				gradLat[2] = Points[z + 2][x].lat - Points[z][x].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt*HeightExa, Points[z + 1][x].lat);

				//[z][x+1]
				gradLong[0] = Points[z][x + 2].lng - Points[z][x].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z][x + 2].hgt - Points[z][x].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 1][x + 1].hgt - Points[z - 1][x + 1].hgt);
				gradLat[2] = Points[z + 1][x + 1].lat - Points[z - 1][x + 1].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt*HeightExa, Points[z][x + 1].lat);

				//[z+1][x+1]
				gradLong[0] = Points[z + 1][x + 2].lng - Points[z + 1][x].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z + 1][x + 2].hgt - Points[z + 1][x].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 2][x + 1].hgt - Points[z][x + 1].hgt);
				gradLat[2] = Points[z + 2][x + 1].lat - Points[z][x + 1].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z + 1][x + 1].s, Points[z + 1][x + 1].t);
				glVertex3f(Points[z + 1][x + 1].lng, Points[z + 1][x + 1].hgt*HeightExa, Points[z + 1][x + 1].lat);

				//[z+1][x]
				gradLong[0] = Points[z + 1][x + 1].lng - Points[z + 1][x - 1].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z + 1][x + 1].hgt - Points[z + 1][x - 1].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 2][x].hgt - Points[z][x].hgt);
				gradLat[2] = Points[z + 2][x].lat - Points[z][x].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt*HeightExa, Points[z + 1][x].lat);

				//[z][x+1]
				gradLong[0] = Points[z][x + 2].lng - Points[z][x].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z][x + 2].hgt - Points[z][x].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 1][x + 1].hgt - Points[z - 1][x + 1].hgt);
				gradLat[2] = Points[z + 1][x + 1].lat - Points[z - 1][x + 1].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt*HeightExa, Points[z][x + 1].lat);
			}
		}
	}
	else
	{
		glShadeModel(GL_FLAT);
		for (int z = 0; z < NUMLATS - 2; z++)
		{
			for (int x = 0; x < NUMLNGS - 2; x++)
			{
				//[z][x];[z+1][x];[z][x+1]
				v01[0] = Points[z + 1][x].lng - Points[z][x].lng;
				v01[1] = HeightExa*LHFactor*(Points[z + 1][x].hgt - Points[z][x].hgt);
				v01[2] = Points[z + 1][x].lat - Points[z][x].lat;

				v02[0] = Points[z][x + 1].lng - Points[z][x].lng;
				v02[2] = Points[z][x + 1].lat - Points[z][x].lat;
				v02[1] = HeightExa*LHFactor*(Points[z][x + 1].hgt - Points[z][x].hgt);

				Cross(v01, v02, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x].s, Points[z][x].t);
				//glNormal3f();
				glVertex3f(Points[z][x].lng, Points[z][x].hgt*HeightExa, Points[z][x].lat);

				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				//glNormal3f();
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt*HeightExa, Points[z + 1][x].lat);

				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				//glNormal3f();
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt*HeightExa, Points[z][x + 1].lat);

				//[z][x+1];[z+1][x];[z+1][x+1]
				v01[0] = Points[z + 1][x].lng - Points[z][x + 1].lng;
				v01[1] = HeightExa*LHFactor*(Points[z + 1][x].hgt - Points[z][x + 1].hgt);
				v01[2] = Points[z + 1][x].lat - Points[z][x + 1].lat;

				v02[0] = Points[z + 1][x + 1].lng - Points[z][x + 1].lng;
				v02[2] = Points[z + 1][x + 1].lat - Points[z][x + 1].lat;
				v02[1] = HeightExa*LHFactor*(Points[z + 1][x + 1].hgt - Points[z][x + 1].hgt);

				Cross(v01, v02, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				//glNormal3f();
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt*HeightExa, Points[z][x + 1].lat);

				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				//glNormal3f();
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt*HeightExa, Points[z + 1][x].lat);

				glTexCoord2f(Points[z + 1][x + 1].s, Points[z + 1][x + 1].t);
				//glNormal3f();
				glVertex3f(Points[z + 1][x + 1].lng, Points[z + 1][x + 1].hgt*HeightExa, Points[z + 1][x + 1].lat);
			}
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	if (CloudOn == GLUITRUE)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		DrawCloud(150. * cos(2 * 3.1415926535 * Time), 150. * sin(2 * 3.1415926535 * Time));
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	// draw some gratuitous text that just rotates on top of the scene:

	/*glDisable(GL_DEPTH_TEST);
	glColor3f(0., 1., 1.);
	DoRasterString(0., 1., 0., "Text That Moves");*/


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

	// swap the double-buffered framebuffers:

	glutSwapBuffers();


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
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
	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms = ms % PERIOD_MS;
	Time = (float)ms / (float)(PERIOD_MS - 1);


	// force a call to Display( ) next time it is convenient:

	glutSetWindow(MainWindow);
	glutPostRedisplay();

	glutSetWindow(GraphWindow);
	glutPostRedisplay();

	glutSetWindow(RideWindow);
	glutPostRedisplay();
}




//
// glui buttons callback:
//

void
Buttons(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
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
	float gradLong[3], gradLat[3];
	float inten;
	float v01[3], v02[3];
	float norm[3];

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

	//glShadeModel(GL_SMOOTH);


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
	//glMatrixMode(GL_TEXTURE);
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !

	gluLookAt(0., 300., 200., 0., 0., 0., 0., 1., 0.);


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

	//glCallList(BoxList);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	switch (WhichTexture)
	{
	case NO_TEXTURE:
		glDisable(GL_TEXTURE_2D);
		break;
	case GL_REPLACE_TEXTURE:
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		break;
	case GL_MODULATE_TEXTURE:
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		break;
	}

	glBindTexture(GL_TEXTURE_2D, TerrainTex);
	glBegin(GL_TRIANGLES);
	if (SmoothLightingOn == GLUITRUE)
	{
		glShadeModel(GL_SMOOTH);
		for (int z = 1; z < NUMLATS - 2; z++)
		{
			for (int x = 1; x < NUMLNGS - 2; x++)
			{
				//[z][x]
				gradLong[0] = Points[z][x + 1].lng - Points[z][x - 1].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z][x + 1].hgt - Points[z][x - 1].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 1][x].hgt - Points[z - 1][x].hgt);
				gradLat[2] = Points[z + 1][x].lat - Points[z - 1][x].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x].s, Points[z][x].t);
				glVertex3f(Points[z][x].lng, Points[z][x].hgt*HeightExa, Points[z][x].lat);

				//[z+1][x]
				gradLong[0] = Points[z+1][x + 1].lng - Points[z+1][x - 1].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z+1][x + 1].hgt - Points[z+1][x - 1].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 2][x].hgt - Points[z][x].hgt);
				gradLat[2] = Points[z + 2][x].lat - Points[z][x].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z+1][x].s, Points[z+1][x].t);
				glVertex3f(Points[z+1][x].lng, Points[z+1][x].hgt*HeightExa, Points[z+1][x].lat);

				//[z][x+1]
				gradLong[0] = Points[z][x + 2].lng - Points[z][x].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z][x + 2].hgt - Points[z][x].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 1][x+1].hgt - Points[z - 1][x+1].hgt);
				gradLat[2] = Points[z + 1][x+1].lat - Points[z - 1][x+1].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x+1].s, Points[z][x+1].t);
				glVertex3f(Points[z][x+1].lng, Points[z][x+1].hgt*HeightExa, Points[z][x+1].lat);

				//[z+1][x+1]
				gradLong[0] = Points[z+1][x + 2].lng - Points[z+1][x].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z+1][x + 2].hgt - Points[z+1][x].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 2][x+1].hgt - Points[z][x+1].hgt);
				gradLat[2] = Points[z + 2][x+1].lat - Points[z][x+1].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z+1][x+1].s, Points[z+1][x+1].t);
				glVertex3f(Points[z+1][x+1].lng, Points[z+1][x+1].hgt*HeightExa, Points[z+1][x+1].lat);

				//[z+1][x]
				gradLong[0] = Points[z + 1][x + 1].lng - Points[z + 1][x - 1].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z + 1][x + 1].hgt - Points[z + 1][x - 1].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 2][x].hgt - Points[z][x].hgt);
				gradLat[2] = Points[z + 2][x].lat - Points[z][x].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt*HeightExa, Points[z + 1][x].lat);

				//[z][x+1]
				gradLong[0] = Points[z][x + 2].lng - Points[z][x].lng;
				gradLong[1] = HeightExa*LHFactor*(Points[z][x + 2].hgt - Points[z][x].hgt);
				gradLong[2] = 0.;

				gradLat[0] = 0.;
				gradLat[1] = HeightExa*LHFactor*(Points[z + 1][x + 1].hgt - Points[z - 1][x + 1].hgt);
				gradLat[2] = Points[z + 1][x + 1].lat - Points[z - 1][x + 1].lat;

				Cross(gradLong, gradLat, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt*HeightExa, Points[z][x + 1].lat);
			}
		}
	}
	else
	{
		glShadeModel(GL_FLAT);
		for (int z = 0; z < NUMLATS - 2; z++)
		{
			for (int x = 0; x < NUMLNGS - 2; x++)
			{
				//[z][x];[z+1][x];[z][x+1]
				v01[0] = Points[z + 1][x].lng - Points[z][x].lng;
				v01[1] = HeightExa*LHFactor*(Points[z + 1][x].hgt - Points[z][x].hgt);
				v01[2] = Points[z + 1][x].lat - Points[z][x].lat;

				v02[0] = Points[z][x + 1].lng - Points[z][x].lng;
				v02[2] = Points[z][x + 1].lat - Points[z][x].lat;
				v02[1] = HeightExa*LHFactor*(Points[z][x + 1].hgt - Points[z][x].hgt);

				Cross(v01, v02, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x].s, Points[z][x].t);
				//glNormal3f();
				glVertex3f(Points[z][x].lng, Points[z][x].hgt*HeightExa, Points[z][x].lat);

				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				//glNormal3f();
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt*HeightExa, Points[z + 1][x].lat);

				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				//glNormal3f();
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt*HeightExa, Points[z][x + 1].lat);

				//[z][x+1];[z+1][x];[z+1][x+1]
				v01[0] = Points[z + 1][x].lng - Points[z][x + 1].lng;
				v01[1] = HeightExa*LHFactor*(Points[z + 1][x].hgt - Points[z][x + 1].hgt);
				v01[2] = Points[z + 1][x].lat - Points[z][x + 1].lat;

				v02[0] = Points[z + 1][x + 1].lng - Points[z][x + 1].lng;
				v02[2] = Points[z + 1][x + 1].lat - Points[z][x + 1].lat;
				v02[1] = HeightExa*LHFactor*(Points[z + 1][x + 1].hgt - Points[z][x + 1].hgt);

				Cross(v01, v02, norm);
				Unit(norm, norm);

				inten = fabs(norm[1]);
				glColor3f(inten, inten, inten);

				glTexCoord2f(Points[z][x + 1].s, Points[z][x + 1].t);
				//glNormal3f();
				glVertex3f(Points[z][x + 1].lng, Points[z][x + 1].hgt*HeightExa, Points[z][x + 1].lat);

				glTexCoord2f(Points[z + 1][x].s, Points[z + 1][x].t);
				//glNormal3f();
				glVertex3f(Points[z + 1][x].lng, Points[z + 1][x].hgt*HeightExa, Points[z + 1][x].lat);

				glTexCoord2f(Points[z + 1][x + 1].s, Points[z + 1][x + 1].t);
				//glNormal3f();
				glVertex3f(Points[z + 1][x + 1].lng, Points[z + 1][x + 1].hgt*HeightExa, Points[z + 1][x + 1].lat);
			}
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	if (CloudOn == GLUITRUE)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glColor4f(0., 255., 255., 0.5);
		DrawCloud(150. * cos(2 * 3.1415926535 * Time), 150. * sin(2 * 3.1415926535 * Time));
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	// draw some gratuitous text that just rotates on top of the scene:

	/*glDisable(GL_DEPTH_TEST);
	glColor3f(0., 1., 1.);
	DoRasterString(0., 1., 0., "Text That Moves");*/


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
	DoRasterString(5., 5., 0., "Project #8");


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
	//add for project8
	GLUI_Spinner *HESpinner;
	GLUI_Spinner *LHFSpinner;

	// setup the glui window:

	glutInitWindowPosition(INIT_WINDOW_SIZE + 50, 0);
	Glui = GLUI_Master.create_glui((char *)GLUITITLE);


	Glui->add_statictext((char *)GLUITITLE);
	Glui->add_separator();

	Glui->add_checkbox("Axes", &AxesOn);

	Glui->add_checkbox("Perspective", &WhichProjection);

	Glui->add_checkbox("Intensity Depth Cue", &DepthCueOn);

	Glui->add_checkbox("Cloud", &CloudOn);

	/*panel = Glui->add_panel("Axes Color");
	group = Glui->add_radiogroup_to_panel(panel, &WhichColor);
	Glui->add_radiobutton_to_group(group, "Red");
	Glui->add_radiobutton_to_group(group, "Yellow");
	Glui->add_radiobutton_to_group(group, "Green");
	Glui->add_radiobutton_to_group(group, "Cyan");
	Glui->add_radiobutton_to_group(group, "Blue");
	Glui->add_radiobutton_to_group(group, "Magenta");
	Glui->add_radiobutton_to_group(group, "White");
	Glui->add_radiobutton_to_group(group, "Black");*/

	panel = Glui->add_panel("Texture Environment");
	group = Glui->add_radiogroup_to_panel(panel, &WhichTexture);
	Glui->add_radiobutton_to_group(group, "No Texture");
	Glui->add_radiobutton_to_group(group, "GL_REPLACE");
	Glui->add_radiobutton_to_group(group, "GL_MODULATE");

	Glui->add_checkbox("UseSmoothLighting", &SmoothLightingOn);

	HESpinner = Glui->add_spinner("Height Exaggeration", GLUI_SPINNER_FLOAT, &HeightExa, 0, (GLUI_Update_CB)SetSpinner);
		HESpinner->set_float_limits(2.5, 50.);

	LHFSpinner = Glui->add_spinner("Lighting Height Factor", GLUI_SPINNER_FLOAT, &LHFactor, 0, (GLUI_Update_CB)SetSpinner);
		LHFSpinner->set_float_limits(2.5, 50.);

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
	unsigned char *Texture;
	int Width, Height;
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

	glutInitWindowPosition(0, INIT_WINDOW_SIZE+75);
	glutInitWindowSize(INIT_WINDOW_SIZE/2, INIT_WINDOW_SIZE/2);
	GraphWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);


	glutInitWindowPosition(INIT_WINDOW_SIZE / 2+25, INIT_WINDOW_SIZE+75);
	glutInitWindowSize(INIT_WINDOW_SIZE/2, INIT_WINDOW_SIZE/2);
	RideWindow = glutCreateWindow(WINDOWTITLE);
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

	Texture = BmpToTexture("Oregon2048.bmp", &Width, &Height);
	if (Texture == NULL || Width != 2048|| Height != 2048)
	{
		printf("Wrong Reading BMP File!");
		exit(1);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &TerrainTex);
	glBindTexture(GL_TEXTURE_2D, TerrainTex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);

	FILE *fp = fopen("or.hgt", "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open 'or.hgt'\n");
		exit(1);
	}

	float dlng = 2.88;
	float dlat = 3.89;

	float lat, lng;
	int z, x;

	for (z = 0, lat = LATMAX; z < NUMLATS; z++, lat -= dlat)
	{
		for (x = 0, lng = LNGMIN; x < NUMLNGS; x++, lng += dlng)
		{
			Points[z][x].lng = lng;
			Points[z][x].lat = lat;
			fscanf(fp, "%f", &Points[z][x].hgt);
			Points[z][x].s = (lng - LNGMIN) / (LNGMAX - LNGMIN);	// 0. -> 1.
			Points[z][x].t = (lat - LATMIN) / (LATMAX - LATMIN);	// 0. -> 1.
			Points[z][x].t = Points[z][x].t * 0.718;			// 0. -> 0.718
		}
	}

	glutSetWindow(GraphWindow);
	glutDisplayFunc(DisplayG);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);

	glutSetWindow(RideWindow);
	glutDisplayFunc(DisplayR);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &TerrainTexRide);
	glBindTexture(GL_TEXTURE_2D, TerrainTexRide);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists(void)
{
	float dx = BOXSIZE / 2.;
	float dy = BOXSIZE / 2.;
	float dz = BOXSIZE / 2.;

	// create the object:

	BoxList = glGenLists(1);
	glNewList(BoxList, GL_COMPILE);

	glBegin(GL_QUADS);

	glColor3f(0., 0., 1.);
	glNormal3f(0., 0., 1.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(-dx, dy, dz);

	glNormal3f(0., 0., -1.);
	glTexCoord2f(0., 0.);
	glVertex3f(-dx, -dy, -dz);
	glTexCoord2f(0., 1.);
	glVertex3f(-dx, dy, -dz);
	glTexCoord2f(1., 1.);
	glVertex3f(dx, dy, -dz);
	glTexCoord2f(1., 0.);
	glVertex3f(dx, -dy, -dz);

	glColor3f(1., 0., 0.);
	glNormal3f(1., 0., 0.);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(dx, dy, dz);

	glNormal3f(-1., 0., 0.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, dy, dz);
	glVertex3f(-dx, dy, -dz);
	glVertex3f(-dx, -dy, -dz);

	glColor3f(0., 1., 0.);
	glNormal3f(0., 1., 0.);
	glVertex3f(-dx, dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(-dx, dy, -dz);

	glNormal3f(0., -1., 0.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, -dy, -dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, -dy, dz);

	glEnd();

	glEndList();


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

	case 'p':
		Paused = !Paused;
		if (Paused)
			GLUI_Master.set_glutIdleFunc(NULL);
		else
			GLUI_Master.set_glutIdleFunc(Animate);
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
	AxesOn = GLUIFALSE;
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

	//add for project8
	CloudOn = GLUITRUE;
	WhichTexture = NO_TEXTURE;
	SmoothLightingOn = GLUIFALSE;
	HeightExa = 2.5;
	LHFactor = 2.5;
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

unsigned char *
BmpToTexture(char *filename, int *width, int *height)
{

	int s, t, e;		// counters
	int numextra;		// # extra bytes each line in the file is padded with
	FILE *fp;
	unsigned char *texture;
	int nums, numt;
	unsigned char *tp;


	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}

	FileHeader.bfType = ReadShort(fp);


	// if bfType is not 0x4d42, the file is not a bmp:

	if (FileHeader.bfType != 0x4d42)
	{
		fprintf(stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType);
		fclose(fp);
		return NULL;
	}


	FileHeader.bfSize = ReadInt(fp);
	FileHeader.bfReserved1 = ReadShort(fp);
	FileHeader.bfReserved2 = ReadShort(fp);
	FileHeader.bfOffBits = ReadInt(fp);


	InfoHeader.biSize = ReadInt(fp);
	InfoHeader.biWidth = ReadInt(fp);
	InfoHeader.biHeight = ReadInt(fp);

	nums = InfoHeader.biWidth;
	numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort(fp);
	InfoHeader.biBitCount = ReadShort(fp);
	InfoHeader.biCompression = ReadInt(fp);
	InfoHeader.biSizeImage = ReadInt(fp);
	InfoHeader.biXPelsPerMeter = ReadInt(fp);
	InfoHeader.biYPelsPerMeter = ReadInt(fp);
	InfoHeader.biClrUsed = ReadInt(fp);
	InfoHeader.biClrImportant = ReadInt(fp);


	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );


	texture = new unsigned char[3 * nums * numt];
	if (texture == NULL)
	{
		fprintf(stderr, "Cannot allocate the texture array!\b");
		return NULL;
	}


	// extra padding bytes:

	numextra = 4 * (((3 * InfoHeader.biWidth) + 3) / 4) - 3 * InfoHeader.biWidth;


	// we do not support compression:

	if (InfoHeader.biCompression != birgb)
	{
		fprintf(stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression);
		fclose(fp);
		return NULL;
	}



	rewind(fp);
	fseek(fp, 14 + 40, SEEK_SET);

	if (InfoHeader.biBitCount == 24)
	{
		for (t = 0, tp = texture; t < numt; t++)
		{
			for (s = 0; s < nums; s++, tp += 3)
			{
				*(tp + 2) = fgetc(fp);		// b
				*(tp + 1) = fgetc(fp);		// g
				*(tp + 0) = fgetc(fp);		// r
			}

			for (e = 0; e < numextra; e++)
			{
				fgetc(fp);
			}
		}
	}

	fclose(fp);

	*width = nums;
	*height = numt;
	return texture;
}



int
ReadInt(FILE *fp)
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc(fp);
	b1 = fgetc(fp);
	b2 = fgetc(fp);
	b3 = fgetc(fp);
	return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}


short
ReadShort(FILE *fp)
{
	unsigned char b1, b0;
	b0 = fgetc(fp);
	b1 = fgetc(fp);
	return (b1 << 8) | b0;
}

void DrawCloud(double xc, double zc)
{
	int i;
	double theta;
	double x, z;
	double xold, zold;
	float hsv[3], rgb[3];

	hsv[0] = 360. - 360. * Time;
	hsv[1] = 1.;
	hsv[2] = 1.;
	HsvRgb(hsv, rgb);

	theta = 2 * 3.1415926535 * (0.5) / 200.;
	xold = xc + 200. * cos(theta);
	zold = zc + 200. * sin(theta);

	glColor4f(rgb[0], rgb[1], rgb[2], 0.5);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i <= 200; i++)
	{
		theta = 2 * 3.1415926535 * (i + 0.5) / 200.;
		if ((i / 20) % 2 == 0)
		{
			x = xc + (40. + cos(2 * 3.1415926535*Time) * (i % 20)) * cos(theta);
			z = zc + (40. + cos(2 * 3.1415926535*Time) * (i % 20)) * sin(theta);
		}
		else
		{
			x = xc + (40. + cos(2 * 3.1415926535*Time) * (20 - i % 20)) * cos(theta);
			z = zc + (40. + cos(2 * 3.1415926535*Time) * (20 - i % 20)) * sin(theta);
		}
		glVertex3f(xc, 75.0, zc);
		glVertex3f(x, 75.0, z);
		glVertex3f(xold, 75.0, zold);

		xold = x;
		zold = z;
	}
	glEnd();
}