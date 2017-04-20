
#include <GL/glut.h>
#include "glext.h"
#ifndef WIN32
#define GLX_GLXEXT_LEGACY
#include <GL/glx.h>
#define wglGetProcAddress glXGetProcAddressARB
#endif
#include <stdlib.h>
#include <stdio.h>
#include "colors.h"
#include "targa.h"

// wska?nik na funkcję glWindowPos2i

PFNGLWINDOWPOS2IPROC glWindowPos2i = NULL;

// stałe do obsługi menu podręcznego

enum
{
	PERSPECTIVE_CORRECTION_FASTEST,     // korekcja perspektywy - GL_FASTEST
	PERSPECTIVE_CORRECTION_DONT_CARE,   // korekcja perspektywy - GL_DONT_CARE
	PERSPECTIVE_CORRECTION_NICEST,      // korekcja perspektywy - GL_NICEST
	GENERATE_MIPMAP_FASTEST,            // jakość generowania mipmap - GL_FASTEST
	GENERATE_MIPMAP_DONT_CARE,          // jakość generowania mipmap - GL_DONT_CARE
	GENERATE_MIPMAP_NICEST,             // jakość generowania mipmap - GL_NICEST
	FULL_WINDOW,                        // aspekt obrazu - całe okno
	ASPECT_1_1,                         // aspekt obrazu 1:1
	EXIT                                // wyjście
};

// aspekt obrazu

int aspect = FULL_WINDOW;

// usunięcie definicji makr near i far

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

// rozmiary bryły obcinania

const GLdouble left = -2.0;
const GLdouble right = 2.0;
const GLdouble bottom = -2.0;
const GLdouble top = 2.0;
const GLdouble near = 3.0;
const GLdouble far = 7.0;

// kąty obrotu

GLfloat rotatex = 0.0;
GLfloat rotatey = 0.0;

// wska?nik naciśnięcia lewego przycisku myszki

int button_state = GLUT_UP;

// poło?enie kursora myszki

int button_x, button_y;

// współczynnik skalowania

GLfloat scale = 1.5;

// identyfikatory tekstur

GLuint GROUND, WOOD, ROOF,OKNO;

// identyfikatory list wyświetlania

GLint GROUND_LIST, WOOD_LIST, ROOF_LIST,OKNO_LIST;

// filtr pomniejszający

GLint min_filter = GL_LINEAR_MIPMAP_LINEAR;

// wskazówki do korekcji perspektywy przy renderingu tekstur

GLint perspective_correction_hint = GL_DONT_CARE;

// wskazówki do automatycznego generowania mipmap

GLint mipmap_generation_hint = GL_DONT_CARE;

// funkcja rysująca napis w wybranym miejscu
// (wersja korzystająca z funkcji glWindowPos2i)

void DrawString(GLint x, GLint y, char *string)
{
	// poło?enie napisu
	glWindowPos2i(x, y);

	// wyświetlenie napisu
	int len = strlen(string);
	for (int i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, string[i]);
}

// funkcja generująca scenę 3D

void DisplayScene()
{
	// kolor tła - zawartość bufora koloru
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// czyszczenie bufora koloru i bufora głębokości
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// wybór macierzy modelowania
	glMatrixMode(GL_MODELVIEW);

	// macierz modelowania = macierz jednostkowa
	glLoadIdentity();

	// przesunięcie układu współrzędnych obiektów do środka bryły odcinania
	glTranslatef(0.0, 0.0, -(near + far) / 2);

	// obroty obiektu
	glRotatef(rotatex, 1.0, 0.0, 0.0);
	glRotatef(rotatey, 0.0, 1.0, 0.0);

	// skalowanie obiektu - klawisze "+" i "-"
	glScalef(scale, scale, scale);

	// włączenie testu bufora głębokości
	glEnable(GL_DEPTH_TEST);

	// włączenie teksturowania dwuwymiarowego
	glEnable(GL_TEXTURE_2D);

	// filtr powiększający
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// filtr pomniejszający
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

	// wskazówki do korekcji perspektywy przy renderingu tekstur
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, perspective_correction_hint);

	// ustawienie parametów środowiska tekstur
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// rysowanie podło?a
	glBindTexture(GL_TEXTURE_2D, GROUND);
	glPushMatrix();
	glTranslatef(0.0, -1.0, 0.0);
	glCallList(GROUND_LIST);
	glPopMatrix();

	// rysowanie ścian domku
	glBindTexture(GL_TEXTURE_2D, WOOD);
	glPushMatrix();
	glTranslatef(0.0, -0.5, 0.0);
	glScalef(0.5, 0.5, 0.5);
	glCallList(WOOD_LIST);
	glPopMatrix();

	// rysowanie okien
	glBindTexture(GL_TEXTURE_2D, OKNO);
	glPushMatrix();
	glTranslatef(0.0, -0.5, 0.0);
	glScalef(0.5, 0.5, 0.5);
	glCallList(OKNO_LIST);
	glPopMatrix();

	// rysowanie dachu domku
	glBindTexture(GL_TEXTURE_2D, ROOF);
	glPushMatrix();
	glTranslatef(0.0, -0.5, 0.0);
	glScalef(0.5, 0.5, 0.5);
	glCallList(ROOF_LIST);
	glPopMatrix();

	// wyłączenie teksturowania dwuwymiarowego
	glDisable(GL_TEXTURE_2D);

	// wyświetlenie wybranych informacje
	char string[200];
	GLint var;
	glColor3fv(Black);

	// pobranie informacji o filtrze pomniejszający
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &var);
	switch (var)
	{
		// filtr GL_NEAREST_MIPMAP_NEAREST
	case GL_NEAREST_MIPMAP_NEAREST:
		sprintf(string, "GL_TEXTURE_MIN_FILTER = GL_NEAREST_MIPMAP_NEAREST");
		break;

		// filtr GL_NEAREST_MIPMAP_LINEAR
	case GL_NEAREST_MIPMAP_LINEAR:
		sprintf(string, "GL_TEXTURE_MIN_FILTER = GL_NEAREST_MIPMAP_LINEAR");
		break;

		// filtr GL_LINEAR_MIPMAP_NEAREST
	case GL_LINEAR_MIPMAP_NEAREST:
		sprintf(string, "GL_TEXTURE_MIN_FILTER = GL_LINEAR_MIPMAP_NEAREST");
		break;

		// filtr GL_LINEAR_MIPMAP_LINEAR
	case GL_LINEAR_MIPMAP_LINEAR:
		sprintf(string, "GL_TEXTURE_MIN_FILTER = GL_LINEAR_MIPMAP_LINEAR");
		break;
	}
	DrawString(2, glutGet(GLUT_WINDOW_HEIGHT) - 17, string);

	// pobranie informacji o wskazówkach do automatycznego generowania mipmap
	glGetIntegerv(GL_PERSPECTIVE_CORRECTION_HINT, &var);
	switch (var)
	{
	case GL_FASTEST:
		sprintf(string, "GL_PERSPECTIVE_CORRECTION_HINT = GL_FASTEST");
		break;
	case GL_DONT_CARE:
		sprintf(string, "GL_PERSPECTIVE_CORRECTION_HINT = GL_DONT_CARE");
		break;
	case GL_NICEST:
		sprintf(string, "GL_PERSPECTIVE_CORRECTION_HINT = GL_NICEST");
		break;
	}
	DrawString(2, glutGet(GLUT_WINDOW_HEIGHT) - 33, string);

	// pobranie informacji o wskazówkach do korekcji perspektywy przy renderingu tekstur
	glGetIntegerv(GL_GENERATE_MIPMAP_HINT, &var);
	switch (var)
	{
	case GL_FASTEST:
		sprintf(string, "GL_GENERATE_MIPMAP_HINT = GL_FASTEST");
		break;
	case GL_DONT_CARE:
		sprintf(string, "GL_GENERATE_MIPMAP_HINT = GL_DONT_CARE");
		break;
	case GL_NICEST:
		sprintf(string, "GL_GENERATE_MIPMAP_HINT = GL_NICEST");
		break;
	}
	DrawString(2, glutGet(GLUT_WINDOW_HEIGHT) - 49, string);

	// skierowanie polece? do wykonania
	glFlush();

	// zamiana buforów koloru
	glutSwapBuffers();
}

// zmiana wielkości okna

void Reshape(int width, int height)
{
	// obszar renderingu - całe okno
	glViewport(0, 0, width, height);

	// wybór macierzy rzutowania
	glMatrixMode(GL_PROJECTION);

	// macierz rzutowania = macierz jednostkowa
	glLoadIdentity();

	// parametry bryły obcinania
	if (aspect == ASPECT_1_1)
	{
		// wysokość okna większa od wysokości okna
		if (width < height && width > 0)
			glFrustum(left, right, bottom*height / width, top*height / width, near, far);
		else

			// szerokość okna większa lub równa wysokości okna
			if (width >= height && height > 0)
				glFrustum(left*width / height, right*width / height, bottom, top, near, far);
	}
	else
		glFrustum(left, right, bottom, top, near, far);

	// generowanie sceny 3D
	DisplayScene();
}

// obsługa klawiatury

void Keyboard(unsigned char key, int x, int y)
{
	// klawisz +
	if (key == '+')
		scale += 0.05;
	else

		// klawisz -
		if (key == '-' && scale > 0.05)
			scale -= 0.05;

	// narysowanie sceny
	DisplayScene();
}

// obsługa przycisków myszki

void MouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		// zapamiętanie stanu lewego przycisku myszki
		button_state = state;

		// zapamiętanie poło?enia kursora myszki
		if (state == GLUT_DOWN)
		{
			button_x = x;
			button_y = y;
		}
	}
}

// obsługa ruchu kursora myszki

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

// utworzenie tekstur

void GenerateTextures()
{
	// zmienne u?yte przy obsłudze plików TARGA
	GLsizei width, height;
	GLenum format, type;
	GLvoid *pixels;

	// tryb upakowania bajtów danych tekstury
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// wskazówki do automatycznego generowania mipmap
	glHint(GL_GENERATE_MIPMAP_HINT, mipmap_generation_hint);

	// wczytanie tekstury ground1-2.tga
	GLboolean error = load_targa("ground1-2.tga", width, height, format, type, pixels);

	// błąd odczytu pliku
	if (error == GL_FALSE)
	{
		printf("Niepoprawny odczyt pliku ground1-2.tga");
		exit(0);
	}

	// utworzenie identyfikatora tekstury
	glGenTextures(1, &GROUND);

	// dowiązanie stanu tekstury
	glBindTexture(GL_TEXTURE_2D, GROUND);

	// włączenie automatycznego generowania mipmap
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// definiowanie tekstury (z mipmapami)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, type, pixels);

	// porządki
	delete[](unsigned char*)pixels;

	// wczytanie tekstury wall_wood_verti_color.tga
	error = load_targa("wall_wood_verti_color.tga", width, height, format, type, pixels);

	// błąd odczytu pliku
	if (error == GL_FALSE)
	{
		printf("Niepoprawny odczyt pliku wall_wood_verti_color.tga");
		exit(0);
	}

	// utworzenie identyfikatora tekstury
	glGenTextures(1, &WOOD);

	// dowiązanie stanu tekstury
	glBindTexture(GL_TEXTURE_2D, WOOD);

	// włączenie automatycznego generowania mipmap
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// definiowanie tekstury (z mipmapami)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, type, pixels);

	// porządki
	delete[](unsigned char*)pixels;

	// wczytanie tekstury roof_old_rectangle_color.tga
	error = load_targa("roof_old_rectangle_color.tga", width, height, format, type, pixels);

	// błąd odczytu pliku
	if (error == GL_FALSE)
	{
		printf("Niepoprawny odczyt pliku roof_old_rectangle_color.tga");
		exit(0);
	}

	// utworzenie identyfikatora tekstury
	glGenTextures(1, &ROOF);

	// dowiązanie stanu tekstury
	glBindTexture(GL_TEXTURE_2D, ROOF);

	// włączenie automatycznego generowania mipmap
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// definiowanie tekstury (z mipmapami)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, type, pixels);

	// porządki
	delete[](unsigned char*)pixels;

	//////////////////////////////////////////////////////////////
	// wczytanie tekstury okno.tga
	error = load_targa("okno.tga", width, height, format, type, pixels);

	// błąd odczytu pliku
	if (error == GL_FALSE)
	{
		printf("Niepoprawny odczyt pliku okno.tga");
		exit(0);
	}

	// utworzenie identyfikatora tekstury
	glGenTextures(1, &OKNO);

	// dowiązanie stanu tekstury
	glBindTexture(GL_TEXTURE_2D, OKNO);

	// włączenie automatycznego generowania mipmap
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// definiowanie tekstury (z mipmapami)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, type, pixels);

	// porządki
	delete[](unsigned char*)pixels;

}

// obsługa menu podręcznego

void Menu(int value)
{
	switch (value)
	{
		// filtr pomniejszający
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_LINEAR:
		min_filter = value;
		DisplayScene();
		break;

		// wskazówka GL_FASTEST do korekcji perspektywy przy renderingu tekstur
	case PERSPECTIVE_CORRECTION_FASTEST:
		perspective_correction_hint = GL_FASTEST;
		DisplayScene();
		break;

		// wskazówka GL_DONT_CARE do korekcji perspektywy przy renderingu tekstur
	case PERSPECTIVE_CORRECTION_DONT_CARE:
		perspective_correction_hint = GL_DONT_CARE;
		DisplayScene();
		break;

		// wskazówka GL_NICEST do korekcji perspektywy przy renderingu tekstur
	case PERSPECTIVE_CORRECTION_NICEST:
		perspective_correction_hint = GL_NICEST;
		DisplayScene();
		break;

		// wskazówka do automatycznego generowania mipmap
	case GENERATE_MIPMAP_FASTEST:
		GenerateTextures();
		mipmap_generation_hint = GL_FASTEST;
		DisplayScene();
		break;

		// wskazówka GL_DONT_CARE do automatycznego generowania mipmap
	case GENERATE_MIPMAP_DONT_CARE:
		GenerateTextures();
		mipmap_generation_hint = GL_DONT_CARE;
		DisplayScene();
		break;

		// wskazówka GL_NICEST do automatycznego generowania mipmap
	case GENERATE_MIPMAP_NICEST:
		GenerateTextures();
		mipmap_generation_hint = GL_NICEST;
		DisplayScene();
		break;

		// obszar renderingu - całe okno
	case FULL_WINDOW:
		aspect = FULL_WINDOW;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;

		// obszar renderingu - aspekt 1:1
	case ASPECT_1_1:
		aspect = ASPECT_1_1;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;

		// wyjście
	case EXIT:
		exit(0);
	}
}

// utworzenie list wyświetlania

void GenerateDisplayLists()
{
	// generowanie identyfikatora pierwszej listy wyświetlania
	GROUND_LIST = glGenLists(1);

	// pierwsza lista wyświetlania - podło?e
	glNewList(GROUND_LIST, GL_COMPILE);

	// czworokąt
	glBegin(GL_QUADS);
	glTexCoord2f(16.0, 0.0);
	glVertex3f(-8.0, 0.0, -8.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-8.0, 0.0, 8.0);
	glTexCoord2f(0.0, 16.0);
	glVertex3f(8.0, 0.0, 8.0);
	glTexCoord2f(16.0, 16.0);
	glVertex3f(8.0, 0.0, -8.0);
	glEnd();

	// koniec pierwszej listy wyświetlania
	glEndList();

	// generowanie identyfikatora drugiej listy wyświetlania
	WOOD_LIST = glGenLists(1);

	// druga lista wyświetlania - ściany chatki
	glNewList(WOOD_LIST, GL_COMPILE);

	// seria trójkątów
	glBegin(GL_TRIANGLES);

	// przednia ściana
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0, 1.0);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(1.0, -1.0, 1.0);
	glTexCoord2f(2.0, 2.0);
	glVertex3f(1.0, 1.0, 1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0, 1.0);
	glTexCoord2f(2.0, 2.0);
	glVertex3f(1.0, 1.0, 1.0);
	glTexCoord2f(0.0, 2.0);
	glVertex3f(-1.0, 1.0, 1.0);

	// prawa boczna ściana
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0, 1.0, -1.0);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(1.0, 1.0, 1.0);
	glTexCoord2f(2.0, 2.0);
	glVertex3f(1.0, -1.0, 1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0, 1.0, -1.0);
	glTexCoord2f(2.0, 2.0);
	glVertex3f(1.0, -1.0, 1.0);
	glTexCoord2f(0.0, 2.0);
	glVertex3f(1.0, -1.0, -1.0);

	// lewa boczna ściana
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0, -1.0);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(-1.0, -1.0, 1.0);
	glTexCoord2f(0.0, 2.0);
	glVertex3f(-1.0, 1.0, -1.0);
	glTexCoord2f(0.0, 2.0);
	glVertex3f(-1.0, 1.0, -1.0);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(-1.0, -1.0, 1.0);
	glTexCoord2f(2.0, 2.0);
	glVertex3f(-1.0, 1.0, 1.0);

	// tylna ściana
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0, -1.0);
	glTexCoord2f(0.0, 2.0);
	glVertex3f(-1.0, 1.0, -1.0);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(1.0, 1.0, -1.0);
	glTexCoord2f(2.0, 2.0);
	glVertex3f(1.0, -1.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0, -1.0);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(1.0, 1.0, -1.0);

	// przedni szczyt
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, 1.0, 1.0);
	glTexCoord2f(0.0, 2.0);
	glVertex3f(1.0, 1.0, 1.0);
	glTexCoord2f(1.0, 2.0);
	glVertex3f(0.0, 2.0, 1.0);

	// tylny szczyt
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, 1.0, -1.0);
	glTexCoord2f(1.0, 2.0);
	glVertex3f(0.0, 2.0, -1.0);
	glTexCoord2f(0.0, 2.0);
	glVertex3f(1.0, 1.0, -1.0);
	glEnd();

	// koniec drugiej listy wyświetlania
	glEndList();

	// generowanie identyfikatora trzeciej listy wyświetlania
	ROOF_LIST = glGenLists(1);

	// trzecia lista wyświetlania - dach chatki
	glNewList(ROOF_LIST, GL_COMPILE);

	// dwa czworokąty
	glBegin(GL_QUADS);

	// lewa część dachu
	glTexCoord2f(2.0, 2.0);
	glVertex3f(0.0, 2.0, 1.2);
	glTexCoord2f(0.0, 2.0);
	glVertex3f(0.0, 2.0, -1.2);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, 1.0, -1.2);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(-1.0, 1.0, 1.2);

	// prawa część dachu
	glTexCoord2f(2.0, 2.0);
	glVertex3f(0.0, 2.0, -1.2);
	glTexCoord2f(0.0, 2.0);
	glVertex3f(0.0, 2.0, 1.2);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0, 1.0, 1.2);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(1.0, 1.0, -1.2);
	glEnd();

	// koniec trzeciej listy wyświetlania
	glEndList();
	////////////////////////////////
	// generowanie identyfikatora czwartej listy wyświetlania
	OKNO_LIST = glGenLists(1);

	// czwarta lista wyświetlania - okno
	glNewList(OKNO_LIST, GL_COMPILE);


	// seria trójkątów
	glBegin(GL_TRIANGLES);
	// przednie okno 1
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.2, -0.33, 1.001);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-0.8, -0.33, 1.001);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-0.8, 0.66, 1.001);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.2, -0.33, 1.001);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-0.8, 0.66, 1.001);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-0.2, 0.66, 1.001);


	// przednie okno 2
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.2, -0.33, 1.001);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(0.8, -0.33, 1.001);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.8, 0.66, 1.001);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.2, -0.33, 1.001);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.8, 0.66, 1.001);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(0.2, 0.66, 1.001);

	// tylnie okno 1
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.95, -0.33, -1.001);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-0.4, -0.33, -1.001);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-0.4, 0.66, -1.001);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.95, -0.33, -1.001);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-0.4, 0.66, -1.001);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-0.95, 0.66, -1.001);

	// tylnie okno 2
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.3, -0.33, -1.001);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(0.3, -0.33, -1.001);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.3, 0.66, -1.001);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.3, -0.33, -1.001);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.3, 0.66, -1.001);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-0.3, 0.66, -1.001);

	// tylnie okno 3
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.4, -0.33, -1.001);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(0.95, -0.33, -1.001);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.95, 0.66, -1.001);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.4, -0.33, -1.001);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.95, 0.66, -1.001);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(0.4, 0.66, -1.001);

	// prawe okno 1
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.001, 0.66, 0.2);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.001, 0.66, 0.8);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.001, -0.33, 0.8);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.001, 0.66, 0.2);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.001, -0.33, 0.8);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.001, -0.33, 0.2);

	// prawe okno 2
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.001, 0.66, -0.2);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.001, 0.66, -0.8);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.001, -0.33, -0.8);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.001, 0.66, -0.2);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.001, -0.33, -0.8);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.001, -0.33, -0.2);

	
	// lewe okno 1
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.001, 0.66, 0.2);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-1.001, 0.66, 0.8);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.001, -0.33, 0.8);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.001, 0.66, 0.2);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.001, -0.33, 0.8);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.001, -0.33, 0.2);

	// lewe okno 2
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.001, 0.66, -0.2);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-1.001, 0.66, -0.8);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.001, -0.33, -0.8);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.001, 0.66, -0.2);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.001, -0.33, -0.8);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.001, -0.33, -0.2);
	glEnd();

	// koniec CZWARTEJ listy wyświetlania
	glEndList();
}

// sprawdzenie i przygotowanie obsługi wybranych rozszerze?

void ExtensionSetup()
{
	// pobranie numeru wersji biblioteki OpenGL
	const char *version = (char*)glGetString(GL_VERSION);

	// odczyt wersji OpenGL
	int major = 0, minor = 0;
	if (sscanf(version, "%d.%d", &major, &minor) != 2)
	{
#ifdef WIN32
		printf("Błędny format wersji OpenGL\n");
#else

		printf("Bledny format wersji OpenGL\n");
#endif

		exit(0);
	}

	// sprawdzenie czy jest co najmniej wersja 1.4 OpenGL lub
	// czy jest obsługiwane rozszerzenie GL_SGIS_generate_mipmap
	if (!(major > 1 || minor >= 4) &&
		!glutExtensionSupported("GL_SGIS_generate_mipmap"))
	{
		printf("Brak rozszerzenia GL_SGIS_generate_mipmap!\n");
		exit(0);
	}

	// sprawdzenie czy jest co najmniej wersja 1.4
	if (major > 1 || minor >= 4)
	{
		// pobranie wska?nika wybranej funkcji OpenGL 1.4
		glWindowPos2i = (PFNGLWINDOWPOS2IPROC)wglGetProcAddress("glWindowPos2i");
	}
	else
		// sprawdzenie czy jest obsługiwane rozszerzenie ARB_window_pos
		if (glutExtensionSupported("GL_ARB_window_pos"))
		{
			// pobranie wska?nika wybranej funkcji rozszerzenia ARB_window_pos
			glWindowPos2i = (PFNGLWINDOWPOS2IPROC)wglGetProcAddress
				("glWindowPos2iARB");
		}
		else
		{
			printf("Brak rozszerzenia ARB_window_pos!\n");
			exit(0);
		}
}

int main(int argc, char *argv[])
{
	// inicjalizacja biblioteki GLUT
	glutInit(&argc, argv);

	// inicjalizacja bufora ramki
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// rozmiary głównego okna programu
	glutInitWindowSize(500, 500);

	// utworzenie głównego okna programu
	glutCreateWindow("Mipmapy 2D");

	// dołączenie funkcji generującej scenę 3D
	glutDisplayFunc(DisplayScene);

	// dołączenie funkcji wywoływanej przy zmianie rozmiaru okna
	glutReshapeFunc(Reshape);

	// dołączenie funkcji obsługi klawiatury
	glutKeyboardFunc(Keyboard);

	// obsługa przycisków myszki
	glutMouseFunc(MouseButton);

	// obsługa ruchu kursora myszki
	glutMotionFunc(MouseMotion);

	// utworzenie podmenu - Filtr pomniejszający
	int MenuMinFilter = glutCreateMenu(Menu);
	glutAddMenuEntry("GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST);
	glutAddMenuEntry("GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR);
	glutAddMenuEntry("GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST);
	glutAddMenuEntry("GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR);

	// utworzenie podmenu - GL_PERSPECTIVE_CORRECTION_HINT
	int PerspectiveCorrectionHint = glutCreateMenu(Menu);
	glutAddMenuEntry("GL_FASTEST", PERSPECTIVE_CORRECTION_FASTEST);
	glutAddMenuEntry("GL_DONT_CARE", PERSPECTIVE_CORRECTION_DONT_CARE);
	glutAddMenuEntry("GL_NICEST", PERSPECTIVE_CORRECTION_NICEST);

	// utworzenie podmenu - GL_GENERATE_MIPMAP_HINT
	int GenerateMipmapHint = glutCreateMenu(Menu);
	glutAddMenuEntry("GL_FASTEST", GENERATE_MIPMAP_FASTEST);
	glutAddMenuEntry("GL_DONT_CARE", GENERATE_MIPMAP_DONT_CARE);
	glutAddMenuEntry("GL_NICEST", GENERATE_MIPMAP_NICEST);

	// utworzenie podmenu - Aspekt obrazu
	int MenuAspect = glutCreateMenu(Menu);
#ifdef WIN32

	glutAddMenuEntry("Aspekt obrazu - całe okno", FULL_WINDOW);
#else

	glutAddMenuEntry("Aspekt obrazu - cale okno", FULL_WINDOW);
#endif

	glutAddMenuEntry("Aspekt obrazu 1:1", ASPECT_1_1);

	// menu główne
	glutCreateMenu(Menu);

#ifdef WIN32

	glutAddSubMenu("Filtr pomniejszający", MenuMinFilter);
	glutAddSubMenu("GL_PERSPECTIVE_CORRECTION_HINT", PerspectiveCorrectionHint);
	glutAddSubMenu("GL_GENERATE_MIPMAP_HINT", GenerateMipmapHint);
	glutAddSubMenu("Aspekt obrazu", MenuAspect);
	glutAddMenuEntry("Wyjście", EXIT);
#else

	glutAddSubMenu("Filtr pomniejszajacy", MenuMinFilter);
	glutAddSubMenu("GL_PERSPECTIVE_CORRECTION_HINT", PerspectiveCorrectionHint);
	glutAddSubMenu("GL_GENERATE_MIPMAP_HINT", GenerateMipmapHint);
	glutAddSubMenu("Aspekt obrazu", MenuAspect);
	glutAddMenuEntry("Wyjscie", EXIT);
#endif

	// określenie przycisku myszki obsługującego menu podręczne
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// utworzenie tekstur
	GenerateTextures();

	// sprawdzenie i przygotowanie obsługi wybranych rozszerze?
	ExtensionSetup();

	// utworzenie list wyświetlania
	GenerateDisplayLists();

	// wprowadzenie programu do obsługi pętli komunikatów
	glutMainLoop();
	return 0;
}
