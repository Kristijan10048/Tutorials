#include "main.h"
#include "glut.h"

#include <cmath>
#include <cstdio>

#include <Windows.h>
#include <Ole2.h>

#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>

//Added logging for conlse 
#include "loguru.hpp"

#include <fstream>
#include <string>

using namespace std;

// OpenGL Variables
long depthToRgbMap[width*height*2];

// We'll be using buffer objects to store the kinect point cloud
GLuint vboId;
GLuint cboId;

// Kinect variables
HANDLE depthStream;
HANDLE rgbStream;
INuiSensor* sensor;

//tmp file loggin
std::ofstream log_file("log_file.txt", std::ios_base::out | std::ios_base::app);

int g_iFrmCount;

float deltaAngle = 0.01f;
int xOrigin = -1;
// angle of rotation for the camera direction
float g_fAngle = 0.0f;

// actual vector representing the camera's direction
float g_fLx = 0.0f, g_fLz = -1.0f;

// XZ position of the camera
float g_fX = 0.0f, g_fZ = 5.0f;

//glut view params
GlViewData g_viewParam{ .0, .0, .0, .0, .0, .0, .0, .0, .0, };

bool initKinect() {
    // Get a working kinect sensor
    int numSensors;
    if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1) 
		return false;
    
	if (NuiCreateSensorByIndex(0, &sensor) < 0) 
		return false;

    // Initialize sensor
    sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH | NUI_INITIALIZE_FLAG_USES_COLOR);
    sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, // Depth camera or rgb camera?
	    NUI_IMAGE_RESOLUTION_640x480,                // Image resolution
        0,        // Image stream flags, e.g. near mode
        2,        // Number of frames to buffer
        NULL,     // Event handle
        &depthStream);
	sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, // Depth camera or rgb camera?
		NUI_IMAGE_RESOLUTION_640x480,                // Image resolution
        0,      // Image stream flags, e.g. near mode
        2,      // Number of frames to buffer
        NULL,   // Event handle
		&rgbStream);
    return sensor;
}

void getDepthData(GLubyte* dest)
{
	float x;
	float y;
	float z;

	float* fdest = ( float* ) dest;
	long* depth2rgb = ( long* ) depthToRgbMap;
	NUI_IMAGE_FRAME imageFrame;
	NUI_LOCKED_RECT LockedRect;
	

	if( sensor->NuiImageStreamGetNextFrame(depthStream, 0, &imageFrame) < 0 ) 
		return;

	std::ofstream log_frame(".\\log_frames\\log_frame" + to_string(g_iFrmCount) + ".txt", std::ios_base::out | std::ios_base::app);

	INuiFrameTexture* texture = imageFrame.pFrameTexture;
	texture->LockRect(0, &LockedRect, NULL, 0);
	if( LockedRect.Pitch != 0 )
	{
		const USHORT* curr = ( const USHORT* ) LockedRect.pBits;
		for( int j = 0; j < height; ++j )
		{
			for( int i = 0; i < width; ++i )
			{
				// Get depth of pixel in millimeters
				USHORT depth = NuiDepthPixelToDepth(*curr++);
				// Store coordinates of the point corresponding to this pixel
				Vector4 pos = NuiTransformDepthImageToSkeleton(i, j, depth << 3, NUI_IMAGE_RESOLUTION_1280x960);
				x = pos.x / pos.w;
				y = pos.y / pos.w;
				z = pos.z / pos.w;

				*fdest++ = x;
				*fdest++ = y;
				*fdest++ = z;

				//Log(pos.x, pos.y, pos.z);
				//log_frame << to_string(x) << " " << to_string(y) << " " << to_string(z) <<  "\n";

				// Store the index into the color array corresponding to this pixel
				NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
					NUI_IMAGE_RESOLUTION_1280x960, NUI_IMAGE_RESOLUTION_1280x960, NULL,
					i, j, depth << 3, depth2rgb, depth2rgb + 1);
				depth2rgb += 2;
			}
		}
	}
	texture->UnlockRect(0);
	sensor->NuiImageStreamReleaseFrame(depthStream, &imageFrame);
	
	g_iFrmCount++;
	log_frame.close();
}

void getRgbData(GLubyte* dest)
{
	float* fdest = ( float* ) dest;
	long* depth2rgb = ( long* ) depthToRgbMap;
	NUI_IMAGE_FRAME imageFrame;
	NUI_LOCKED_RECT LockedRect;

	if( sensor->NuiImageStreamGetNextFrame(rgbStream, 0, &imageFrame) < 0 ) 
		return;

	INuiFrameTexture* texture = imageFrame.pFrameTexture;
	texture->LockRect(0, &LockedRect, NULL, 0);
	if( LockedRect.Pitch != 0 )
	{
		const BYTE* start = ( const BYTE* ) LockedRect.pBits;
		for( int j = 0; j < height; ++j )
		{
			for( int i = 0; i < width; ++i )
			{
				// Determine rgb color for each depth pixel
				long x = *depth2rgb++;
				long y = *depth2rgb++;
				// If out of bounds, then don't color it at all
				if( x < 0 || y < 0 || x > width || y > height )
				{
					for( int n = 0; n < 3; ++n ) 
						*(fdest++) = 0.0f;
				}
				else
				{
					const BYTE* curr = start + (x + width * y) * 4;
					for( int n = 0; n < 3; ++n ) 
						*(fdest++) = curr[2 - n] / 255.0f;
				}

			}
		}
	}
	texture->UnlockRect(0);
	sensor->NuiImageStreamReleaseFrame(rgbStream, &imageFrame);
}

void getKinectData()
{
	GLubyte* ptr;

	//Depth
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	ptr = ( GLubyte* ) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	if( ptr )
	{
		getDepthData(ptr);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	//Color 
	glBindBuffer(GL_ARRAY_BUFFER, cboId);
	ptr = ( GLubyte* ) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	if( ptr )
	{
		getRgbData(ptr);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void rotateCamera()
{
	static double angle = 0.;
	static double radius = 2.;

	double x = radius * sin(g_fAngle);
	double z = radius * (1 - cos(g_fAngle)) - radius / 2;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(x, 0, z,
			  0, 0, radius / 2,
			  0, 1, 0);
	//angle += 0.01;
}

void drawKinectData()
{
	getKinectData();
	rotateCamera();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, cboId);
	glColorPointer(3, GL_FLOAT, 0, NULL);

	glPointSize(1.f);
	glDrawArrays(GL_POINTS, 0, width * height);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

void OnMouseMoveCb(int x, int y)
{
	// this will only be true when the left button is down
	if( xOrigin >= 0 )
	{

		// update deltaAngle
		deltaAngle = (x - xOrigin) * 0.001f;

		// update camera's direction
		g_fLx = sin(g_fAngle + deltaAngle);
		g_fLz = -cos(g_fAngle + deltaAngle);
	}
}

void OnMouseButtonCb(int button, int state, int x, int y)
{

	// only start motion if the left button is pressed
	if( button == GLUT_LEFT_BUTTON )
	{

		// when the button is released
		if( state == GLUT_UP )
		{
			g_fAngle += deltaAngle;
			xOrigin = -1;
		}
		else
		{// state = GLUT_DOWN
			xOrigin = x;
		}
	}
}

void Log(const float x, const float y, const float z)
{
	log_file << to_string(x) << " " << to_string(y) << " " << to_string(z) << "\n";
}


void Log(const std::string& text)
{	
	log_file << text;// << std::end;
}

int main(int argc, char* argv[])
{
	Log("Staring main app 1");

	if( !init(argc, argv) )
		return 1;

	if( !initKinect() )
		return 1;
	
	g_iFrmCount = 0;
	// OpenGL setup
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);

	// Set up array buffers
	const int dataSize = width * height * 3 * 4;
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, dataSize, 0, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &cboId);
	glBindBuffer(GL_ARRAY_BUFFER, cboId);
	glBufferData(GL_ARRAY_BUFFER, dataSize, 0, GL_DYNAMIC_DRAW);

	// Camera setup
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, width / ( GLdouble ) height, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 0, 0, 0, 1, 0, 1, 0);

	// Main loop
	execute();
	log_file.close();
	return 0;
}