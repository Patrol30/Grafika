
#include <GL/glut.h>
#include "GL/glext.h"
#include <stdlib.h>
#include <stdio.h>
#include "materials.h"
#include <array>
#include "colors.h"

#define wglGetProcAddress glXGetProcAddress

#include "gltools_extracted.h"
#include <math.h>

// Adds two vectors together
void gltAddVectors(const GLTVector3 vFirst, const GLTVector3 vSecond, GLTVector3 vResult) {
	vResult[0] = vFirst[0] + vSecond[0];
	vResult[1] = vFirst[1] + vSecond[1];
	vResult[2] = vFirst[2] + vSecond[2];
}

// Subtract one vector from another
void gltSubtractVectors(const GLTVector3 vFirst, const GLTVector3 vSecond, GLTVector3 vResult)
{
	vResult[0] = vFirst[0] - vSecond[0];
	vResult[1] = vFirst[1] - vSecond[1];
	vResult[2] = vFirst[2] - vSecond[2];
}

// Scales a vector by a scalar
void gltScaleVector(GLTVector3 vVector, const GLfloat fScale)
{
	vVector[0] *= fScale; vVector[1] *= fScale; vVector[2] *= fScale;
}

// Gets the length of a vector squared
GLfloat gltGetVectorLengthSqrd(const GLTVector3 vVector)
{
	return (vVector[0] * vVector[0]) + (vVector[1] * vVector[1]) + (vVector[2] * vVector[2]);
}

// Gets the length of a vector
GLfloat gltGetVectorLength(const GLTVector3 vVector)
{
	return (GLfloat)sqrt(gltGetVectorLengthSqrd(vVector));
}

// Scales a vector by it's length - creates a unit vector
void gltNormalizeVector(GLTVector3 vNormal)
{
	GLfloat fLength = 1.0f / gltGetVectorLength(vNormal);
	gltScaleVector(vNormal, fLength);
}

// Copies a vector
void gltCopyVector(const GLTVector3 vSource, GLTVector3 vDest)
{
	memcpy(vDest, vSource, sizeof(GLTVector3));
}

// Get the dot product between two vectors
GLfloat gltVectorDotProduct(const GLTVector3 vU, const GLTVector3 vV)
{
	return vU[0] * vV[0] + vU[1] * vV[1] + vU[2] * vV[2];
}

// Calculate the cross product of two vectors
void gltVectorCrossProduct(const GLTVector3 vU, const GLTVector3 vV, GLTVector3 vResult)
{
	vResult[0] = vU[1] * vV[2] - vV[1] * vU[2];
	vResult[1] = -vU[0] * vV[2] + vV[0] * vU[2];
	vResult[2] = vU[0] * vV[1] - vV[0] * vU[1];
}



// Given three points on a plane in counter clockwise order, calculate the unit normal
void gltGetNormalVector(const GLTVector3 vP1, const GLTVector3 vP2, const GLTVector3 vP3, GLTVector3 vNormal)
{
	GLTVector3 vV1, vV2;

	gltSubtractVectors(vP2, vP1, vV1);
	gltSubtractVectors(vP3, vP1, vV2);

	gltVectorCrossProduct(vV1, vV2, vNormal);
	gltNormalizeVector(vNormal);
}



// Transform a point by a 4x4 matrix
void gltTransformPoint(const GLTVector3 vSrcVector, const GLTMatrix mMatrix, GLTVector3 vOut)
{
	vOut[0] = mMatrix[0] * vSrcVector[0] + mMatrix[4] * vSrcVector[1] + mMatrix[8] * vSrcVector[2] + mMatrix[12];
	vOut[1] = mMatrix[1] * vSrcVector[0] + mMatrix[5] * vSrcVector[1] + mMatrix[9] * vSrcVector[2] + mMatrix[13];
	vOut[2] = mMatrix[2] * vSrcVector[0] + mMatrix[6] * vSrcVector[1] + mMatrix[10] * vSrcVector[2] + mMatrix[14];
}

// Rotates a vector using a 4x4 matrix. Translation column is ignored
void gltRotateVector(const GLTVector3 vSrcVector, const GLTMatrix mMatrix, GLTVector3 vOut)
{
	vOut[0] = mMatrix[0] * vSrcVector[0] + mMatrix[4] * vSrcVector[1] + mMatrix[8] * vSrcVector[2];
	vOut[1] = mMatrix[1] * vSrcVector[0] + mMatrix[5] * vSrcVector[1] + mMatrix[9] * vSrcVector[2];
	vOut[2] = mMatrix[2] * vSrcVector[0] + mMatrix[6] * vSrcVector[1] + mMatrix[10] * vSrcVector[2];
}


// Gets the three coefficients of a plane equation given three points on the plane.
void gltGetPlaneEquation(GLTVector3 vPoint1, GLTVector3 vPoint2, GLTVector3 vPoint3, GLTVector3 vPlane)
{
	// Get normal vector from three points. The normal vector is the first three coefficients
	// to the plane equation...
	gltGetNormalVector(vPoint1, vPoint2, vPoint3, vPlane);

	// Final coefficient found by back substitution
	vPlane[3] = -(vPlane[0] * vPoint3[0] + vPlane[1] * vPoint3[1] + vPlane[2] * vPoint3[2]);
}

// Determine the distance of a point from a plane, given the point and the
// equation of the plane.
GLfloat gltDistanceToPlane(GLTVector3 vPoint, GLTVector4 vPlane)
{
	return vPoint[0] * vPlane[0] + vPoint[1] * vPlane[1] + vPoint[2] * vPlane[2] + vPlane[3];
}

PFNGLWINDOWPOS2IPROC glWindowPos2i = nullptr;
bool rescale_normal = false;

enum
{
	
	POLISHED_SILVER,      // polerowane srebro
	NORMALS_SMOOTH,       // jeden wektor normalny na wierzcho³ek
	NORMALS_FLAT,         // jeden wektor normalny na œcianê
	FULL_WINDOW,          // aspekt obrazu - ca³e okno
	ASPECT_1_1,           // aspekt obrazu 1:1
	EXIT,                 // wyjœcie

	LIGHT_DIRECTIONAL,
	LIGHT_AMBIENT,
	LIGHT_SPOT
};

int aspect = FULL_WINDOW;

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif


const int N = 9;

const GLdouble left = -1.0;
const GLdouble right = 1.0;
const GLdouble bottom = -1.0;
const GLdouble top = 1.0;
const GLdouble near = 3.0;
const GLdouble far = 7.0;

GLfloat rotatex = 0.0;
GLfloat rotatey = 0.0;

int button_state = GLUT_UP;
int button_x, button_y;

GLfloat scale = 1.0;

const GLfloat *ambient = PolishedSilverAmbient;
const GLfloat *diffuse = PolishedSilverDiffuse;
const GLfloat *specular = PolishedSilverSpecular;
GLfloat shininess = PolishedSilverShininess;





int normals = NORMALS_FLAT;
int light = LIGHT_AMBIENT;
GLfloat light_color[4] = { 0.2, 0.2, 0.2, 1.0 };
GLfloat light_position[4] =
{
	0.0,0.0,2.0,1.0
};

// k¹ty obrotu po³o¿enia Ÿród³a œwiat³a

GLfloat light_rotatex = 0.0;
GLfloat light_rotatey = 0.0;

GLfloat spot_direction[3] = { 0.0,0.0,-1.0 };

GLfloat spot_cutoff = 180.0;
GLfloat spot_exponent = 128.0;
GLfloat constant_attenuation = 1.0;
GLfloat linear_attenuation = 0.0;
GLfloat quadratic_attenuation = 0.0;

std::array<GLfloat, 3 * (N + 2)> vertices;
std::array<int, (N + 1) * 2 * 3> triangles;

void Normalize(GLfloat *v)
{
	GLfloat d = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (d)
	{
		v[0] /= d;
		v[1] /= d;
		v[2] /= d;
	}
}

void Normal(GLfloat *n, int i)
{
	GLfloat v1[3], v2[3];

	// obliczenie wektorów na podstawie wspó³rzêdnych wierzcho³ków trójk¹tów
	v1[0] = vertices[3 * triangles[3 * i + 1] + 0] - vertices[3 * triangles[3 * i + 0] + 0];
	v1[1] = vertices[3 * triangles[3 * i + 1] + 1] - vertices[3 * triangles[3 * i + 0] + 1];
	v1[2] = vertices[3 * triangles[3 * i + 1] + 2] - vertices[3 * triangles[3 * i + 0] + 2];
	v2[0] = vertices[3 * triangles[3 * i + 2] + 0] - vertices[3 * triangles[3 * i + 1] + 0];
	v2[1] = vertices[3 * triangles[3 * i + 2] + 1] - vertices[3 * triangles[3 * i + 1] + 1];
	v2[2] = vertices[3 * triangles[3 * i + 2] + 2] - vertices[3 * triangles[3 * i + 1] + 2];

	// obliczenie waktora normalnego przy pomocy iloczynu wektorowego
	n[0] = v1[1] * v2[2] - v1[2] * v2[1];
	n[1] = v1[2] * v2[0] - v1[0] * v2[2];
	n[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void DrawString(GLint x, GLint y, char *string)
{
	// po³o¿enie napisu
	glWindowPos2i(x, y);

	// wyœwietlenie napisu
	int len = strlen(string);
	for (int i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, string[i]);
}

void Display()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -(near + far) / 2);

	glRotatef(rotatex, 1.0, 0, 0);
	glRotatef(rotatey, 0, 1.0, 0);

	glScalef(scale, scale, scale);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	switch (light)
	{
	case LIGHT_AMBIENT:
		glDisable(GL_LIGHT0);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_color);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT);
		break;

	case LIGHT_SPOT:
		glEnable(GL_LIGHT0);
		glDisable(GL_COLOR_MATERIAL);
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, spot_cutoff);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, spot_exponent);
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, constant_attenuation);
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, linear_attenuation);
		glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, quadratic_attenuation);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(0, 0, -(near + far) / 2);
		glRotatef(light_rotatex, 1.0, 0, 0);
		glRotatef(light_rotatey, 0, 1.0, 0);
		glTranslatef(light_position[0], light_position[1], light_position[2]);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
		glPushAttrib(GL_LIGHTING_BIT);
		glDisable(GL_LIGHT0);
		glMaterialfv(GL_FRONT, GL_EMISSION, Red);
		glutSolidSphere(0.1, 30, 20);
		glPopAttrib();
		glPopMatrix();
		break;

	case LIGHT_DIRECTIONAL:
		glEnable(GL_LIGHT0);
		glPushMatrix();
		glLoadIdentity();
		glRotatef(light_rotatex, 1.0, 0, 0);
		glRotatef(light_rotatey, 0, 1.0, 0);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		glPopMatrix();
		break;
	}


	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);


	glEnable(rescale_normal ? GL_RESCALE_NORMAL : GL_NORMALIZE);

	glBegin(GL_TRIANGLES);

	if (normals == NORMALS_SMOOTH)
	{
		for (int i = 0; i < triangles.size() / 3; i++)
		{
			// obliczanie wektora normalnego dla pierwszego wierzcho³ka
			GLfloat n[3];
			n[0] = n[1] = n[2] = 0.0;

			// wyszukanie wszystkich œcian posiadaj¹cych bie?¹cy wierzcho³ek
			for (int j = 0; j < triangles.size() / 3; j++)
				if (3 * triangles[3 * i + 0] == 3 * triangles[3 * j + 0] ||
					3 * triangles[3 * i + 0] == 3 * triangles[3 * j + 1] ||
					3 * triangles[3 * i + 0] == 3 * triangles[3 * j + 2])
				{
					// dodawanie wektorów normalnych poszczególnych œcian
					GLfloat nv[3];
					Normal(nv, j);
					n[0] += nv[0];
					n[1] += nv[1];
					n[2] += nv[2];
				}

			// uœredniony wektor normalny jest normalizowany tylko, gdy biblioteka
			// obs³uguje automatyczne skalowania jednostkowych wektorów normalnych
			if (rescale_normal == true)
				Normalize(n);
			glNormal3fv(n);
			glVertex3fv(&vertices[3 * triangles[3 * i + 0]]);

			// obliczanie wektora normalnego dla drugiego wierzcho³ka
			n[0] = n[1] = n[2] = 0.0;

			// wyszukanie wszystkich œcian posiadaj¹cych bie?¹cy wierzcho³ek
			for (int j = 0; j < triangles.size() / 3; j++)
				if (3 * triangles[3 * i + 1] == 3 * triangles[3 * j + 0] ||
					3 * triangles[3 * i + 1] == 3 * triangles[3 * j + 1] ||
					3 * triangles[3 * i + 1] == 3 * triangles[3 * j + 2])
				{
					// dodawanie wektorów normalnych poszczególnych œcian
					GLfloat nv[3];
					Normal(nv, j);
					n[0] += nv[0];
					n[1] += nv[1];
					n[2] += nv[2];
				}

			// uœredniony wektor normalny jest normalizowany tylko, gdy biblioteka
			// obs³uguje automatyczne skalowania jednostkowych wektorów normalnych
			if (rescale_normal == true)
				Normalize(n);
			glNormal3fv(n);
			glVertex3fv(&vertices[3 * triangles[3 * i + 1]]);

			// obliczanie wektora normalnego dla trzeciego wierzcho³ka
			n[0] = n[1] = n[2] = 0.0;

			// wyszukanie wszystkich œcian posiadaj¹cych bie?¹cy wierzcho³ek
			for (int j = 0; j < triangles.size() / 3; j++)
				if (3 * triangles[3 * i + 2] == 3 * triangles[3 * j + 0] ||
					3 * triangles[3 * i + 2] == 3 * triangles[3 * j + 1] ||
					3 * triangles[3 * i + 2] == 3 * triangles[3 * j + 2])
				{
					// dodawanie wektorów normalnych poszczególnych œcian
					GLfloat nv[3];
					Normal(nv, j);
					n[0] += nv[0];
					n[1] += nv[1];
					n[2] += nv[2];
				}

			// uœredniony wektor normalny jest normalizowany tylko, gdy biblioteka
			// obs³uguje automatyczne skalowania jednostkowych wektorów normalnych
			if (rescale_normal == true)
				Normalize(n);
			glNormal3fv(n);
			glVertex3fv(&vertices[3 * triangles[3 * i + 2]]);
		}
	}
	else
	{
		// generowanie obiektu "p³askiego" - jeden wektor normalny na œcianê
		for (int i = 0; i < triangles.size() / 3; i++)
		{
			GLfloat* a = &vertices[3 * triangles[3 * i + 0]];
			GLfloat* b = &vertices[3 * triangles[3 * i + 1]];
			GLfloat* c = &vertices[3 * triangles[3 * i + 2]];

			GLTVector3 n;
			gltGetNormalVector(a, b, c, n);

			if (rescale_normal == true)
				Normalize(n);

			glNormal3fv(n);
			glVertex3fv(a);
			glVertex3fv(b);
			glVertex3fv(c);
		}
	}

	glEnd();
	glFlush();
	glutSwapBuffers();
}

// zmiana wielkoœci okna

void Reshape(int width, int height)
{
	// obszar renderingu - ca³e okno
	glViewport(0, 0, width, height);

	// wybór macierzy rzutowania
	glMatrixMode(GL_PROJECTION);

	// macierz rzutowania = macierz jednostkowa
	glLoadIdentity();

	// parametry bry³y obcinania
	if (aspect == ASPECT_1_1)
	{
		// wysokoœæ okna wiêksza od wysokoœci okna
		if (width < height && width > 0)
			glFrustum(left, right, bottom*height / width, top*height / width, near, far);
		else

			// szerokoœæ okna wiêksza lub równa wysokoœci okna
			if (width >= height && height > 0)
				glFrustum(left*width / height, right*width / height, bottom, top, near, far);
	}
	else
		glFrustum(left, right, bottom, top, near, far);

	// generowanie sceny 3D
	Display();
}

template<typename T>
T clamp(T min, T max, T value)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'R':
		light_color[0] = clamp(0.0f, 1.0f, light_color[0] + 0.05f);
		break;

	case 'r':
		light_color[0] = clamp(0.0f, 1.0f, light_color[0] - 0.05f);
		break;

	case 'G':
		light_color[1] = clamp(0.0f, 1.0f, light_color[1] + 0.05f);
		break;

	case 'g':
		light_color[1] = clamp(0.0f, 1.0f, light_color[1] - 0.05f);
		break;

	case 'B':
		light_color[2] = clamp(0.0f, 1.0f, light_color[2] + 0.05f);
		break;

	case 'b':
		light_color[2] = clamp(0.0f, 1.0f, light_color[2] - 0.05f);
		break;

	case '+':
		scale += 0.05;
		break;

	case '-':
		if (scale > 0.05)
			scale -= 0.05;
		break;

	case 'S':
		if (spot_cutoff == 90)
			spot_cutoff = 180;
		else if (spot_cutoff < 90)
			spot_cutoff++;
		break;

	case 's':
		if (spot_cutoff == 180)
			spot_cutoff = 90;
		else if (spot_cutoff > 0)
			spot_cutoff--;
		break;

	case 'E':
		if (spot_exponent < 128)
			spot_exponent++;
		break;

	case 'e':
		if (spot_exponent > 0)
			spot_exponent--;
		break;

	case 'C':
		constant_attenuation += 0.1;
		break;

	case 'c':
		if (constant_attenuation > 0)
			constant_attenuation -= 0.1;
		break;

	case 'L':
		linear_attenuation += 0.1;
		break;

	case 'l':
		if (linear_attenuation > 0)
			linear_attenuation -= 0.1;
		break;

	case 'Q':
		quadratic_attenuation += 0.1;
		break;

	case 'q':
		if (quadratic_attenuation > 0)
			quadratic_attenuation -= 0.1;
		break;
	}

	// narysowanie sceny
	Display();
}

// obs³uga klawiszy funkcyjnych i klawiszy kursora

void SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
		// kursor w lewo
	case GLUT_KEY_LEFT:
		rotatey -= 1;
		break;

		// kursor w górê
	case GLUT_KEY_UP:
		rotatex -= 1;
		break;

		// kursor w prawo
	case GLUT_KEY_RIGHT:
		rotatey += 1;
		break;

		// kursor w dó³
	case GLUT_KEY_DOWN:
		rotatex += 1;
		break;
	}

	// odrysowanie okna
	Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

// obs³uga przycisków myszki

void MouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		// zapamiêtanie stanu lewego przycisku myszki
		button_state = state;

		// zapamiêtanie po³o?enia kursora myszki
		if (state == GLUT_DOWN)
		{
			button_x = x;
			button_y = y;
		}
	}
}

// obs³uga ruchu kursora myszki

void MouseMotion(int x, int y)
{
	if (button_state == GLUT_DOWN)
	{
		rotatey += 30 * (right - left) / glutGet(GLUT_WINDOW_WIDTH) * (x - button_x);
		button_x = x;
		rotatex -= 30 * (top - bottom) / glutGet(GLUT_WINDOW_HEIGHT) * (button_y - y);
		button_y = y;
		glutPostRedisplay();
	}
}

// obs³uga menu podrêcznego

void Menu(int value)
{
	switch (value)
	{
		

		// materia³ - polerowane srebro
	case POLISHED_SILVER:
		ambient = PolishedSilverAmbient;
		diffuse = PolishedSilverDiffuse;
		specular = PolishedSilverSpecular;
		shininess = PolishedSilverShininess;
		Display();
		break;


		// wektory normalne - GLU_SMOOTH
	case NORMALS_SMOOTH:
		normals = NORMALS_SMOOTH;
		Display();
		break;

		// wektory normalne - GLU_FLAT
	case NORMALS_FLAT:
		normals = NORMALS_FLAT;
		Display();
		break;

		// obszar renderingu - ca³e okno
	case FULL_WINDOW:
		aspect = FULL_WINDOW;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;

		// obszar renderingu - aspekt 1:1
	case ASPECT_1_1:
		aspect = ASPECT_1_1;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;

	case LIGHT_AMBIENT:
	case LIGHT_DIRECTIONAL:
	case LIGHT_SPOT:
		light = value;
		Display();
		break;
		// wyjœcie
	case EXIT:
		exit(0);
	}
}

// sprawdzenie i przygotowanie obs³ugi wybranych rozszerze?

void ExtensionSetup()
{
	// pobranie numeru wersji biblioteki OpenGL
	const char *version = (char*)glGetString(GL_VERSION);

	// odczyt wersji OpenGL
	int major = 0, minor = 0;
	if (sscanf(version, "%d.%d", &major, &minor) != 2)
	{
		printf("Bledny format wersji OpenGL\n");

		exit(0);
	}

	// sprawdzenie czy jest co najmniej wersja 1.2
	if (major > 1 || minor >= 2)
		rescale_normal = true;
	else if (glutExtensionSupported("GL_EXT_rescale_normal"))
		rescale_normal = true;

	// sprawdzenie czy jest co najmniej wersja 1.4
	if (major > 1 || minor >= 4)
	{
		// pobranie wskaŸnika wybranej funkcji OpenGL 1.4
		//glWindowPos2i = (PFNGLWINDOWPOS2IPROC)wglGetProcAddress("glWindowPos2i");
	}
	else if (glutExtensionSupported("GL_ARB_window_pos"))
	{
		// pobranie wskaŸnika wybranej funkcji rozszerzenia ARB_window_pos
		//glWindowPos2i = (PFNGLWINDOWPOS2IPROC)wglGetProcAddress("glWindowPos2iARB");
	}
	else
	{
		printf("Brak rozszerzenia ARB_window_pos!\n");
		exit(0);
	}
}

void BuildPyramid()
{
	vertices[3 * N + 0] = 0.0f;
	vertices[3 * N + 1] = 0.0f;
	vertices[3 * N + 2] = 0.0f;
	vertices[3 * (N + 1) + 0] = 0.0f;
	vertices[3 * (N + 1) + 1] = 0.0f;
	vertices[3 * (N + 1) + 2] = 1.0f;

	// podstawa
	for (int i = 0; i < N; ++i)
	{
		const GLfloat x = 0.5f * std::sin(2.0f * 3.1415f * ((GLfloat)i / N));
		const GLfloat y = 0.5f * std::cos(2.0f * 3.1415f * ((GLfloat)i / N));

		vertices[3 * i + 0] = x;
		vertices[3 * i + 1] = y;
		vertices[3 * i + 2] = 0.0f;
	}

	// górne œciany boczne
	for (int i = 0; i <= N; ++i)
	{
		triangles[3 * i + 0] = N;
		triangles[3 * i + 1] = i % N;
		triangles[3 * i + 2] = (i + 1) % N;

		triangles[3 * (i + N + 1) + 0] = (i + 1) % N;
		triangles[3 * (i + N + 1) + 1] = i % N;
		triangles[3 * (i + N + 1) + 2] = N + 1;
	}
}

int main(int argc, char *argv[])
{
	BuildPyramid();

	// inicjalizacja biblioteki GLUT
	glutInit(&argc, argv);

	// inicjalizacja bufora ramki
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// rozmiary g³ównego okna programu
	glutInitWindowSize(500, 500);

	// utworzenie g³ównego okna programu
	glutCreateWindow("Wektory normalne");

	// do³¹czenie funkcji generuj¹cej scenê 3D
	glutDisplayFunc(Display);

	// do³¹czenie funkcji wywo³ywanej przy zmianie rozmiaru okna
	glutReshapeFunc(Reshape);

	// do³¹czenie funkcji obs³ugi klawiatury
	glutKeyboardFunc(Keyboard);

	// do³¹czenie funkcji obs³ugi klawiszy funkcyjnych i klawiszy kursora
	glutSpecialFunc(SpecialKeys);

	// obs³uga przycisków myszki
	glutMouseFunc(MouseButton);

	// obs³uga ruchu kursora myszki
	glutMotionFunc(MouseMotion);

	// utworzenie menu podrêcznego
	glutCreateMenu(Menu);

	// utworzenie podmenu - Materia³
	int MenuMaterial = glutCreateMenu(Menu);

	
	glutAddMenuEntry("Polerowane srebro", POLISHED_SILVER);
	

	int MenuNormals = glutCreateMenu(Menu);
	glutAddMenuEntry("Jeden wektor normalny na wierzcholek", NORMALS_SMOOTH);
	glutAddMenuEntry("Jeden wektor normalny na sciane", NORMALS_FLAT);

	int MenuAspect = glutCreateMenu(Menu);
	glutAddMenuEntry("Aspekt obrazu - cale okno", FULL_WINDOW);
	glutAddMenuEntry("Aspekt obrazu 1:1", ASPECT_1_1);

	int MenuLight = glutCreateMenu(Menu);
	glutAddMenuEntry("Kierunkowe", LIGHT_DIRECTIONAL);
	glutAddMenuEntry("Otaczajace", LIGHT_AMBIENT);
	glutAddMenuEntry("Reflektor", LIGHT_SPOT);

	glutCreateMenu(Menu);

	glutAddSubMenu("Material", MenuMaterial);
	glutAddSubMenu("Swiatlo", MenuLight);
	glutAddSubMenu("Wektory normalne", MenuNormals);
	glutAddSubMenu("Aspekt obrazu", MenuAspect);
	glutAddMenuEntry("Wyjscie", EXIT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	ExtensionSetup();

	glutMainLoop();
	return 0;
}

