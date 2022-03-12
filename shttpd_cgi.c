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

static int GenerateDirFile(struct worker_ctl* wctl) {
	struct conn_request* req = &wctl->conn.con_req;
	struct conn_response* res = &wctl->conn.con_res;
	char* command = strstr(req->uri, CGISTR) + strlen(CGISTR);
	char *arg[ARGNUM];
	int num = 0;
	char *rpath = wctl->conn.con_req.rpath;
	stat *fs = &wctl->conn.con_res.fstate;
	//open dir;
	DIR* dir = opendir(rpath);
	if (dir == NULL) {
		res->status = 500;
		return -1;
	}
	File *tmpfile;
	char tmpbuff[2048];
	int filesize = 0;
	char *uri = wctl->conn.con_req.uri;
	tmpfile = tmpfile();
	sprintf(tmpbuff, "%s%s%s","<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n<HTML>\
			<HEAD><TITLE>", uri, "</TITLE></HEAD>\n");
	fprintf(tmpfile, "%s", tmpbuff);
	filesize += strlen(tmpbuff);
	sprintf(tmpbuff, "%s %s %s", "<BODY><H1>Index of:", uri, " </H1> <HR><P><I>Date: </I> <T>Size:\
			 </I></P><HR>");
	fprintf(tmpfile, "%s", tmpbuff);
	filesize += strlen(tmpbuff);
	struct dirent* de;
#define PATHLENGTH 2048
	char path[PATHLENGTH];
	char tmpath[PATHLENGTH];
	char linkname[PATHLENGTH];
	struct stat fs;
	strcpy(path, rpath);
	if (rpath[strlen(rpath)] != '/') {
		rpath[strlen(rpath)] = '/';
	}

	while ((de = readdir(dir)) != NULL) {
		memset(tmpath, 0, sizeof(tmpath));
		memset(linkname, 0, sizeof(linkname));
		if (strcmp(de->d_name, ".")) {
			if (strcmp(de->d_name, "..")) {
				strcpy(linkname, de->d_name);
			} else {
				strcpy(linkname, "Parent Directory");
			}

			sprintf(tmpath, "%s%s", path, de->d_name);
			stat(tmpath, &fs);
			if (S_ISDIR(fs.st_mode)) {
				sprintf(tmpbuff, "<A HREF=\"%s/\">%s/</A><BR>\n", de->d_name, tmpath);
			} else {
				char size_str[32];
				off_t size_int;
				size_int = fs.st_size;
				if (size_int < 1024) {
					sprintf(size_str, "%d bytes", (int)size_int);
				} else if (size_int < 1024 * 1024) {
					sprintf(size_str, "%1.2f Kbytes", (float)size_int / 1024);
				} else{
					sprintf(size_str, "1.2f Mbytes", (float)size_int / (1024 * 1024));
				}
				sprintf(tmpbuff, "<A HREF=\"%s\">%s</A>(%s)<BR>\n", de->d_name, linkname, size_int);
			}
			fprintf(tmpfile, "%s", tmpbuff);
			filesize += strlen(tmpbuff);
		}
	}
	fs.st_ctime = time(NULL);
	fs.st_mtime = time(NULL);
	fs.st_size = filesize;
	fseek(tmpfile, (long)0, SEEK_SET);
	return 0;
}

int cgiHandler(struct worker_ctl* wctl) {
	struct conn_request* req = &wctl->conn.con_req;
	struct conn_response* res = &wctl->conn.con_res;
	char* command = strstr(req->uri, CGISTR) + strlen(CGISTR);
	char *arg[ARGNUM];
	int num = 0;
	char *rpath = wctl->conn.con_req.rpath;
	stat *fs = &wctl->conn.con_res.fstate;
	int retval = -1;
	//construct cgi routine
	char *pos = NULL;
	for (pos = command; *pos != '?' && *pos != '\0'; pos++);
	*pos = '\0';
	sprintf(rpath, "%s%s", conf_para.CGIRoot, command);
	//find cgi argument
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
	} else if ((fs->st_mode & S_IXUSR) != S_IXUSR) {
		res->status = 403;
		retval = -1;
		return retval;
	}

	int pipe_in[2];
	int pipe_out[2];

	if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) {
		res->status = 500;
		retval = -1;
		return retval;
	}

	pid_t pid = fork();
	if (pid < 0) {
		res->status = 500;
		retval = -1;
		return retval;
	} else if (pid > 0) {
		close(pipe_out[WRITEOUT]);
		close(pipe_in[READIN]);
		int size = 0;
		int end = 0;
		while (size > 0 || !end) {
			size = read(pipe_out[READIN], res->res.ptr, sizeof(wctl->conn.dres));
			if (size > 0) {
				send(wctl->conn.cs, res->res.ptr, strlen(res->res.ptr));
			} else {
				end = 1;
			}
		}
		wait(&end);
		close(pipe_out[READIN]);
		close(pipe_in[WRITEOUT]);
		retval = 0;
	} else {
		char cmdarg[2048], onearg[2048];
		char *pos = NULL;
		int i = 0;
		memset(onearg, 0, 2048);
		for (i = 0; i < num; ++i) {
			sprintf(cmdarg, "%s %s", onearg, arg[i]);
		}
		close(pipe_out[READIN]);
		dup2(pipe_out[WRITEOUT], 1);
		close(pipe_out[WRITEOUT]);

		close(pipe_in[WRITEOUT]);
		dup2(pipe_in[READIN], 0);
		close(pipe_in[READIN]);

		execlp(rpath, arg);
	}
	return retval;
}


