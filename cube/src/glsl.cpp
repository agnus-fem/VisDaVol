/*
 * glsl.cpp
 *
 *  Created on: 30/10/2011
 *      Author: aahmgbr
 */

#include "glsl.h"

//YEAH, YEAH, YEAH ... global vars, this is a freggn demo!
	GLuint v,f,f2,p,g;		//Handlers for our vertex, geometry, and fragment shaders
	int gw,gh;				//Keep track of window width and height

//Function from: http://www.evl.uic.edu/aej/594/code/ogl.cpp
//Read in a textfile (GLSL program)
// we need to pass it as a string to the GLSL driver
char *textFileRead(char *fn)
{
	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {

		fp = fopen(fn,"rt");

		if (fp != NULL) {

			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);

		}
	}

	return content;
}

//Function from: http://www.evl.uic.edu/aej/594/code/ogl.cpp
//Read in a textfile (GLSL program)
// we can use this to write to a text file
int textFileWrite(char *fn, char *s)
{
	FILE *fp;
	int status = 0;

	if (fn != NULL) {
		fp = fopen(fn,"w");

		if (fp != NULL) {
			if (fwrite(s,sizeof(char),strlen(s),fp) == strlen(s))
				status = 1;
			fclose(fp);
		}
	}
	return(status);
}

//Got this from http://www.lighthouse3d.com/opengl/glsl/index.php?oglinfo
// it prints out shader info (debugging!)
void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("printShaderInfoLog: %s\n",infoLog);
		free(infoLog);
	}else{
		printf("Shader Info Log: OK\n");
	}
}

//Got this from http://www.lighthouse3d.com/opengl/glsl/index.php?oglinfo
// it prints out shader info (debugging!)
void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("printProgramInfoLog: %s\n",infoLog);
		free(infoLog);
	}else{
		printf("Program Info Log: OK\n");
	}
}

//Got this from http://cirl.missouri.edu/gpu/glsl_lessons/glsl_geometry_shader/index.html
//Setup shaders
void setShaders(GLuint *p)
{
	//a few strings
	// will hold onto the file read in!
	char *vs = NULL, *fs = NULL, *fs2 = NULL, *gs = NULL;

	//First, create our shaders
	printf("First, create our shaders\n");
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	//g = glCreateShader(GL_GEOMETRY_SHADER_EXT);

	//Read in the programs
	printf("Read in the programs\n");
	//vs = textFileRead("GLSL/v2/shader.vert");
	//printf("Vertex Shader:\n%s\n",vs);
	fs = textFileRead("GLSL/v3/postInttf.frag");
	//fs = textFileRead("GLSL/v2/shader.frag");
	printf("Fragment Shader:\n%s\n",fs);
	//gs = textFileRead("GLSL/shader.geom");
	//printf("Geometry Shader:\n%s\n",gs);

	//Setup a few constant pointers for below
	const char * ff = fs;
	//const char * vv = vs;
	//const char * gg = gs;

    printf("Loading Shader Source\n");
	//glShaderSource(v, 1, &vv, NULL);
	glShaderSource(f, 1, &ff, NULL);
	//glShaderSource(g, 1, &gg, NULL);

	//free(vs);
	free(fs);
	//free(gs);

    printf("Compiling Source\n");
	//glCompileShader(v);
	glCompileShader(f);
	//glCompileShader(f2);
	//glCompileShader(g);

	printf("Creating Program\n");
	*p = glCreateProgram();

	printf("Attaching Shaders to Program Shader\n");
	printf("Fragment Shader\n");
	glAttachShader(*p,f);
	//printf("Vertex Shader\n");
	//glAttachShader(p,v);
	//glAttachShader(p,f2);
	//printf("Geometric Shader\n");
	//glAttachShader(p,g);

    //printf("Program Parameter 1\n");
	//glProgramParameteriEXT(p,GL_GEOMETRY_INPUT_TYPE_EXT,GL_LINES);
	//glProgramParameteriEXT(p,GL_GEOMETRY_OUTPUT_TYPE_EXT,GL_LINE_STRIP);

	//int temp;
	//printf("Program Parameter 2\n");
	//glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&temp);
	//printf("GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT = %d \n", (int)temp);
	//glProgramParameteriEXT(p,GL_GEOMETRY_VERTICES_OUT_EXT,temp);

    printf("Link Program\n");
	glLinkProgram(*p);
	//printf("Use Program\n");
	//glUseProgram(p);

	//printShaderInfoLog(v);
	printShaderInfoLog(f);
	//printShaderInfoLog(f2);
	//printShaderInfoLog(g);
	//printProgramInfoLog(p);
}
