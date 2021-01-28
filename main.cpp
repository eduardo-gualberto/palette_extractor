// g++ main.cpp -o main `pkg-config --cflags --libs opencv`

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <math.h>

#define N 8
#define paletteN 5

typedef cv::Point3_<uint8_t> Pixel;
enum COLOR_MASK
{
  RED,
  ORANGE,
  YELLOW,
  GREEN,
  CYAN,
  BLUE,
  PURPLE,
  PINK,
};
typedef struct color
{
  int count, h_sum, s_sum, v_sum;
} Color;

void count_color(cv::Mat img, Color *result);
Pixel get_hsv_color_mean(Color c);
Pixel hsv2rgb(Pixel p);
void kthBiggest(Color *v, int p, int q, int k);
float rearrange(Color *v, int p, int q);
void switch_values(Color *a, Color *b);

int main(int argc, char **argv)
{
  //ARGS SETUP
  if (argc != 2)
  {
    std::cout << "Parameter required: number os colors to include in the palet" << std::endl;
    return 0;
  }

  //IMAGE SETUP
  cv::Mat picture = cv::imread(argv[1], cv::IMREAD_COLOR);
  cv::Mat result;

  //COUNT COLORS AND ADD TO Ni DIVISION, i IS THE DIVISION OF THE i-th PIXEL
  Color count[N];
  for (int i = 0; i < N; i++)
    count[i].count = count[i].h_sum = count[i].s_sum = count[i].v_sum = 0;

  count_color(picture, count);

  Pixel mean = get_hsv_color_mean(count[BLUE]);
  Pixel rgb_mean = hsv2rgb(mean);

  kthBiggest(count, 0, N, 5);
  printf("%-17s%-17s%-17s\n\n", "HSV", "RGB", "HEX:");
  for (int i = 0; i < paletteN; i++)
  {
    Pixel hsv_mean = get_hsv_color_mean(count[i]);
    Pixel rgb_mean = hsv2rgb(hsv_mean);
    printf("(%3d, %3d, %3d)\t (%3d, %3d, %3d)  #%02X%02X%02X\n",
           hsv_mean.x, hsv_mean.y, hsv_mean.z,
           rgb_mean.x, rgb_mean.y, rgb_mean.z,
           rgb_mean.x, rgb_mean.y, rgb_mean.z);
  }
  printf("\n");

  system("exit");
  return 0;
}

int check_hue(Pixel &p, enum COLOR_MASK color)
{
  uint8_t p_hue = p.x;
  switch (color)
  {
  case RED:
    return p_hue > 0 && p_hue < 9 || p_hue > 170;
  case ORANGE:
    return p_hue >= 9 && p_hue < 18;
  case YELLOW:
    return p_hue >= 18 && p_hue < 31;
  case GREEN:
    return p_hue >= 31 && p_hue < 71;
  case CYAN:
    return p_hue >= 71 && p_hue < 94;
  case BLUE:
    return p_hue >= 94 && p_hue < 123;
  case PURPLE:
    return p_hue >= 123 && p_hue < 150;
  case PINK:
    return p_hue >= 150 && p_hue <= 170;
  }
}

void count_color(cv::Mat img, Color *result)
{
  cv::Mat hsv_img;
  cv::cvtColor(img, hsv_img, cv::COLOR_BGR2HSV);

  for (static int iColor = 0; iColor < N; iColor++)
  {
    hsv_img.forEach<Pixel>([result](Pixel &p, const int *i) -> void {
      if (check_hue(p, static_cast<COLOR_MASK>(iColor)))
      {
        result[iColor].count++;
        result[iColor].h_sum += p.x;
        result[iColor].s_sum += p.y;
        result[iColor].v_sum += p.z;
      }
    });
  }
}

Pixel get_hsv_color_mean(Color c)
{
  if (c.count == 0)
    return Pixel(0, 0, 0);
  else
    return Pixel(c.h_sum / c.count, c.s_sum / c.count, c.v_sum / c.count);
}

Pixel hsv2rgb(Pixel p)
{
  int H = p.x * 2;
  double S = (double)p.y / 255;
  double V = (double)p.z / 255;

  double C = V * S;
  double X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
  double m = V - C;

  double r, g, b;
  if (H >= 0 && H < 60)
    r = C, g = X, b = 0;
  else if (H >= 60 && H < 120)
    r = X, g = C, b = 0;
  else if (H >= 120 && H < 180)
    r = 0, g = C, b = X;
  else if (H >= 180 && H < 240)
    r = 0, g = X, b = C;
  else if (H >= 240 && H < 300)
    r = X, g = 0, b = C;
  else if (H >= 300 && H < 360)
    r = C, g = 0, b = X;

  Pixel rgb = Pixel((r + m) * 255, (g + m) * 255, (b + m) * 255);
  return rgb;
}

void kthBiggest(Color *v, int p, int q, int k)
{
  //certifica que existe mais que 0 elementos no array
  if (q - p <= 0 && k < q - p)
    return;

  //variaveis auxiliares para ajudar na seleção de qual parte do vetor continuar a procura pelo
  //elemento de posição 'k'
  int start = p, end = q;

  //laço que interrompe quando o k-ésimo elemento é encontrado
  int pivot = rearrange(v, start, end);
  while (pivot != k)
  {
    //condicionais para otimizar busca da função de rearranjo dos elementos do vetor
    if (pivot > k) //se o pivot achado vir depois de k, procurar no subvetor anterior a ele
      end = pivot;
    else //se o pivot achado vir antes de k, procurar no subvetor sucessor a ele
      start = pivot + 1;
    //--------------------

    //atualizar o pivot de acordo com o subvetor de v delimitado por 'start' e 'end'
    pivot = rearrange(v, start, end);
  }
  return;
}

float rearrange(Color *v, int p, int q)
{
  int random_pos = (rand() % (q - p)) + p;
  switch_values(&v[random_pos], &v[q - 1]);

  int j, i = p, pivot = v[q - 1].count;
  for (j = p; j < q - 1; j++)
    if (v[j].count > pivot)
      switch_values(&v[i++], &v[j]);
  switch_values(&v[i], &v[j]);
  return i;
}

void switch_values(Color *a, Color *b)
{
  Color aux = *a;
  *a = *b;
  *b = aux;
}