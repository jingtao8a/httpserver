/*************************************************************************
	> File Name: shttpd_cgi.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月11日 星期五 21时06分53秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include "datatype.h"
#define CGISTR "/cgi-bin/"
#define ARGNUM 16
#define READIN 0
#define WRITEOUT 1

int C(struct worker_ctl* wctl) {
	struct conn_request* req = &wctl->conn.con_req;
	struct conn_response* res = &wctl->conn.con_res;
	char* command = strstr(req->uri, CGISTR) + strlen(CGISTR);
	char *arg[ARGNUM];
	int num = 0;
	char *rpath = wctl->conn.con_req.rpath;
	stat *fs = &wctl->conn.con_res.fstate;

	int retval = -1;
	for (; *pos != '?' && *pos != '\0'; pos++) {
		*pos = '\0';
	}
	sprintf(rpath, "%s%s", conf_para.CGIRoot, command);
	pos++;
	while(*pos != '\0' && num < ARGNUM) {
		arg[num] = pos;
		for(; *pos != '+' && *pos != '\0'; ++pos);
		if (*pos == '+') {
			*pos = '\0';
			pos++;
			num++;
		}
	}
	arg[num] = NULL;
	if (stat(rpath, fs) < 0) {
		res->status = 403;
		retval = -1;
		return retval;
	} else if ((fs->st_mode & S_IFDIR) == S_IFDIR) {
		GenerateDirFile(wctl);
	}
}


