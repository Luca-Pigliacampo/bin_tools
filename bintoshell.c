#include<stdio.h>
#include<stdint.h>

int main(void){
	int c = getchar();
	char chr[6] = {'\\','0','0','0','0','\0'};
	while(c != EOF){
		for(int i = 2; i>=0; i--){
			chr[4-i] = '0'+((c & (7 << (3*i)) & 0xff)>>(3*i));
		}
		printf("%s",chr);
		c = getchar();
	}
	putchar('\n');
}
