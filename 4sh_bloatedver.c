#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <glob.h>

char hst[256][4096]; int hc=0, hi=0;
void exec(char *b) {
    char *p=strdup(b), *a[256], *pipe_ptr=strchr(p,'|');
    if (pipe_ptr) {
        *pipe_ptr=0; int fd[2]; pipe(fd);
        if (fork()==0) { dup2(fd[1],1); close(fd[0]); close(fd[1]); exec(p); exit(0); }
        if (fork()==0) { dup2(fd[0],0); close(fd[0]); close(fd[1]); exec(pipe_ptr+1); exit(0); }
        close(fd[0]); close(fd[1]); wait(NULL); wait(NULL); free(p); return;
    }
    int i=0; char *t=strtok(p," "); if (!t) { free(p); return; }
    if (!strcmp(t,"exit")) exit(0);
    if (!strcmp(t,"cd")) { chdir(strtok(NULL," ")); free(p); return; }
    if (!strcmp(t,"export")) {
        char *kv=strtok(NULL," ");
        if (kv) { char *k=strtok(kv,"="), *v=strtok(NULL,""); if (k&&v) setenv(k,v,1); }
        free(p); return;
    }
    glob_t g; int first=1;
    while (t) {
        if (strpbrk(t,"*?[")) {
            glob(t,first?0:GLOB_APPEND,NULL,&g); first=0;
        } else {
            a[i++]=t;
        }
        t=strtok(NULL," ");
    }
    if (!first) { for(int j=0;j<g.gl_pathc;j++) a[i++]=g.gl_pathv[j]; }
    a[i]=NULL;
    if (fork()==0) { execvp(a[0],a); perror(a[0]); exit(1); }
    wait(NULL); if (!first) globfree(&g); free(p);
}

int main() {
    char b[4096], c, *home=getenv("HOME"), rf[512], hf[512], *ps;
    struct termios o, n;
    sprintf(rf,"%s/.4shrc",home); sprintf(hf,"%s/.4sh_history",home);
    FILE *f=fopen(rf,"r");
    while (f && fgets(b,4096,f)) { b[strcspn(b,"\n")]=0; exec(b); }
    if (f) fclose(f);
    f=fopen(hf,"r");
    while (f && hc<256 && fgets(hst[hc],4096,f)) hst[hc][strcspn(hst[hc++],"\n")]=0;
    if (f) fclose(f);
    tcgetattr(0,&o); n=o; n.c_lflag &= ~(ICANON|ECHO);
    while (1) {
        ps=getenv("PS1"); if (!ps) ps="4$ "; write(1,ps,strlen(ps));
        int l=0; hi=hc; memset(b,0,4096); tcsetattr(0,TCSANOW,&n);
        while (read(0,&c,1) && c!='\n') {
            if (c==27) {
                read(0,&c,1); read(0,&c,1);
                if (c=='A' || c=='B') {
                    while (l--) write(1,"\b \b",3);
                    hi += (c=='A'?-1:1); if (hi<0) hi=0; if (hi>hc) hi=hc;
                    strcpy(b, hi<hc?hst[hi]:""); l=strlen(b); write(1,b,l);
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
