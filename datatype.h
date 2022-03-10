/*************************************************************************
	> File Name: datatype.h
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月10日 星期四 16时10分40秒
 ************************************************************************/

#ifndef DATATYPE_H_
#define DATATYPE_H_
#include <pthread.h>

struct conf_opts{
	char CGIRoot[128];  //CGI routine
	char DefaultFile[128]; //default file name
	char DocumentRoot[128];	//root file routine
	char ConfigFile[128];	//config file routine
	int ListenPort;
	int MaxClient;
	int TimeOut;
	int InitClient;	//init thread num
};

extern struct worker_conn;
extern struct worker_opts;
extern struct worker_ctl;

struct worker_conn {
#define K 1024
	char dreq[16 * K];
	char dres[16 * K];
	int cs;
	int to;
	struct conn_response con_res;
	struct conn_request con_req;
	struct worker_ctl* work;
}

struct worker_opts{
	pthread_t th;
	int flags;
	pthread_mutex_t mutex;
	struct worker_ctl* work;
};

struct worker_ctl{
	struct worker_opts opts;
	struct worker_conn conn;
}
#endif
