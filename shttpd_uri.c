/*************************************************************************
	> File Name: shttpd_uri.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月11日 星期五 10时09分03秒
 ************************************************************************/

#include <stdio.h>
#include <ctype.h>

static int uri_decode(char* src, int src_len, char* dst, int dst_len) {
	int i, j, a, b;
#define HEXTOI(x)	(isdigit(x) ? x - '0' : x - 'A' + 10)
	for (i = j = 0; i < src_len && j < dst_len - 1; ++i, ++j) {
		switch(src[i]) {
			case '%':
				if (isxdigit(((unsigned char*)src)[i + 1]) && isxdigit(((unsigned char*)src)[i + 2])) {
					a = tolower(((unsigned char*)src)[i + 1]);
					b = tolower(((unsigned char*)src)[i + 2]);
					dst[j] = (HEXTOI(a) <<  4) | HEXTOI(b);
					i += 2;
				} else {
					dst[j] = '%';
				}
				break;
			default:
				dst[j] = src[i];
				break;

		}
	}
	dst[j] = '\0';
	return j;
}

static void remove_double_dots(char* s) {
	char *p = s;
	while (*s != '\0') {
		*p = *s;
		p++; s++;
		if (*(s - 1) == '/' || *(s - 1) == '\\') {
			while (*s == '.' || *s == '/' || *s == '\\') {
				s++;
			}
		}
	}
	*p = '\0';
}

void uri_parse(char* src, int len) {
	uri_decode(src,len - 1, src, len);
	remove_double_dots(src);
}
