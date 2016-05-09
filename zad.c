#include <X11/Xlib.h>
#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MLD 10000000000.0
Display *mydisplay;
Window mywindow;
XSetWindowAttributes mywindowattributes;
XGCValues mygcvalues;
GC mygc;
Visual *myvisual;
int mydepth;
int myscreen;
Colormap mycolormap;
XColor mycolor,mycolor1,dummy;
XEvent myevent;
int p;
char *myname;

double T;
	struct timespec t1, t2;

int wyswietl(char *adres)
{
   mydisplay = XOpenDisplay(adres);
   myscreen = DefaultScreen(mydisplay);
   myvisual = DefaultVisual(mydisplay,myscreen);
   mydepth = DefaultDepth(mydisplay,myscreen);
   mywindowattributes.background_pixel = XWhitePixel(mydisplay,myscreen);
   mywindowattributes.override_redirect = False;
   
   mywindow = XCreateWindow(mydisplay,XRootWindow(mydisplay,myscreen),
                            100,100,500,500,10,mydepth,InputOutput,
                            myvisual,CWBackPixel|CWOverrideRedirect,
                            &mywindowattributes);

   XSelectInput(mydisplay,mywindow,ExposureMask|KeyPressMask);
   mycolormap = DefaultColormap(mydisplay,myscreen);
         XAllocNamedColor(mydisplay,mycolormap,"cyan",&mycolor,&dummy);
         XAllocNamedColor(mydisplay,mycolormap,"red",&mycolor1,&dummy);                 
         myname="Pierwszy";
   XStoreName(mydisplay,mywindow,myname);   
   XMapWindow(mydisplay,mywindow);
   mygc = DefaultGC(mydisplay,myscreen);
   while (1)
   {
      XNextEvent(mydisplay,&myevent);
      switch (myevent.type)
      {
         case Expose:
              XSetForeground(mydisplay,mygc,mycolor.pixel);
              XFillRectangle(mydisplay,mywindow,mygc,100,100,300,300);
              XSetForeground(mydisplay,mygc,mycolor1.pixel);
              XSetFunction(mydisplay,mygc,GXcopy);
              XSetLineAttributes(mydisplay,mygc,10,LineSolid,CapProjecting,JoinMiter);
              XDrawLine(mydisplay,mywindow,mygc,100,100,400,400);
              XDrawLine(mydisplay,mywindow,mygc,100,400,400,100);
              XFlush(mydisplay);
              clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
              break;
         case KeyPress:
              XCloseDisplay(mydisplay);
              clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
              T = (t2.tv_sec/MLD)-(t1.tv_sec/MLD);
              printf("%s: %f\n", adres, T);
              exit(0); 
      }
  }
}

int main(int argc, char **argv) {
	int i;
	for(i=1; i<argc; i++) {
		if(fork() == 0) {
			wyswietl(argv[i]);
		}
	}
	return 0;
}
