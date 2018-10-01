/* show tws in more readable form */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN 1000000
#define DIRMAX 10

unsigned char tws[MAXLEN];
int twslen;
int level;
int movelen[]={0,4,4};
int movelenlower[]={0,1,2};
int starttime;

void error(char *s) { puts(s); exit(1); }

void loadtws(char *filename) {
	FILE *f=fopen(filename,"rb");
	if(!f) error("couldn't open tws file");
	fseek(f,0,SEEK_END);
	twslen=ftell(f);
	if(twslen>=MAXLEN) error("tws file too large, increase MAXLEN and recompile");
	fseek(f,0,SEEK_SET);
	if(twslen!=fread(tws,1,twslen,f)) error("couldn't read entire tws file");
	if(fclose(f)) error("couldn't close tws file after reading");
	puts("tws file loaded");
}

unsigned get4u(unsigned char *p) { return p[0]+(p[1]<<8)+(p[2]<<16)+(p[3]<<24); }
int get2u(unsigned char *p) { return p[0]+(p[1]<<8); }

char getdir(int v) {
	switch(v&3) {
	case 0: return 'U';
	case 1: return 'L';
	case 2: return 'D';
	case 3: return 'R';
	}
	return 'z';
}

char dirs[8][3]={"U","L","D","R","UL","DL","UR","DR"};

char *getdirstr(int v) { return dirs[v&7]; }

void show(char *arg) {
	int pos,lv,at;
	unsigned offset;
	unsigned char b;
	level=strtol(arg,0,10);
	if(level<1) error("level must be at least 1");
	pos=8;
	while(pos<twslen) {
		offset=get4u(tws+pos);
		lv=get2u(tws+pos+4);
		if(lv==level) {
			at=20;
			while(at<offset+4) {
				b=tws[pos+at];
				if((b&3)==0) {
					printf("%c %c %c\n",getdir(b>>2),getdir(b>>4),getdir(b>>6));
					at++;
				} else if((b&3)==1) {
					printf("(wait %d) %s\n",b>>5,getdirstr(b>>2));
					at++;
				} else if((b&3)==2) {
					printf("(wait %d) %s\n",(b>>5)+tws[pos+at+1]*8,getdirstr(b>>2));
					at+=2;
				} else error("not implemented yet");
			}
			at=get4u(tws+pos+16);
			printf("solution uses %d steps and %.2f seconds (=> %.2f)\n",at,at*0.05,starttime+0.95-at*0.05);
		}
		pos+=offset+4;
	}
}

void usage() {
	puts("usage: tws tws-file level-number\n");
	puts("  tws-file: tile world solution file");
	puts("  level-number: which level to show solution for");
}

int main(int argc,char **argv) {
	puts("twsdump v1.0\n");
	if(argc<3) { usage(); return 0; }
	starttime=1000;
	if(argc>3) starttime=strtol(argv[3],0,10);
	loadtws(argv[1]);
	show(argv[2]);
	return 0;
}
