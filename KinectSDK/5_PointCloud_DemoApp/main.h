#pragma once
const int width = 640;
const int height = 480;

void drawKinectData();
void Log(const float x, const float y, const float z);
void OnMouseMoveCb(int x, int y);
void OnMouseButtonCb(int button, int state, int x, int y);

void OnPressKeyCb(int key, int xx, int yy);
void OnReleaseKeyCb(int key, int x, int y);
void OnProcessNormalKeysCb(unsigned char key, int xx, int yy);