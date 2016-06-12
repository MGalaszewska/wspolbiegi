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
#define shm_klucz 133
#define klucz 13

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

int id;

typedef struct player {
	int id;
	int points;
} player;

typedef struct circle {
	int x;
	int y;
	int size;
	int xn;
	int yn;
	int number;
}circle;

typedef struct all {
	circle *circles;
	player *players;
} all;

int getrand(int min, int max) {
	return (rand()%(max-min)+min);
}

int semafory, pamiec;

char* adres;
const int SEM_ID = 9432;

pthread_t tid,tid1;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

void end() {
	pthread_mutex_destroy(&lock);
}
struct sembuf bufor_zabierajacy_dostep1 = {0, -1, 0};
struct sembuf bufor_dajacy_dostep1 = {0, 1, 0};
struct sembuf bufor_zabierajacy_dostep2 = {1, -1, 0};
struct sembuf bufor_dajacy_dostep2 = {1, 1, 0};

int nr_pid;
key_t klucz_pamieci;

int sprawdz_czy_dotkniety(int id, player *gracz, int x, int y, circle *circles) {
	for(i=0; i<SIZE; i++) {
		if(x >= circles[i].x-circles[i].size && x <= circles[i].x+circles[i].size
		&& y >= circles[i].y-circles[i].size && y <= circles[i].y+circles[i].size) {
			gracz[id].points+=1;
			XSetForeground(mydisplay,mygc,mycolor3.pixel);
			XFillArc(mydisplay, mywindow, mygc, circles[i].x-(circles[i].size/2), circles[i].y-(circles[i].size/2), circles[i].size, circles[i].size, 0, 360*64);
			char napis[2];
		    sprintf(napis, "%d", i);
		    XSetForeground(mydisplay,mygc,mycolor1.pixel);
		    XDrawString(mydisplay, mywindow, mygc, circles[i].xn, circles[i].yn, napis, strlen(napis));
		    circles[i].number = i;
			break;
		}
	}
	return gracz[id].points;
}

void *reader(void *argum) {
   struct buffer *buf;
   int xr,yr,xr1,yr1;
   int fd;

   buf=(struct buffer *) malloc(bufsize);
   while (!(fd=open("rysunek",O_RDONLY,0700)));
//   printf("Plik otwarty do odczytu\n");
   while (1) {  
      if (read(fd,buf,bufsize)>0) {
//         printf("Odczytano wspolrzedne: %5d, %5d\n",buf->x,buf->y);
//         printf("pid= %5d p= %5d argum= %8d\n",buf->pid,p,(int)argum);
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

void rysuj_plansze(circle *circles) {
	int x=0;
  int y=0;
  i=0;
	while(i<SIZE) {
	  for(j=0; j<SIZE/4;j++) {
		  for(k=0; k<SIZE/4; k++) {
			  int size = getrand(MINSIZE, MAXSIZE);
			  circles[i].x = circles[i].xn = getrand(x+size/2, x+150-size);
			  circles[i].y = circles[i].yn = getrand(y+size/2, y+150-size);
			  circles[i].size = size;
			  i++;
			  x += 150;
		  } y += 150; x=0;
	  }
    }
}

int wyswietl(all *adres, int id) {
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
//   printf("Rozmiar bufora: %5d\n",bufsize);
   bufw=(struct buffer *) malloc(bufsize);
   p=getpid();
   
   fdw=open("rysunek",O_WRONLY|O_CREAT|O_APPEND,0777);
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
//              printf("button: %3d  %3d\n",xw1,yw1);
              bufw->x=xw1;
              bufw->y=yw1;
              bufw->prev=0;
              bufw->pid=p;
              write(fdw,bufw,bufsize);
              break;
         
         case ButtonRelease:
			  if(id == 0) {
				if(semop(semafory, &bufor_dajacy_dostep2, 1) == -1) {
					perror("Cos poszlo nie tak (operacja dajaca dostep1)\n");
					exit(1);
				}
				if(semop(semafory, &bufor_zabierajacy_dostep1, 1) == -1) {
					perror("Cos poszlo nie tak (operacja zabierajaca dostep1)\n");
					exit(1);
				}
			}
			else {
				if(semop(semafory, &bufor_dajacy_dostep1, 1) == -1) {
					perror("Cos poszlo nie tak (operacja dajaca dostep1)\n");
					exit(1);
				}
				if(semop(semafory, &bufor_zabierajacy_dostep2, 1) == -1) {
					perror("Cos poszlo nie tak (operacja zabierajaca dostep2)\n");
					exit(1);
				}
			}
			printf("%d\n", sprawdz_czy_dotkniety(id, adres->players, xw, yw, adres->circles));
			  break;

         case MotionNotify:
              xw=myevent.xmotion.x;
              yw=myevent.xmotion.y;
//              printf("motion: %3d  %3d\n",xw,yw);
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
			  for(i=0; i<SIZE;i++) {
				  if(i==0) {
					  XSetForeground(mydisplay,mygc,mycolor3.pixel);
					XFillArc(mydisplay, mywindow, mygc, adres->circles[i].x-(adres->circles[i].size/2), adres->circles[i].y-(adres->circles[i].size/2), adres->circles[i].size, adres->circles[i].size, 0, 360*64);
				} else {
				  XSetForeground(mydisplay,mygc,mycolor.pixel);
				  XFillArc(mydisplay, mywindow, mygc, adres->circles[i].x-(adres->circles[i].size/2), adres->circles[i].y-(adres->circles[i].size/2), adres->circles[i].size, adres->circles[i].size, 0, 360*64);
			  }
			  }
			  XSetForeground(mydisplay,mygc,mycolor1.pixel);
			  for(i=0; i<SIZE; i++) {
				  char napis[2];
				  sprintf(napis, "%d", i);
				  XDrawString(mydisplay, mywindow, mygc, adres->circles[i].xn, adres->circles[i].yn, napis, strlen(napis));
				  adres->circles[i].number = i;
			  }
              XSetForeground(mydisplay,mygc,mycolor2.pixel);
              XFlush(mydisplay);
              break;
              
			  case KeyPress:
			  if(myevent.xkey.keycode == 0x09) {
			  //free(adres);
			  semctl(semafory, 0, IPC_RMID, 0);
			  shmdt(adres);
			  shmctl(pamiec, IPC_RMID, 0);
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
XInitThreads();
srand(time(NULL));

all *adres;
id=0;

pamiec = shmget(SEM_ID, 2048, 0777|IPC_CREAT|IPC_EXCL);

if(pamiec == -1){
	id++;
}	

pamiec = shmget(SEM_ID, 1024, 0777|IPC_CREAT);
adres = shmat(pamiec, 0, 0);

adres->circles = (circle*)malloc(sizeof(circle)*SIZE);
adres->players = (player*)malloc(sizeof(player)*2);

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

if(id == 0) {
	int x=0;
  int y=0;
  i=0;
	while(i<SIZE) {
	  for(j=0; j<SIZE/4;j++) {
		  for(k=0; k<SIZE/4; k++) {
			  int size = getrand(MINSIZE, MAXSIZE);
			  adres->circles[i].x = adres->circles[i].xn = getrand(x+size/2, x+150-size);
			  adres->circles[i].y = adres->circles[i].yn = getrand(y+size/2, y+150-size);
			  adres->circles[i].size = size;
			  i++;
			  x += 150;
		  } y += 150; x=0;
	  }
    }
	wyswietl(adres, id);
} else {
wyswietl(adres, id);
}

	if(semop(semafory, &bufor_zabierajacy_dostep2, 1) == -1) {
		perror("Cos poszlo nie tak (operacja zabierajaca2)");
		exit(1);
	}

if(semctl(semafory, 1, IPC_RMID, 0) == -1) {
	perror("Cos poszlo nie tak (semctl2)\n");
	exit(1);
}
	return 0;
}
