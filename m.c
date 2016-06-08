#include <X11/Xlib.h>
#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>

const int SEM_ID = 9432;

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

int pamiec;
char *adres;

typedef enum { false, true } bool;

typedef struct circle {
	int x;
	int y;
	int size;
	int ID;
	bool Gracz_w_grze;
}circle;

int id;

void dostepni_gracze(circle *circles)
{
	int i;
	for(i = 0; i < 6; i++)
	{
		circles[i].Gracz_w_grze = false;
		circles[i].ID = i;
	}
}

int znajdz_wolne_miejsce(circle *circles) {
	int i;
	for(i=0; i<6; i++) {
		if(circles[i].Gracz_w_grze == false) {
			circles[i].Gracz_w_grze = true;
			break;
		}
	} return i;
}

void pokaz_wszystkich(circle *circles) {
	int i;
	for(i=0; i<6; i++) {
		if(circles[i].Gracz_w_grze == true) {
			circles[id].x = 100;
			circles[id].y = 100;
			circles[id].size = 60;
			XSetForeground(mydisplay,mygc,mycolor2.pixel);
			XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
			XFlush(mydisplay);
		}
	}
}

int wyswietl(circle *circles, int id) {
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
         myname="Berek";
   XStoreName(mydisplay,mywindow,myname);
   XMapWindow(mydisplay,mywindow);
   mygc = DefaultGC(mydisplay,myscreen);
   circles = malloc(sizeof(circle));
   while (1)
   {
	   if(XPending(mydisplay) > 0) {
		  XNextEvent(mydisplay,&myevent);
		  switch (myevent.type)
		  {
			 case Expose:
				  XSetFunction(mydisplay,mygc,GXcopy);
				  XSetForeground(mydisplay,mygc,mycolor.pixel);
				  circles[id].x = 100;
				  circles[id].y = 100;
				  circles[id].size = 60;
				  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
				  pokaz_wszystkich(circles);
				  XFlush(mydisplay);
				  break;
				  
			  case KeyPress:
				  //printf("%x\n", myevent.xkey.keycode);
				  //EXIT
				  if(myevent.xkey.keycode == 0x09) {
					  XCloseDisplay(mydisplay);
					  shmdt(adres);
					  shmctl(pamiec, IPC_RMID, 0);
					  exit(0);
				  }
				  //UP
				  if(myevent.xkey.keycode == 0x6f) {
					  XSetForeground(mydisplay,mygc,mycolor1.pixel);
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XSetForeground(mydisplay,mygc,mycolor.pixel);
					  circles[id].y -= 5;
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XFlush(mydisplay);
				  }
				  //RIGHT
				  if(myevent.xkey.keycode == 0x72) {
					  XSetForeground(mydisplay,mygc,mycolor1.pixel);
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XSetForeground(mydisplay,mygc,mycolor.pixel);
					  circles[id].x += 5;
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XFlush(mydisplay);
				  }
				  //LEFT
				  if(myevent.xkey.keycode == 0x71) {
					  XSetForeground(mydisplay,mygc,mycolor1.pixel);
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XSetForeground(mydisplay,mygc,mycolor.pixel);
					  circles[id].x -= 5;
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XFlush(mydisplay);
				  }
				  //DOWN
				  if(myevent.xkey.keycode == 0x74) {
					  XSetForeground(mydisplay,mygc,mycolor1.pixel);
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XSetForeground(mydisplay,mygc,mycolor.pixel);
					  circles[id].y += 5;
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XFlush(mydisplay);
				  }			  
			  break;
		}
	}
  }
}

int main(int argc, char **argv) {
	
circle *adres;

key_t klucz_pamieci = ftok("wejscie", 7);

id = 0;
pamiec = shmget(SEM_ID, 1024, 0777|IPC_CREAT|IPC_EXCL);

if(pamiec == -1) {
	id++;
}

pamiec = shmget(SEM_ID, 1024, 0777|IPC_CREAT);

adres = shmat(pamiec, 0, 0);

if(id == 0) {
	dostepni_gracze(adres);
}
	id = znajdz_wolne_miejsce(adres);
	printf("%d\n", id);
	wyswietl(adres, id);
	
	return 0;
}
