/*************************************************************************
	> File Name: shttpd_method.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月11日 星期五 20时02分13秒
 ************************************************************************/

#include<stdio.h>
#include "datatype.h"
#include "shttpd_mine.h"
static int Method_DoGet(struct worker_ctl* wctl) {
	struct conn_response* res = &wctl->conn.con_res;
	struct conn_request* req = &wctl->conn.con_req;
	char path[URI_MAX];
	memset(path, 0, URI_MAX);
	size_t n;
	unsigned long r1, r2;
	char *fmt = "%a %d %b %Y %H:%M:%S GMT";

	size_t status = 200;
	char *msg = "OK";
	char date[64] = "";//current time
	char lm[64] = "";//last modify time
	char etag[64] = "";
	long int cl;
	char range[64] = "";

	time_t t = time(NULL);
	strftime(date, sizeof(date), localtime(&t));
	strftime(lm, sizeof(lm), localtime(req->fstate.st_mtime));

	snprintf(etag, sizeof(etag), "%lx.%lx", (unsigned long)res->fstate.st_mtime, (unsigned long)res->fstate.st_size);

	mine = Mine_Type(req->uri, stlen(req->uri), wctl);

	cl = (long int) res->fstate.st_size;

	memset(range, 0, sizeof(range));
	n = -1;
	if (req->ch.range.v_vec.len > 0) {
		printf("request range: %d\n", req->ch.range.v_vec.len);
		n = sscanf(req->ch.range.v_vec.ptr, "bytes=%lu-%lu", &r1, &r2);
	}
	printf("n:%d\n", n);
	if (n > 0) {
		status = 206;
		fseek(res->fd, r1, SEEK_SET);
		cl = n == 2 ? r2 - r1 + 1 : cl - r1;
		snprintf(range, sizeof(range), "Content-Range: bytes %lu-%lu/%lu\r\n", r1, r1 + cl - 1, (unsigned long)res->fstate.st_size);
		msg = "Partial Content";
	}
	memset(res->res.ptr, 0, sizeof(wctl->conn.dres));
	snprintf(res->res.ptr, sizeof(wctl->conn,dres), \
			"HTTP/1.1 %d %s\r\n\
			Date: %s\r\n\
			Last-Modified: %s\r\n\
			Etag: \"%s\"\r\n\
			Content-Type: %.*s\r\n\
			Content-Length: %lu\r\n\
			Accept-Ranges: bytes\r\n\
			%s\r\n",\
			satus, msg,\
			date,\
			lm,\
			etag,\
			strlen(mine->mime_type),\
			mine->mime_type,\
			cl,\
			range);
	res->cl = cl;
	res->status = status;
	return 0;
}

void Method_Do(struct worker_ctl* wctl) {
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
