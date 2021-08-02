#pragma once
#include <Windows.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glut.h>


struct GlViewData
{
	GLdouble EyeX;
	GLdouble EyeY;
	GLdouble EyeZ;

	GLdouble CenterX;
	GLdouble CenterY;
	GLdouble CenterZ;
	
	GLdouble UpX;
	GLdouble UpY;
	GLdouble UpZ;
};

bool init(int argc, char* argv[]);
void draw();
void execute();