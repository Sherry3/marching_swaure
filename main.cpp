#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "file_utils.h"
#include "math_utils.h"

typedef struct Vt {
	float x,y,z,value;
}Vertex;

typedef struct ofmodel {
	Vector3f *offVertices;			//Vertex coordiantes
	int numberOfVertices;			
 	int numberOfPolygons;
	int *indices;					//Indices values for solid(i.e. triangles)
	GLuint VAO;
	GLuint VBO;
	GLuint IBO;
}Ofmodel;

typedef struct onmodel {
	Vertex vertices[100][100];			//Vertex coordiantes	
	Vertex lines[400][2];		
	int check[400][4];
	int b[400];
	int kvalue[400];
}OnModel;

/********************************************************************/
/*   Variables */

char * theProgramTitle = "Visualization of marching squares";
int theWindowWidth = 700, theWindowHeight = 700;
int theWindowPositionX = 40, theWindowPositionY = 40;
bool isFullScreen = false;
bool isAnimating = false;
bool boolGrid = true;
bool boolBg = true;
bool boolOutput = true;
bool boolContour = false;
float rotation = 0.0f;
int speed = 1;

char *file="bg2_100.off";

GLuint gWorldLocation;
GLuint uflag1;
float gflag1=1.0;
GLuint utrans1;
Matrix4f mtrans1;
GLuint utheta;
Matrix4f mtheta;
GLuint uscale1;
Matrix4f mscale1;
GLuint uworldTheta;
Matrix4f mworldTheta;

Ofmodel *cylinder;
Ofmodel *sphere;
Ofmodel *bg;
OnModel *output;
Ofmodel *op1;
Ofmodel *blue;

int boxSpeed=0;
int boxPos=0;
int toDraw=0;
float ivalue=0.2f;
int a=0;

GLuint V, C;

/* Methos signatures*/
void fillOutputVBO();
void cb();
void initMatrices();
void drawGrid();
void drawBox();
void drawVertices();
void drawBg();
void drawLine();
void drawLineAsC();
void createCylinder();
void createSphere(int shape, float radius);
Ofmodel* readOffFile(char * OffFile);
int FreeOfmodel(Ofmodel *model);
OnModel* getInfo(char * OffFile);
OnModel* readOffFile(char * OffFile, int a);
void MarchingSquare(OnModel *model);

/* Constants */
const int ANIMATION_DELAY = 0; /* milliseconds between rendering */
const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

/********************************************************************
  Utility functions
 */

/* post: compute frames per second and display in window's title bar */
void computeFPS() {
	static int frameCount = 0;
	static int lastFrameTime = 0;
	static char * title = NULL;
	int currentTime;

	if (!title)
		title = (char*) malloc((strlen(theProgramTitle) + 20) * sizeof (char));
	frameCount++;
	currentTime = glutGet((GLenum) (GLUT_ELAPSED_TIME));
	if (currentTime - lastFrameTime > 1000) {
		sprintf(title, "%s [ FPS: %4.2f ]",
			theProgramTitle,
			frameCount * 1000.0 / (currentTime - lastFrameTime));
		glutSetWindowTitle(title);
		lastFrameTime = currentTime;
		frameCount = 0;
	}
}

static void CreateVertexBuffer() {
	
	//Cylinder
	glGenVertexArrays(1, &(cylinder->VAO));
	glBindVertexArray(cylinder->VAO);

	glEnableVertexAttribArray(V);

	glGenBuffers(1, &(cylinder->VBO));
	glBindBuffer(GL_ARRAY_BUFFER, (cylinder->VBO));
	glBufferData(GL_ARRAY_BUFFER, sizeof (Vector3f)*cylinder->numberOfVertices, cylinder->offVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(V, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glGenBuffers(1, &(cylinder->IBO));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (cylinder->IBO));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (int)*cylinder->numberOfPolygons * 3, cylinder->indices, GL_STATIC_DRAW);


	//Sphere
	glGenVertexArrays(1, &(sphere->VAO));
	glBindVertexArray(sphere->VAO);

	glEnableVertexAttribArray(V);

	glGenBuffers(1, &(sphere->VBO));
	glBindBuffer(GL_ARRAY_BUFFER, (sphere->VBO));
	glBufferData(GL_ARRAY_BUFFER, sizeof (Vector3f)*sphere->numberOfVertices, sphere->offVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(V, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glGenBuffers(1, &(sphere->IBO));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (sphere->IBO));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (int)*sphere->numberOfPolygons * 3, sphere->indices, GL_STATIC_DRAW);


	//Background
	glGenVertexArrays(1, &(bg->VAO));
	glBindVertexArray(bg->VAO);

	glEnableVertexAttribArray(V);

	glGenBuffers(1, &(bg->VBO));
	glBindBuffer(GL_ARRAY_BUFFER, (bg->VBO));
	glBufferData(GL_ARRAY_BUFFER, sizeof (Vector3f)*bg->numberOfVertices, bg->offVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(V, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glGenBuffers(1, &(bg->IBO));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (bg->IBO));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (int)*bg->numberOfPolygons * 3, bg->indices, GL_STATIC_DRAW);


	//Blue
	blue=(Ofmodel*)malloc(sizeof(Ofmodel));
	blue->offVertices=(Vector3f*)malloc(sizeof(Vector3f)*4);

	blue->offVertices[0]=Vector3f(0.6, 0.6, -0.07);
	blue->offVertices[1]=Vector3f(0.6, 1.0, -0.07);
	blue->offVertices[2]=Vector3f(1.0, 0.6, -0.07);
	blue->offVertices[3]=Vector3f(1.0, 1.0, -0.07);

	glGenVertexArrays(1, &(blue->VAO));
	glBindVertexArray(blue->VAO);

	glEnableVertexAttribArray(V);

	glGenBuffers(1, &(blue->VBO));
	glBindBuffer(GL_ARRAY_BUFFER, (blue->VBO));
	glBufferData(GL_ARRAY_BUFFER, sizeof (Vector3f)*4, blue->offVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(V, 3, GL_FLOAT, GL_FALSE, 0, 0);


	//Output
	fillOutputVBO();

	glGenVertexArrays(1, &(op1->VAO));
	glBindVertexArray(op1->VAO);

	glEnableVertexAttribArray(V);

	glGenBuffers(1, &(op1->VBO));
	glBindBuffer(GL_ARRAY_BUFFER, (op1->VBO));
	glBufferData(GL_ARRAY_BUFFER, sizeof (Vector3f)*a*2, op1->offVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(V, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void fillOutputVBO()
{
	op1=(Ofmodel*)malloc(sizeof(Ofmodel));
	op1->offVertices=(Vector3f*)malloc(sizeof(Vector3f)*2*a);

	for(int i=0,j=0;i<a;i++,j++)
	{
		op1->offVertices[j].x=output->lines[i][0].x;
		op1->offVertices[j].y=output->lines[i][0].y;
		op1->offVertices[j].z=0.0f;
		
		j++;	

		op1->offVertices[j].x=output->lines[i][1].x;
		op1->offVertices[j].y=output->lines[i][1].y;
		op1->offVertices[j].z=0.0f;
	}
}


void cb()
{
	output = getInfo(file);

	//Output
	fillOutputVBO();

	glGenVertexArrays(1, &(op1->VAO));
	glBindVertexArray(op1->VAO);

	glEnableVertexAttribArray(V);

	glGenBuffers(1, &(op1->VBO));
	glBindBuffer(GL_ARRAY_BUFFER, (op1->VBO));
	glBufferData(GL_ARRAY_BUFFER, sizeof (Vector3f)*a*2, op1->offVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(V, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) {
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	const GLchar * p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

using namespace std;

static void CompileShaders() {
	GLuint ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	string vs, fs;

	if (!ReadFile(pVSFileName, vs)) {
		exit(1);
	}

	if (!ReadFile(pFSFileName, fs)) {
		exit(1);
	}

	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = {0};

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof (ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof (ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ShaderProgram);

	V = glGetAttribLocation(ShaderProgram, "Position");   //Link V to Position of shader program
	//C = glGetAttribLocation(ShaderProgram, "Color");

	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
	uworldTheta = glGetUniformLocation(ShaderProgram, "worldTheta");
	uflag1 = glGetUniformLocation(ShaderProgram, "flag1");
	utrans1 = glGetUniformLocation(ShaderProgram, "trans1");
	utheta = glGetUniformLocation(ShaderProgram, "theta");
	uscale1 = glGetUniformLocation(ShaderProgram, "scale1");
}

/********************************************************************
 Callback Functions
 These functions are registered with the glut window and called 
 when certain events occur.
 */

void onInit(int argc, char * argv[])
/* pre:  glut window has been initialized
   post: model has been initialized */ {
	/* by default the back ground color is black */
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CompileShaders();
	CreateVertexBuffer();

	mworldTheta.InitIdentity();	
	
	/* set to draw in window based on depth  */
	glEnable(GL_DEPTH_TEST); 
}

static void onDisplay() {

	if(boxPos>=400  || boxPos<0)
	{	
		toDraw=0;
		boxPos=0;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Matrix4f World;

	World.m[0][0] = cosf(rotation); World.m[0][1] = 0.0f; 		   World.m[0][2] = -sinf(rotation); 		   World.m[0][3] = 0.0f;
	World.m[1][0] = 0.0f; World.m[1][1] = 1.0;  World.m[1][2] = 0.0; World.m[1][3] = 0.0f;
	World.m[2][0] = sinf(rotation); World.m[2][1] = 0.0;  World.m[2][2] = cosf(rotation); World.m[2][3] = 0.0f;
	World.m[3][0] = 0.0f; World.m[3][1] = 0.0f;            World.m[3][2] = 0.0f; 		   World.m[3][3] = 1.0f;

	glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World.m[0][0]);
	glUniformMatrix4fv(uworldTheta, 1, GL_TRUE, &mworldTheta.m[0][0]);

	initMatrices();

	//Draw here

	//Draw line
	if(boolContour)
		drawLine();

	//Draw output
	if(boolOutput)
		drawLineAsC();

	//4 Vertices
	drawVertices();

	//Box	
	drawBox();

	//Grid
	if(boolGrid)
		drawGrid();

	//Backfound
	if(boolBg)
		drawBg();

	/* check for any errors when rendering */
	GLenum errorCode = glGetError();
	if (errorCode == GL_NO_ERROR) {
		/* double-buffering - swap the back and front buffers */
		glutSwapBuffers();
	} else {
		fprintf(stderr, "OpenGL rendering error %d\n", errorCode);
	}
}

void initMatrices()
{
	mscale1.InitIdentity();
	mtrans1.InitIdentity();
	mtheta.InitIdentity();
	gflag1=1;

	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);
	glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
	glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
	glUniform1f(uflag1, gflag1);
}

void drawGrid()
{
	glBindVertexArray(cylinder->VAO);
	
	mscale1.InitIdentity();
	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);
	
	gflag1=1.0f;
	glUniform1f(uflag1, gflag1);

	mtheta.InitRotateTransform(90.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
		
	for(int i=0;i<21;i++)
	{
		mtrans1.InitTranslationTransform(i*(2.0/20.0)-1.0, 0.0f, 0.0f);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}

	mtheta.InitRotateTransform(0.0f, 90.0f, 0.0f);
	glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
		
	for(int i=0;i<21;i++)
	{
		mtrans1.InitTranslationTransform(0.0f, i*(2.0/20.0)-1.0, 0.0f);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}
}

void drawBox()
{
	glBindVertexArray(cylinder->VAO);

	gflag1=2.0f;
	glUniform1f(uflag1, gflag1);

	float tx=float(boxPos%20)/10;
	float ty=float(boxPos/20)/10;

	mtheta.InitRotateTransform(90.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);

	mscale1.InitScaleTransform(1.0f, 1.0f, 0.05f);		
	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);

	for(int i=0;i<2;i++)
	{
		mtrans1.InitTranslationTransform(i*(2.0/20.0)-1.0+tx, 0.95-ty, -0.05f);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);	
	}

	mtheta.InitRotateTransform(0.0f, 90.0f, 0.0f);
	glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
		
	for(int i=0;i<2;i++)
	{
		mtrans1.InitTranslationTransform(tx-0.95, -i*(2.0/20.0)+1.0-ty, -0.05f);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}


//	if(!isAnimating)
	{
		mtheta.InitRotateTransform(90.0f, 0.0f, 0.0f);
		glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);

		mscale1.InitScaleTransform(1.0f, 1.0f, 0.2f);		
		glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);

		for(int i=0;i<2;i++)
		{
			mtrans1.InitTranslationTransform(1.0-i*0.4-0.005, 0.8-0.005, -0.1f);
			glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
			glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);	
		}

		mtheta.InitRotateTransform(0.0f, 90.0f, 0.0f);
		glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
		
		for(int i=0;i<2;i++)
		{
			mtrans1.InitTranslationTransform(0.8-0.005, 1.0-i*0.4-0.005, -0.1f);
			glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
			glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
		}

		glBindVertexArray(blue->VAO);	
		initMatrices();

		gflag1=7.0f;
		glUniform1f(uflag1, gflag1);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	}
}

void drawVertices()
{
	glBindVertexArray(sphere->VAO);

	float tx=float(boxPos%20)/10;
	float ty=float(boxPos/20)/10;

	mtheta.InitIdentity();
	glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);

	mscale1.InitScaleTransform(0.015f, 0.015f, 0.015f);		
	glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);

	mtrans1.InitIdentity();		
	int dfs;

	for(int i=0;i<4;i++)
	{
		if(output->check[boxPos][i]==1)
		{
			gflag1=3.5f;
		}
		else
		{
			gflag1=3.0f;
		}

		if(i==1 || i==2)
			dfs=1;
		else
			dfs=0;

		glUniform1f(uflag1, gflag1);
		mtrans1.InitTranslationTransform((dfs)*(2.0/20.0)-1.0+tx, -(i/2)*(2.0/20.0)+1.0-ty, -0.05f);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glDrawElements(GL_TRIANGLES, sphere->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}

//	if(!isAnimating)
	{
		for(int i=0;i<4;i++)
		{
			if(output->check[boxPos][i]==1)
			{
				gflag1=3.5f;
			}
			else
			{
				gflag1=3.0f;
			}

			if(i==1 || i==2)
				dfs=1;
			else
				dfs=0;

			glUniform1f(uflag1, gflag1);
			mtrans1.InitTranslationTransform(0.6+(dfs)*(0.4)-0.005, 1.0-(i/2)*(0.4)-0.005, -0.1f);
			glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
			glDrawElements(GL_TRIANGLES, sphere->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
		}
	}


	glBindVertexArray(cylinder->VAO);
	
	gflag1=8.0f;
	glUniform1f(uflag1, gflag1);

	{
		Vector3f cylin=Vector3f(0.0f, 0.0f, 2.0f);
		Vector3f u, v;

		int rv = output->check[boxPos][0] + output->check[boxPos][1] * 2 + output->check[boxPos][2] * 4 + output->check[boxPos][3] * 8;
			
		if(rv==1 || rv==14)
		{
			u=Vector3f(0.8f, 1.0f, 0.0f);	
			v=Vector3f(0.6f, 0.8f, 0.0f);
		}
		else if(rv==2 || rv==13)
		{
			u=Vector3f(0.8f, 1.0f, 0.0f);	
			v=Vector3f(1.0f, 0.8f, 0.0f);
		}
		else if(rv==4 || rv==11)
		{
			u=Vector3f(0.8f, 0.6f, 0.0f);	
			v=Vector3f(1.0f, 0.8f, 0.0f);
		}
		else if(rv==8 || rv==7)
		{
			u=Vector3f(0.8f, 0.6f, 0.0f);	
			v=Vector3f(0.6f, 0.8f, 0.0f);
		}
		else if(rv==3 || rv==12)
		{
			u=Vector3f(0.6f, 0.8f, 0.0f);	
			v=Vector3f(1.0f, 0.8f, 0.0f);
		}
		else if(rv==9 || rv==6)
		{
			u=Vector3f(0.8f, 0.6f, 0.0f);	
			v=Vector3f(0.8f, 1.0f, 0.0f);
		}

		//Scale upto |u2-u1|
		Vector3f w=v-u;
		float length=w.length();

		mscale1.InitScaleTransform(1.0f, 1.0f, length/2);
		float angle=acos(w.z/length);

		w=w.Cross(cylin);
		w=w.Normalize();	

		mtheta.InitAxisRotateTransform(w, -angle);

		//Translate point on origin to u
		mtrans1.m[0][3]=u.x;
		mtrans1.m[1][3]=u.y;
		mtrans1.m[2][3]=-0.9f;
	
		glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);
			
		glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}

}

void drawBg()
{
	initMatrices();
	gflag1=4.0;
	glUniform1f(uflag1, gflag1);

	mtrans1.InitTranslationTransform(0.01, 0.01, 0.0);
	glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);

	glBindVertexArray(bg->VAO);
	glDrawElements(GL_TRIANGLES, bg->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);	
}


void drawLine()
{
	initMatrices();
	gflag1=6.0;
	glUniform1f(uflag1, gflag1);

	mtrans1.InitTranslationTransform(0.01, 0.01, -0.2f);
	glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
	
	glBindVertexArray(op1->VAO);

	glDrawArrays(GL_LINES, 0, (a-1)*2);
}


void drawLineAsC()
{
	glBindVertexArray(cylinder->VAO);
	
	gflag1=5.0f;
	glUniform1f(uflag1, gflag1);

	if(output->kvalue[toDraw-1]>=boxPos)
		toDraw--;

	if(output->kvalue[toDraw]==boxPos)
		toDraw++;

	for(int i=0,j=0;i<toDraw;i++,j+=2)
	{
		Vector3f cylin=Vector3f(0.0f, 0.0f, 2.0f);	
		Vector3f u=op1->offVertices[j];	
		Vector3f v=op1->offVertices[j+1];

		//Scale upto |u2-u1|
		Vector3f w=v-u;
		float length=w.length();

		mscale1.InitScaleTransform(1.0f, 1.0f, length/2);
		float angle=acos(w.z/length);

		w=w.Cross(cylin);
		w=w.Normalize();	

		mtheta.InitAxisRotateTransform(w, -angle);

		//Translate point on origin to u
		mtrans1.m[0][3]=u.x+0.01;
		mtrans1.m[1][3]=u.y+0.01;
		mtrans1.m[2][3]=u.z-0.5;
	
		glUniformMatrix4fv(utheta, 1, GL_TRUE, &mtheta.m[0][0]);
		glUniformMatrix4fv(utrans1, 1, GL_TRUE, &mtrans1.m[0][0]);
		glUniformMatrix4fv(uscale1, 1, GL_TRUE, &mscale1.m[0][0]);
			
		glDrawElements(GL_TRIANGLES, cylinder->numberOfPolygons * 3, GL_UNSIGNED_INT, 0);
	}
}

/* pre:  glut window has been resized
 */
static void onReshape(int width, int height) {
	glViewport(0, 0, width, height);
	if (!isFullScreen) {
		theWindowWidth = width;
		theWindowHeight = height;
	}
	// update scene based on new aspect ratio....
}

/* pre:  glut window is not doing anything else
   post: scene is updated and re-rendered if necessary */
static void onIdle() {
	static int oldTime = 0;
	if (isAnimating) {
		int currentTime = glutGet((GLenum) (GLUT_ELAPSED_TIME));
		/* Ensures fairly constant framerate */
		if (currentTime - oldTime > ANIMATION_DELAY) {
			// do animation....
			//rotation += speed;
			boxSpeed+=speed;

			if(boxSpeed>=100)
			{
				boxPos++;
				boxSpeed=0;
			}
			else if(boxSpeed<=-100)
			{
				boxPos--;
				boxSpeed=0;
			}


			oldTime = currentTime;
			/* compute the frame rate */
			computeFPS();
			/* notify window it has to be repainted */
			glutPostRedisplay();
		}
	}
}

/* pre:  mouse is dragged (i.e., moved while button is pressed) within glut window
   post: scene is updated and re-rendered  */
static void onMouseMotion(int x, int y) {
	/* notify window that it has to be re-rendered */
	glutPostRedisplay();
}

/* pre:  mouse button has been pressed while within glut window
   post: scene is updated and re-rendered */
static void onMouseButtonPress(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		// Left button pressed
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP){
		// Left button un pressed
	}
	/* notify window that it has to be re-rendered */
	glutPostRedisplay();
}

/* pre:  key has been pressed
   post: scene is updated and re-rendered */
static void onAlphaNumericKeyPress(unsigned char key, int x, int y) {
	switch (key) {
			/* toggle animation running */
		case 'a':
			isAnimating = !isAnimating;
			break;
			/* reset */
		case 'r':
			rotation = 0;
			break;
		case 'g':
			boolGrid=!boolGrid;
			break;
		case 'c':
			boolContour=!boolContour;
			break;
		case 'x':
			ivalue+=0.01;
			cb();
			break;
		case 'v':
			ivalue-=0.01;
			cb();
			break;
		case 'b':
			boolBg=!boolBg;
			break;
		case 'o':
			boolOutput=!boolOutput;
			break;
		case 'd':
			boxPos++;
			break;
		case 's':
			boxPos--;
			break;
		case '+':
			speed+=2;
			break;
		case '-':
			speed-=2;
			break;
			/* quit! */
		case 'Q':
		case 'q':
		case 27:
			exit(0);
	}

	/* notify window that it has to be re-rendered */
	glutPostRedisplay();
}

/* pre:  arrow or function key has been pressed
   post: scene is updated and re-rendered */
static void onSpecialKeyPress(int key, int x, int y) {
	/* please do not change function of these keys */
	switch (key) {
			/* toggle full screen mode */
		case GLUT_KEY_F1:
			isFullScreen = !isFullScreen;
			if (isFullScreen) {
				theWindowPositionX = glutGet((GLenum) (GLUT_WINDOW_X));
				theWindowPositionY = glutGet((GLenum) (GLUT_WINDOW_Y));
				glutFullScreen();
			} else {
				glutReshapeWindow(theWindowWidth, theWindowHeight);
				glutPositionWindow(theWindowPositionX, theWindowPositionY);
			}
			break;
	}

	/* notify window that it has to be re-rendered */
	glutPostRedisplay();
}

/* pre:  glut window has just been iconified or restored 
   post: if window is visible, animate model, otherwise don't bother */
static void onVisible(int state) {
	if (state == GLUT_VISIBLE) {
		/* tell glut to show model again */
		glutIdleFunc(onIdle);
	} else {
		glutIdleFunc(NULL);
	}
}

static void InitializeGlutCallbacks() {
	/* tell glut how to display model */
	glutDisplayFunc(onDisplay);
	/* tell glutwhat to do when it would otherwise be idle */
	glutIdleFunc(onIdle);
	/* tell glut how to respond to changes in window size */
	glutReshapeFunc(onReshape);
	/* tell glut how to handle changes in window visibility */
	glutVisibilityFunc(onVisible);
	/* tell glut how to handle key presses */
	glutKeyboardFunc(onAlphaNumericKeyPress);
	glutSpecialFunc(onSpecialKeyPress);
	/* tell glut how to handle the mouse */
	glutMotionFunc(onMouseMotion);
	glutMouseFunc(onMouseButtonPress);
}

void createCylinder()
{
	int tempV=37*2;
	int tempP=2*tempV;

	cylinder = (Ofmodel*)malloc(sizeof(Ofmodel));
	cylinder->offVertices=(Vector3f*)malloc(sizeof(Vector3f)*(tempV));
	cylinder->indices=(int*)malloc(sizeof(int)*tempP*3);
	
	float i=0, j=0;
	float t1=0;

	int count=0;
	
	while(t1<=360)
	{
		i=0.005*cos(ToRadian(t1));			
		j=0.005*sin(ToRadian(t1));
		cylinder->offVertices[count] = Vector3f(i, j, 1.0f);
		count++;
		cylinder->offVertices[count] = Vector3f(i, j, -1.0f);
		count++;
		t1+=10;
	}

	for(int j=0, ii=0; ii<count-2; ii++,j+=3)
	{
		cylinder->indices[j] = ii;
		cylinder->indices[j+1] = ii+1;
		cylinder->indices[j+2] = ii+2;
	}

	cylinder->numberOfPolygons=tempP;
	cylinder->numberOfVertices=tempV;
}


void createSphere(int shape, float radius)
{
	int tempc=(18/shape+1)*36;

	sphere = (Ofmodel*)malloc(sizeof(Ofmodel));
	sphere->numberOfVertices=tempc;
	sphere->numberOfPolygons=tempc*2;
	sphere->offVertices=(Vector3f*)malloc(sizeof(Vector3f)*(sphere->numberOfVertices));
	sphere->indices=(int*)malloc(sizeof(int)*(sphere->numberOfPolygons)*3);

	float i=0, j=0, k=0;
	float t1=0, t2=0;

	int count=0;
	
	while(t2<360)
	{
		while(t1<=180)
		{
			i=radius*cos(ToRadian(t2))*sin(ToRadian(t1));			
			j=radius*sin(ToRadian(t2))*sin(ToRadian(t1));
			k=radius*cos(ToRadian(t1));
			sphere->offVertices[count] = Vector3f(i, j, k);
			count++;
			t1+=10*shape;
		}
		t1=0;
		t2+=10;
	}

	for(int j=0, ii=0; ii<count-(18/shape+1); ii++,j+=6)
	{
		sphere->indices[j] = ii;
		sphere->indices[j+1] = ii+1;
		sphere->indices[j+2] = ii+(18/shape+1);

		sphere->indices[j+3] = ii;
		sphere->indices[j+4] = ii+(18/shape+1)-1;
		sphere->indices[j+5] = ii+(18/shape+1);
	}

	for(int j=6*(count-(18/shape+1)), ii=count-(18/shape+1); ii<count; ii++,j+=6)
	{
		sphere->indices[j] = ii;
		sphere->indices[j+1] = ii+1;
		sphere->indices[j+2] = ii-(count-(18/shape+1));

		sphere->indices[j+3] = ii;
		sphere->indices[j+4] = ii+(18/shape+1)-1-count;
		sphere->indices[j+5] = ii-(count-(18/shape+1));
	}

	for(int j=0, ii=0; ii<count-(18/shape+1); ii++,j+=6)
	{
		//A triangle
		sphere->indices[j] = ii;
		sphere->indices[j+1] = ii+1;
		sphere->indices[j+2] = ii+(18/shape+1);

		//Another triangle
		sphere->indices[j+3] = ii;
		sphere->indices[j+4] = ii+(18/shape+1)-1;
		sphere->indices[j+5] = ii+(18/shape+1);
	}

}

/************************************************************/
//OFF file reading code

Ofmodel* readOffFile(char * OffFile) {
	FILE * input;
	char type[3]; 
	int noEdges;
	int i,j;
	float x,y,z;
	int n, v;
	int nv, np;
	Ofmodel *model;
	input = fopen(OffFile, "r");
	
	int neveruseit = fscanf(input, "%s", type);
	/* First line should be OFF */
	if(strcmp(type,"OFF")) {
		printf("Not a OFF file");
		exit(1);
	}

	/* Read the no. of vertices, faces and edges */
	neveruseit = fscanf(input, "%d", &nv);
	neveruseit = fscanf(input, "%d", &np);
	neveruseit = fscanf(input, "%d", &noEdges);

	model = (Ofmodel*)malloc(sizeof(Ofmodel));
	model->numberOfVertices = nv;
	model->numberOfPolygons = np;
	
	/* allocate required data */
	model->offVertices = (Vector3f *) malloc(nv * sizeof(Vector3f));
	//model->offColors = (Vector3f *) malloc(nv * sizeof(Vector3f));
	model->indices = (int *) malloc(3 * sizeof(int) * np);
	
	float dummy;
	/* Read the vertices' location*/	
	for(i = 0;i < nv;i ++) {
		neveruseit = fscanf(input, "%f %f %f %f", &x,&y,&z,&dummy);
		model->offVertices[i] = Vector3f(x, y, z);
	}

	/* Read the Polygons */	
	for(i=0 ; i < 3*np ; i++) {
		/* No. of sides of the polygon (Eg. 3 => a triangle) */
		if(i%3==0)
		{
			neveruseit = fscanf(input, "%d", &n);
			neveruseit = fscanf(input, "%d", &v);
			model->indices[i]=v;
		}
		else
		{
		/* read the vertices that make up the polygon */
			neveruseit = fscanf(input, "%d", &v);
			model->indices[i]=v;
		}
	}

	fclose(input);
	return model;
}

/*
int FreeOfmodel(Ofmodel *model)
{
	if( model == NULL )
		return 0;
	free(model->offVertices);
	free(model->indices);
	free(model);
	return 1;
}*/

OnModel* getInfo(char * OffFile) {
	OnModel *model;

//	printf("Enter Scalar Value :: ");
//	scanf("%f", &ivalue);

	model=readOffFile(OffFile, 1);
	MarchingSquare(model);

	return model;
}

OnModel* readOffFile(char * OffFile, int a) {
	FILE * input;
	char type[3]; 
	int i,j;
	float x,y,z,value;
	int nv, np, noEdges;
	OnModel *model;

	input = fopen(OffFile, "r");
	int neveruseit = fscanf(input, "%s", type);
	/* First line should be OFF */
	if(strcmp(type,"OFF")) {
		printf("Not a OFF file");
		exit(1);
	}
	/* Read the no. of vertices, faces and edges */
	neveruseit = fscanf(input, "%d", &nv);
	neveruseit = fscanf(input, "%d", &np);
	neveruseit = fscanf(input, "%d", &noEdges);

	model = (OnModel*)malloc(sizeof(OnModel));
	
	/* Read the vertices' location*/	
	for(i = 0;i < 100;i ++) {
		for(j = 0;j < 100;j ++){
		fscanf(input, "%f %f %f %f", &x,&y,&z,&value);
		(model->vertices[99-i][j]).x = x;
		(model->vertices[99-i][j]).y = y;
		(model->vertices[99-i][j]).z = z;
		(model->vertices[99-i][j]).value = value;

		//if(j%5==0 && (99-i+1)%5==0)
		//	printf("%d %d :: %f %f :: %f\n", (99-i)/5, j/5, x, y, value);
		}
	}
	fclose(input);
	return model;
}

void MarchingSquare(OnModel *model){
	int i=0,j=0,k=0,l=0;
	int flag1,flag2;
	int axc=5, ayc=5;

	a=0;

	for(k=0;k<400;k++){

		if((model->vertices[i][j]).value>=ivalue) model->check[k][0]=1;
		else model->check[k][0]=0;

		if((model->vertices[i][j+axc]).value>=ivalue) model->check[k][1]=1;
		else model->check[k][1]=0;

		if((model->vertices[i+ayc][j+axc]).value>=ivalue) model->check[k][2]=1;
		else model->check[k][2]=0;

		if((model->vertices[i+ayc][j]).value>=ivalue) model->check[k][3]=1;
		else model->check[k][3]=0;

		//printf("%d - %d %d %d %d :: %f %f %f %f\n", k, model->check[k][0], model->check[k][1], model->check[k][2], model->check[k][3], model->vertices[i][j].value, model->vertices[i][j+5].value, model->vertices[i+5][j+5].value, model->vertices[i+5][j].value);
			
		//printf("%d %d :: %f %f :: %f\n", i, j, (model->vertices[i][j]).x, (model->vertices[i][j]).y, (model->vertices[i][j]).value);
		
		if(j==90)
			axc=4;

		if(j==95){
			j=0;
			axc=5;
			ayc=5;

			if(i==90)
				ayc=4;

			i+=5;
		}
		else j+=5;

	}

	for(k=0;k<400;k++){
		i=k/20;j=k%20;

		i*=5;
		j*=5;

		flag1=0;flag2=0;

		//2 Points
		//0 1 in
		if((model->check[k][0]==1 && model->check[k][1]==1 && model->check[k][2]==0 && model->check[k][3]==0)){
			for(l=0;l<=5;l++){
				if(model->vertices[i+l][j].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].x=model->vertices[i+l][j].x;
					model->lines[a][0].y=(model->vertices[i+l-1][j].y) +
							(((model->vertices[i+l][j].y)-(model->vertices[i+l-1][j].y))*(ivalue-model->vertices[i+l-1][j].value)) / 
								((model->vertices[i+l][j].value-(model->vertices[i+l-1][j].value)));

					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 1);						

				}
				if(model->vertices[i+l][j+5].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].x=model->vertices[i+l][j+5].x;
					model->lines[a][1].y=(model->vertices[i+l-1][j+5].y) +
							(((model->vertices[i+l][j+5].y)-(model->vertices[i+l-1][j+5].y))*(ivalue-model->vertices[i+l-1][j+5].value)) /  ((model->vertices[i+l][j+5].value-(model->vertices[i+l-1][j+5].value)));				

					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 1);
	
				}
			}
		model->b[a]=1;
		model->kvalue[a]=k;
		a++;
		}
		
		//2 3 in
		else if(model->check[k][0]==0 && model->check[k][1]==0 && model->check[k][2]==1 && model->check[k][3]==1){			

			for(l=0;l<=5;l++){
				if(model->vertices[i-l][j].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].x=model->vertices[i-l][j].x;
					model->lines[a][0].y=(model->vertices[i-l+1][j].y) +
							(((model->vertices[i-l][j].y)-(model->vertices[i-l+1][j].y))*(ivalue-model->vertices[i-l+1][j].value)) / 
								((model->vertices[i-l][j].value-(model->vertices[i-l+1][j].value)));					
					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 2);

				}
				if(model->vertices[i-l][j+5].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].x=model->vertices[i-l][j+5].x;
					model->lines[a][1].y=(model->vertices[i-l+1][j+5].y) +
							(((model->vertices[i-l][j+5].y)-(model->vertices[i-l+1][j+5].y))*(ivalue-model->vertices[i-l+1][j+5].value)) /  ((model->vertices[i-l][j+5].value-(model->vertices[i-l+1][j+5].value)));
						
					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 2);
					
				}
			}
		model->b[a]=2;
		model->kvalue[a]=k;
		a++;
		}

		//0 3 in
		else if(model->check[k][0]==1 && model->check[k][1]==0 && model->check[k][2]==0 && model->check[k][3]==1){			

			for(l=0;l<=5;l++){
				if(model->vertices[i][j+l].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].y=model->vertices[i][j+l].y;
					model->lines[a][0].x=(model->vertices[i][j+l-1].x) +
							(((model->vertices[i][j+l].x)-(model->vertices[i][j+l-1].x))*(ivalue-model->vertices[i][j+l-1].value)) / 
								((model->vertices[i][j+l].value-(model->vertices[i][j+l-1].value)));					

					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 3);						

				}
				if(model->vertices[i+5][j+l].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].y=model->vertices[i+5][j+l].y;
					model->lines[a][1].x=(model->vertices[i+5][j+l-1].x) +
							(((model->vertices[i+5][j+l].x)-(model->vertices[i+5][j+l-1].x))*(ivalue-model->vertices[i+5][j+l-1].value)) /  ((model->vertices[i+5][j+l].value-(model->vertices[i+5][j+l-1].value)));					
						
					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 3);

				}
			}
		model->b[a]=3;
		model->kvalue[a]=k;
		a++;
		}

		//1 2 in		
		else if(model->check[k][0]==0 && model->check[k][1]==1 && model->check[k][2]==1 && model->check[k][3]==0){			
			for(l=0;l<=5;l++){

				if(model->vertices[i][j-l].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].y=model->vertices[i][j-l].y;
					model->lines[a][0].x=(model->vertices[i][j-l+1].x) +
							(((model->vertices[i][j-l].x)-(model->vertices[i][j-l+1].x))*(ivalue-model->vertices[i][j-l+1].value)) / 
								((model->vertices[i][j-l].value-(model->vertices[i][j-l+1].value)));					

					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 4);

				}
				if(model->vertices[i+5][j-l].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].y=model->vertices[i+5][j-l].y;
					model->lines[a][1].x=(model->vertices[i+5][j-l+1].x) +
							(((model->vertices[i+5][j-l].x)-(model->vertices[i+5][j-l+1].x))*(ivalue-model->vertices[i+5][j-l+1].value)) /  ((model->vertices[i+5][j-l].value-(model->vertices[i+5][j-l+1].value)));	

					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 4);
				
				}
			}
		model->b[a]=4;
		model->kvalue[a]=k;
		a++;
		}

		//3 points or 1 point
		//0 1 2 in
		else if(model->check[k][0]==1 && model->check[k][1]==1 && model->check[k][2]==1 && model->check[k][3]==0){			
			for(l=0;l<=5;l++){
				if(model->vertices[i+l][j].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].x=model->vertices[i+l][j].x;
					model->lines[a][0].y=(model->vertices[i+l-1][j].y) +
							(((model->vertices[i+l][j].y)-(model->vertices[i+l-1][j].y))*(ivalue-model->vertices[i+l-1][j].value)) / 
								((model->vertices[i+l][j].value-(model->vertices[i+l-1][j].value)));					
					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 5);

				}
				if(model->vertices[i+5][j+l].value>ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].y=model->vertices[i+5][j+l].y;
					model->lines[a][1].x=(model->vertices[i+5][j+l-1].x) +
							(((model->vertices[i+5][j+l].x)-(model->vertices[i+5][j+l-1].x))*(ivalue-model->vertices[i+5][j+l-1].value)) /  ((model->vertices[i+5][j+l].value-(model->vertices[i+5][j+l-1].value)));
						
					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 5);
					
				}
			}
		model->b[a]=5;
		model->kvalue[a]=k;
		a++;
		}
		//only 3 in
		else if(model->check[k][0]==0 && model->check[k][1]==0 && model->check[k][2]==0 && model->check[k][3]==1){			
			for(l=0;l<=5;l++){
				if(model->vertices[i+5-l][j].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].x=model->vertices[i+5-l][j].x;
					model->lines[a][0].y=(model->vertices[i+5-l+1][j].y) +
							(((model->vertices[i+5-l][j].y)-(model->vertices[i+5-l+1][j].y))*(ivalue-model->vertices[i+5-l+1][j].value)) /  ((model->vertices[i+5-l][j].value-(model->vertices[i+5-l+1][j].value)));					

					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 6);

				}
				if(model->vertices[i+5][j+l].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].y=model->vertices[i+5][j+l].y;
					model->lines[a][1].x=(model->vertices[i+5][j+l-1].x) +
							(((model->vertices[i+5][j+l].x)-(model->vertices[i+5][j+l-1].x))*(ivalue-model->vertices[i+5][j+l-1].value)) /  ((model->vertices[i+5][j+l].value-(model->vertices[i+5][j+l-1].value)));					
						
					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 6);

				}
			}
		model->b[a]=6;
		model->kvalue[a]=k;
		a++;
		}

		//1 2 3 in
		else if(model->check[k][0]==0 && model->check[k][1]==1 && model->check[k][2]==1 && model->check[k][3]==1){			
			for(l=0;l<=5;l++){
				if(model->vertices[i][j+5-l].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].y=model->vertices[i][j+5-l].y;
					model->lines[a][0].x=(model->vertices[i][j+5-l+1].x) +
							(((model->vertices[i][j+5-l].x)-(model->vertices[i][j+5-l+1].x))*(ivalue-model->vertices[i][j+5-l+1].value)) / ((model->vertices[i][j+5-l].value-(model->vertices[i][j+5-l+1].value)));					

					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 7);

				}
				if(model->vertices[i+5-l][j].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].x=model->vertices[i+5-l][j].x;
					model->lines[a][1].y=(model->vertices[i+5-l+1][j].y) +
							(((model->vertices[i+5-l][j].y)-(model->vertices[i+5-l+1][j].y))*(ivalue-model->vertices[i+5-l+1][j].value)) / ((model->vertices[i+5-l][j].value-(model->vertices[i+5-l+1][j].value)));
						
					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 7);	
				}
			}
		model->b[a]=7;
		model->kvalue[a]=k;
		a++;
		}

		//only 0 in
		else if(model->check[k][0]==1 && model->check[k][1]==0 && model->check[k][2]==0 && model->check[k][3]==0){		//Lack of evidence, can't model->check correctness	
			for(l=0;l<=5;l++){
				if(model->vertices[i+l][j].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].x=model->vertices[i+l][j].x;
					model->lines[a][0].y=(model->vertices[i+l-1][j].y) +
							(((model->vertices[i+l][j].y)-(model->vertices[i+l-1][j].y))*(ivalue-model->vertices[i+l-1][j].value)) / 
								((model->vertices[i+l][j].value-(model->vertices[i+l-1][j].value)));					

					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 8);

				}
				if(model->vertices[i][j+l].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].y=model->vertices[i][j+l].y;
					model->lines[a][1].x=(model->vertices[i][j+l-1].x) +
							(((model->vertices[i][j+l].x)-(model->vertices[i][j+l-1].x))*(ivalue-model->vertices[i][j+l-1].value)) / 
								((model->vertices[i][j+l].value-(model->vertices[i][j+l-1].value)));					
						
					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 8);

				}
			}
		model->b[a]=8;
		model->kvalue[a]=k;
		a++;
		}

		//2 3 0 in
		else if(model->check[k][0]==1 && model->check[k][1]==0 && model->check[k][2]==1 && model->check[k][3]==1){			
			for(l=0;l<=5;l++){
				if(model->vertices[i][j+l].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].y=model->vertices[i][j+l].y;
					model->lines[a][0].x=(model->vertices[i][j+l-1].x) +
							(((model->vertices[i][j+l].x)-(model->vertices[i][j+l-1].x))*(ivalue-model->vertices[i][j+l-1].value)) / 
								((model->vertices[i][j+l].value-(model->vertices[i][j+l-1].value)));					

					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 9);

				}
				if(model->vertices[i+5-l][j+5].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].x=model->vertices[i+5-l][j+5].x;
					model->lines[a][1].y=(model->vertices[i+5-l+1][j+5].y) +
							(((model->vertices[i+5-l][j+5].y)-(model->vertices[i+5-l+1][j+5].y))*(ivalue-model->vertices[i+5-l+1][j+5].value)) /  ((model->vertices[i+5-l][j+5].value-(model->vertices[i+5-l+1][j+5].value)));					
						
					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 9);

				}
			}
		model->b[a]=9;
		model->kvalue[a]=k;
		a++;
		}

		//1 in
		else if(model->check[k][0]==0 && model->check[k][1]==1 && model->check[k][2]==0 && model->check[k][3]==0){			
			for(l=0;l<=5;l++){
				if(model->vertices[i][j+5-l].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].y=model->vertices[i][j+5-l].y;
					model->lines[a][0].x=(model->vertices[i][j+5-l+1].x) +
							(((model->vertices[i][j+5-l].x)-(model->vertices[i][j+5-l+1].x))*(ivalue-model->vertices[i][j+5-l+1].value)) /  ((model->vertices[i][j+5-l].value-(model->vertices[i][j+5-l+1].value)));					

					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 10);

				}
				if(model->vertices[i+l][j+5].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].x=model->vertices[i+l][j+5].x;
					model->lines[a][1].y=(model->vertices[i+l-1][j+5].y) +
							(((model->vertices[i+l][j+5].y)-(model->vertices[i+l-1][j+5].y))*(ivalue-model->vertices[i+l-1][j+5].value)) /  ((model->vertices[i+l][j+5].value-(model->vertices[i+l-1][j+5].value)));					

					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 10);

				}
			}
		model->b[a]=10;
		model->kvalue[a]=k;
		a++;
		}

		//3 0 1 in
		else if(model->check[k][0]==1 && model->check[k][1]==1 && model->check[k][2]==0 && model->check[k][3]==1){			
			for(l=0;l<=5;l++){
				if(model->vertices[i+l][j+5].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].x=model->vertices[i+l][j+5].x;
					model->lines[a][0].y=(model->vertices[i+l-1][j+5].y) +
							(((model->vertices[i+l][j+5].y)-(model->vertices[i+l-1][j+5].y))*(ivalue-model->vertices[i+l-1][j+5].value)) /  ((model->vertices[i+l][j+5].value-(model->vertices[i+l-1][j+5].value)));					

					//printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 11);

				}
				if(model->vertices[i+5][j+l].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].y=model->vertices[i+5][j+l].y;
					model->lines[a][1].x=(model->vertices[i+5][j+l-1].x) +
							(((model->vertices[i+5][j+l].x)-(model->vertices[i+5][j+l-1].x))*(ivalue-model->vertices[i+5][j+l-1].value)) /  ((model->vertices[i+5][j+l].value-(model->vertices[i+5][j+l-1].value)));					

					//printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 11);

				}
			}
		model->b[a]=11;
		model->kvalue[a]=k;
		a++;
		}

		//only 2 in
		else if(model->check[k][0]==0 && model->check[k][1]==0 && model->check[k][2]==1 && model->check[k][3]==0){			
			for(l=0;l<=5;l++){
				if(model->vertices[i+5-l][j+5].value<=ivalue && flag1==0){
					flag1=1;
					model->lines[a][0].x=model->vertices[i+5-l][j+5].x;
					model->lines[a][0].y=(model->vertices[i+5-l+1][j+5].y) +
							(((model->vertices[i+5-l][j+5].y)-(model->vertices[i+5-l+1][j+5].y))*(ivalue-model->vertices[i+5-l+1][j+5].value)) /  ((model->vertices[i+5-l][j+5].value-(model->vertices[i+5-l+1][j+5].value)));					

				//	printf("1 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][0].x, model->lines[a][0].y, model->lines[a][0].z, k, 12);

				}
				if(model->vertices[i+5][j+5-l].value<=ivalue && flag2==0){
					flag2=1;
					model->lines[a][1].y=model->vertices[i+5][j+5-l].y;
					model->lines[a][1].x=(model->vertices[i+5][j+5-l+1].x) +
							(((model->vertices[i+5][j+5-l].x)-(model->vertices[i+5][j+5-l+1].x))*(ivalue-model->vertices[i+5][j+5-l+1].value)) / ((model->vertices[i+5][j+5-l].value-(model->vertices[i+5][j+5-l+1].value)));					
						
				//	printf("2 :: %f, %f, %f :: %d   ::  %d\n", model->lines[a][1].x, model->lines[a][1].y, model->lines[a][1].z, k, 12);

				}
			}
		model->b[a]=12;
		model->kvalue[a]=k;
		a++;
		}
	
	}
	
}


int main(int argc, char** argv) {

	//menu();

	createCylinder();
	createSphere(3, 1);
	bg = readOffFile(file);
	output = getInfo(file);


	for(int i=0;i<a;i++){
		//printf("(%f,%f) , (%f,%f)::: %d ::: %d \n", output->lines[i][0].x, output->lines[i][0].y, output->lines[i][1].x, output->lines[i][1].y, output->b[i], output->kvalue[i]);
	}

	glutInit(&argc, argv);

	/* request initial window size and position on the screen */
	glutInitWindowSize(theWindowWidth, theWindowHeight);
	glutInitWindowPosition(theWindowPositionX, theWindowPositionY);
	/* request full color with double buffering and depth-based rendering */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	/* create window whose title is the name of the executable */
	glutCreateWindow(theProgramTitle);

	InitializeGlutCallbacks();

	// Must be done after glut is initialized!
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	/* initialize model */
	onInit(argc, argv);

	/* give control over to glut to handle rendering and interaction  */
	glutMainLoop();

	/* program should never get here */

	return 0;
}

