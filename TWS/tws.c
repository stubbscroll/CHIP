/* convert text solution into tile world solution file.
   the solution will be appended to the tws (if the level has no solution)
   or will overwrite (if the level has a solution).
   see solution.c in tileworld for a description of the tws format.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN 1000000
#define DIRMAX 10

unsigned char tws[MAXLEN];
unsigned char buf[MAXLEN];
int twslen;
int buflen;
int ruleset;
int ticks;
int level;
int starttime;
int movelen[]={0,4,4};
int movelenlower[]={0,1,2};

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

int unpushed[4];
int nunpushed;

int putmove2(int offset,int *dir,int *dur,int dirs) {
	int dirval,t;
	if(dirs==2) {
		if(dir[0]>dir[1]) t=dir[0],dir[0]=dir[1],dir[1]=t;
		if(dir[0]==0 && dir[1]==1) dirval=4;
		else if(dir[0]==1 && dir[1]==2) dirval=5;
		else if(dir[0]==0 && dir[1]==3) dirval=6;
		else if(dir[0]==2 && dir[1]==3) dirval=7;
		else dirval=-1;
	} else dirval=dir[0];
	if(dirval<0) error("illegal dir");
	if(offset<(1<<3) && dirval<8) {
		buf[buflen++]=1+(dirval<<2)+(offset<<5);
//		printf("output 1-byte dir %d offset %d\n",dir[0],offset);
	} else if(offset<(1<<11) && dirval<8) {
		buf[buflen++]=2+(dirval<<2)+((offset&255)<<5);
		buf[buflen++]=(offset>>3);
//		printf("output 2-byte dir %d offset %d\n",dir[0],offset);
	} else error("not supported yet");
	return dur[0];
}

void flushmove() {
	int i,dir2[1],dur2[1];
	dur2[0]=4;
	for(i=0;i<nunpushed;i++) {
		dir2[0]=unpushed[i];
		putmove2(3,dir2,dur2,1);
	}
	nunpushed=0;
}

int putmove(int offset,int *dir,int *dur,int dirs) {
	if(dir[0]==4) return offset+dur[0];
	if(offset) offset--;
	if(offset==3 && dir[0]<4 && dirs==1) {
		unpushed[nunpushed++]=dir[0];
		if(nunpushed==3) {
			buf[buflen++]=(unpushed[0]<<2)+(unpushed[1]<<4)+(unpushed[2]<<6);
			nunpushed=0;
//			printf("output triple move %d %d %d\n",unpushed[0],unpushed[1],unpushed[2]);
		}
		return dur[0];
	} else flushmove();
	return putmove2(offset,dir,dur,dirs);
}

void parsetxt(char *filename,unsigned char *buf) {
	char s[10000],t[20];
	int i,force,stepping,p,cnt,offset=0,dirs,dir[DIRMAX],dur[DIRMAX];
	unsigned seed;
	FILE *f=fopen(filename,"r");
	if(!f) error("couldn't open txt file");
	/* init buffer */
	ruleset=1; /* assume lynx if nothing is stated */
	starttime=1000;
	ticks=force=stepping=0;
	for(i=4;i<20;i++) buf[i]=0;
	buflen=20;
	nunpushed=0;
	/* defensive programming because i'm too lazy to find out if scanf %#s adds
	   null-terminator if string exceeds size limit */
	t[19]=0; s[9998]=s[9999]=0;
	/* parse preamble */
	while(fgets(s,9998,f)) {
		if(!sscanf(s,"%19s",t) || s[0]=='#' || t[0]=='#') continue;
		if(!strcmp(t,"level")) {
			if(2!=sscanf(s,"level %d %19s",&level,t)) error("level command has wrong format");
			if(strlen(t)!=4) error("password has length different from 4");
			buf[4]=level&255;
			buf[5]=level>>8;
			buf[6]=t[0];
			buf[7]=t[1];
			buf[8]=t[2];
			buf[9]=t[3];
		} else if(!strcmp(t,"lynx")) ruleset=1;
		else if(!strcmp(t,"ms")) ruleset=2;
		else if(!strcmp(t,"seed")) {
			if(1!=sscanf(s,"seed %u",&seed)) error("seed command has wrong format");
			buf[12]=seed&255;
			buf[13]=(seed>>8)&255;
			buf[14]=(seed>>16)&255;
			buf[15]=seed>>24;
		} else if(!strcmp(t,"force")) {
			if(1!=sscanf(s,"force %19s",t)) error("force command has wrong format");
			/* dunno why dirs don't match documentation */
			if(!strcmp(t,"north")) force=1;
			else if(!strcmp(t,"west")) force=2;
			else if(!strcmp(t,"south")) force=3;
			else if(!strcmp(t,"east")) force=0;
			else error("illegal force command argument");
			buf[11]=(buf[11]&(~7))|force;
		} else if(!strcmp(t,"stepping")) {
			if(1!=sscanf(s,"stepping %19s",t)) error("stepping command has wrong format");
			if(!strcmp(t,"even") || !strcmp(t,"0")) stepping=0;
			else if(!strcmp(t,"+1") || !strcmp(t,"1")) stepping=1;
			else if(!strcmp(t,"+2") || !strcmp(t,"2")) stepping=2;
			else if(!strcmp(t,"+3") || !strcmp(t,"3")) stepping=3;
			else if(!strcmp(t,"odd") || !strcmp(t,"4")) stepping=4;
			else if(!strcmp(t,"odd+1") || !strcmp(t,"5")) stepping=5;
			else if(!strcmp(t,"odd+2") || !strcmp(t,"6")) stepping=6;
			else if(!strcmp(t,"odd+3") || !strcmp(t,"7")) stepping=7;
			else error("illegal stepping command argument");
			buf[11]=(buf[11]&(~56))|(stepping<<3);
		} else if(!strcmp(t,"starttime")) {
			if(1!=sscanf(s,"starttime %d",&starttime)) error("starttime command has wrong format");
		} else if(!strcmp(t,"route")) break;
		else printf("illegal command %s\n",t),exit(0);
	}
	/* parse route */
	while(fgets(s,9998,f)) {
		for(p=0;s[p];) {
			if(isspace(s[p])) { p++; continue; }
			if(s[p]=='#') break;
			if(s[p]=='!') goto endofroute;
			if(isdigit(s[p])) {
				cnt=s[p++]-48;
				while(isdigit(s[p])) cnt=cnt*10+s[p++]-48;
			} else cnt=1;
			dirs=0;
#define GETMOVE \
			if(isupper(s[p])) { \
				if(s[p]=='U') dir[dirs]=0,dur[dirs++]=movelen[ruleset]; \
				else if(s[p]=='L') dir[dirs]=1,dur[dirs++]=movelen[ruleset]; \
				else if(s[p]=='D') dir[dirs]=2,dur[dirs++]=movelen[ruleset]; \
				else if(s[p]=='R') dir[dirs]=3,dur[dirs++]=movelen[ruleset]; \
				else error("unknown letter"); \
			} else if(islower(s[p])) { \
				if(s[p]=='u') dir[dirs]=0,dur[dirs++]=movelenlower[ruleset]; \
				else if(s[p]=='l') dir[dirs]=1,dur[dirs++]=movelenlower[ruleset]; \
				else if(s[p]=='d') dir[dirs]=2,dur[dirs++]=movelenlower[ruleset]; \
				else if(s[p]=='r') dir[dirs]=3,dur[dirs++]=movelenlower[ruleset]; \
				else error("unknown letter"); \
			} else if(s[p]=='.') dir[dirs]=4,dur[dirs++]=movelenlower[ruleset]; \
			else if(s[p]==',') dir[dirs]=4,dur[dirs++]=movelen[ruleset];
			GETMOVE
			else if(s[p]=='{') {
				p++;
				while(s[p] && s[p]!='}') {
					if(dirs==DIRMAX) error("too many dirs");
					GETMOVE
					p++;
				}
			}
			p++;
#undef GETMOVE
			while(cnt--) {
				if(dirs>2) error("too many movements at once (max 2)");
				for(i=1;i<dirs;i++) if(dur[i]!=dur[0]) error("different durations within diagonal move");
				if(dirs>1) for(i=0;i<dirs;i++) if(dir[i]==4) error("cannot stand still in diagonal move");
				offset=putmove(offset,dir,dur,dirs);
				ticks+=dur[0];
			}
		}
	}
endofroute:
	/* have no idea about this, but in my tws files 4 ticks are added in lynx */
	/* maybe change if level ends with non-normal move */
	if(ruleset==2) ticks-=dur[0];
	flushmove();
	fclose(f);
	buf[0]=(buflen-4)&255;
	buf[1]=((buflen-4)>>8)&255;
	buf[2]=((buflen-4)>>16)&255;
	buf[3]=(buflen-4)>>24;
	buf[16]=ticks&255;
	buf[17]=(ticks>>8)&255;
	buf[18]=(ticks>>16)&255;
	buf[19]=ticks>>24;
	printf("txt file converted to tws, %d steps, %.2f seconds (%.2f)\n",ticks,ticks*0.05,starttime+0.95-ticks*0.05);
}

unsigned get4u(unsigned char *p) { return p[0]+(p[1]<<8)+(p[2]<<16)+(p[3]<<24); }
int get2u(unsigned char *p) { return p[0]+(p[1]<<8); }

void savetws(char *filename) {
	int pos,lv,exists=0;
	unsigned offset;
	FILE *f=fopen(filename,"wb");
	if(!f) error("couldn't open tws for writing");
	pos=8;
	if(pos!=fwrite(tws,1,pos,f)) error("wrong number of bytes written to tws");
	while(pos<twslen) {
		offset=get4u(tws+pos);
		lv=get2u(tws+pos+4);
		if(lv==level) {
			if(buflen!=fwrite(buf,1,buflen,f)) error("wrong number of bytes written to tws");
			printf("replaced solution to level %d in tws\n",level);
			exists=1;
		} else if(offset+4!=fwrite(tws+pos,1,offset+4,f)) error("wrong number of bytes written to tws");
		pos+=offset+4;
	}
	if(!exists) {
		if(buflen!=fwrite(buf,1,buflen,f)) error("wrong number of bytes written to tws");
		printf("added solution to level %d to tws\n",level);
	}
	if(fclose(f)) error("couldn't close tws file after writing");
	puts("tws file saved");
}

void usage() {
	puts("usage: tws txt-file tws-file\n");
	puts("  txt-file: text file with a solution");
	puts("  tws-file: tile world solution file");
}

int main(int argc,char **argv) {
	puts("txt-to-tws v1.0\n");
	if(argc<3) { usage(); return 0; }
	loadtws(argv[2]);
	parsetxt(argv[1],buf);
	savetws(argv[2]);
	return 0;
}
