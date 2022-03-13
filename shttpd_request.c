/*************************************************************************
	> File Name: shttpd_request.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月11日 星期五 15时11分14秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "datatype.h"
#include "shttpd_error.h"
#include "shttpd_method.h"
extern struct conf_opts conf_para;

struct vec _shttpd_methods[] = {
	{"GET", 3, METHOD_GET},
	{"POST", 4, METHOD_POST},
	{"PUT", 3, METHOD_PUT},
	{"DELETE", 6, METHOD_DELETE},
	{"HEAD", 4, METHOD_HEAD},
	{NULL, 0, 0}
};

static struct http_header http_headers[] = {
	{16, HDR_INT, OFFSET(cl), "Content-Length: "},
	{14, HDR_STRING, OFFSET(ct), "Content-Type:"},
	{12, HDR_STRING, OFFSET(useragent), "User-Agent: "},
	{19, HDR_DATE, OFFSET(ims), "If-Modified-Since: "},
	{15, HDR_STRING, OFFSET(auth), "Authorization: "},
	{9, HDR_STRING, OFFSET(referer), "Referer: "},
	{8, HDR_STRING, OFFSET(cookie), "Cookie:"},
	{10, HDR_STRING, OFFSET(location), "Location: "},
	{8, HDR_INT, OFFSET(status), "Status: "},
	{7, HDR_STRING, OFFSET(range), "Range: "},
	{12, HDR_STRING, OFFSET(connection), "Connection: "},
	{19, HDR_STRING, OFFSET(transenc), "Transfer-Encoding: "},
	{0, HDR_INT, 0, NULL}
};

static int montoi(char* s);
static time_t date_to_epoch(char* s);

static int montoi(char* s) {
	static char *ar[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
						 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	size_t i;
	for (i = 0; i < 12; ++i ) {
		if (!strncmp(s, ar[i], 3)) {
			return i;
		}
	}
	return -1;
}

static time_t date_to_epoch(char* s) {
	struct tm t;
	char mon[32];
	int sec, min, hour, mday, month, year;
	memset(&t, 0, sizeof(t));
	sec = min = hour = mday = month = year = 0;
	if (((sscanf(s, "%d/%3s/%d %d:%d:%d", &mday, mon, &year, &hour, &min, &sec) == 6) ||
		(sscanf(s, "%d %3s %d %d:%d:%d", &mday, mon, &year, &hour, &min, &sec) == 6) ||
		(sscanf(s, "%d-%3s-%d %d:%d:%d", &mday, mon, &year, &hour, &min, &sec) == 6)) &&
		(month = montoi(mon) != -1)) {
		t.tm_mday = mday;
		t.tm_mon = month;
		t.tm_year = year;
		t.tm_hour = hour;
		t.tm_min = min;
		t.tm_sec = sec;
	}

	if (t.tm_year > 1900) {
		t.tm_year -= 1900;
	} else {
		t.tm_year += 100;
	}
	return mktime(&t);
}


static void Request_HeaderParse(char* s, int len, struct headers* parsed) {
	struct http_header *h;
	union variant *v;
	char *p, *e = s + len;
	while (s < e) {
		for (p = s; p < e && *p != '\n';) {
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
				if (p[-1] == '\r' && v->v_vec.len > 0) {
					v->v_vec.len--;
					p[-1] = '\0';
				}
				*p = '\0';
				DBG("%s : %s\n", h->name, v->v_vec.ptr);
			} else if (h->type == HDR_INT){
				v->v_big_int = strtoul(s, NULL, 10);
				DBG("%s : %lu\n", h->name, v->v_big_int)
			} else if (h->type == HDR_DATE) {
				v->v_time = date_to_epoch(s);
				struct tm* tmp = localtime(&v->v_time);
				DBG("%s : %d %d %d %d %d %d\n", h->name, tmp->tm_mday, tmp->tm_mon, tmp->tm_year, tmp->tm_hour, tmp->tm_min, tmp->tm_sec)
			}
		}
		s = p + 1;
	}
}


int Request_Parse(struct worker_ctl* wctl) {
	DBG("\n\n\n\nREQUEST PARSE\n");
	struct worker_conn* c = &wctl->conn;
	struct conn_request* req = &c->con_req;
	struct conn_response* res = &c->con_res;
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
	struct vec* m = NULL;
	for (m = _shttpd_methods; m->ptr != NULL; m++) {
		if (!strncmp(p, m->ptr, m->len)) {
			req->method = m->type;
			found = 1;
			DBG("http_method: %s\n", m->ptr);
			break;
		}
	}
	if (found == 0) {
		DBG("http method wrong\n");
		return 400;//bad request
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
	DBG("uri: %s\n", req->uri);
	if (!strcmp(req->uri, "/")) {
		snprintf(req->rpath, URI_MAX, "%s%s", conf_para.DocumentRoot, conf_para.DefaultFile);
	} else {
		snprintf(req->rpath, URI_MAX, "%s%s", conf_para.DocumentRoot, req->uri);
	}
	DBG("rpath: %s\n", req->rpath);
	res->fd = open(req->rpath, O_RDONLY);
	if (res->fd != -1) {
		fstat(res->fd, &res->fstate);
		if (S_ISDIR(res->fstate.st_mode)) {
			DBG("request forbidden\n")
			return 403;//forbidden
		}
	} else {
		DBG("file not found\n");
		return 404;//not found
	}

	pos++;
	len--;
	while(*pos == ' ') {
		pos++;
		len--;
	}
	p = pos;
	sscanf(p, "HTTP/%lu.%lu", &req->major, &req->minor);
	DBG("http version %s\n", p);
	if (!((req->major == 0 && req->minor == 9) || (req->major == 1 & req->minor == 0)||
		(req->major == 1 && req->minor == 1))) {
		DBG("http version not support\n");
		return 505;//version not support
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
	DBG("HEADER PARSE..............\n");
	Request_HeaderParse(p, len, &req->ch);
	DBG("HEADER_PARSE END..........\n");
	return 200;//ok
}

int Request_Handle(struct worker_ctl* wctl) {
	DBG("\n\n\nREQUEST HANDLE\n");
	int err = wctl->conn.con_req.err;
	int cs = wctl->conn.cs;//client_sockfd
	char* ptr = wctl->conn.con_res.res.ptr;
	int fd = wctl->conn.con_res.fd;
	int n, cl;
	switch(err) {
		case 200:
			DBG("200  ok\n");
			my_Method_Do(wctl);
			cl = wctl->conn.con_res.cl;
			n = write(cs, ptr, strlen(ptr));
			DBG("%s", ptr);
			DBG("write %d bytes\n\n", n)
			if (fd != -1) {
				lseek(fd, 0, SEEK_SET);
				
				for (n = 0; cl > 0; cl -= n) {
					memset(ptr, 0, sizeof(wctl->conn.dres));
					n = read(fd, ptr, sizeof(wctl->conn.dres) > cl ? cl : sizeof(wctl->conn.dres));
					DBG("%s",ptr);
					DBG("read %d bytes\n", n)
					if (n > 0) {
						n = write(cs, ptr, n);
						DBG("write %d bytes\n\n", n);
					}
				}
				close(fd);
				wctl->conn.con_res.fd = -1;
			}
			break;
		case 400://bad request
		case 403://forbidden
		case 404://not found
		case 505://version not support;
			my_GenerateErrorMine(wctl);
			n = write(cs, ptr, strlen(ptr));//send to client
			DBG("send error information to client successfully\n");
			break;
	}
	return 0;
}
