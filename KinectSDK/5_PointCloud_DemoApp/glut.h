#pragma once
#include <Windows.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glut.h>
#include <string>

using namespace std;

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

public:
	string toString()
	{
		return
			"Eye : [" + to_string(EyeX) + " ," + to_string(EyeY) + " , " + to_string(EyeZ) + " ] " +	
			"Center: [" + to_string(CenterX) + " , " + to_string(CenterY) + " , " + to_string(CenterZ) + " ] " +
			"Up : [" + to_string(UpX) +" , " + to_string(UpY) + " , " + to_string(UpZ) + " ]"	;
	}
};

bool init(int argc, char* argv[]);
void draw();
void execute();