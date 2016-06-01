#include <X11/Xlib.h>
#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define LEFT 0x08fb
#define RIGHT 0x08fd
#define UP 0x08fc
#define DOWN 0x08fe
#define ESC 0x1b

Display *mydisplay;
Window mywindow;
XSetWindowAttributes mywindowattributes;
XGCValues mygcvalues;
GC mygc;
Visual *myvisual;
int mydepth;
int myscreen;
Colormap mycolormap;
XColor mycolor,mycolor1, mycolor2, dummy;
XEvent myevent;
char *myname;

typedef struct circle {
	int x;
	int y;
	int size;
	int xn;
	int yn;
}circle;

circle *circles;

int wyswietl() {
   mydisplay = XOpenDisplay("");
   myscreen = DefaultScreen(mydisplay);
   myvisual = DefaultVisual(mydisplay,myscreen);
   mydepth = DefaultDepth(mydisplay,myscreen);
   mywindowattributes.background_pixel = XWhitePixel(mydisplay,myscreen);
   mywindowattributes.override_redirect = False;
   mywindowattributes.backing_store = Always;
   mywindowattributes.bit_gravity = NorthWestGravity;
   mywindow = XCreateWindow(mydisplay,XRootWindow(mydisplay,myscreen),
                            100,100,600,600,10,mydepth,InputOutput,
                            myvisual,CWBackingStore|CWBackingPlanes|CWBitGravity|
                            CWBackPixel|CWOverrideRedirect,&mywindowattributes);
   XSelectInput(mydisplay,mywindow,ExposureMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonMotionMask);
   mycolormap = DefaultColormap(mydisplay,myscreen);
         XAllocNamedColor(mydisplay,mycolormap,"blue",&mycolor,&dummy);
         XAllocNamedColor(mydisplay,mycolormap,"white",&mycolor1,&dummy);
         XAllocNamedColor(mydisplay,mycolormap,"black",&mycolor2,&dummy);
         myname="Meh";
   XStoreName(mydisplay,mywindow,myname);
   XMapWindow(mydisplay,mywindow);
   mygc = DefaultGC(mydisplay,myscreen);
   circles = malloc(sizeof(circle));
   while (1)
   {
      XNextEvent(mydisplay,&myevent);
      switch (myevent.type)
      {
         case Expose:
              XSetFunction(mydisplay,mygc,GXcopy);
              XSetForeground(mydisplay,mygc,mycolor.pixel);
              circles[0].x = 100;
              circles[0].y = 100;
              circles[0].size = 60;
			  XFillArc(mydisplay, mywindow, mygc, circles[0].x, circles[0].y, circles[0].size, circles[0].size, 0, 360*64);
              XFlush(mydisplay);
              break;
              
			  case KeyPress:
			  //printf("%x\n", myevent.xkey.keycode);
			  //EXIT
			  if(myevent.xkey.keycode == 0x09) {
				  XCloseDisplay(mydisplay);
				  exit(0);
			  }
			  //UP
              if(myevent.xkey.keycode == 0x6f) {
				  XSetForeground(mydisplay,mygc,mycolor1.pixel);
				  XFillArc(mydisplay, mywindow, mygc, circles[0].x, circles[0].y, circles[0].size, circles[0].size, 0, 360*64);
				  XSetForeground(mydisplay,mygc,mycolor.pixel);
				  circles[0].y -= 5;
				  XFillArc(mydisplay, mywindow, mygc, circles[0].x, circles[0].y, circles[0].size, circles[0].size, 0, 360*64);
				  XFlush(mydisplay);
			  }
			  //RIGHT
			  if(myevent.xkey.keycode == 0x72) {
				  XSetForeground(mydisplay,mygc,mycolor1.pixel);
				  XFillArc(mydisplay, mywindow, mygc, circles[0].x, circles[0].y, circles[0].size, circles[0].size, 0, 360*64);
				  XSetForeground(mydisplay,mygc,mycolor.pixel);
				  circles[0].x += 5;
				  XFillArc(mydisplay, mywindow, mygc, circles[0].x, circles[0].y, circles[0].size, circles[0].size, 0, 360*64);
				  XFlush(mydisplay);
			  }
			  //LEFT
			  if(myevent.xkey.keycode == 0x71) {
				  XSetForeground(mydisplay,mygc,mycolor1.pixel);
				  XFillArc(mydisplay, mywindow, mygc, circles[0].x, circles[0].y, circles[0].size, circles[0].size, 0, 360*64);
				  XSetForeground(mydisplay,mygc,mycolor.pixel);
				  circles[0].x -= 5;
				  XFillArc(mydisplay, mywindow, mygc, circles[0].x, circles[0].y, circles[0].size, circles[0].size, 0, 360*64);
				  XFlush(mydisplay);
			  }
			  //DOWN
			  if(myevent.xkey.keycode == 0x74) {
				  XSetForeground(mydisplay,mygc,mycolor1.pixel);
				  XFillArc(mydisplay, mywindow, mygc, circles[0].x, circles[0].y, circles[0].size, circles[0].size, 0, 360*64);
				  XSetForeground(mydisplay,mygc,mycolor.pixel);
				  circles[0].y += 5;
				  XFillArc(mydisplay, mywindow, mygc, circles[0].x, circles[0].y, circles[0].size, circles[0].size, 0, 360*64);
				  XFlush(mydisplay);
			  }			  
			  break;
      }
  }
}

int main(int argc, char **argv) {
			wyswietl();
	return 0;
}
