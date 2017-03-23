#include <cmath>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

GLfloat x_1 = 100.0f;
GLfloat y_1 = 150.0f;
GLsizei rsize = 50;
GLfloat xstep = 1.0f;
GLfloat ystep = 1.0f;
GLfloat windowWidth;
GLfloat windowHeight;

void RenderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(0.0f, 1.0f, 0.0f);

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x_1, y_1);
	for (size_t i = 0; i <= 9; i++)
	{
		glVertex2f(x_1 + (8 * cos(i * 2 * 3.14 / 8)) * 10,
			y_1 + (8 * sin(i * 2 * 3.14 / 8)) * 10);
	}
	glEnd();
	glFlush();
	glutSwapBuffers();
}

void TimerFunction(int value) {

	if (x_1 > windowWidth - rsize || x_1 < 0)
		xstep = -xstep;

	if (y_1 > windowHeight - rsize || y_1 < 0)
		ystep = -ystep;

	if (x_1 > windowWidth - rsize)
		x_1 = windowWidth - rsize - 1;

	if (y_1 > windowHeight - rsize)
		y_1 = windowHeight - rsize - 1;

	x_1 += xstep;
	y_1 += ystep;

	glutPostRedisplay();
	glutTimerFunc(33, TimerFunction, 1);
}

void SetupRC(void) {
	glClearColor(0.21f, 0.52f, 0.78f, 0.0f);
}

void ChangeSize(GLsizei w, GLsizei h) {

	if (h == 0)
		h = 1;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (w <= h)
	{
		windowHeight = 250.0f*h / w;
		windowWidth = 250.0f;
	}
	else
	{
		windowWidth = 250.0f*w / h;
		windowHeight = 250.0f;
	}

	glOrtho(0.0f, windowWidth, 0.0f, windowHeight, 1.0f, -1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Wielok¹t");
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutTimerFunc(33, TimerFunction, 1);
	SetupRC();
	glutMainLoop();
}