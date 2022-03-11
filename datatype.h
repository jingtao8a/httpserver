/*************************************************************************
	> File Name: datatype.h
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月10日 星期四 16时10分40秒
 ************************************************************************/

#ifndef DATATYPE_H_
#define DATATYPE_H_
#include <pthread.h>
#include <time.h>

enum{
	WORKER_INITED,//initialized
	WORKER_RUNNING,//running
	WORKER_DETACHING,//unloading
	WORKER_DETACHED;//unloaded
	WORKER_IDEL//free
};

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
extern struct conn_request;
extern struct conn_response;
extern struct vec;
extern struct headers;

struct vec{
	char *ptr;
	int len;
	SHTTPD_METHOD_TYPE type;
};

struct headers{
	union variant cl;
	union variant ct;
	union variant connection;
	union variant ims;
	union variant user;
	union variant auth;
	union variant useragent;
	union variant referer;
	union variant range;
	union variant cookie;
	union variant range;
	union variant status;
	union variant transenc;
};

struct conn_response{
	struct vec res;
	time_t birth_time;
	time_t expire_time;
	int status;
	int cl;
	int fd;
	struct stat fsate;
	struct worker_conn *conn;
};

struct conn_request{
	struct vec req;
	char* head;
	char* uri;
	char rpath[URI_MAX];
	//HTTP VERSION
	unsigned long major;
	unsigned long minor;
	struct headers ch;
	struct worker_conn* conn;
	int err;
};

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
