/*************************************************************************
	> File Name: cmdparse.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月10日 星期四 16时50分14秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include "datatype.h"

extern struct conf_opts conf_para;

static char* shortopts = "c:f:d:o:l:m:t:h";

static char *l_opt_arg;
struct option longopts[] = {
	{"CGIRoot",required_argument , NULL, 'c'},
	{"ConfigFile",required_argument	,NULL, 'f'},
	{"DefaultFile",required_argument ,NULL, 'd'},
	{"DocumentRoot",required_argument ,NULL, 'o'},
	{"ListenPort",required_argument , NULL, 'l'},
	{"MaxClient",required_argument , NULL, 'm'},
	{"TimeOut",required_argument , NULL, 't'},
	{"Help",no_argument , NULL, 'h'},
	{0, 0, 0, 0}
};
static void display_usage() {
	printf("CGIRoot,1 , NULL, c\n");
	printf("ConfigFile,1	,NULL, f\n");
	printf("DefaultFile,1 ,NULL, d\n");
	printf("DocumentRoot,1 ,NULL, o\n");
	printf("ListenPort,1 , NULL, l\n");
	printf("MaxClient,1 , NULL, m\n");
	printf("TimeOut,1 , NULL, t\n");
	printf("Help,0 , NULL, h\n");
	fflush(stdout);	
	exit(0);
}

int Para_CmdParse(int argc, char **argv) {
	char c;
	int len;
	int value;
	while ((c = getopt_long_only(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch(c) {
			case '?':
				printf("invalid para\n");
				return -1;
			case 'c':
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					memcpy(conf_para.CGIRoot, l_opt_arg, len + 1);
				}
				break;
			case 'd':
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					memcpy(conf_para.DefaultFile, l_opt_arg, len + 1);
				}
				break;
			case 'f':
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					memcpy(conf_para.ConfigFile, l_opt_arg, len + 1);
				}
				break;
			case 'o':
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					memcpy(conf_para.DocumentRoot, l_opt_arg, len + 1);
				}
				break;
			case 'l':
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					value = strtol(l_opt_arg, NULL, 10);
					if (value != LONG_MAX && value != LONG_MIN) {
						conf_para.ListenPort = value;
					}
				}
				break;
			case 'm':
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					value = strtol(l_opt_arg, NULL, 10);
					if (value != LONG_MAX && value != LONG_MIN) {
						conf_para.MaxClient = value;
					}
				}
				break;
			case 't':
				l_opt_arg = optarg;
				if (l_opt_arg && l_opt_arg[0] != ':') {
					len = strlen(l_opt_arg);
					value = strtol(l_opt_arg, NULL, 10);
					if (value != LONG_MAX && value != LONG_MIN) {
						conf_para.TimeOut = value;
					}
				}
				break;
			case 'h':
				display_usage();
				break;

		}
	}
	return 0;
}

