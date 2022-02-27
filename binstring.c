#include<stdio.h>
#include<unistd.h>
#include<stdint.h>
#include<stdlib.h>
#include<endian.h>
#include<limits.h>

#define U64MAX 0xffffffffffffffff

ssize_t parse(char*, ssize_t);

void exit_err_end(char* src, ssize_t i){
	fprintf(stderr, "unexpected end of string near: \"%s\"\n",&src[(i-16>0)?(i-16):0]);
	exit(1);
}

void exit_err_chr()
{
	fprintf(stderr, "%s\n", "illegal character found");
	exit(1);
}

void exit_err_val()
{
	fprintf(stderr, "%s\n", "number of excessive size in decimal field");
	exit(1);
}

void exit_err_siz(char c){
	fprintf(stderr, "unsupported integer size '%c'\n", c);
	exit(1);
}

void exit_err_ndn(char c){
	fprintf(stderr, "unsupported endianess value '%c'\n", c);
	exit(1);
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

uint64_t _dec_get(char* src, ssize_t* i)
{
	uint64_t res = 0;
	while(src[*i]>='0' && src[*i]<='9'){
		if(res > (U64MAX/10)){
			exit_err_val();
			return 0;
		}
		res *= 10;
		if(res > (U64MAX-(src[*i]-'0'))){
			exit_err_val();
			return 0;
		}
		res += src[*i]-'0';
		*i += 1;
	}
	return res;
}

ssize_t _dec_get_ssi(char* src, ssize_t* i)
{
	uint64_t res = _dec_get(src, i);
	if(res > SSIZE_MAX){
		exit_err_val();
		return 0;
	}
	return (ssize_t)res;
}

ssize_t repeat(char* src, ssize_t i)
{
	ssize_t j = i+1;
	ssize_t count = _dec_get_ssi(src, &j);
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

ssize_t num_parse(char* src, ssize_t i)
{
	union{
		uint64_t d;
		uint32_t w;
		uint16_t h;
		uint8_t b;
	} res;

	ssize_t j = i+1;
	char siz = src[j];
	if((siz != '1') && (siz != '2') && (siz != '4') && (siz != '8')){
		exit_err_siz(siz);
		return 0;
	}
	j++;
	char en = src[j];
	if((en != 'b') && (en != 'l')){
		exit_err_ndn(en);
		return 0;
	}
	j++;
	if(src[j] == '\0'){
		exit_err_end(src, j);
		return 0;
	}
	if(siz == '1'){
		putchar((char)(_dec_get(src, &j)&0xff));
	}
	else if(siz == '2'){
		res.h = (uint16_t)(_dec_get(src, &j)&0xffff);
		if(en == 'b')
			res.h = htobe16(res.h);
		else if(en == 'l')
			res.h = htole16(res.h);

		fwrite((char*)(&res.h), 1, 2, stdout);
	}
	else if(siz == '4'){
		res.w = (uint32_t)(_dec_get(src, &j)&0xffffffff);
		if(en == 'b')
			res.w = htobe32(res.w);
		else if(en == 'l')
			res.w = htole32(res.w);

		fwrite((char*)(&res.w), 1, 4, stdout);
	}
	else{
		res.d = _dec_get(src, &j);
		if(en == 'b')
			res.d = htobe64(res.d);
		else if(en == 'l')
			res.d = htole64(res.d);

		fwrite((char*)(&res.d), 1, 8, stdout);
	}

	if(src[j] == '\0'){
		exit_err_end(src, j);
		return 0;
	}
	if(src[j] != '#'){
		exit_err_chr();
		return 0;
	}

	return (j-i)+1;
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
		putchar(c);
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
	putchar(src[i]);
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
		case '#':
			return num_parse(src, i);
			break;
		case '\\':
			return escape(src, i);
			break;
		default:
			putchar(src[i]);
			return 1;
			break;
	}
	return 0;
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
	return 0;
}
