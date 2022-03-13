/*************************************************************************
	> File Name: shttpd.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月10日 星期四 16时09分59秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "datatype.h"
#include "cmdparse.h"
#include "fileparse.h"
#include "shttpd_worker.h"

struct conf_opts conf_para = {
	"/usr/local/var/www/cgi-bin",	//CGIRoot
	"/index.html",	//DefalutFile
	"/htmldb",	//DocumetRoot
	"/etc/SHTTPD.conf",	//ConfigFile
	8080,	//port
	4,	//MaxClient
	3,	//timeout
	2	//threadnum
};


static void sig_int(int num) {
	Worker_ScheduleStop();
	return;
}


static int do_listen(){
	struct sockaddr_in server;
	int err = -1;
	int ret =-1;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(conf_para.ListenPort);

	int ss = socket(AF_INET, SOCK_STREAM, 0);
	if (ss == -1) {
		perror("socket()");
		exit(1);
	}
	int reuse = 1;
	err = setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));//端口重用
	if (err == -1) {
		perror("setsocket SO_REUSEADDR");
		exit(1);
	}
	
	bind(ss, (struct sockaddr*)&server, sizeof(struct sockaddr));
	listen(ss, conf_para.MaxClient * 2);
	DBG("socket bind listen............\n");
	return ss;
}

int main(int argc, char** argv){
	signal(SIGINT, sig_int);
	Para_CmdParse(argc, argv);
	Para_FileParse(conf_para.ConfigFile);
	int s = do_listen();
	Worker_ScheduleRun(s);
	return 0;
}
