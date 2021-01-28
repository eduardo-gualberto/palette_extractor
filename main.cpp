// g++ main.cpp -o main `pkg-config --cflags --libs opencv`

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <math.h>

#define N 8

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

int main(int argc, char **argv)
{
  //ARGS SETUP
  if (argc != 3)
  {
    std::cout << "Parameters required: \n1. number os colors to include in the palet" << std::endl
              << "2. picture to perform work upon." << std::endl;
    return 0;
  }
  int K = atoi(argv[1]);

  //IMAGE SETUP
  cv::Mat picture = cv::imread(argv[2], cv::IMREAD_COLOR);
  cv::Mat result;

  //COUNT COLORS AND ADD TO Ni DIVISION, i IS THE DIVISION OF THE i-th PIXEL
  Color count[N];
  for (int i = 0; i < N; i++)
    count[i].count = count[i].h_sum = count[i].s_sum = count[i].v_sum = 0;

  count_color(picture, count);

  /////////////////////////////////////////////////////////////////////////////////
  for (int i = 0; i < N; i++)
    printf("%d ", count[i].count);
  printf("\n");

  Pixel mean = get_hsv_color_mean(count[BLUE]);
  Pixel rgb_mean = hsv2rgb(mean);
  printf("BLUE MEAN COLOR: (h,s,v) = (%d,%d,%d)\n", mean.x, mean.y, mean.z);
  printf("BLUE MEAN COLOR: (r,g,b) = (%d,%d,%d)\n", rgb_mean.x, rgb_mean.y, rgb_mean.z);
  printf("BLUE MEAN COLOR: hex = %X%X%X\n", rgb_mean.x, rgb_mean.y, rgb_mean.z);

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