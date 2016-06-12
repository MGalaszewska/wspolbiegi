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

const int SEM_ID = 9433;

Display *mydisplay;
Window mywindow;
XSetWindowAttributes mywindowattributes;
XGCValues mygcvalues;
GC mygc;
Visual *myvisual;
int mydepth;
int myscreen;
Colormap mycolormap;
XColor mycolor,mycolor1, mycolor2, mycolor3, dummy;
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
	bool czy_berek;
}circle;

int id;

void dostepni_gracze(circle *circles)
{
	int i;
	for(i = 0; i < 6; i++)
	{
		circles[i].ID = i;
		circles[i].Gracz_w_grze = false;
		circles[i].czy_berek = false;
		if(circles[i].ID == 0) {
			circles[i].czy_berek = true;
		}
	}
}

int znajdz_wolne_miejsce(circle *circles) {
	int i;
	for(i=0; i<6; i++) {
		circles[i].ID = i;
		if(circles[i].Gracz_w_grze == false) {
			circles[i].Gracz_w_grze = true;
			break;
		}
	}
	  return i;
}

void pokaz_wszystkich(circle *circles, int id) {
	int i;
	for(i=0; i<6; i++) {
		if(circles[i].Gracz_w_grze == true) {
//			if(i != id) {
printf("%d %d %d\n", id, circles[i].x, circles[i].y);
			  if(circles[i].czy_berek == true) { XSetForeground(mydisplay,mygc,mycolor3.pixel); }
			  else { XSetForeground(mydisplay,mygc,mycolor2.pixel); }
			  XFillArc(mydisplay, mywindow, mygc, circles[i].x, circles[i].y, circles[i].size, circles[i].size, 0, 360*64);
			  XFlush(mydisplay);
//			}
		}
	}
}

int ilosc_graczy(circle *circles) {
	int i, ilosc=0;
	for(i=0; i<6; i++) {
		if(circles[i].Gracz_w_grze == true) {
			ilosc++;
//			printf("%d\n", ilosc);
		}
	} return ilosc;
}

void sprawdz(circle *circles, int id) {
	int i;
	for(i=0; i<6; i++) {
	   if(circles[id].Gracz_w_grze == true && i != id && ilosc_graczy(circles)>1) {
		if(circles[id].x >= circles[i].x-circles[i].size && circles[id].x <= circles[i].x+circles[i].size &&
		   circles[id].y >= circles[i].y-circles[i].size && circles[id].y <= circles[i].y+circles[i].size) {
			usleep(7500);
//			XSetForeground(mydisplay, mygc, mycolor3.pixel);
//			XFillArc(mydisplay, mywindow, mygc, circles[i].x, circles[i].y, circles[i].size, circles[i].size, 0, 360*64);
			circles[i].czy_berek = true;
			circles[id].czy_berek = false;
		}
			break;
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
	 XAllocNamedColor(mydisplay,mycolormap,"red",&mycolor3,&dummy);
         myname="Berek";
   XStoreName(mydisplay,mywindow,myname);
   XMapWindow(mydisplay,mywindow);
   mygc = DefaultGC(mydisplay,myscreen);
   while (1)
   {
	   if(XPending(mydisplay) > 0) {
		  XNextEvent(mydisplay,&myevent);
		  switch (myevent.type)
		  {
			 case Expose:
				  XSetFunction(mydisplay,mygc,GXcopy);
pokaz_wszystkich(circles, id);
if(circles[id].czy_berek == true) { XSetForeground(mydisplay,mygc,mycolor3.pixel); }
				  else { XSetForeground(mydisplay,mygc,mycolor.pixel); }
				  circles[id].x = 100;
				  circles[id].y = 100;
				  circles[id].size = 60;
//				  pokaz_wszystkich(circles, id);
				  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
				  //pokaz_wszystkich(circles, id);
//int i; for(i=0; i<6; i++) {printf("%d %d %d\n", circles[i].ID, circles[i].x, circles[i].y); }
				  XFlush(mydisplay);
				  break;

			  case KeyPress:
//				  printf("%x\n", myevent.xkey.keycode);
				  //EXIT
XSetForeground(mydisplay, mygc, mycolor1.pixel);
XFillRectangle(mydisplay, mywindow, mygc, 0, 0, 600, 600);
sprawdz(circles, id);
				  pokaz_wszystkich(circles, id);
				  if(myevent.xkey.keycode == 0x09) {
					  XCloseDisplay(mydisplay);
					  shmdt(adres);
					  shmctl(pamiec, IPC_RMID, NULL);
					  exit(0);
				  }
				  //UP
				  if(myevent.xkey.keycode == 0x6f) {//0x62) {
					  sprawdz(circles, id);
					  XSetForeground(mydisplay,mygc,mycolor1.pixel);
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  if(circles[id].czy_berek == true) { XSetForeground(mydisplay,mygc,mycolor3.pixel); }
					  else {XSetForeground(mydisplay,mygc,mycolor.pixel); }
					  circles[id].y -= 5;
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XFlush(mydisplay);
				  }
				  //RIGHT
				  if(myevent.xkey.keycode == 0x72) {//0x66) {
					  sprawdz(circles, id);
					  XSetForeground(mydisplay,mygc,mycolor1.pixel);
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  if(circles[id].czy_berek == true) { XSetForeground(mydisplay,mygc,mycolor3.pixel); }
					  else {XSetForeground(mydisplay,mygc,mycolor.pixel); }
					  circles[id].x += 5;
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XFlush(mydisplay);
				  }
				  //LEFT
				  if(myevent.xkey.keycode == 0x71) {//0x64) {
					  sprawdz(circles, id);
					  XSetForeground(mydisplay,mygc,mycolor1.pixel);
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  if(circles[id].czy_berek == true) { XSetForeground(mydisplay,mygc,mycolor3.pixel); }
					  else {XSetForeground(mydisplay,mygc,mycolor.pixel); }
					  circles[id].x -= 5;
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  XFlush(mydisplay);
				  }
				  //DOWN
				  if(myevent.xkey.keycode == 0x74) {//0x68) {
					  sprawdz(circles, id);
					  XSetForeground(mydisplay,mygc,mycolor1.pixel);
					  XFillArc(mydisplay, mywindow, mygc, circles[id].x, circles[id].y, circles[id].size, circles[id].size, 0, 360*64);
					  if(circles[id].czy_berek == true) { XSetForeground(mydisplay,mygc,mycolor3.pixel); }
					  else {XSetForeground(mydisplay,mygc,mycolor.pixel); }
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
