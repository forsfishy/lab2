// Доп задание: наклон (shear) фигуры по осям x и y аффинным преобразованием,
// чтобы квадрат мог наклоняться в любую сторону и становиться параллелограммом
#include "draw.h"

#include <algorithm>
#include <cmath>

namespace {

const double PI = 3.14159265358979323846;
const int K = 4; //  k*pi/m
const int MAX_VERTICES = 12;

struct Point {
  double x, y;
};

struct Matrix {
  double a[3][3];
};

Matrix identity()
{
  return {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
}

Matrix multiply(const Matrix &l, const Matrix &r)
{
  Matrix res{};
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      for (int k = 0; k < 3; ++k)
        res.a[i][j] += l.a[i][k] * r.a[k][j];
  return res;
}

Matrix translation(double dx, double dy)
{
  Matrix m = identity();
  m.a[0][2] = dx;
  m.a[1][2] = dy;
  return m;
}

Matrix scaling(double s)
{
  Matrix m = identity();
  m.a[0][0] = m.a[1][1] = s;
  return m;
}

// x' = x + kx * y, y' = ky * x + y
Matrix shear(double kx, double ky)
{
  Matrix m = identity();
  m.a[0][1] = kx;
  m.a[1][0] = ky;
  return m;
}

Matrix rotation(double angle)
{
  Matrix m = identity();
  m.a[0][0] = m.a[1][1] = std::cos(angle);
  m.a[0][1] = -std::sin(angle);
  m.a[1][0] = std::sin(angle);
  return m;
}

Matrix transform;
Point pivot;
int figure_count;
int vertex_count;

//  домножаем преобразование на общую матрицу 
void apply(const Matrix &m)
{
  transform = multiply(m, transform);
}

void apply_around_pivot(const Matrix &m)
{
  apply(multiply(translation(pivot.x, pivot.y),
                 multiply(m, translation(-pivot.x, -pivot.y))));
}

Point to_screen(Point p)
{
  return {transform.a[0][0] * p.x + transform.a[0][1] * p.y + transform.a[0][2],
          transform.a[1][0] * p.x + transform.a[1][1] * p.y + transform.a[1][2]};
}

void put_pixel(SDL_Surface *s, int x, int y, Uint32 color)
{
  if (x < 0 || y < 0 || x >= s->w || y >= s->h)
    return;
  Uint8 *row = static_cast<Uint8 *>(s->pixels) + y * s->pitch;
  reinterpret_cast<Uint32 *>(row)[x] = color;
}

// Алгоритм Брезенхейма 
void bresenham(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 color)
{
  int dx = std::abs(x2 - x1);
  int dy = std::abs(y2 - y1);
  int sx = x2 >= x1 ? 1 : -1;
  int sy = y2 >= y1 ? 1 : -1;
  bool steep = dy > dx;
  if (steep)
    std::swap(dx, dy);

  int d = 2 * dy - dx;
  int d1 = 2 * dy;
  int d2 = 2 * (dy - dx);
  int x = x1, y = y1;
  put_pixel(s, x, y, color);

  for (int i = 1; i <= dx; ++i) {
    if (d > 0) {
      d += d2;
      if (steep)
        x += sx;
      else
        y += sy;
    } else {
      d += d1;
    }
    if (steep)
      y += sy;
    else
      x += sx;
    put_pixel(s, x, y, color);
  }
}

void draw_segment(SDL_Surface *s, Point a, Point b, Uint32 color)
{
  Point p = to_screen(a);
  Point q = to_screen(b);
  int x1 = static_cast<int>(std::lround(p.x));
  int y1 = static_cast<int>(std::lround(p.y));
  int x2 = static_cast<int>(std::lround(q.x));
  int y2 = static_cast<int>(std::lround(q.y));
  bresenham(s, x1, y1, x2, y2, color);
}

double calc_mu()
{
  double phi = 2 * PI / vertex_count;
  double t = std::tan(K * PI / (vertex_count * (figure_count - 1)));
  return t / (std::sin(phi) + t * (1 - std::cos(phi)));
}

} 

void reset_scene()
{
  transform = translation(SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0);
  pivot = {SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0};
  figure_count = 24;
  vertex_count = 4;
}

void resize_scene(int width, int height)
{
  double dx = (width - SCREEN_WIDTH) / 2.0;
  double dy = (height - SCREEN_HEIGHT) / 2.0;
  SCREEN_WIDTH = width;
  SCREEN_HEIGHT = height;
  apply(translation(dx, dy));
  pivot.x += dx;
  pivot.y += dy;
}

void handle_key(SDL_Scancode key)
{
  const double step = 10;
  const double angle = 5 * PI / 180;
  const double zoom = 1.1;
  const double tilt = 0.1;

  switch (key) {
  case SDL_SCANCODE_W:
  case SDL_SCANCODE_UP:
    apply(translation(0, -step));
    break;
  case SDL_SCANCODE_S:
  case SDL_SCANCODE_DOWN:
    apply(translation(0, step));
    break;
  case SDL_SCANCODE_A:
  case SDL_SCANCODE_LEFT:
    apply(translation(-step, 0));
    break;
  case SDL_SCANCODE_D:
  case SDL_SCANCODE_RIGHT:
    apply(translation(step, 0));
    break;
  case SDL_SCANCODE_Q:
    apply_around_pivot(rotation(angle));
    break;
  case SDL_SCANCODE_E:
    apply_around_pivot(rotation(-angle));
    break;
  case SDL_SCANCODE_EQUALS:
  case SDL_SCANCODE_KP_PLUS:
    apply_around_pivot(scaling(zoom));
    break;
  case SDL_SCANCODE_MINUS:
  case SDL_SCANCODE_KP_MINUS:
    apply_around_pivot(scaling(1 / zoom));
    break;
  case SDL_SCANCODE_J: 
    apply_around_pivot(shear(tilt, 0));
    break;
  case SDL_SCANCODE_L: 
    apply_around_pivot(shear(-tilt, 0));
    break;
  case SDL_SCANCODE_I: 
    apply_around_pivot(shear(0, -tilt));
    break;
  case SDL_SCANCODE_K: 
    apply_around_pivot(shear(0, tilt));
    break;
  case SDL_SCANCODE_N:
    figure_count = std::min(figure_count + 1, 100);
    break;
  case SDL_SCANCODE_M:
    figure_count = std::max(figure_count - 1, 6);
    break;
  case SDL_SCANCODE_RIGHTBRACKET:
    vertex_count = std::min(vertex_count + 1, MAX_VERTICES);
    break;
  case SDL_SCANCODE_LEFTBRACKET:
    vertex_count = std::max(vertex_count - 1, 3);
    break;
  case SDL_SCANCODE_R:
    reset_scene();
    break;
  default:
    break;
  }
}

void set_pivot(int x, int y)
{
  pivot = {static_cast<double>(x), static_cast<double>(y)};
}

void draw(SDL_Surface *surface)
{
  SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 255, 255));
  Uint32 red = SDL_MapRGB(surface->format, 220, 30, 30);
  Uint32 gray = SDL_MapRGB(surface->format, 120, 120, 120);


  bresenham(surface, 0, SCREEN_HEIGHT / 2,
            SCREEN_WIDTH - 1, SCREEN_HEIGHT / 2, gray);
  bresenham(surface, SCREEN_WIDTH / 2, 0,
            SCREEN_WIDTH / 2, SCREEN_HEIGHT - 1, gray);

 
  Point poly[MAX_VERTICES];
  for (int i = 0; i < vertex_count; ++i) {
    double a = PI / 2 + PI / vertex_count + 2 * PI * i / vertex_count;
    poly[i] = {220 * std::cos(a), 220 * std::sin(a)};
  }

  double mu = calc_mu();
  for (int f = 0; f < figure_count; ++f) {
    Point inner[MAX_VERTICES];
    for (int i = 0; i < vertex_count; ++i) {
      Point a = poly[i];
      Point b = poly[(i + 1) % vertex_count];
      draw_segment(surface, a, b, red);

      // параметрическое уравнение отрезка: P = (1 - mu) * A + mu * B
      inner[i] = {(1 - mu) * a.x + mu * b.x, (1 - mu) * a.y + mu * b.y};
    }
    std::copy(inner, inner + vertex_count, poly);
  }


  Uint32 blue = SDL_MapRGB(surface->format, 0, 90, 255);
  int px = static_cast<int>(std::lround(pivot.x));
  int py = static_cast<int>(std::lround(pivot.y));
  bresenham(surface, px - 8, py, px + 8, py, blue);
  bresenham(surface, px, py - 8, px, py + 8, blue);
}
