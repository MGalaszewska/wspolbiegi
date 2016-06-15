#include <X11/Xlib.h>
#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>

#define MINSIZE 60
#define MAXSIZE 80
#define SIZE 16
//#define shm_klucz 133
//#define klucz 13

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
int p;
char *myname;

int i, j, k, amount;
int sprawdzany;

struct buffer { int x; int y; int pid; int prev; };
int p;
int xw, yw, xw1, yw1;
struct buffer *bufw;
int bufsize=sizeof(struct buffer);

typedef enum { false, true } bool;

typedef struct player {
	int player_id;
	int points;
} player;

typedef struct circle {
	int x;
	int y;
	int size;
	int xn;
	int yn;
	int number;
	bool odwiedzony;
	int points;
	bool czerwony;
}circle;

//circle *circles;

int getrand(int min, int max) {
	return (rand()%(max-min)+min);
}

int start, koniec;

int semafory, pamiec;
int id;
char* adres;
const int SEM_ID = 9432;

pthread_t tid,tid1;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

struct sembuf bufor_zabierajacy_dostep1 = {0, -1, 0};
struct sembuf bufor_dajacy_dostep1 = {0, 1, 0};
struct sembuf bufor_zabierajacy_dostep2 = {1, -1, 0};
struct sembuf bufor_dajacy_dostep2 = {1, 1, 0};

//int nr_pid;
//key_t klucz_pamieci;

void end() {
	pthread_mutex_destroy(&lock);
}

void rysuj(circle *circles) {
	for(i=0; i<SIZE;i++) {
	  if(i==0) {
		circles[i].odwiedzony = true;
		circles[i].czerwony = true;
		XSetForeground(mydisplay,mygc,mycolor3.pixel);
		XFillArc(mydisplay, mywindow, mygc, circles[i].x-(circles[i].size/2), circles[i].y-(circles[i].size/2), circles[i].size, circles[i].size, 0, 360*64);
	} else {
		if(circles[i].odwiedzony == true) {
			XSetForeground(mydisplay,mygc,mycolor3.pixel);
		} else {
		XSetForeground(mydisplay,mygc,mycolor.pixel);
	}
		XFillArc(mydisplay, mywindow, mygc, circles[i].x-(circles[i].size/2), circles[i].y-(circles[i].size/2), circles[i].size, circles[i].size, 0, 360*64);
  }
  }
  XSetForeground(mydisplay,mygc,mycolor1.pixel);
  for(i=0; i<SIZE; i++) {
	  char napis[2];
	  sprintf(napis, "%d", circles[i].number);
	  XDrawString(mydisplay, mywindow, mygc, circles[i].xn, circles[i].yn, napis, strlen(napis));
  }
}

int punkt_startowy(int x, int y, circle *circles) {
	int start;
	for(i=0; i<SIZE; i++) {
		if(x >= circles[i].x-circles[i].size && x <= circles[i].x+circles[i].size
		&& y >= circles[i].y-circles[i].size && y <= circles[i].y+circles[i].size) {
			start = i;
			break;
		}
	} return start;
}

int punkt_koncowy(int x, int y, circle *circles) {
	int koniec;
	for(i=0; i<SIZE; i++) {
		if(x >= circles[i].x-circles[i].size && x <= circles[i].x+circles[i].size
		&& y >= circles[i].y-circles[i].size && y <= circles[i].y+circles[i].size) {
			koniec = i;
			break;
		}
	} return koniec;
	}
	
void oznaczony(circle *circles, int koniec, player gracz) {
	circles[koniec].odwiedzony = true;
	circles[koniec].czerwony = true;
	circles[gracz.player_id].points++;
}

int daj_punkty(player gracz, circle *circles) {
	return circles[gracz.player_id].points;
}

int odejmij_punkt(circle *circles, player gracz) {
	circles[gracz.player_id].points--;
	return circles[gracz.player_id].points;
}

void koniec_gry() {
	semctl(semafory, 0, IPC_RMID, 0);
    shmdt(adres);
    shmctl(pamiec, IPC_RMID, NULL);
    XCloseDisplay(mydisplay);
    remove("rysunek");
    end();
	exit(0);
}

void *reader(void *argum) {
   struct buffer *buf;
   int xr,yr,xr1,yr1;
   int fd;

   buf=(struct buffer *) malloc(bufsize);
   while (!(fd=open("rysunek",O_RDONLY,0700)));
   while (1) {  
      if (read(fd,buf,bufsize)>0) {
         if (((buf->pid==p)&&(argum!=NULL))||((buf->pid!=p)&&(argum==NULL))) {
            pthread_mutex_lock(&lock);
            XSetForeground(mydisplay,mygc,mycolor2.pixel);
            XSetLineAttributes(mydisplay,mygc,1,LineSolid,CapProjecting,JoinMiter);
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

int wyswietl(circle *circles, player gracz) {
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
   XSelectInput(mydisplay,mywindow,ExposureMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonMotionMask|ButtonReleaseMask);
   mycolormap = DefaultColormap(mydisplay,myscreen);
         XAllocNamedColor(mydisplay,mycolormap,"blue",&mycolor,&dummy);
         XAllocNamedColor(mydisplay,mycolormap,"green",&mycolor1,&dummy);
         XAllocNamedColor(mydisplay,mycolormap,"black",&mycolor2,&dummy);
         XAllocNamedColor(mydisplay,mycolormap,"red",&mycolor3,&dummy);
         myname="Meh";
   XStoreName(mydisplay,mywindow,myname);
   XMapWindow(mydisplay,mywindow);
   mygc = DefaultGC(mydisplay,myscreen);
   
   int xw,yw,xw1,yw1;
   int fdw;
   struct buffer *bufw;
   
   bufsize=sizeof(struct buffer);
   bufw=(struct buffer *) malloc(bufsize);
   p=getpid();
   
   fdw=open("rysunek",O_WRONLY|O_CREAT|O_APPEND,0700);
   pthread_create(&tid,NULL,reader,(void *) &p);
   pthread_create(&tid1,NULL,reader,NULL);
   
   while (1)
   {
      XNextEvent(mydisplay,&myevent);
      switch (myevent.type)
      {
		  case ButtonPress:
              xw1=myevent.xbutton.x;
              yw1=myevent.xbutton.y;
              bufw->x=xw1;
              bufw->y=yw1;
              bufw->prev=0;
              bufw->pid=p;
              write(fdw,bufw,bufsize);
              start = punkt_startowy(xw1, yw1, circles);
              break;
         
         case ButtonRelease:
			  xw1=myevent.xbutton.x;
              yw1=myevent.xbutton.y;
			  if(gracz.player_id == 1) {
				  koniec = punkt_koncowy(xw1, yw1, circles);
				  if(koniec == start+1 && circles[start].czerwony == true && circles[koniec].czerwony == false) {
					  oznaczony(circles, koniec, gracz);
					  if(koniec == SIZE-1) {
						  printf("Koniec gry.\n");
						  close(fdw);
						  koniec_gry();
					  }
				  }
				  else {
					  odejmij_punkt(circles, gracz);
				  }
				  rysuj(circles);
				  XFlush(mydisplay);
				  printf("gracz %d. Punkty: %d\n", gracz.player_id, circles[gracz.player_id].points);
				if(semop(semafory, &bufor_dajacy_dostep2, 1) == -1) {
					perror("Cos poszlo nie tak (operacja dajaca dostep1)\n");
					exit(1);
				}
				if(semop(semafory, &bufor_zabierajacy_dostep1, 1) == -1) {
					perror("Cos poszlo nie tak (operacja zabierajaca dostep1)\n");
					exit(1);
				}
				rysuj(circles);
			}
			else {
				koniec = punkt_koncowy(xw1, yw1, circles);
				if(koniec == start+1 && circles[start].czerwony == true && circles[koniec].czerwony == false) {
					  oznaczony(circles, koniec, gracz);
					  if(koniec == SIZE-1) {
						  printf("Koniec gry.\n");
						  close(fdw);
						  koniec_gry();
					  }
				  }
				  else {
					  odejmij_punkt(circles, gracz);
				  }
				rysuj(circles);
				XFlush(mydisplay);
				printf("gracz %d. Punkty: %d\n", gracz.player_id, circles[gracz.player_id].points);
				if(semop(semafory, &bufor_dajacy_dostep1, 1) == -1) {
					perror("Cos poszlo nie tak (operacja dajaca dostep1)\n");
					exit(1);
				}
				if(semop(semafory, &bufor_zabierajacy_dostep2, 1) == -1) {
					perror("Cos poszlo nie tak (operacja zabierajaca dostep2)\n");
					exit(1);
				}
				rysuj(circles);
			}
			  break;

         case MotionNotify:
              xw=myevent.xmotion.x;
              yw=myevent.xmotion.y;
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
			  rysuj(circles);
              XFlush(mydisplay);
              break;
              
			  case KeyPress:
			  if(myevent.xkey.keycode == 0x09) {
              remove("rysunek");
              koniec_gry();
		  }
      }
  }
}

int main(int argc, char **argv) {
	XInitThreads();
srand(time(NULL));

player gracz;
circle *adres;

gracz.player_id = 1;
gracz.points = 0;

pamiec = shmget(SEM_ID, 1024, 0777|IPC_CREAT|IPC_EXCL);

if(pamiec == -1){
	gracz.player_id = 2;
	gracz.points = 0;
}	

pamiec = shmget(SEM_ID, 1024, 0777|IPC_CREAT);
adres = shmat(pamiec, 0, 0);
semafory = semget(SEM_ID, 2, 0777|IPC_CREAT);
if(semafory == -1) {
	perror("Blad podczas tworzenia semaforow\n");
	exit(1);
}
if(semctl(semafory, 0, SETVAL, (int)0) == -1) {
	perror("Cos poszlo nie tak(semctl 1)\n");
	exit(1);
}
if(semctl(semafory, 1, SETVAL, (int)0) == -1) {
	perror("Cos poszlo nie tak(semctl 2)\n");
	exit(1);
}

if(gracz.player_id == 1) {
  int x=0;
  int y=0;
  i=0;
	while(i<SIZE) {
	  for(j=0; j<SIZE/4;j++) {
		  for(k=0; k<SIZE/4; k++) {
			  int size = getrand(MINSIZE, MAXSIZE);
			  adres[i].x = adres[i].xn = getrand(x+size/2, x+150-size);
			  adres[i].y = adres[i].yn = getrand(y+size/2, y+150-size);
			  adres[i].size = size;
			  adres[i].number = i;
			  adres[i].points = 0;
			  adres[i].odwiedzony = false;
			  adres[i].czerwony = false;
			  i++;
			  x += 150;
		  } y += 150; x=0;
	  }
    }
    /*
    for(i=0; i<SIZE; i++) {
		int temp = adres[i].number;
		int random = rand()%SIZE;
		adres[i].number = adres[random].number;
		adres[random].number = temp;
	} */
	wyswietl(adres, gracz);
}
else {
	wyswietl(adres, gracz);
}

if(semctl(semafory, 1, IPC_RMID, 0) == -1) {
	perror("Cos poszlo nie tak (semctl2)\n");
	exit(1);
}
	return 0;
}
