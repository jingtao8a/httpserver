/*************************************************************************
	> File Name: shttpd_worker.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月10日 星期四 20时43分40秒
 ************************************************************************/

#include <stdio.h>
#include "datatype.h"
extern struct config_opts config_para;

static void Worker_Init() {
	int i = 0;
	wctls = (struct worker_ctl*)malloc(sizeof(struct worker_ctl) * iconf_para.MaxClient);
	memset(wctls, 0, sizeof(struct worker_ctl) * conf_para.MaxClient);
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
					i = WORKER_ISSTATUS(WORKER_IDEL);
					if (i == -1) {
						i = WORKER_ISSTATUS(WORKER_DETACHED);
						if (i != -1)	
							Worker_Add(i);
					} else {
						wctls[i].conn.cs = sc;
						pthread_mutex_unlock(&wctls[i].opts.mutex);
					}
				}
		}
	}
	return 0;
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
