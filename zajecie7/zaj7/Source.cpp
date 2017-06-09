#include <GL/glut.h>
#include <stdlib.h>
#include "colors.h"
#include "gltools_extracted.h"
#include "VectorMath.cpp"
#include <array>
#include <vector>

const GLint N = 9;

// rozmiary bry³y obcinania
const GLdouble left = -2.0;
const GLdouble right = 2.0;
const GLdouble bottom = -2.0;
const GLdouble top = 2.0;
const GLdouble near_ = 3.0;
const GLdouble far_ = 7.0;
// sta³e do obs³ugi menu podrêcznego
enum
{
	CUTTING_PLANE, // p³aszczyzna przekroju
	POLYGON_OFFSET, // przesuniêcie wartoœci g³êbi
	EXIT // wyjœcie
};
// k¹t obrotu kuli
GLfloat angle = 0.0;
// k¹ty obrotu szeœcianu
GLfloat rotatex = 0.0;
GLfloat rotatey = 0.0;
// wskaŸnik rysowania p³aszczyzna przekroju
bool cutting_plane = true;
// wskaŸnik przesuniêcia wartoœci g³êbi
bool polygon_offset = true;
// wskaŸnik naciœniêcia lewego przycisku myszki
int button_state = GLUT_UP;
// po³o¿enie kursora myszki
int button_x, button_y;


void drawPyramid(GLuint n, GLfloat size, GLenum type, bool drawBase)
{
	GLTVector3 topVertice{ 0.0f, +size, 0.0f };
	GLTVector3 baseCenter{ 0.0f, -size / 2.0, 0.0f };

	std::vector<GLTVector3> bottomVertices{ n };

	for (int i = 0; i < n; ++i)
	{
		const GLfloat angle = (GLfloat)i / n * 3.1415926 * 2.0;

		bottomVertices[i][0] = size * sin(angle);
		bottomVertices[i][1] = -size / 2;
		bottomVertices[i][2] = size * cos(angle);
	}

	if (drawBase)
	{
		for (int i = 0; i <= n; ++i)
		{
			glBegin(type);
			glNormal3f(0.0f, -1.0f, 0.0f);
			glVertex3fv(baseCenter);
			glVertex3fv(bottomVertices[(i + 1) % n]);
			glVertex3fv(bottomVertices[i % n]);
			glEnd();
		}
	}
	for (int i = 0; i <= n; ++i)
	{
		glBegin(type);
		GLTVector3 normal;
		gltGetNormalVector(bottomVertices[i % n], bottomVertices[(i + 1) % n], topVertice, normal);
		glNormal3fv(normal);

		glVertex3fv(bottomVertices[i % n]);
		glVertex3fv(bottomVertices[(i + 1) % n]);
		glVertex3fv(topVertice);

		glEnd();
	}
}

void drawSolidPyramid(GLint n, GLdouble size)
{
	drawPyramid(n, size, GL_TRIANGLES, true);
}

void drawWirePyramid(GLint n, GLdouble size)
{
	drawPyramid(n, size, GL_LINE_LOOP, false);
}

// funkcja generuj¹ca scenê 3D
void Display()
{
	// kolor t³a - zawartoœæ bufora koloru
	glClearColor(1.0, 1.0, 1.0, 1.0);
	// czyszczenie bufora koloru i bufora g³êbokoœci
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// wybór macierzy modelowania
	glMatrixMode(GL_MODELVIEW);
	// macierz modelowania = macierz jednostkowa
	glLoadIdentity();
	// przesuniêcie uk³adu wspó³rzêdnych szeœcianu do œrodka bry³y odcinania
	glTranslatef(0, 0, -(near_ + far_) / 2);
	// obroty szeœcianu
	glRotatef(rotatex, 1.0, 0, 0);
	glRotatef(rotatey, 0, 1.0, 0);
	// niewielkie powiêkszenie szeœcianu
	//glScalef(1.15, 1.15, 1.15);
	// w³¹czenie testu bufora g³êbokoœci
	glEnable(GL_DEPTH_TEST);
	// najpierw rysujemy kulê obracaj¹c¹ siê wewn¹trz szeœcianu;
	// z uwagi na celowy brak efektów oœwietlenia, obrót kuli
	// podkreœla druga kula w wersji "szkieletowej"
	glPushMatrix();
	angle += 0.2;
	glRotatef(angle, 1.0, 1.0, 0.0);
	glColor3fv(Yellow);
	if (polygon_offset)
		glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);
	glutSolidSphere(0.5, 10, 10);
	glColor3fv(Black);
	glutWireSphere(0.5, 10, 10);
	if (polygon_offset)
		glDisable(GL_POLYGON_OFFSET_FILL);
	glPopMatrix();


	const GLfloat a = 0.5;

	// w drugiej kolejnoœci rysujemy wnêtrze szeœcianu;
	// rysowane s¹ tylko przednie strony wewnêtrznych œcian
	// no i nie jest rysowana œciana przednia, w której bêdzie otwór
	glEnable(GL_CULL_FACE);
	glBegin(GL_QUADS);
	glColor3fv(Blue);
	glVertex3f(-a, -2 * a, -a);
	glVertex3f(a, -2 * a, -a);
	glVertex3f(a, a, -a);
	glVertex3f(-a, a, -a);
	glColor3fv(Magenta);
	glVertex3f(-a, a, a);
	glVertex3f(a, a, a);
	glVertex3f(a, -2 * a, a);
	glVertex3f(-a, -2 * a, a);
	glColor3fv(Lime);
	glVertex3f(-a, a, a);
	glVertex3f(-a, -2 * a, a);
	glVertex3f(-a, -2 * a, -a);
	glVertex3f(-a, a, -a);
	glColor3fv(Cyan);
	glVertex3f(a, a, a);
	glVertex3f(-a, a, a);
	glVertex3f(-a, a, -a);
	glVertex3f(a, a, -a);
	glColor3fv(Green);
	glVertex3f(a, -2 * a, a);
	glVertex3f(a, a, a);
	glVertex3f(a, a, -a);
	glVertex3f(a, -2 * a, -a);
	glColor3fv(Cyan);
	glVertex3f(-a, -2 * a, a);
	glVertex3f(a, -2 * a, a);
	glVertex3f(a, -2 * a, -a);
	glVertex3f(-a, -2 * a, -a);
	glEnd();
	glDisable(GL_CULL_FACE);
	// rysowanie p³aszczyzny otworu w szeœcianie
	if (cutting_plane)
	{
		// wy³¹czenie rysowania w buforze kolorów
		glDrawBuffer(GL_NONE);
		// rysowanie kwadratu czêœciowo ods³aniaj¹cego wnêtrze szeœcianu
		// (kwadrat jest po³o¿ony o 0,001 jednostki nad bokiem szeœcianu)
		glBegin(GL_QUADS);
		glVertex3f(-0.5, -1.001, -0.5);
		glVertex3f(0.5, -1.001, -0.5);
		glVertex3f(0.5, -1.001, 0.5);
		glVertex3f(-0.5, -1.001, 0.5);
		glEnd();
		// w³¹czenie rysowania w buforze kolorów
		glDrawBuffer(GL_BACK);
	}
	// w³aœciwy szeœcian z obramowaniem, którego rysowanie wymusza brak oœwietlenia
	glColor3fv(Red);
	if (polygon_offset)
		glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(1.0, 1.0);
	drawSolidPyramid(N, 2.0);
	glColor3fv(Black);
	drawWirePyramid(N, 2.0);
	if (polygon_offset)
		glDisable(GL_POLYGON_OFFSET_FILL);
	// skierowanie poleceñ do wykonania
	glFlush();
	// zamiana buforów koloru
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
	glFrustum(left, right, bottom, top, near_, far_);
	// generowanie sceny 3D
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
		// zapamiêtanie po³o¿enia kursora myszki
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
		rotatey += 30 * (right - left) / glutGet(GLUT_WINDOW_WIDTH) *(x - button_x);
		button_x = x;
		rotatex -= 30 * (top - bottom) / glutGet(GLUT_WINDOW_HEIGHT) *(button_y - y);
		button_y = y;
		glutPostRedisplay();
	}
}
// obs³uga menu podrêcznego
void Menu(int value)
{
	switch (value)
	{
		// p³aszczyzna przekroju
	case CUTTING_PLANE:
		cutting_plane = !cutting_plane;
		Display();
		break;
		// przesuniêcie wartoœci g³êbi
	case POLYGON_OFFSET:
		polygon_offset = !polygon_offset;
		Display();
		break;
		// wyjœcie
	case EXIT:
		exit(0);
	}
}
int main(int argc, char * argv[])
{
	// inicjalizacja biblioteki GLUT
	glutInit(&argc, argv);
	// inicjalizacja bufora ramki
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	// rozmiary g³ównego okna programu
	glutInitWindowSize(500, 500);
	// utworzenie g³ównego okna programu
	glutCreateWindow("Z-bufor");
	// do³¹czenie funkcji generuj¹cej scenê 3D
	glutDisplayFunc(Display);
	// do³¹czenie funkcji wywo³ywanej przy zmianie rozmiaru okna
	glutReshapeFunc(Reshape);
	// do³¹czenie funkcji obs³ugi klawiszy funkcyjnych i klawiszy kursora
	glutSpecialFunc(SpecialKeys);
	// obs³uga przycisków myszki
	glutMouseFunc(MouseButton);
	// obs³uga ruchu kursora myszki
	glutMotionFunc(MouseMotion);
	// utworzenie menu podrêcznego
	glutCreateMenu(Menu);
	// menu g³ówne
	glutCreateMenu(Menu);
#ifdef WIN32
	glutAddMenuEntry("P³aszczyzna przekroju: rysowana/nierysowana", CUTTING_PLANE);
	glutAddMenuEntry("Przesuniêcie wartoœci g³êbi: w³¹cz/wy³¹cz", POLYGON_OFFSET);
	glutAddMenuEntry("Wyjœcie", EXIT);
#else
	glutAddMenuEntry("Plaszczyzna przekroju: rysowana/nierysowana", CUTTING_PLANE);
	glutAddMenuEntry("Przesuniecie wartosci glebi: wlacz/wylacz", POLYGON_OFFSET);
	glutAddMenuEntry("Wyjscie", EXIT);
#endif
	// okreœlenie przycisku myszki obs³uguj¹cej menu podrêczne
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	// do³¹czenie funkcji wywo³ywanej podczas "bezczynnoœci" systemu
	glutIdleFunc(Display);
	// wprowadzenie programu do obs³ugi pêtli komunikatów
	glutMainLoop();
	return 0;
}