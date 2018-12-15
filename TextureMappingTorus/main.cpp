#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>

#define PI 3.14159265
#define EPS 0.001	// epsilon

using namespace std;

static float viewer[3];
static float torus[36][18][3] = {
	{{0,0,1}}
};
static float smallTorus[36][18][3] = { 0, };
static float bigTorus[36][18][3] = { 0, };

static float normalVectorOfPolygons[36][18][3] = { 0, };
static float polygonCenterPoint[36][18][3] = { 0, };
static float normalVectorOfPoints[36][18][3] = { 0, };
static float circleCenterPoint[36][3] = { 0, };

static int TorusTypes = 1;

GLubyte checkTexels[512][512][3];
GLubyte marbleTexels[512][512][3];
GLubyte woodTexels[512][512][3];

/**
	Scale the point
*/
float* Scaling(float* point, float* centerPoint, float size)
{
	float inputMatrix[4] = { 1, };
	float resultMatrix[3] = { 0, };
	float calculateMatrix[4][4] = { 0, };

	memcpy(inputMatrix, point, sizeof(float) * 3);

	calculateMatrix[0][0] = size;
	calculateMatrix[1][1] = size;
	calculateMatrix[2][2] = size;
	calculateMatrix[3][3] = 1;

	calculateMatrix[0][3] = (1 - size)*centerPoint[0];
	calculateMatrix[1][3] = (1 - size)*centerPoint[1];
	calculateMatrix[2][3] = (1 - size)*centerPoint[2];

	for (int y = 0; y < 3; y++)
	{
		float sum = 0;
		for (int x = 0; x < 4; x++)
		{
			sum += calculateMatrix[y][x] * inputMatrix[x];
		}
		resultMatrix[y] = sum;
	}

	return resultMatrix;
}

/**
	Rotate theta degrees on the y axis.
*/
float* MajorRotate(int nCircle, int nPoint, float theta)
{
	float sinTheta = sin(theta*PI / 180.0);
	float cosTheta = cos(theta*PI / 180.0);
	float inputPoint[4];
	float resultPoint[3];
	float calculateMatrix[4][4] = { 0, };

	for (int i = 0; i < 3; i++)
		inputPoint[i] = torus[nCircle][nPoint][i];
	inputPoint[3] = 1;

	// init calculation matrix
	calculateMatrix[0][0] = cosTheta;
	calculateMatrix[0][2] = -sinTheta;
	calculateMatrix[1][1] = 1;
	calculateMatrix[2][0] = sinTheta;
	calculateMatrix[2][2] = cosTheta;
	calculateMatrix[3][3] = 1;

	for (int rol = 0; rol < 3; rol++)
	{
		float sum = 0;
		for (int col = 0; col < 4; col++)
		{
			sum += calculateMatrix[rol][col] * inputPoint[col];
		}
		resultPoint[rol] = sum;
	}

	return resultPoint;
}

/**
	Move Point by -x, -y and rotate Point by theta, move Point by x, y.
*/
float* MinorRotate(int nCircle, int nPoint, float theta, float x, float y)
{
	float sinTheta = sin(theta*PI / 180.0);
	float cosTheta = cos(theta*PI / 180.0);
	float inputPoint[3];
	float resultPoint[2];
	float calculateMatrix[3][3] = { 0, };

	for (int i = 0; i < 2; i++)
		inputPoint[i] = torus[nCircle][nPoint][i];
	inputPoint[2] = 1;

	// init calculation matrix
	calculateMatrix[0][0] = cosTheta;
	calculateMatrix[0][1] = -sinTheta;
	calculateMatrix[0][2] = x * (1 - cosTheta) + y * sinTheta;
	calculateMatrix[1][0] = sinTheta;
	calculateMatrix[1][1] = cosTheta;
	calculateMatrix[1][2] = y * (1 - cosTheta) - x * sinTheta;
	calculateMatrix[2][2] = 1;

	for (int rol = 0; rol < 2; rol++)
	{
		float sum = 0;
		for (int col = 0; col < 3; col++)
		{
			sum += calculateMatrix[rol][col] * inputPoint[col];
		}
		resultPoint[rol] = sum;
	}

	return resultPoint;
}

/**
	Calculate torus's Points.
*/
void InitDrawTorus(float minorRadius, float majorRadius, float height)
{
	glColor3f(0, 0, 0);

	// set start point using minor radius
	float startPoint[3] = { 0,0,0 };
	float* nextPoint;
	startPoint[0] = majorRadius + minorRadius * 2;
	startPoint[1] = height;
	memcpy(torus[0][0], startPoint, sizeof(startPoint));

	for (int nPoint = 0; nPoint < 18; nPoint++)
	{
		for (int nCircle = 0; nCircle < 36; nCircle++)
		{
			// rotate circle's Point by y-axis
			nextPoint = MajorRotate(nCircle, nPoint, 10.0);
			memcpy(torus[(nCircle + 1) % 36][nPoint], nextPoint, sizeof(float) * 3);
		}

		// rotate Point in circle
		nextPoint = MinorRotate(0, nPoint, 20.0, majorRadius + minorRadius, height);
		memcpy(torus[0][(nPoint + 1) % 18], nextPoint, sizeof(float) * 2);
	}

	for (int nCircle = 0; nCircle < 36; nCircle++)
	{
		for (int i = 0; i < 3; i++)
		{
			circleCenterPoint[nCircle][i] = (torus[nCircle][0][i] + torus[nCircle][9][i]) / 2;
		}
	}

	// get big torus and small torus 
	for (int nCircle = 0; nCircle < 36; nCircle++)
	{
		for (int nPoint = 0; nPoint < 18; nPoint++)
		{
			nextPoint = Scaling(torus[nCircle][nPoint], circleCenterPoint[nCircle], 1 + EPS);
			memcpy(bigTorus[nCircle][nPoint], nextPoint, sizeof(float) * 3);
			nextPoint = Scaling(torus[nCircle][nPoint], circleCenterPoint[nCircle], 1 - EPS);
			memcpy(smallTorus[nCircle][nPoint], nextPoint, sizeof(float) * 3);
		}
	}

	// get normal vector of polygons
	for (int nCircle = 0; nCircle < 36; nCircle++)
	{
		for (int nPoint = 0; nPoint < 18; nPoint++)
		{
			for (int i = 0; i < 3; i++)
			{	// get Vertical vector
				for (int y = 0; y < 2; y++)
				{
					for (int x = 0; x < 2; x++)
					{
						polygonCenterPoint[nCircle][nPoint][i] += torus[(nCircle + y) % 36][(nPoint + x) % 18][i];
					}
				}
				polygonCenterPoint[nCircle][nPoint][i] /= 4;
			}

			for (int i = 0; i < 3; i++)
			{	// get normal vector
				normalVectorOfPolygons[nCircle][nPoint][i] = (polygonCenterPoint[nCircle][nPoint][i] - (circleCenterPoint[nCircle][i] + circleCenterPoint[(nCircle + 1) % 36][i]) / 2.0) / minorRadius;
			}
		}
	}

	// get normal vector of points
	for (int nCircle = 0; nCircle < 36; nCircle++)
	{
		for (int nPoint = 0; nPoint < 18; nPoint++)
		{
			float sum = 0;
			for (int i = 0; i < 3; i++)
			{	// get Vertical vector

				for (int y = 0; y < 2; y++)
				{
					for (int x = 0; x < 2; x++)
					{
						normalVectorOfPoints[nCircle][nPoint][i] += normalVectorOfPolygons[(nCircle - y < 0 ? 36 - y : nCircle - y)][(nPoint - x < 0 ? 18 - x : nPoint - x)][i];
					}
				}
				sum += normalVectorOfPoints[nCircle][nPoint][i] * normalVectorOfPoints[nCircle][nPoint][i];
			}
			sum = sqrt(sum);
			for (int i = 0; i < 3; i++)
			{	// get normal vector
				normalVectorOfPoints[nCircle][nPoint][i] = normalVectorOfPoints[nCircle][nPoint][i] / sum;
			}
		}
	}

}

/**
	Draw Coodrination system.
*/
void DrawCoordinationSystem(float length)
{
	glBegin(GL_LINES);
	{
		// x-axis
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(length, 0, 0);
		// y-axis
		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, length, 0);
		// z-axis
		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, length);
	}
	glEnd();
}

/**
	Read texels raw files
*/
void ReadTexels(string path, GLubyte (*texels)[512][3])
{
	FILE* fp = fopen(path.c_str(), "rb");
	if (fp == NULL)
	{
		cout << "No texture map files." << endl;
		return;
	}

	for (int h = 0; h < 512; h++)
	{
		fread(texels[h], sizeof(GLubyte) * 3, 512, fp);
	}

	fclose(fp);
}

/**
	Init texels
*/
void InitTexels()
{
	ReadTexels("wood.raw", woodTexels);
	ReadTexels("check.raw", checkTexels);
	ReadTexels("marble.raw", marbleTexels);

	// gl Texels' settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}


/**
	Draw torus
*/
void DrawTorus()
{
	int nCircle = 36;
	int nPoint = 18;
	glColor3f(0, 0, 0);

	// Set lighting
	glEnable(GL_LIGHTING);

	float ambientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	float diffuseColor[] = { 1.5f, 1.5f, 1.5f, 10.0f };
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
	
	glEnable(GL_LIGHT0);

	// Draw Torus with texel
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	{
		for (int circle = 0; circle < nCircle; circle++)
		{
			for (int point = 0; point < nPoint; point++)
			{
				glBegin(GL_POLYGON);
				{
					for (int x = 0; x < 2; x++)
					{
						for (int y = 0; y < 2; y++)
						{
							glTexCoord2f((double)(circle + 1 - x) / 36.0f, (double)(point + 1 - (x + y) % 2) / 18.f);
							glNormal3fv(normalVectorOfPoints[(circle + 1 - x) % 36][(point + 1 - (x + y) % 2) % 18]);
							glVertex3fv(torus[(circle + 1 - x) % 36][(point + 1 - (x + y) % 2) % 18]);
						}
					}
				}
				glEnd();
			}
		}
	}
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

}


void RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(viewer[0], viewer[1], viewer[2], 0, 0, 0, 0, 1, 0);

	// Draw cooddination system
	DrawCoordinationSystem(20);

	InitTexels();

	switch (TorusTypes)
	{
	case 1:	// Draw torus with marbleTexels
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, marbleTexels);
		DrawTorus();
		break;
	case 2:	// Draw torus with woodTexels
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, woodTexels);
		DrawTorus();
		break;
	case 3:	// Draw torus with checkTexels
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, checkTexels);
		DrawTorus();
		break;
	}

	glutSwapBuffers();
	glFlush();
}

void init(void)
{
	viewer[0] = 15;
	viewer[1] = 15;
	viewer[2] = 15;
}

void SetupRC(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);	// set clear color
	glEnable(GL_DEPTH_TEST);
}

void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// calculate aspect ratio of the window
	gluPerspective(45.f, (GLfloat)w / h, 0.1f, 1000.f);

	//set modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void Keyboard(unsigned char key, int x, int y)
{
	if (key == '1') TorusTypes = 1;
	if (key == '2') TorusTypes = 2;
	if (key == '3') TorusTypes = 3;

	RenderScene();
}

void main(int argc, char* argv[])
{
	init();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Torus");
	InitDrawTorus(1.5, 3.0, 5.0);
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutKeyboardFunc(Keyboard);
	SetupRC();	// Init Function
	glutMainLoop();
}