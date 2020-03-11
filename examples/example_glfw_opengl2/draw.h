#pragma once


#include <iostream>
#include <fstream>
using namespace std;
// int nx = 1200;
// int ny = 600;
// int nx = 600;
// int ny = 300;
int nx = 300;
int ny = 150;
int ns = 100;
int *framebuffer;
int display_w, display_h;

void RGB2Color(int &hex, const int &r, const int &g, const int &b);
void Color2RGB(const int &hex, int &r, int &g, int &b);
void DrawPixel(int x, int y, int c);
void DrawPixel(const int &x, const int &y, const int &r, const int &g, const int &b);
void DrawFrame();

void RGB2Color(int &c, const int &r, const int &g, const int &b)
{
    c = (r << 16) | (g << 8) | b;
}

void DrawPixel(int x, int y, int c)
{
    y = ny - y;
    framebuffer[y * display_w + x] = c;
}

void DrawPixel(const int &x, const int &y, const int &r, const int &g, const int &b)
{
    int c;
    RGB2Color(c, r, g, b);
    DrawPixel(x, y, c);
}

void Color2RGB(const int &c, int &r, int &g, int &b)
{
    r = (0xff << 16 & c) >> 16;
    g = (0xff << 8 & c) >> 8;
    b = 0xff & c;
}


void Framebuffer2File(int nx, int ny, int ns, int *fb, ofstream &outFile, float &progressDone)
{
    for (int j = (ny - 1); j >= 0; j--)
    {
        progressDone = float(ny - 1 - j) / (ny - 1);
        for (int i = 0; i < nx; i++)
        {
            int x = i;
            int y = ny - 1 - j;
            int c = fb[y * display_w + x];
            int r, g, b = 0;

            Color2RGB(c, r, g, b);

            outFile << r << " " << g << " " << b << endl;
        }
    }
}


//long GetCurrentTimeMs()
//{
//    timeval time;
//    gettimeofday(&time, NULL);
//    long millis = (time.tv_sec * 1000) + (time.tv_usec / 1000);
//    return millis;
//}