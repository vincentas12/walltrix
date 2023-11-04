#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include "config.h"

Display *display;
Window root, window;
GC gc;
Colormap colormap;

#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif

int previousChoice = -1;

int main()
{
  daemonize(); 

  init_x11();
  create_random_pattern_window();
  event_loop();

  XCloseDisplay(display);

  return 0;
}

void daemonize()
{
  pid_t pid, sid;

  pid = fork();
  if (pid < 0)
  {
    exit(EXIT_FAILURE);
  }
  if (pid > 0)
  {
    exit(EXIT_SUCCESS);
  }

  umask(0);

  sid = setsid();
  if (sid < 0)
  {
    exit(EXIT_FAILURE);
  }

  if ((chdir("/")) < 0)
  {
    exit(EXIT_FAILURE);
  }


  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}

void init_x11()
{
  display = XOpenDisplay(NULL);
  if (!display)
  {
    fprintf(stderr, "Error: couldn't open display\n");
    exit(1);
  }

  root = DefaultRootWindow(display);
  colormap = DefaultColormap(display, DefaultScreen(display));
}

void create_random_pattern_window()
{
  XSetWindowAttributes attrs;
  attrs.override_redirect = True;
  window = XCreateWindow(display, root, 0, 0,
                         DisplayWidth(display, 0), DisplayHeight(display, 0), 0,
                         CopyFromParent, InputOutput, CopyFromParent,
                         CWOverrideRedirect, &attrs);

  Atom below = XInternAtom(display, "_NET_WM_STATE_BELOW", False);
  Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);

  XChangeProperty(display, window, wm_state, wm_state, 32, PropModeReplace, (unsigned char *)&below, 1);

  XSelectInput(display, window, ExposureMask);
  XMapWindow(display, window);

  XLowerWindow(display, window);

  gc = XCreateGC(display, window, 0, NULL);
}

void fill_window_with_random_pattern()
{
  int width = DisplayWidth(display, 0);
  int height = DisplayHeight(display, 0);

  XSetForeground(display, gc, WhitePixel(display, 0));
  XFillRectangle(display, window, gc, 0, 0, width, height);

  int choice;

  do
  {
    choice = rand() % NUM_PATTERN_CHOICES;
  } while (choice == previousChoice);

  previousChoice = choice;

  switch (choice)
  {
  // 1 is random black and white and makes like qr code
  case 0:
    XSetForeground(display, gc, BlackPixel(display, 0));
    for (int i = 0; i < width; i += 20)
    {
      for (int j = 0; j < height; j += 20)
      {
        if (rand() % 2 == 0)
        {
          XFillRectangle(display, window, gc, i, j, 20, 20);
        }
      }
    }
    break;
  // 2 is random color pixel added to grid 
  case 1:
    for (int i = 0; i < width; i += 10)
    {
      for (int j = 0; j < height; j += 10)
      {
        XColor color;
        color.red = rand() % (1 << 16);
        color.green = rand() % (1 << 16);
        color.blue = rand() % (1 << 16);

        XAllocColor(display, colormap, &color);

        XSetForeground(display, gc, color.pixel);
        XFillRectangle(display, window, gc, i, j, 10, 10);
      }
    }
    break;
  //3 is flewors like wallpaper
  case 2:
    for (int i = 0; i < width; i += 80)
    {
      for (int j = 0; j < height; j += 80)
      {
        XSetForeground(display, gc, BlackPixel(display, 0));
        XFillRectangle(display, window, gc, i, j, 80, 80);

        if (rand() % 10 == 0)
        {
          int flowerSize = 40;
          int petalSize = 20;

          XPoint mainFlower[5];
          for (int k = 0; k < 360; k += 60)
          {
            mainFlower[0].x = i + 40;
            mainFlower[0].y = j + 40;
            mainFlower[1].x = i + 40 + flowerSize * cos(k * M_PI / 180.0);
            mainFlower[1].y = j + 40 + flowerSize * sin(k * M_PI / 180.0);
            mainFlower[2].x = i + 40 + flowerSize * cos((k + 30) * M_PI / 180.0);
            mainFlower[2].y = j + 40 + flowerSize * sin((k + 30) * M_PI / 180.0);

            XSetForeground(display, gc, rand() | (rand() << 16) | (rand() << 24));
            XFillPolygon(display, window, gc, mainFlower, 3, Convex, CoordModeOrigin);
          }

          XPoint leaf[3];
          for (int k = 0; k < 360; k += 45)
          {
            leaf[0].x = i + 40;
            leaf[0].y = j + 40;
            leaf[1].x = i + 40 + petalSize * cos(k * M_PI / 180.0);
            leaf[1].y = j + 40 + petalSize * sin(k * M_PI / 180.0);
            leaf[2].x = i + 40 + petalSize * cos((k + 22.5) * M_PI / 180.0);
            leaf[2].y = j + 40 + petalSize * sin((k + 22.5) * M_PI / 180.0);

            XSetForeground(display, gc, rand() | (rand() << 16) | (rand() << 24));
            XFillPolygon(display, window, gc, leaf, 3, Convex, CoordModeOrigin);
          }
        }
      }
    }
    break;
  }
}

void switch_to_next_screen()
{
  XRRScreenResources *res = XRRGetScreenResources(display, root);
  if (res->noutput > 1)
  {
    int current_screen = 0;

    for (int i = 0; i < res->noutput; i++)
    {
      if (res->outputs[i] == root)
      {
        current_screen = i;
        break;
      }
    }

    int next_screen = (current_screen + 1) % res->noutput;
    Window next_root = res->outputs[next_screen];

    XRRSetOutputPrimary(display, root, next_root);
    XRRFreeScreenResources(res);
  }
}
int interval = WINDOW_UPDATE_INTERVAL * 1000000;

void event_loop()
{
  while (1)
  {
    fill_window_with_random_pattern();
    usleep(interval);
    switch_to_next_screen();
  }
}