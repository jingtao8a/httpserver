/*************************************************************************
	> File Name: shttpd_request.h
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月11日 星期五 15时11分20秒
 ************************************************************************/

#ifndef SHTTPD_REQUEST_H
#define SHTTPD_REQUEST_H
#include "datatype.h"

int Request_Parse(struct worker_ctl* wctl);

int Request_Handle(struct worker_ctl* wctl);

#endif
