/*************************************************************************
	> File Name: shttpd_method.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月11日 星期五 20时02分13秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "datatype.h"
#include "shttpd_mine.h"

extern struct conf_opts conf_para;

static int Method_DoPut(struct worker_ctl* wctl){
	return 0;
}
static int Method_DoDelete(struct worker_ctl* wctl) {
	return 0;
}

static int Method_DoGet(struct worker_ctl* wctl) {
	struct conn_response* res = &wctl->conn.con_res;
	struct conn_request* req = &wctl->conn.con_req;
	char path[URI_MAX];
	memset(path, 0, URI_MAX);
	


	int status = 200;//status code
	char *msg = "OK";//status msg
	//********************************
	char *fmt = "%a %d %b %Y %H:%M:%S GMT";
	char date[64];//current time
	char lm[64];//last modify time
	time_t t = time(NULL);
	strftime(date, sizeof(date), fmt, localtime(&t));//current time
	strftime(lm, sizeof(lm), fmt, localtime(&res->fstate.st_mtime));//last modify time
	DBG("current-time: %s\n", date)
	DBG("request file last_modify_time: %s\n", lm)
	//***********************************
	char etag[64];//etag
	snprintf(etag, sizeof(etag), "%lx.%lx", (unsigned long)res->fstate.st_mtime, (unsigned long)res->fstate.st_size);
	DBG("etag: %s\n", etag)
	//**********************************
	unsigned long cl;//Content-Lenth
	cl = (unsigned long) res->fstate.st_size;//file size
	DBG("content-length: %lu\n", cl);
	//************************************
	struct mine_type* m = NULL;
	if (!strcmp(req->uri, "/")) {
		m = Mine_Type(conf_para.DefaultFile, strlen(conf_para.DefaultFile), wctl);
	} else {
		m = Mine_Type(req->uri, strlen(req->uri), wctl);//Content-Type
	}
	//*************************************
	char range[64];//range
	memset(range, 0, sizeof(range));
	int n;
	unsigned long r1, r2;
	n = -1;
	if (req->ch.range.v_vec.len > 0) {
		DBG("Range = %s\n", req->ch.range.v_vec.ptr)
		n = sscanf(req->ch.range.v_vec.ptr, "bytes=%lu-%lu", &r1, &r2);
		DBG("n = %d\n", n)
		if (n > 0) {//range is exiting
			status = 206;//status code
			msg = "Partial Content";//status content
			lseek(res->fd, r1, SEEK_SET);
			cl = (n == 2 ? r2 - r1 + 1 : cl - r1);
			DBG("content-length -> : %lu", cl);
			snprintf(range, sizeof(range), "Content-Range: bytes %lu-%lu/%lu\r\n", r1, r1 + cl - 1, (unsigned long)res->fstate.st_size);
		}
	}
	

	//***************************************
	memset(res->res.ptr, 0, sizeof(wctl->conn.dres));//
	if (n == -1) {
		snprintf(res->res.ptr, sizeof(wctl->conn.dres),\
"HTTP/1.1 %d %s\r\n\
Date: %s\r\n\
Last-Modified: %s\r\n\
Etag: \"%s\"\r\n\
Content-Type: %s\r\n\
Content-Length: %lu\r\n\
\r\n",\
		status, msg,\
		date,\
		lm,\
		etag,\
		m->mine_type,\
		cl);
	} else {
		snprintf(res->res.ptr, sizeof(wctl->conn.dres),\
"HTTP/1.1 %d %s\r\n\
Date: %s\r\n\
Last-Modified: %s\r\n\
Etag: \"%s\"\r\n\
Content-Type: %s\r\n\
Content-Length: %lu\r\n\
Accept-Ranges: bytes\r\n\
%s\r\n",\
	status, msg,\
	date,\
		lm,\
		etag,\
		m->mine_type,\
		cl,\
		range);
	}
	res->cl = cl;
	res->status = status;
	return 0;
}

static int Method_DoPost(struct worker_ctl* wctl) {
	return 0;
}

static int Method_DoHead(struct worker_ctl* wctl) {
	return 0;
}

static int Method_DoList(struct worker_ctl* wctl) {
	return 0;
}
void my_Method_Do(struct worker_ctl* wctl) {
	//Method_DoCGI(wctl);
	switch(wctl->conn.con_req.method) {
		case METHOD_PUT:
			Method_DoPut(wctl);
			break;
		case METHOD_DELETE:
			Method_DoDelete(wctl);
			break;
		case METHOD_GET:
			Method_DoGet(wctl);
			break;
		case METHOD_POST:
			Method_DoPost(wctl);
			break;
		case METHOD_HEAD:
			Method_DoHead(wctl);
			break;
		default:
			Method_DoList(wctl);
	}
}
