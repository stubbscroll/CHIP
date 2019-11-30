/* convert succ directions to stepmania */

#include <stdio.h>
#include <ctype.h>

char s[10000];

int main(void) {
	scanf("%9998s",s);
	int i;
	for(i=0;i<16;i++)	puts("0000");
	puts(",");
	for(i=0;s[i];i++) {
		switch(toupper(s[i])) {
		case 'L': puts("1000"); break;
		case 'D': puts("0100"); break;
		case 'U': puts("0010"); break;
		case 'R': puts("0001"); break;
		default: puts("0000");
		}
		if(i%16==15) puts(",");
	}
	for(;i%16;i++)	puts("0000");
	puts(";");
	return 0;
}
