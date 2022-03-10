/*************************************************************************
	> File Name: fileparse.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月10日 星期四 17时04分40秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "datatype.h"

extern struct conf_opts conf_para;

void Para_FileParse(const char* file) {
#define LINELENGTH 256
	char line[LINELENGTH];
	char *name = NULL, *value = NULL;
	FILE* fd = NULL;
	int n = 0;
	fd = fopen(file, "r");
	if (fd == NULL) return;

	while (1) {
		fgets(line, LINELENGTH, fd);
		if (feof(fd)) break;
		char *pos = line;
		
		while (isspace(*pos)) {
			pos++;
		}

		if (*pos == '#') {
			continue;
		}
		name = pos;
		while (!isspace(*pos) && *pos != '=') {
			pos++;
		}

		if (*pos == '=') {
			*pos = '\0';
			pos++;
		} else {
			*pos = '\0';
			pos++;
			while (*pos != '=')
				pos++;
			pos++;
		}

		while (isspace(*pos))
			pos++;

		value = pos;
		while (!isspace(*pos) && *pos != '\r' && *pos != '\n') {
			pos++;
		}
		*pos = '\0';

		int ivalue;

		if (!strcmp("CGIRoot", name)) {
			memcpy(conf_para.CGIRoot, value, strlen(value) + 1);
		} else if (!strcmp("DefaultFile", name)){
			memcpy(conf_para.DefaultFile, value, strlen(value) + 1);
		} else if (!strcmp("DocumentRoot", name)){
			memcpy(conf_para.DocumentRoot, value, strlen(value) + 1);
		} else if (!strcmp("ListenPort", name)){
			ivalue = strtol(value, NULL, 10);
			conf_para.ListenPort = ivalue;
		} else if (!strcmp("MaxClient", name)){
			ivalue = strtol(value, NULL, 10);
			conf_para.MaxClient = ivalue;
		} else if (!strcmp("TimeOut", name)){
			ivalue = strtol(value, NULL, 10);
			conf_para.TimeOut = ivalue;
		}
		fclose (fd);
	}
	return;
}
