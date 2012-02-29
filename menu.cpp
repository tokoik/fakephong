#include <stdlib.h>
#if defined(WIN32)
#  include "glut.h"
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include "menu.h"

static int shape = 0;  /* 表示形状 */

/*
** メニューエントリの更新 （選択項目の頭に c を付ける)
*/
static void markEntry(int i, char c, char *s)
{
  char buffer[128], *p = buffer;
  int n = sizeof(buffer) / sizeof(char) - 1;

  *p = c;
  do {
    if (--n <= 0) {
      *p = '\0';
      break;
    }
  }
  while ((*++p = *++s) != '\0');

  glutChangeToMenuEntry(i + 1, buffer, i);
}

/*
** 図形のメニュー項目
*/
static char *shapeName[] = {
  "*Teapot",
  " Cube",
  " Sphere",
  " Torus",
};

/*
** 図形表示
*/
void showShape(void)
{
  switch (shape) {
  case 0:
    glutSolidTeapot(1.0);
    break;
  case 1:
    glutSolidCube(2.0);
    break;
  case 2:
    glutSolidSphere(1.0, 16, 8);
    break;
  case 3:
    glutSolidTorus(0.5, 1.0, 12, 16);
    break;
  }
}

/*
** メニュー選択
*/
static void shapeMenu(int m)
{
  markEntry(shape, ' ', shapeName[shape]);
  shape = m;
  markEntry(shape, '*', shapeName[shape]);
  glutPostRedisplay();
}

/*
** メインメニュー
*/
static void mainMenu(int m)
{
  switch(m) {
  case 0:
    exit(0);
  default:
    break;
  }
}

/*
** メニュー作成
*/
void menu(void)
{
  int shapeId = glutCreateMenu(shapeMenu);

  for (unsigned int i = 0; i < (sizeof shapeName) / (sizeof (char *)); ++i)
    glutAddMenuEntry(shapeName[i], i);

  glutCreateMenu(mainMenu);
  glutAddSubMenu("Shape", shapeId);
  glutAddMenuEntry("Quit", 0);

  glutAttachMenu(GLUT_RIGHT_BUTTON);
}
