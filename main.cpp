#if defined(__APPLE__) || defined(MACOSX)
#  define GL_SILENCE_DEPRECATION
#  include <GLUT/glut.h>
#  include <OpenGL/glext.h>
#else
#  if defined(_WIN32)
//#    pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#    define _USE_MATH_DEFINES
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  include <GL/glut.h>
#  include <GL/glext.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* メニュー関連の関数の宣言 */
#include "menu.h"

/* トラックボール処理用関数の宣言 */
#include "trackball.h"

/*
** テクスチャサイズ
*/
#define TEXWIDTH  256                               /* テクスチャの幅　 */
#define TEXHEIGHT 256                               /* テクスチャの高さ */

/*
** 色設定
*/
#define COLOR 1

/*
** 光源
*/
#if COLOR
static const GLfloat lpos[] = { 0.4f, 0.6f, 2.0f, 0.0f };   /* 位置　　　　 */
static const GLfloat lcol[] = { 1.0f, 1.0f, 1.0f, 1.0f };   /* 直接光強度　 */
static const GLfloat lamb[] = { 0.1f, 0.1f, 0.1f, 1.0f };   /* 環境光強度　 */
#else
static const GLfloat lpos[] = { 0.0f, 0.0f, 1.0f, 0.0f };   /* 位置　　　　 */
static const GLfloat lcol[] = { 1.0f, 1.0f, 1.0f, 1.0f };   /* 直接光強度　 */
static const GLfloat lamb[] = { 0.2f, 0.2f, 0.2f, 1.0f };   /* 環境光強度　 */
#endif

/*
** マテリアル
*/
#if COLOR
static const GLfloat kdiff[] = { 0.4f, 0.4f, 0.3f, 1.0f };  /* 拡散反射係数 */
#else
static const GLfloat kdiff[] = { 0.0f, 0.1f, 0.3f, 1.0f };  /* 拡散反射係数 */
#endif

static const GLfloat kspec[] = { 0.6f, 0.6f, 0.6f, 1.0f };  /* 鏡面反射係数 */
static const GLfloat kshi = 20.0f;                          /* 輝き係数　　 */

/*
** テクスチャの作成
*/
static void makeTexture(unsigned char *t, int w, int h)
{
  /* 光線ベクトルと視線ベクトルの中間ベクトル (hx, hy, hz) を求める */
  float l2 = lpos[0] * lpos[0] + lpos[1] * lpos[1] + lpos[2] * lpos[2];
  float l = sqrtf(l2);
  float hx = lpos[0], hy = lpos[1], hz = lpos[2] + l;
  
  l2 += lpos[2] * l;
  if (l2 > 0.0) {
    float m = sqrtf(l2 + l2);
    hx /= m;
    hy /= m;
    hz /= m;
  }
	
  /* 中間ベクトルと法線ベクトルの内積値でテクスチャを作る */
  for (int y = 0; y < h; ++y) {
    float ny = (float)(y + y - h) / (float)h;
    float ny2 = ny * ny;
    
    for (int x = 0; x < w; ++x) {
      float nx = (float)(x + x - w) / (float)w;
      float nx2 = nx * nx;
      float nz2 = 1.0f - nx2 - ny2;
      
      /* nz2 >= 0 なら「円内」 */
      if (nz2 >= 0.0f) {
        float nz = sqrtf(nz2);
#define MODEL 0
#if MODEL == 0
        float rs = powf(hx * nx + hy * ny + hz * nz, kshi) * 255.0f;
#elif MODEL == 1
        float rs = (hx * nx + hy * ny + hz * nz > 0.97f) ? 255.0f : 0.0f;
#elif MODEL == 2
        float rs = hx * nx - hy * ny - hz * nz;
				rs = pow(1.0f - rs * rs, 5.0) * 255.0f;
#endif
        *(t++) = (GLubyte)(kspec[0] * rs * lcol[0]);
        *(t++) = (GLubyte)(kspec[1] * rs * lcol[1]);
        *(t++) = (GLubyte)(kspec[2] * rs * lcol[2]);
      }
      else {
        *(t++) = 0;
        *(t++) = 0;
        *(t++) = 0;
      }
    }
  }

#if 0
  FILE *fp = fopen("aaa.raw", "wb");
  if (fp) {
    fwrite(t - sizeof(unsigned char) * 3 * w * h, sizeof(unsigned char) * 3, w * h, fp);
    fclose(fp);
  }
#endif
}

/*
** 初期化
*/
static void init(void)
{
  /* テクスチャの読み込みに使う配列 */
  static GLubyte texture[TEXHEIGHT * TEXWIDTH * 3];

  /* テクスチャの作成 */
  makeTexture(texture, TEXWIDTH, TEXHEIGHT);

  /* テクスチャ画像はバイト単位に詰め込まれている */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
  /* テクスチャの割り当て */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXWIDTH, TEXHEIGHT, 0,
    GL_RGB, GL_UNSIGNED_BYTE, texture);
  
  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  
  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  /* スフィアマッピングする */
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  
  /* テクスチャ環境（ポリゴンの陰影に加算する） */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
  
  /* 光源の初期設定 */
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lcol);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lcol);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lamb);

  /* 拡散反射光と鏡面反射光を別々に補間する */
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
  
  /* その他の初期設定 */
#if COLOR
  glClearColor(0.2f, 0.6f, 0.2f, 0.0f);
#else
  glClearColor(0.3f, 0.3f, 1.0f, 0.0f);
#endif
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}

/*
** シーンの描画
*/
static void scene(void)
{
  /* 鏡面反射成分をテクスチャマッピングするときの鏡面反射係数 */
  static const GLfloat knone[] = { 0.0, 0.0, 0.0, 1.0 };
  
  /* 材質の設定 */
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, kdiff);
  glMaterialf(GL_FRONT, GL_SHININESS, kshi);
  
  /* テクスチャをマッピングせずに図形を描く */
  glMaterialfv(GL_FRONT, GL_SPECULAR, kspec);
  glPushMatrix();
  glTranslated(-1.7, 0.0, 0.0);
  glMultMatrixd(trackballRotation());
  showShape();
  glPopMatrix();

  /* テクスチャをマッピングして図形を描く */
  glMaterialfv(GL_FRONT, GL_SPECULAR, knone);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glPushMatrix();
  glTranslated( 1.7, 0.0, 0.0);
  glMultMatrixd(trackballRotation());
  showShape();
  glPopMatrix();
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_2D);
}


/****************************
** GLUT のコールバック関数 **
****************************/

static void display(void)
{
  /* モデルビュー変換行列の設定 */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  /* 画面クリア */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* モデルビュー変換行列の初期化 */
  glLoadIdentity();
  
  /* 光源の位置を設定 */
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);
  
  /* 視点の移動 */
  glTranslated(0.0, 0.0, -7.5);
  
  /* シーンの描画 */
  scene();
  
  /* ダブルバッファリング */
  glutSwapBuffers();
}

static void resize(int w, int h)
{
  /* トラックボールする範囲 */
  trackballRegion(w, h);
  
  /* ウィンドウ全体をビューポートにする */
  glViewport(0, 0, w, h);
  
  /* 透視変換行列の指定 */
  glMatrixMode(GL_PROJECTION);
  
  /* 透視変換行列の初期化 */
  glLoadIdentity();
  gluPerspective(30.0, (GLdouble)w / (GLdouble)h, 1.0, 100.0);
}

static void idle(void)
{
  /* 画面の描き替え */
  glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
  switch (button) {
  case GLUT_LEFT_BUTTON:
    switch (state) {
    case GLUT_DOWN:
      /* トラックボール開始 */
      trackballStart(x, y);
      glutIdleFunc(idle);
      break;
    case GLUT_UP:
      /* トラックボール停止 */
      trackballStop(x, y);
      glutIdleFunc(0);
      break;
    default:
      break;
    }
    break;
    default:
      break;
  }
}

static void motion(int x, int y)
{
  /* トラックボール移動 */
  trackballMotion(x, y);
}

static void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 'q':
  case 'Q':
  case '\033':
    /* ESC か q か Q をタイプしたら終了 */
    exit(0);
  default:
    break;
  }
}

/*
** メインプログラム
*/
int main(int argc, char *argv[])
{
  glutInitWindowSize(480, 240);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  init();
  menu();
  glutMainLoop();
  return 0;
}