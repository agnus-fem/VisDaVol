#include <unistd.h>     // needed to sleep
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>

#include <gtk/gtk.h>
#include <GL/gl.h>

#include <gtk/gtkgl.h>
#include <gdk/gdk.h>

#include "glsl.h"
#include <GL/glut.h>    // Header File For The GLUT Library
#include "transf.h"

#define X 0
#define Y 1
#define Z 2

/* ASCII code for the escape key. */
#define ESCAPE 27

/* Texture */
GLuint texid_3D, texid_1D;
int texwidth = 384;
int texheight = 384;
int texdepth = 384;

float tick = 0;
float stick = 1;

GLubyte * texData;
int d_width, d_height, d_slices, d_nsli;

GLubyte * readRAW(int argc, char **argv);
GLubyte * raw;

int winWidth, winHeight;
int oldXr, oldYr;
int oldXp, oldYp;

float tk;

GLuint theVolume[6];

float m[16];

int TproxyGeometry = 0;


GLuint ProgramObject;
COLORMAP cm;

GtkBuilder *builder;
GtkWidget *theDrawingArea1;
GdkGLDrawable *theGLDrawable;

//Necessários para exibir o contexto openGL
static float ang = 90.;
int BBox = 1;
int LuzAmbEhMax = 0;

GLuint theObject;

/* coords of a cube */
static GLfloat cube[8][3] = { { 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 0.0,
		1.0 }, { 0.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0 }, { 1.0, 1.0, 1.0 }, { 0.0,
		1.0, 1.0 }, { 0.0, 1.0, 0.0 } };

static int faceIndex[6][4] = { { 0, 1, 2, 3 }, { 1, 4, 5, 2 }, { 4, 7, 6, 5 },
		{ 7, 0, 3, 6 }, { 3, 2, 5, 6 }, { 7, 4, 1, 0 } };

void drawWireframe(int face) {
	int i;
	glBegin(GL_LINE_LOOP);
	for (i = 0; i < 4; i++)
		glVertex3fv((GLfloat *) cube[faceIndex[face][i]]);
	glEnd();
}

//!
/*!
 * @brief Esta função faz com que um valor fique restrito dentro de uma faixa de valores definida por um valor mínimo e máximo
 *
 * \author Agnus A. Horta.
 * \since 30/10/2011
 * \version 1.0
 * \param value um float que representa o valor a ser truncado.
 * \param min um inteiro que representa constante que define o valor inicial de truncamento.
 * \param max um inteiro que representa constante que define o valor final de truncamento.
 * \return um inteiro.
 */
GLubyte clamp(double value, const int min, const int max) {
	return (value < min) ? min : ((value > max) ? max : floor(value));
}

/* Funções adicionais para uso do slice direction */
float abs(float x) {
	return ((x < 0) ? (-x) : x);
}

int FindAbsMaximum(GLfloat pViewVector[4]) {

	int aux = 0;
	float v = abs(pViewVector[0]);
	for (int i = 1; i < 3; i++) {
		if (abs(pViewVector[i]) > v) {
			v = abs(pViewVector[i]);
			aux = i;
		}
	}

	return aux;
}

void MatrixMultiply(GLfloat mtxin1[16], GLfloat mtxin2[16], GLfloat mtxout[16]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			float s = 0;
			for (int k = 0; k < 4; k++)
				s += (mtxin1[k + i * 4] * mtxin2[j + k * 4]);
			mtxout[j + i * 4] = s;
		}
	}
}

// adaptadas de http://www.gamedev.net/topic/180189-matrix-inverse/
void MatrixInvRotate(GLfloat mtxin[16], GLfloat mtxout[16]) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			mtxout[j * 4 + i] = mtxin[i * 4 + j];
		}
	}
}

void VectorRotate(const GLfloat vin[4], const GLfloat mtx[16], GLfloat vout[4]) {
	vout[0] = vin[0] * mtx[0] + vin[1] * mtx[4] + vin[2] * mtx[8];
	vout[1] = vin[0] * mtx[1] + vin[1] * mtx[5] + vin[2] * mtx[9];
	vout[2] = vin[0] * mtx[2] + vin[1] * mtx[6] + vin[2] * mtx[10];
}

void InvertMatrix(GLfloat mtxin[16], GLfloat mtxout[16]) {

	MatrixInvRotate(mtxin, mtxout);

	GLfloat vTmp[4], vTmp2[4];

	vTmp[0] = -mtxin[12];
	vTmp[1] = -mtxin[13];
	vTmp[2] = -mtxin[14];

	VectorRotate(vTmp, mtxout, vTmp2);

	mtxout[12] = vTmp2[0];
	mtxout[13] = vTmp2[1];
	mtxout[14] = vTmp2[2];

	mtxout[3] = mtxout[7] = mtxout[11] = 0.0f;
	mtxout[15] = 1.0f;

}
// gamedev

void DrawSliceStack(int proxyGeometry) {

	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(-0.5f, -0.5f, -0.5f);

	glDisable(GL_DEPTH_TEST); // Enable Depth Testing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Enable Alpha Blending (disable alpha testing)
	glEnable(GL_BLEND);

	glCallList(theVolume[proxyGeometry]);
	glFlush();

}

void MatVecMultiply(GLfloat pModelViewMatrixInv[16], GLfloat pViewVector[4]) {
	GLfloat pViewVector1[4];

	pViewVector1[0] = pModelViewMatrixInv[0] * pViewVector[0]
			+ pModelViewMatrixInv[4] * pViewVector[1] + pModelViewMatrixInv[8]
			* pViewVector[2];
	pViewVector1[1] = pModelViewMatrixInv[1] * pViewVector[0]
			+ pModelViewMatrixInv[5] * pViewVector[1] + pModelViewMatrixInv[9]
			* pViewVector[2];
	pViewVector1[2] = pModelViewMatrixInv[2] * pViewVector[0]
			+ pModelViewMatrixInv[6] * pViewVector[1] + pModelViewMatrixInv[10]
			* pViewVector[2];

	pViewVector[0] = pViewVector1[0];
	pViewVector[1] = pViewVector1[1];
	pViewVector[2] = pViewVector1[2];

}
void DrawCube() {
	//Listing 3.2. OpenGL code for selecting the slice direction.
	//Real Time Volume Graphics page 52

	GLfloat pModelViewMatrix[16];
	GLfloat pModelViewMatrixInv[16];
	//GLfloat pMatrixIdentity[16];
	// get the current modelview matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, pModelViewMatrix);
	// invert the modelview matrix
	InvertMatrix(pModelViewMatrix, pModelViewMatrixInv);
	// rotate the initial viewing direction
	GLfloat pViewVector[4] = { 0.0f, 0.0f, -1.0f, 0.0f };
	//GLfloat pViewVector1[4];

	MatVecMultiply(pModelViewMatrixInv, pViewVector);

	// find the maximal vector component
	int nMax = FindAbsMaximum(pViewVector);

	//printf("%d %f %f %f %f \n", nMax, pViewVector[0],pViewVector[1],pViewVector[2],pViewVector[3]);
	switch (nMax) {
	case X:
		if (pViewVector[X] > 0.0f) {
			DrawSliceStack(1);
		} else {
			DrawSliceStack(0);
		}
		break;
	case Y:
		if (pViewVector[Y] > 0.0f) {
			DrawSliceStack(3);
		} else {
			DrawSliceStack(2);
		}
		break;
	case Z:
		if (pViewVector[Z] > 0.0f) {
			DrawSliceStack(5);
		} else {
			DrawSliceStack(4);
		}
		break;
	}

}

void InitTexture() {

	int s, t, p;
	int wc, hc, dc;

	GLubyte *ptr;
	long int i = 0;

	// Create the 3D texture
	ptr = (GLubyte *) malloc(texwidth * texheight * texdepth * sizeof(GLubyte)
			* 1);
	memset(ptr, 0x00, texwidth * texheight * texdepth * sizeof(GLubyte) * 1);

	texData = ptr;

	wc = (texwidth - d_width) / 2;
	hc = (texheight - d_height) / 2;
	dc = (texdepth - d_slices) / 2;

	//ptr+=(wc+hc*texwidth+dc*texwidth*texheight);

	for (p = 0; p < d_slices; p++) {
		//printf("\ndepth=%d\n", p);
		for (t = 0; t < d_height; t++) {
			//printf("\nheight=%d ",t);
			for (s = 0; s < d_width; s++) {
				ptr = texData + ((wc + s) * 1 + (hc + t) * texwidth * 1 + (dc
						+ p) * texwidth * texheight * 1);
				//if (raw[i] > 1) {
				ptr[0] = raw[i]; // red
				//ptr[1]=raw[i];   // green
				//ptr[2]=raw[i];    // blue
				//ptr[3]=raw[i];//cm[raw[i]].alfa; // alpha
				//ptr[0] = cm[raw[i]].r; // red
				//ptr[1] = cm[raw[i]].g; // green
				//ptr[2] = cm[raw[i]].b; // blue
				//ptr[3] = cm[raw[i]].alfa; // alpha
				//}
				i++;
			}
		}
	}

	printf("Carregou textura 3D\n");

	GLubyte tex1D[256][4];
	//tex1D = (GLuint *) malloc(1024 * sizeof(GLuint));

	for (i = 0; i < 256; i++) {
		tex1D[i][0] = (GLuint) cm[i].r;
		tex1D[i][1] = (GLuint) cm[i].g;
		tex1D[i][2] = (GLuint) cm[i].b;
		tex1D[i][3] = (GLuint) cm[i].alfa;
	}

	printf("Carregou textura 1D\n");

	//glUseProgram(ProgramObject);

	printf("Carregou toda a textura!\n");

	glGenTextures(1, &texid_3D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texid_3D); //Sélectionne ce n°

	glTexImage3D(GL_TEXTURE_3D, //Type : texture 3D
			0, //Mipmap : aucun
			GL_LUMINANCE8, // (red,green,blue,alpha)
			texwidth, // Largeur
			texheight, // Hauteur
			texdepth, // profondeur
			0, //Largeur du bord : 0
			GL_LUMINANCE, //Format : RGBA
			GL_UNSIGNED_BYTE, //Type des couleurs
			texData //Addresse de l'image
	);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &texid_1D);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, texid_1D); //Sélectionne ce n°
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexImage1D(GL_TEXTURE_1D, //Type : texture 3D
			0, //Mipmap : aucun
			GL_RGBA8, // (red,green,blue,alpha)
			256, // Largeur
			0, //Largeur du bord : 0
			GL_RGBA, //Format : RGBA
			GL_UNSIGNED_BYTE, //Type des couleurs
			tex1D //Addresse de l'image
	);

	//Activate and select 3D Texture

	glBindTexture(GL_TEXTURE_3D, texid_3D);
	glEnable(GL_TEXTURE_3D);

	//Activate and select 1D Texture
	glBindTexture(GL_TEXTURE_1D, texid_1D);
	glEnable(GL_TEXTURE_1D);

}

void InitDraw(void) {

	for (int i = 0; i < 6; i++) {

		theVolume[i] = glGenLists(1);
		glNewList(theVolume[i], GL_COMPILE);

		tick = 0.0f;

		glBegin(GL_QUADS);
		do {

			switch (i) {
			case 0: // Stack PositiveX
				//glNormal3f(  1.0f,0.0f, 0.0f);
				glTexCoord3f(tick, 0.0f, 0.0f);
				glVertex3f(tick, 0.0f, 0.0f);
				glTexCoord3f(tick, 1.0f, 0.0f);
				glVertex3f(tick, 1.0f, 0.0f);
				glTexCoord3f(tick, 1.0f, 1.0f);
				glVertex3f(tick, 1.0f, 1.0f);
				glTexCoord3f(tick, 0.0f, 1.0f);
				glVertex3f(tick, 0.0f, 1.0f);
				break;

			case 1: // Stack NegativeX
				//glNormal3f(  -1.0f,0.0f, 0.0f);
				glTexCoord3f(1.0f - tick, 0.0f, 0.0f);
				glVertex3f(1.0f - tick, 0.0f, 0.0f);
				glTexCoord3f(1.0f - tick, 1.0f, 0.0f);
				glVertex3f(1.0f - tick, 1.0f, 0.0f);
				glTexCoord3f(1.0f - tick, 1.0f, 1.0f);
				glVertex3f(1.0f - tick, 1.0f, 1.0f);
				glTexCoord3f(1.0f - tick, 0.0f, 1.0f);
				glVertex3f(1.0f - tick, 0.0f, 1.0f);
				break;

			case 2: // Stack PositiveY
				//glNormal3f(  0.0f,1.0f, 0.0f);
				glTexCoord3f(0.0f, tick, 0.0f);
				glVertex3f(0.0f, tick, 0.0f);
				glTexCoord3f(0.0f, tick, 1.0f);
				glVertex3f(0.0f, tick, 1.0f);
				glTexCoord3f(1.0f, tick, 1.0f);
				glVertex3f(1.0f, tick, 1.0f);
				glTexCoord3f(1.0f, tick, 0.0f);
				glVertex3f(1.0f, tick, 0.0f);
				break;

			case 3: // Stack NgativeY
				//glNormal3f(  0.0f,-1.0f, 0.0f);
				glTexCoord3f(0.0f, 1 - tick, 0.0f);
				glVertex3f(0.0f, 1 - tick, 0.0f);
				glTexCoord3f(0.0f, 1 - tick, 1.0f);
				glVertex3f(0.0f, 1 - tick, 1.0f);
				glTexCoord3f(1.0f, 1 - tick, 1.0f);
				glVertex3f(1.0f, 1 - tick, 1.0f);
				glTexCoord3f(1.0f, 1 - tick, 0.0f);
				glVertex3f(1.0f, 1 - tick, 0.0f);
				break;

			case 4: // Stack PositiveZ
				//glNormal3f(  0.0f,0.0f, 1.0f);
				glTexCoord3f(0.0f, 0.0f, tick);
				glVertex3f(0.0f, 0.0f, tick);
				glTexCoord3f(0.0f, 1.0f, tick);
				glVertex3f(0.0, 1.0f, tick);
				glTexCoord3f(1.0f, 1.0f, tick);
				glVertex3f(1.0f, 1.0f, tick);
				glTexCoord3f(1.0f, 0.0f, tick);
				glVertex3f(1.0f, 0.0, tick);
				break;

			case 5: // Stack NegativeZ
				//glNormal3f(  0.0f,0.0f, -1.0f);
				glTexCoord3f(0.0f, 0.0f, 1.0f - tick);
				glVertex3f(0.0f, 0.0f, 1.0f - tick);
				glTexCoord3f(0.0f, 1.0f, 1.0f - tick);
				glVertex3f(0.0, 1.0f, 1.0f - tick);
				glTexCoord3f(1.0f, 1.0f, 1.0f - tick);
				glVertex3f(1.0f, 1.0f, 1.0f - tick);
				glTexCoord3f(1.0f, 0.0f, 1.0f - tick);
				glVertex3f(1.0f, 0.0, 1.0f - tick);
				break;

			}
			tick += tk;

		} while (tick <= 1.0f);
		glEnd();

		glEndList();

	}
}
/* rotation angle for the cube. */
float rcubex = 0.0f;
float rcubey = 0.0f;

float pcube = -2.0f;

char btStatus = 0;

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void InitGL(int Width, int Height) // We call this right after our OpenGL window is created.
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // This Will Clear The Background Color To Black
	glClearDepth(1.0); // Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_GREATER); // The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glShadeModel(GL_SMOOTH); // Enables Smooth Color Shading

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // Reset The Projection Matrix

	gluPerspective(30.0f, (GLfloat) Width / (GLfloat) Height, 0.1f, 100.0f); // Calculate The Aspect Ratio Of The Window

	glMatrixMode(GL_MODELVIEW);

	//http://cirl.missouri.edu/gpu/glsl_lessons/glsl_geometry_shader/index.html

	setShaders(&ProgramObject);

	//http://www.opengl.org/wiki/Texture_Sampling
	//Setting shader's uniform variables
	GLint volume_location = glGetUniformLocation(ProgramObject,
			"volume_texture");
	GLint tf_location =
			glGetUniformLocation(ProgramObject, "transfer_function");

	//Checking if the state of the shader is also consider invalid.
	int isValid;
	glValidateProgram(ProgramObject);
	glGetProgramiv(ProgramObject, GL_VALIDATE_STATUS, &isValid);
	if (isValid)
		printf("Shader is valid!\n");
	else {
		printf("Shader isn't Valid!");
		exit(1);
	}

	// So, to set up those uniforms, bind the shader and call glUniform1i since they are considered as integers
	glUseProgram(ProgramObject);
	//Bind to tex unit 0
	glUniform1i(volume_location, 0);
	//Bind to tex unit 1
	glUniform1i(tf_location, 1);

	InitTexture();

	InitDraw();

}

/* The function called when our window is resized (which shouldn't happen, because we're fullscreen) */
void ReSizeGLScene(int Width, int Height) {
	if (Height == 0) // Prevent A Divide By Zero If The Window Is Too Small
		Height = 1;

	glViewport(0, 0, Width, Height); // Reset The Current Viewport And Perspective Transformation

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(30.0f, (GLfloat) Width / (GLfloat) Height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	winWidth = Width;
	winHeight = Height;
}

/* The main drawing function. */
void DrawGLScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // Clear The Screen And The Depth Buffer

	glLoadIdentity(); // Reset the transformation matrix.
	glTranslatef(0.0f, 0.f, pcube); // Move Right 3 Units, and back into the screen 7

	glRotatef(rcubex, 1.0f, 0.0f, 0.0f); // Rotate The Cube On X, Y, and Z
	glRotatef(rcubey, 0.0f, 1.0f, 0.0f);
	//glRotatef(rcube,0.0f,0.0f,1.0f);

	DrawCube();

	//rcube-=0.1f;					// Decrease The Rotation Variable For The Cube

	// swap the buffers to display, since double buffering is used.
	glutSwapBuffers();
}


// Função callback chamada sempre que o mouse é movimentado
// sobre a janela GLUT com um botão pressionado
void MoveMouseBotaoPressionado(int x, int y) {

	if (btStatus % 2 != 1) {

		rcubey = rcubey + ((x - oldXr) * 3.0) / winWidth;
		rcubex = rcubex + ((y - oldYr) * 3.0) / winHeight;
	}

	if (btStatus / 2 == 1) {

		pcube = pcube + ((x - oldXp) * 0.1) / winWidth;

	}

	//printf("Botao pressionado old(%d,%d) new(%d,%d) status(%d)\n", oldXr, oldYr, x, y,btStatus);
	glutPostRedisplay();
}

// Função callback chamada sempre que o mouse é movimentado
// sobre a janela GLUT
void MoveMouse(int x, int y) {
	//sprintf(texto, "(%d,%d)", x, y);
	glutPostRedisplay();
}

// Função callback chamada para gerenciar eventos do mouse
void GerenciaMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		printf("Botao Esquerdo\n");
		oldXr = x;
		oldYr = y;
		if (btStatus % 2 != 0)
			btStatus += 1;
	} else if (btStatus % 2 == 1)
		btStatus -= 1;
	if (button == GLUT_RIGHT_BUTTON) {
		printf("Botao Direito\n");
		oldXp = x;
		oldYp = y;
		if (btStatus / 2 != 0)
			btStatus += 2;
	} else if (btStatus / 2 == 1)
		btStatus -= 2;

	glutPostRedisplay();
}

GLubyte * readRAW(int argc, char **argv) {

	GLubyte *data;
	FILE *fp;

	if (!(fp = fopen(argv[1], "rb"))) {
		printf("impossible open file %s!\n", argv[1]);
		exit(2);
	}

	d_width = atoi(argv[2]);
	d_height = atoi(argv[3]);
	d_slices = atoi(argv[4]);
	d_nsli = d_slices;
	tk = atof(argv[5]);

	// allocate buffer
	data = (GLubyte *) malloc(d_width * d_height * d_slices);

	fread(data, d_width * d_height * d_slices, 1, fp);
	fclose(fp);

	return data;

}

//Necessários para exibir o contexto openGL
static gboolean expose(GtkWidget *da, GdkEventExpose *event, gpointer user_data) {
	GdkGLContext *glcontext = gtk_widget_get_gl_context(da);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(da);

	// g_print (" :: expose\n");

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) {
		g_assert_not_reached ();
	}

	/* draw in here */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	// glRotatef(ang, 1, 0, 1);
	glRotatef(ang, 0, 1, 0);
	// glRotatef (ang, 0, 0, 1);

	 DrawGLScene();

	glPopMatrix();

	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);

	else
		glFlush();

	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

//Necessários para exibir o contexto openGL
static gboolean configure(GtkWidget *da, GdkEventConfigure *event,
		gpointer user_data) {
	GdkGLContext *glcontext = gtk_widget_get_gl_context(da);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(da);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) {
		g_assert_not_reached ();
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // This Will Clear The Background Color To Black
	glClearDepth(1.0); // Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_GREATER); // The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glShadeModel(GL_SMOOTH); // Enables Smooth Color Shading

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // Reset The Projection Matrix

	//gluPerspective(30.0f, (GLfloat) Width / (GLfloat) Height, 0.1f, 100.0f); // Calculate The Aspect Ratio Of The Window

	glMatrixMode(GL_MODELVIEW);

	//http://cirl.missouri.edu/gpu/glsl_lessons/glsl_geometry_shader/index.html

	setShaders(&ProgramObject);

	//http://www.opengl.org/wiki/Texture_Sampling
	//Setting shader's uniform variables
	GLint volume_location = glGetUniformLocation(ProgramObject,
			"volume_texture");
	GLint tf_location =
			glGetUniformLocation(ProgramObject, "transfer_function");

	//Checking if the state of the shader is also consider invalid.
	int isValid;
	glValidateProgram(ProgramObject);
	glGetProgramiv(ProgramObject, GL_VALIDATE_STATUS, &isValid);
	if (isValid)
		printf("Shader is valid!\n");
	else {
		printf("Shader isn't Valid!");
		exit(1);
	}

	// So, to set up those uniforms, bind the shader and call glUniform1i since they are considered as integers
	glUseProgram(ProgramObject);
	//Bind to tex unit 0
	glUniform1i(volume_location, 0);
	//Bind to tex unit 1
	glUniform1i(tf_location, 1);

	InitTexture();

	InitDraw();

	//glLoadIdentity();
	glViewport(0, 0, da->allocation.width, da->allocation.height);

	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Fundo de tela preto

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();


	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

//Necessários para exibir o contexto openGL
static gboolean rotate(gpointer user_data) {
	GtkWidget *da = GTK_WIDGET (user_data);

	ang++;

	gdk_window_invalidate_rect(da->window, &da->allocation, FALSE);
	gdk_window_process_updates(da->window, FALSE);

	return TRUE;
}

void on_window_destroy(GtkObject *object, gpointer user_data) {
	gtk_main_quit();
}

int main(int argc, char *argv[]) {
	GtkBuilder *builder;
	GtkWidget *window;
	GdkGLConfig *glConfig;
	GError *theError;

	if (argc < 6) {
		printf("usage: leitura arqin.raw width height slices dslices colormap\n");
		exit(1);
	}

	/*
	 * Init GTK+ and GtkGLExt.
	 */
	gtk_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);
	glutInit(&argc, argv);
		//glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
		glutInitWindowPosition(100, 100);
		glutInitWindowSize(640, 640);
		int kw= glutCreateWindow("Teste de Vizualizacao com textura 3D");


	g_print("GL_Version %d  = %s\n",GL_VERSION, glGetString( GL_VERSION ) );


	/*
	 * Configure a OpenGL-capable context.
	 */
	/* Try double-buffered first */
	glConfig = gdk_gl_config_new_by_mode(GdkGLConfigMode(GDK_GL_MODE_RGB
			| GDK_GL_MODE_DEPTH | GDK_GL_MODE_ALPHA | GDK_GL_MODE_DOUBLE));
	if (glConfig == NULL) {
		g_print("Cannot configure a double-buffered context.\n");
		g_print("Will try a single-buffered context.\n");

		/* If we can't configure a double-buffered context, try for single-buffered. */
		glConfig = gdk_gl_config_new_by_mode(GdkGLConfigMode(GDK_GL_MODE_RGB
				| GDK_GL_MODE_DEPTH | GDK_GL_MODE_ALPHA));
		if (glConfig == NULL) {
			g_critical ("Aargh!  Cannot configure any type of OpenGL-capable context.  Exiting.\n");
			return -1;
		}
	}

	/*
	 * Load the GTK interface.
	 */
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "interface01.glade", NULL);
	if (builder == NULL) {
		g_critical("Argh!  Failed to load an initialise the GTK file.  Sad panda.  Exiting\n");
		return -1;
	}

	/*
	 * Get the top-level window reference from loaded Glade file.
	 */
	window = GTK_WIDGET (gtk_builder_get_object (builder, "window1"));

	/* Set unassigned widgets to get handled automatically by the window manager. */
	gtk_container_set_reallocate_redraws(GTK_CONTAINER (window), TRUE);

	/* Add OpenGL-capability to the drawing area / widget. */
	theDrawingArea1
			= GTK_WIDGET (gtk_builder_get_object (builder, "drawingarea1"));
	gtk_widget_set_gl_capability(theDrawingArea1, glConfig, NULL, TRUE,
			GDK_GL_RGBA_TYPE);

	g_signal_connect (theDrawingArea1, "configure-event", G_CALLBACK (configure), NULL);
	g_signal_connect (theDrawingArea1, "expose-event", G_CALLBACK (expose), NULL);

	gtk_builder_connect_signals(builder, NULL);



	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}

	g_print("GL_Version = %s\n",glGetString( GL_VERSION ) );

	fprintf(stderr,"Passou pelo Glew Init!\n");

	if (glewIsSupported("GL_VERSION_2_1"))
		printf("Ready for OpenGL 2.1\n");
	else {
		printf("OpenGL 2.1 not supported\n");
		exit(1);
	}
	if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader
			&& GL_EXT_geometry_shader4)
		printf("Ready for GLSL - vertex, fragment, and geometry units\n");
	else {
		printf("Not totally ready :( \n");
		exit(1);
	}

	if (argc == 7)
		colorMapRead(argv[6], cm);
	else {
		createColorMap(cm);
		//colorMapWrite("teste.map", cm);
		//colorMapRead("teste.map", cm);
	}

	raw = readRAW(argc, argv);


	/* To do any pre-window-show OpenGL stuff, we need to realize at least one drawable so we have a valid drawable for OpenGL :P */
	gtk_widget_realize(theDrawingArea1);

	theGLDrawable = gtk_widget_get_gl_drawable (theDrawingArea1);

	g_object_unref(G_OBJECT (builder));

	glutDestroyWindow(kw);
	gtk_widget_show(window);
	gtk_main();
	/* Start Event Processing Engine */
		glutMainLoop();
	return 0;
}
