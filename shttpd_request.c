/*************************************************************************
	> File Name: shttpd_request.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月11日 星期五 15时11分14秒
 ************************************************************************/

#include<stdio.h>
#include "datatype.h"
#include "shttpd_error.c"
#include "shttpd_method.c"
extern struct conf_opts conf_para;

struct vec _shttpd_methods[] = {
	{"GET", 3, METHOD_GET},
	{"POST", 4, METHOD_POST},
	{"PUT", 3, METHOD_PUT},
	{"DELETE", 6, METHOD_DELETE},
	{"HEAD", 4, METHOD_HEAD}
	{NULL, 0, 0}
};

static struct http_header http_headers[] = {
	{16, HDR_INT, OFFSET(cl), "Content-Length:"},
	{14, HDR_STRING, OFFSET(ct), "Content-Type:"},
	{12, HDR_STRING, OFFSET(useragent), "User-Agent:"},
	{19, HDR_DATE, OFFSET(ims), "If-Modified-Since:"},
	{15, HDR_STRING, OFFSET(auth), "Authorization:"},
	{9, HDR_STRING, OFFSET(referer), "Referer:"},
	{8, HDR_STRING, OFFSET(cookie), "Cookie:"},
	{10, HDR_STRING, OFFSET(location), "Location"},
	{8, HDR_INT, OFFSET(status), "Status:"},
	{7, HDR_STRING, OFFSET(range), "Range:"},
	{12, HDR_STRING, OFFSET(connection), "Connection:"},
	{19, HDR_STRING, OFFSET(transenc), "Transfer-Encoding:"},
	{0, HDR_INT, 0, NULL}
};

static int montoi(char* s);
static time_t date_to_epoch(char* s);

static int montoi(char* s) {
	static char *ar[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
						 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	size_t i;
	for (i = 0; i < 12; ++i ) {
		if (!strcmp(s, ar[i])) {
			return i;
		}
	}
	return -1;
}

static time_t date_to_epoch(char* s) {
	struct tm tm;
	char mon[32];
	int sec, min, hour, mday, month, year;
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	sec = min = hour = mday = month = year = 0;
	if (((sscanf(s, "%d/%3s/%d %d:%d:%d", &mday, mon, &year, &hour, &min, &sec) == 6) ||
		(sscanf(s, "%d %3s %d %d:%d:%d", &mday, mon, &year, &hour, &min, &sec) == 6) ||
		(sscanf(s, "%d-%3s-%d %d:%d:%d", &mday, mon, &year, &hour, &min, &sec) == 6)) &&
		(month = montoi(mon) != -1)) {
		tm.tm_mday = mday;
		tm.tm_mon = month;
		tm.tm_year = year;
		tm.tm_hour = hour;
		tm.tm_min = min;
		tm.tm_sec = sec;
	}

	if (tm.tm_year > 1900) {
		tm.tm_year -= 1900;
	} else {
		tm.tm_year += 100;
	}
	return mktime(&tm);
}


static void Request_HeaderParse(char* s, int len, struct headers* parsed) {
	struct http_header *h;
	union variant *v;
	char *p, *e = s + len;
	while (s < e) {
		for (p = s; p < e && *p != '\n') {
			p++;
		}
		for (h = http_headers; h->len != 0; ++h) {
			if (e - s > h->len && !strncmp(s, h->name, h->len)) {
				break;
			}
		}
		if (h->len != 0) {
			s += h->len;
			v = (union variant*)((char *)parsed + h->offset);
			if (h->type == HDR_STRING) {
				v->v_vec.ptr = s;
				v->v_vec.len = p - s;
				if (p[-1] == '\r' && v->v_vec.len > 0)
					v->v_vec.len--;
			} else if (h->type == HDR_INT){
				v->v_big_int = strtoul(s, NULL, 10);
			} else if (h->type == HDR_DATE) {
				v->v_time = date_to_epoch(s);
			}
		}
		s = p + 1;
	}
}


int Request_Parse(struct worker_ctl* wctl) {
	struct worker_conn* c = &wctl->conn;
	struct conn_request* req = &c->con_req;
	struct conn_response* res = &c->con_res;
	int retval = 200;
	char *p = req->req.ptr;
	int len = req->req.len;
	char *pos = NULL;

	pos = memchr(p, '\n', len);
	if (pos[-1] == '\r') {
		pos[-1] = '\0';
	}
	*pos = '\0';
	//find the http method
	int found;
	while (*p == ' ') {
		p++;
		len--;
	}
	struct vec* m = NULL
	for (m = _shttpd_methods; m->ptr != NULL; m++) {
		if (!strncmp(p, m->ptr, m->len)) {
			req->method = m->type;
			found = 1;
			break;
		}
	}
	if (found == 0) {
		retval = 400;
		return retval;
	}

	//find the http uri
	pos = p + m->len;
	len -= m->len;
	while(*pos == ' ') {
		pos++;
		len--;
	}
	p = pos;
	while (*pos != ' ') {
		pos++;
		len--;
	}
	*pos = '\0';
	req->uri = (char*)p;
	snprintf(req->rpath, URI_MAX, "%s/%s", conf_para.DocumentRoot, req->uri);
	req->fd = open(req->rpath, O_RDONLY);
	if (res->fd != -1) {
		fstat(res->fd, &res->fstate);
		if (S_ISDIR(res->fstate.st_mode)) {
			retval = 403;
			return retval;
		}
	} else {
		retval = 404;
		return retval;
	}

	pos++;
	len--;
	while(*pos == ' ') {
		pos++;
		len--;
	}
	p = pos;
	sscanf(p, "HTTP/%lu.%lu", &req->major, &req->minor);
	if (!((req->major == 0 && req->minor == 9) || (req->major == 1 & req->req->minor == 0)||
		(req->major == 1 && req->minor == 1))) {
		retval = 505;
		return retval;
	}
	while (*pos != '\0') {
		pos++;
		len--;
	}
	while (*pos == '\0') {
		pos++;
		len--;
	}
	p = pos;
	Request_HeaderParse(p, len, &req->ch);
	return retval;
}

int Request_Handle(struct worker_ctl* wctl) {
	int err = wctl->conn.con_req.err;
	int cs = wctl->conn.cs;
	char* ptr = wctl->conn.con_res.res.ptr;
	int n;
	switch(err) {
		case 200;
			Method_Do(wctl);
			int fd = wctl->conn.con_res.fd;
			cl = wctl->conn.con_res.cl;
			len = strlen(wctl->conn.con_res.res.ptr);
			n = write(cs, ptr, len);
			printf("echo header:%d, write to client %d bytes, status:%d\n", ptr, n, wctl->conn.con_res.status);
			if (fd != -1) {
				lseek(fd, 0, SEEK_SET);
				len = sizeof(wctl->conn.dres);
				printf("response len:%d, content length:%d\n", len, wctl->conn.con_res.cl);
				for (n = 0; cl > 0; cl -= n) {
					n = read(fd, ptr, len > cl ? cl : sizeof(wctl->conn.dres));
					printf("read %d bytes,", n);
					if (n > 0) {
						n = write(cs, ptr, n);
						printf("write %d bytes\n", n);
					}
				}
				close(fd);
				wctl->conn.con_res.fd = -1;
			}
			break;
		case 400:
		case 403:
		case 404:
		case 505:
			GenerateErrorMine(wctl);
			cl = wctl->conn.con_res.cl;
			len = strlen(wctl->conn.con_res.res.ptr);
			n = write(cs, ptr, len);
			break;
	}
	return 0;
}
