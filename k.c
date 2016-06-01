#include <X11/Xlib.h>
#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define MINSIZE 60
#define MAXSIZE 80

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
int p;
char *myname;

int i, j, k, amount;

struct buffer { int x; int y; int pid; int prev; };
int p;
int xw, yw, xw1, yw1;
struct buffer *bufw;
int bufsize=sizeof(struct buffer);

typedef struct circle {
	int x;
	int y;
	int size;
	int xn;
	int yn;
}circle;

int getrand(int min, int max) {
	return (rand()%(max-min)+min);
}

circle c;
circle *circles;

pthread_t tid,tid1;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

void end() {
	pthread_mutex_destroy(&lock);
}

void *reader(void *argum) {
   struct buffer *buf;
   int xr,yr,xr1,yr1;
   int fd;

   buf=(struct buffer *) malloc(bufsize);
//   printf("Zglasza sie reader\n");   
   while (!(fd=open("rysunek",O_RDONLY,0700)));
   printf("Plik otwarty do odczytu\n");
   while (1) {  
      if (read(fd,buf,bufsize)>0) {
         printf("Odczytano wspolrzedne: %5d, %5d\n",buf->x,buf->y);
         printf("pid= %5d p= %5d argum= %8d\n",buf->pid,p,(int)argum);
         if (((buf->pid==p)&&(argum!=NULL))||((buf->pid!=p)&&(argum==NULL))) {
			printf("Probujemy zablokowac\n");
            pthread_mutex_lock(&lock);
            XSetForeground(mydisplay,mygc,mycolor2.pixel);
            XSetLineAttributes(mydisplay,mygc,1,LineSolid,CapProjecting,JoinMiter);
			printf("KOLOR\n");
            if (buf->prev==0) {
               xr1=buf->x;
               yr1=buf->y;
               XDrawPoint(mydisplay,mywindow,mygc,xr1,yr1);
               XFlush(mydisplay);
            }
            else {
               xr=buf->x;
               yr=buf->y;
               XDrawLine(mydisplay,mywindow,mygc,xr,yr,xr1,yr1);
               XFlush(mydisplay);
               xr1=xr;
               yr1=yr;
            }
            pthread_mutex_unlock(&lock);
         }
      }
   }
}


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
   XSelectInput(mydisplay,mywindow,ExposureMask|KeyPressMask|ButtonPressMask|ButtonMotionMask);
   mycolormap = DefaultColormap(mydisplay,myscreen);
         XAllocNamedColor(mydisplay,mycolormap,"blue",&mycolor,&dummy);
         XAllocNamedColor(mydisplay,mycolormap,"red",&mycolor1,&dummy);
         XAllocNamedColor(mydisplay,mycolormap,"black",&mycolor2,&dummy);
         myname="Meh";
   XStoreName(mydisplay,mywindow,myname);
   XMapWindow(mydisplay,mywindow);
   mygc = DefaultGC(mydisplay,myscreen);
   
   int xw,yw,xw1,yw1;
   int fdw;
   struct buffer *bufw;
   
   bufsize=sizeof(struct buffer);
   printf("Rozmiar bufora: %5d\n",bufsize);
   bufw=(struct buffer *) malloc(bufsize);
   p=getpid();
   
   fdw=open("rysunek",O_WRONLY|O_CREAT|O_APPEND,0700);
   printf("Plik otwarty do zapisu\n");
   pthread_create(&tid,NULL,reader,(void *) &p);
   printf("Moj czytelnik utworzony\n");
   pthread_create(&tid1,NULL,reader,NULL);
   printf("Jego czytelnik utworzony\n");
   
   while (1)
   {
      XNextEvent(mydisplay,&myevent);
      switch (myevent.type)
      {
		  case ButtonPress:
              xw1=myevent.xbutton.x;
              yw1=myevent.xbutton.y;
              printf("button: %3d  %3d\n",xw1,yw1);
              bufw->x=xw1;
              bufw->y=yw1;
              bufw->prev=0;
              bufw->pid=p;
              write(fdw,bufw,bufsize);
              break;

         case MotionNotify:
              xw=myevent.xmotion.x;
              yw=myevent.xmotion.y;
              printf("motion: %3d  %3d\n",xw,yw);
              bufw->x=xw;
              bufw->y=yw;
              bufw->prev=1;
              bufw->pid=p;
              write(fdw,bufw,bufsize);
              xw1=xw;
              yw1=yw;
              break;
		  
         case Expose:
              XSetFunction(mydisplay,mygc,GXcopy);
              amount = 16;
              int x=0;
              int y=0;
              i=0;
              circles = malloc(sizeof(circle)*amount);
              while(i<amount) {
				  for(j=0; j<4;j++) {
					  for(k=0; k<4; k++) {
						  int size = getrand(MINSIZE, MAXSIZE);
						  circles[i].x = circles[i].xn = getrand(x+size/2, x+150-size);
						  circles[i].y = circles[i].yn = getrand(y+size/2, y+150-size);
						  circles[i].size = size;
						  i++;
						  x += 150;
					  } y += 150; x=0;
				  }
			  }
			  XSetForeground(mydisplay,mygc,mycolor.pixel);
			  for(i=0; i<amount;i++) {
				  XFillArc(mydisplay, mywindow, mygc, circles[i].x-(circles[i].size/2), circles[i].y-(circles[i].size/2), circles[i].size, circles[i].size, 0, 360*64);
			  }
			  XSetForeground(mydisplay,mygc,mycolor1.pixel);
			  for(i=0; i<amount; i++) {
				  char napis[2];
				  sprintf(napis, "%d", i);
				  XDrawString(mydisplay, mywindow, mygc, circles[i].xn, circles[i].yn, napis, strlen(napis));
			  }
              XSetForeground(mydisplay,mygc,mycolor2.pixel);
              XFlush(mydisplay);
              break;
              
			  case KeyPress:
			  if(myevent.xkey.keycode == 0x09) {
			  free(circles);
              XCloseDisplay(mydisplay);
              close(fdw);
              remove("rysunek");
              end();
              exit(0);
		  }
      }
  }
}

int main(int argc, char **argv) {
srand(time(NULL));
			wyswietl();
	return 0;
}
