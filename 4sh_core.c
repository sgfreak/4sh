#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>

char hst[256][4096]; int hc=0, hi=0;
void exec(char *b) {
    char *a[256], *p=strdup(b); int i=0;
    if (!(a[i++]=strtok(p," "))) { free(p); return; }
    while ((a[i++]=strtok(NULL," ")));
    if (!strcmp(a[0],"exit")) exit(0);
    if (!strcmp(a[0],"cd")) { chdir(a[1]); free(p); return; }
    if (fork()==0) { execvp(a[0],a); perror(a[0]); exit(1); }
    wait(NULL); free(p);
}

int main() {
    char b[4096], c, *home=getenv("HOME"), rf[512], hf[512];
    struct termios o, n;
    sprintf(rf,"%s/.4shrc",home); sprintf(hf,"%s/.4sh_history",home);
    FILE *f=fopen(rf,"r");
    while (f && fgets(b,4096,f)) { b[strcspn(b,"\n")]=0; exec(b); }
    if (f) fclose(f);
    f=fopen(hf,"r");
    while (f && hc<256 && fgets(hst[hc],4096,f)) hst[hc][strcspn(hst[hc++],"\n")]=0;
    if (f) fclose(f);
    tcgetattr(0,&o); n=o; n.c_lflag &= ~(ICANON|ECHO);
    while (write(1,"4$ ",3)) {
        int l=0; hi=hc; memset(b,0,4096); tcsetattr(0,TCSANOW,&n);
        while (read(0,&c,1) && c!='\n') {
            if (c==27) {
                read(0,&c,1); read(0,&c,1);
                if (c=='A' || c=='B') {
                    while (l--) write(1,"\b \b",3);
                    hi += (c=='A'?-1:1);
                    if (hi<0) hi=0; if (hi>hc) hi=hc;
                    strcpy(b, hi<hc?hst[hi]:"");
                    l=strlen(b); write(1,b,l);
                }
            } else if (c==127 || c==8) {
                if (l>0) { write(1,"\b \b",3); b[--l]=0; }
            } else { write(1,&c,1); b[l++]=c; }
        }
        write(1,"\n",1); tcsetattr(0,TCSANOW,&o);
        if (l>0) {
            if (hc==0 || strcmp(hst[hc-1],b)) {
                strcpy(hst[hc++],b); f=fopen(hf,"a"); fprintf(f,"%s\n",b); fclose(f);
            }
            exec(b);
        }
    }
    return 0;
}
