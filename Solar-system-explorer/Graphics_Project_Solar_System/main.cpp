#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <glut.h>
#include <gl\GLU.h>
#include <gl\GL.h>
#include <glaux.h>
#include <Windows.h>
#pragma comment(lib, "GLAUX.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")
#include "Camera.h"
#include "planet.h"

#define WIDTH 1280
#define HEIGHT 640
#define RNDMAX 32767.0
#define M_PI  3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define SD 8.0
#define IMG 16

Camera Cam;
bool key[1024];
bool mouseLeftDown = false;
bool mouseRightDown = false;
int midWidth = WIDTH/2;
int midHeight = HEIGHT/2;
const float translation_speed = 0.001;
const float rotation_speed = M_PI / 180 * 0.1;
double var = 0.0;

double win_aspect_ratio = ((double)WIDTH / (double)HEIGHT);
// Sun & Earth & Moon
static double Day = 0, Time = 0, a=0,c=0;
static int delay = 1;
static double E, M, D;
bool* keyStates = new bool[256]{ false, };
int bindex = 0;
GLuint Texture[IMG];
GLUquadricObj *background, *sun, *mercury, *venus, *earth, *moon, *mars, *jupiter, *saturn, *uranus, *neptune, *pluto;
AUX_RGBImageRec *pTextureImage[11];
AUX_RGBImageRec *LoadBMP(char *);
void initLight();
void display();
void keyUp(unsigned char, int, int);

float RND(float min, float max) {
	return (rand() / RNDMAX)*(max - min) + min;
}

int LoadGLTexture() {
	int S = FALSE;
	sun = gluNewQuadric();
	earth = gluNewQuadric();
	moon = gluNewQuadric();
	gluQuadricDrawStyle(sun, GLU_FILL);
	gluQuadricDrawStyle(earth, GLU_FILL);
	gluQuadricDrawStyle(moon, GLU_FILL);
	gluQuadricTexture(sun, GL_TRUE);
	gluQuadricTexture(earth, GL_TRUE);
	gluQuadricTexture(moon, GL_TRUE);
	memset(pTextureImage, 0, sizeof(void *) * 1);
	if ((pTextureImage[0] = LoadBMP("space4.bmp")) 
		&& (pTextureImage[1] = LoadBMP("space5.bmp")) 
		&& (pTextureImage[2] = LoadBMP("space8.bmp"))
		&& (pTextureImage[3] = LoadBMP("spacebox1.bmp"))
		&& (pTextureImage[4] = LoadBMP("spacebox2.bmp"))
		&& (pTextureImage[5] = LoadBMP("planet/sun.bmp"))
		&& (pTextureImage[6] = LoadBMP("planet/mercury.bmp"))
		&& (pTextureImage[7] = LoadBMP("planet/venus.bmp"))
		&& (pTextureImage[8] = LoadBMP("planet/earth.bmp"))
		&& (pTextureImage[9] = LoadBMP("planet/moon.bmp"))
		&& (pTextureImage[10] = LoadBMP("planet/mars.bmp"))
		&& (pTextureImage[11] = LoadBMP("planet/jupiter.bmp"))
		&& (pTextureImage[12] = LoadBMP("planet/saturn.bmp"))
		&& (pTextureImage[13] = LoadBMP("planet/uranus.bmp"))
		&& (pTextureImage[14] = LoadBMP("planet/neptune.bmp"))
		&& (pTextureImage[15] = LoadBMP("planet/pluto.bmp"))) {
		S = TRUE;
		glGenTextures(IMG, &Texture[0]);
		for (int i = 0; i < IMG; i++) {
			glBindTexture(GL_TEXTURE_2D, Texture[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, 4, pTextureImage[i]->sizeX, pTextureImage[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, pTextureImage[i]->data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glEnable(GL_TEXTURE_2D);
		}
	}
	for (int i = 0; i < IMG; i++) {
		if (pTextureImage[i]) {
			if (pTextureImage[i]->data)
				free(pTextureImage[i]->data);
			free(pTextureImage[i]);
		}
	}
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	return S;
}
AUX_RGBImageRec *LoadBMP(char *szFilename) {
	FILE *pFile = NULL;
	if (!szFilename) return NULL;
	pFile = fopen(szFilename, "r");
	if (pFile) {
		fclose(pFile);
		return auxDIBImageLoad(szFilename);
	}
	return NULL;
}
void initLight() {
	GLfloat light_specular[] = { 0.1,0.1,0.1,1.0 };
	GLfloat light_diffuse[] = { 100.0,100.0,100.0,1.0 };
	GLfloat light_ambient[] = { 50.0,50.0,50.0,1.0 };
	GLfloat light_position[] = { 0.0,0.0,0.0,1.0 };
	//GLfloat light_specref[] = { 1.0,1.0,1.0,1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.0);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.5);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0);
	glEnable(GL_LIGHT0);
}
void setupMaterial(float r, float g, float b) {
	float ambient[] = { 0.1,0.1,0.1,1.0 };
	float diffuse[] = { r,g,b,1.0 };
	float specular[] = { 1.0,1.0,1.0,1.0 };
	float shininess = 100.0;
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}
void planet(double p, double d, double s) {
	glRotatef(p, 0.0, 1.0, 0.0);
	glTranslatef(d, 0.0, 0.0);
	glutWireSphere(s, 100, 100);
}

void trace(double m) {
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	GLfloat x, y, z;
	for (float t = -180; t < 180; t += 0.1) {
		x = m*sin(t*M_PI / 180)*cos(0 * M_PI / 180);
		y = m*sin(0 * M_PI / 180);
		z = m*cos(t*M_PI / 180)*cos(0 * M_PI / 180);
		glVertex3f(x, y, z);
	}
	glEnd();
	glEnable(GL_TEXTURE_2D);
}
void Display() {
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	initLight();
	glEnable(GL_TEXTURE_2D);
	glLoadIdentity();
	Cam.Refresh();


	//The Sun
	glRotatef(90, 1.0, 1.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, Texture[bindex]);
	gluSphere(sun, 300 / SD, 100, 100); // 배경
	glRotatef(-90, 1.0, 0.0, 0.0);
	glPushMatrix(); // 태양 푸쉬
		glColor3f(1.0, 0.3, 0.3);
		glRotatef(90, 1.0, 0.0, 0.0);
		glRotatef(a*0.01, 0, 0, 1); // 태양 자전
		glBindTexture(GL_TEXTURE_2D, Texture[10]);
		gluSphere(sun, 2.5 / SD, 30, 30); // 지층
		glBindTexture(GL_TEXTURE_2D, Texture[5]);
		gluSphere(sun, 0.025 / SD, 30, 30); // 핵
	glPopMatrix();


	glRotatef(60, 0, 0, 1);
	//trace(0.12 / SD);
	glPushMatrix();
		glRotatef(120, 0,1, 0);	
		glRotatef(c, 0, 1, 0);
		glTranslatef(0.120 / SD, 0, 0);
		glBindTexture(GL_TEXTURE_2D, Texture[5]);
		gluSphere(sun, 0.01 / SD, 20, 20); // 핵
	glPopMatrix();
	glRotatef(60, 0, 0, 1);
	//trace(0.12 / SD);
	glPushMatrix();
		glRotatef(240, 0, 1, 0);
		glRotatef(c, 0, 1, 0);
		glTranslatef(0.120 / SD, 0, 0);
		glBindTexture(GL_TEXTURE_2D, Texture[5]);
		gluSphere(sun, 0.01 / SD, 20, 20); // 핵
	glPopMatrix();
	glRotatef(60, 0, 0, 1);
	//trace(0.12 / SD);
	glPushMatrix();
		glRotatef(360, 0, 1, 0);
		glRotatef(c, 0, 1, 0);
		glTranslatef(0.120 / SD, 0, 0);
		glBindTexture(GL_TEXTURE_2D, Texture[5]);
		gluSphere(sun, 0.01 / SD, 20, 20); // 핵
	glPopMatrix();





		
	glPushMatrix();
		glRotatef(a*0.01, 0, 0, 1); // 태양 자전
		glBindTexture(GL_TEXTURE_2D, Texture[5]);
		gluSphere(sun, 4 / SD, 100, 100); // 태양
	glPopMatrix(); // 태양 팝

				   //Earth
	glPushMatrix(); // 지구 푸쉬
		//glRotatef(10, 1.0, 0.0, 0.0);
		trace(6.7 / SD);

		glColor3f(45.0 / 255.0, 155.0 / 255.0, 236.0 / 255.0);
		glRotatef(E, 0.0, 1.0, 0.0); // 지구 공전
		glPushMatrix();
		glTranslatef(6.7 / SD, 0.0, 0.0); // 지구 공전 궤도
		glRotatef(-E, 0.0, 1.0, 0.0);

		/*glLoadIdentity();
		glTranslatef(0, 0.0, -1.5 / SD); */

		glRotatef(-23.4, 0.0, 0.0, 1.0); // 지구 자전축
		glRotatef(D, 0.0, 1.0, 0.0); // 지구 자전
		glRotatef(-270, 1.0, 0.0, 0.0);
		glBindTexture(GL_TEXTURE_2D, Texture[8]);
		gluSphere(earth, 0.5 / SD, 30, 30); // 지구
		glRotatef(90,1.0, 0.0, 0.0);
											//glBegin(GL_LINES); // 지구 자전속도 그림
											//glColor3f(1.0, 1.0, 1.0);
											//glVertex3f(0.8, 0.0, 0.0);
											//glVertex3f(-0.8, 0.0, 0.0);
											//glEnd();
											//glDisable(GL_TEXTURE_2D);
											//glBegin(GL_LINES); // 지구 자전축 그림
											//glColor3f(1.0, 1.0, 1.0);
											//glVertex3f(0.0, 0.8 / SD, 0.0);
											//glVertex3f(0.0, -0.8 / SD, 0.0);
											//glEnd();
		glEnable(GL_TEXTURE_2D);
		glRotatef(-D, 0.0, 1.0, 0.0); // 지구 자전
		glRotatef(23.4, 0.0, 0.0, 1.0); // 지구 자전축

			glPushMatrix(); // 달 푸쉬
			glColor3f(0.5, 0.5, 0.5);
			glRotatef(M, 0.0, 1.0, 0.0);
			glTranslatef(0.8 / SD, 0.0, 0.0);
			glBindTexture(GL_TEXTURE_2D, Texture[9]);
			gluSphere(moon, 0.2 / SD, 30, 30);
			glPopMatrix(); // 달 팝
		glPopMatrix();
	glPopMatrix(); // 지구 팝

	glDisable(GL_TEXTURE_2D);
	glFlush();
	glutSwapBuffers();
}
void KeyUp(unsigned char k, int x, int y){
	key[k] = false;
}
void KeyDown(unsigned char k, int x, int y){
	if (k == 27) {
		exit(0);
	}
	if (k == 'b') {
		bindex = ++bindex%3;
	}
	if (k == '=') {
		var += 1;
	}
	if (k == '-') {
		var -= 1;
	}
	if (k == '1') {
		
	}

	key[k] = true;
}
void MouseMotion(int x, int y)
{
	// This variable is hack to stop glutWarpPointer from triggering an event callback to mouse(...)
	// This avoids it being called recursively and hanging up the event loop
	static bool just_warped = false;

	if (just_warped) {
		just_warped = false;
		return;
	}

	int dx = x - midWidth;
	int dy = midHeight-y;

	if (dx) {
		Cam.RotateYaw(rotation_speed*dx);
	}

	if (dy) {
		Cam.RotatePitch(rotation_speed*dy);
	}

	glutWarpPointer(midWidth, midHeight);

	just_warped = true;
}
void Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			mouseLeftDown = true;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			mouseRightDown = true;
		}
	}
	else if (state == GLUT_UP) {
		if (button == GLUT_LEFT_BUTTON) {
			mouseLeftDown = false;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			mouseRightDown = false;
		}
	}
}
void Timer(int t) {
	c += 1+var;
	a += 1;
	E += 10.0 / 365;
	M += 10.0 / 27.321;
	D += 10.0 / 27.321;
		if (key['w'] || key['W']) {
			Cam.Move(translation_speed);
		}
		if (key['s'] || key['S']) {
			Cam.Move(-translation_speed);
		}
		if (key['a'] || key['A']) {
			Cam.Strafe(translation_speed);
		}
		if (key['d'] || key['D']) {
			Cam.Strafe(-translation_speed);
		}
		if (key['q'] || key['Q']) {
			Cam.RotateRoll(-translation_speed);
		}
		if (key['E'] || key['e']) {
			Cam.RotateRoll(translation_speed);
		}
		if (mouseLeftDown) {
			Cam.Fly(-translation_speed);
		}
		if (mouseRightDown) {
			Cam.Fly(translation_speed);
		}

	glutTimerFunc(1, Timer, 0);
}
void Reshape(int w, int h) {
	win_aspect_ratio = (double)w / (double)h;
	//glViewport(0, 0, WIDTH, HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, win_aspect_ratio, 0.00001, 10000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutPostRedisplay();
}
void Init() {
	Cam.Init();
	LoadGLTexture();
	srand(time(NULL));
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glShadeModel(GL_SMOOTH);
	glutSetCursor(GLUT_CURSOR_NONE);
}
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutGameModeString("1600×900:32@60");
	glutEnterGameMode();

	glutIgnoreKeyRepeat(1);
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMotion);
	glutKeyboardFunc(KeyDown);
	glutKeyboardUpFunc(KeyUp);
	glutIdleFunc(Display);
	glutTimerFunc(delay, Timer, 0);

	Init();

	glutMainLoop();
	return 0;
}

void DrawCube(void)
{
	glBegin(GL_QUADS);
	// new face
	glTexCoord2f(0.0f, 0.0f);	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);	glVertex3f(-1.0f, 1.0f, 1.0f);
	// new face
	glTexCoord2f(0.0f, 0.0f);	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);	glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);	glVertex3f(1.0f, -1.0f, 1.0f);
	// new face
	glTexCoord2f(0.0f, 0.0f);	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);	glVertex3f(1.0f, -1.0f, -1.0f);
	// new face
	glTexCoord2f(0.0f, 0.0f);	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);	glVertex3f(-1.0f, 1.0f, -1.0f);
	// new face
	glTexCoord2f(0.0f, 0.0f);	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);	glVertex3f(-1.0f, 1.0f, 1.0f);
	// new face
	glTexCoord2f(0.0f, 0.0f);	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);	glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);	glVertex3f(-1.0f, -1.0f, 1.0f);

	glEnd();
}
