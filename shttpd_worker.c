/*************************************************************************
	> File Name: shttpd_worker.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月10日 星期四 20时43分40秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include "datatype.h"
#include "shttpd_request.h"
extern struct config_opts config_para;

static void do_work(struct worker_ctl* wctl);
int WORKER_ISSTATUS(int status);
static void Worker_Init();
int Worker_Add(int i);
void Worker_Delete(int i);
void Worker_Destroy();
int Worker_ScheduleRun(int ss);
int Worker_ScheduleStop();


static int workersnum = 0;//work thread num
pthread_mutex_t thread_init = PTHREAD_MUTEX_INITIALIZER;
static struct worker_ctl* wctls = NULL;

static void do_work(struct worker_ctl* wctl) {
	struct timeval tv;
	fd_set rfds;
	int fd = wctl->conn.cs;//sockfd
	struct vec* req = wctl->conn.con_req.req;

	int retval = 1;
	
	while (retval > 0) {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		tv.tv_sec = 300;
		tv.tv_usec = 0;

		retval = select(fd + 1, &rfds, NULL, NULL, tv);
		switch(retval) {
			case -1:
			case 0:
				close(fd);
				break;
			default:
				printf("select retval:%d\n", retval);
				if (FD_ISSET(fd, &rfds)) {
					memset(wctl->conn.dreq, 0, sizeof(wctl->conn.dreq));
					req->len = read(wctl->conn.cs, wctl->conn.dreq, sizeof(wctl->conn.dreq));
					req->ptr = wctl->conn.req;
					if (req->len > 0) {
						wctl->conn.con_req.err = Request_Parse(wctl);
						Request_Handle(wctl);
					}
				} else {
					close(fd);
					retval = -1;
				}
				break;
		}
	}
}

static void* worker(void *arg) {
	struct worker_ctl* ctl = (struct worker_ctl*)arg;
	struct worker_opts* self_opts = &ctl->opts;

	pthread_mutex_unlock(&thread_init);//?????????????
	self_opts->flags = WORKER_IDEL;//free

	while(self_opts->flags != WORKER_DETACHING) {
		int err = pthread_mutex_trylock(&self_opts->mutex);
		if (err) {
			sleep(1);
			continue;
		} else {
			self_opts->flags = WORKER_RUNNING;//running
			do_work(ctl);
			close(ctl->conn.cs);
			ctl->conn.cs = -1;
			if (self_opts->flags == WORKER_DETACHING)
				break;
			else
				self_opts->flags = WORKER_IDEL;
		}
	}
	self_opts->flags = WORKER_DETACHED;
	workersnum--;
	reutrn NULL;
}

int WORKER_ISSTATUS(int status) {
	for (int i = 0; i < conf_para.MaxClient; ++i) {
		if (wctls[i].opts.flags == status) {
			return i;
		}
	}
	reutrn -1;
}

static void Worker_Init() {
	int i = 0;
	wctls = (struct worker_ctl*)malloc(sizeof(struct worker_ctl) * iconf_para.MaxClient);
	memset(wctls, 0, sizeof(struct worker_ctl) * conf_para.MaxClient);
	for (i = 0; i < confif_para.MaxClient; ++i) {
		wctls[i].opts.work = &wctls[i];
		wctls[i].conn.work = &wctls[i];
		wctls[i].opts.flags = WORKER_DETACHED;//unloaded
		pthread_mutex_init(&wctls[i].opts.mutex, NULL);
		pthread_mutex_lock(&wctls[i].opts.mutex);//lock
	
		wctls[i].conn.con_req.conn = &wctls[i].conn;
		wctls[i].conn.con_res.conn = &wctls[i].conn;

		wctls[i].conn.con_req.req.ptr = wctls[i].conn.dreq;
		wctls[i].conn.con_req.head = wctls[i].conn.dreq;
		wctls[i].conn.con_req.uri = wctls[i].conn.dreq;

		wctls[i].conn.con_res.fd = -1;
		wctls[i].conn.con_res.res.ptr = wctls[i].conn.dres;
	}
	for (i = 0; i < conf_para.InitClient; ++i) {
		Worker_Add(i);
	}
}

int Worker_Add(int i) {
	pthread_t th;
	int err = -1;
	if (wctls[i].opts.flags == WORKER_RUNNING) {
		return 1;
	}
	pthread_mutex_lock(&thread_init);//thread_init lock
	wctls[i].opts.flags = WORKER_INITED;//initialize
	pthread_create(&th, NULL, worker, (void*)&wctls[i]);
	pthread_mutex_unlock(&thread_init);//thread_init unlock
	
	wctls[i].opts.th = th;
	workersnum++;
	return 0;
}

int Worker_ScheduleRun(int ss) {
	struct sockaddr_in client;
	socklen_t len;
	Worker_Init();
	
	int i = 0;
	while (SCHEDULESTATUS == STATUS_RUNNING) {
		struct timeval tv;
		fd_set rfds;
		int retval = -1;

		FD_ZERO(&rfds);
		FD_SET(ss, &rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 500000;

		retval = select(ss + 1, &rfds, NULL, NULL, &tv);
		switch(retval) {
			case -1:
			case 0:
				continue;
			default:
				if (FD_ISSET(ss, &rfds)) {
					int s_c = accept(ss, &client, &len);
					i = WORKER_ISSTATUS(WORKER_IDEL);//thread that is free
					if (i == -1) {
						i = WORKER_ISSTATUS(WORKER_DETACHED);
						if (i != -1)	
							Worker_Add(i);
					} else {
						wctls[i].conn.cs = s_c;
						pthread_mutex_unlock(&wctls[i].opts.mutex);
					}
				}
		}
	}
	return 0;
}

void Worker_Delete(int i) {
	wctls[i].opts.flag = WORKER_DETACHING;
}


void Worker_Destroy() {
	int i = 0, clean = 0;
	for (i = 0; i < conf_para.MaxClient; ++i) {
		if (wctls[i].opts.flags != WORKER_DETACHED) {
			Worker_Delete(i);
		}
	}
	while (!clean) {
		clean = 1;
		for (i = 0; i < conf_para.MaxClient; ++i) {
			if (wctls[i].opts.flag = WORKER_RUNNING || wctls[i].opts.flag == IDEL)
				clean = 0;
		}
		if (!clean)
			sleep(1);
	}
}

int Worker_ScheduleStop() {
	SCHEDULESTATUS = STATUS_STOP;
	int i = 0;
	Worker_Destory();
	int allfired = 0;
	while(!allfired) {
		allfired = 1;
		for (i = 0; i < conf_para.MaxClient; ++i) {
			int flags = wctls[i].opts.flags;
			if (flags = WORKER_DETACHING || flags == WORKER_IDEL)
				allfired = 0;
		}
	}
	pthread_mutex_destroy(&thread_init);
	for (i = 0; i < conf_para.MaxCLient; ++i) {
		pthread_mutex_destroy(&wctls[i].opts.mutex);
	}
	free(wctls);

	return 0;
}
