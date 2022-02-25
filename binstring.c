#include<stdio.h>
#include<unistd.h>
#include<stdint.h>
#include<stdlib.h>

#define BUFSIZE 4096

char buffer[BUFSIZE];
ssize_t offset = 0;

ssize_t parse(char*, ssize_t);

void exit_err_end(char* src, ssize_t i){
	write(STDERR_FILENO, "unexpected end of string near: \"", 32);
	write(STDERR_FILENO, &src[(i-16>0)?(i-16):0], (i-16>0)?16:i);
	write(STDERR_FILENO, "\"\n", 2);
	exit(1);
}

void exit_err_chr()
{
	write(STDERR_FILENO, "illegal character found\n",24);
	exit(1);
}

void submit(char c)
{
	buffer[offset] = c;
	offset++;
	if(offset>=BUFSIZE){
		write(STDOUT_FILENO,buffer, BUFSIZE);
		offset=0;
	}
}

uint8_t _hex_get(char c)
{
	if(c>='0' && c<='9'){
		return(c-'0');
	}
	else if(c>='a' && c<='f'){
		return((c-'a')+10);
	}
	else if(c>='A' && c<='F'){
		return((c-'A')+10);
	}
	else{
		exit_err_chr();
		return 0;
	}
}

ssize_t _dec_get(char* src, ssize_t* i)
{
	ssize_t res = 0;
	while(src[*i]>='0' && src[*i]<='9'){
		res *= 10;
		res += src[*i]-'0';
		*i += 1;
	}
	return res;
}

ssize_t repeat(char* src, ssize_t i)
{
	ssize_t j = i+1;
	ssize_t count = _dec_get(src, &j);
	ssize_t k;
	if(src[j]=='\0')
		exit_err_end(src, j);
	if(src[j]!='{')
		exit_err_chr();
	j++;
	for(;count != 0; count--){
		for(k=j;src[k]!='\0' && src[k]!='}';k += parse(src, k)){};
		if(src[k]=='\0') exit_err_end(src, k);
	}
	return (k-i)+1;
}

ssize_t hex_parse(char* src, ssize_t i)
{
	uint8_t c = 0;
	ssize_t j;
	i++;
	for(j = i; src[j] != '\0' && src[j] != '|'; j++){
		c |= _hex_get(src[j]);

		c <<= 4;
		j++;
		if(src[j]=='\0' || src[j]=='|')
			break;
		
		c |= _hex_get(src[j]);
		submit(c);
		c = 0;
	}
	if(src[j] == '\0')
		exit_err_end(src, j);

	return (j-i)+2;
}

ssize_t escape(char* src,ssize_t i)
{
	i++;
	if(src[i]=='\0'){
		exit_err_end(src, i);
	}
	submit(src[i]);
	return 2;
}

ssize_t parse(char* src, ssize_t i)
{
	switch(src[i]){
		case '\0':
			exit_err_end(src, i);
			break;
		case '*':
			return repeat(src, i);
			break;
		case '|':
			return hex_parse(src, i);
			break;
		case '\\':
			return escape(src, i);
			break;
		default:
			submit(src[i]);
			return 1;
			break;
	}
	return 0;
}

void buf_flush()
{
	write(STDOUT_FILENO, buffer, offset);
	offset = 0;
}

int main(int argc, char** argv)
{
	if(argc < 2)
		return EXIT_FAILURE;

	char* source = argv[1];
	ssize_t b_read;

	for(ssize_t i = 0; source[i] != '\0'; i+=b_read){
		b_read = parse(source, i);
	}
	buf_flush();
	return 0;
}
