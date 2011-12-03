/*
 * glsl.h
 *
 *  Created on: 30/10/2011
 *      Author: aahmgbr
 */

#ifndef GLSL_H_
#define GLSL_H_

#include <stdio.h>			//C standard IO
#include <stdlib.h>			//C standard lib
#include <string.h>			//C string lib

#include <GL/glew.h>		//GLEW lib
#include <GL/glut.h>    // Header File For The GLUT Library
//#include <GL/gl.h>	// Header File For The OpenGL32 Library
//#include <GL/glu.h>	// Header File For The GLu32 Library


char *textFileRead(char *fn);
int textFileWrite(char *fn, char *s);
void printShaderInfoLog(GLuint obj);
void printProgramInfoLog(GLuint obj);
void setShaders(GLuint *);


#endif /* GLSL_H_ */
