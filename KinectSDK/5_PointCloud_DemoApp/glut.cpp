#include "glut.h"
#include "main.h"

void draw()
{
	drawKinectData();
	glutSwapBuffers();
}

void execute()
{
	glutMainLoop();
}

bool init(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutCreateWindow("Kinect SDK Tutorial");
	glutDisplayFunc(draw);
	glutIdleFunc(draw);
	//glutPassiveMotionFunc(OnMouseMove);
	// here are the two new functions
	glutMouseFunc(OnMouseButtonCb);
	glutMotionFunc(OnMouseMoveCb);

	glewInit();
	return true;
}
