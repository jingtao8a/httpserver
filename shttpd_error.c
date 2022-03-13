/*************************************************************************
	> File Name: shttpd_error.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月11日 星期五 17时35分43秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include "datatype.h"

enum
{
    ERROR301, ERROR302, ERROR303, ERROR304, ERROR305, ERROR307,
    ERROR400, ERROR401, ERROR402, ERROR403, ERROR404, ERROR405, ERROR406, 
    ERROR407, ERROR408, ERROR409, ERROR410, ERROR411, ERROR412, ERROR413,
    ERROR414, ERROR415, ERROR416, ERROR417,
    ERROR500, ERROR501, ERROR502, ERROR503, ERROR504, ERROR505
};
struct error_mine
{
    int error_code;//错误代码
    char *msg;//含义
    char *content;//错误信息
    int status;//状态
};

//错误信息数组
static struct error_mine _error_http[]=
{
    {ERROR301, "Error: 301", "MOVE FOREVER",         301},
    {ERROR302, "Error: 302", "CREATE",             302},
    {ERROR303, "Error: 303", "OBSERVER OTHER PARTS",     303},
    {ERROR304, "Error: 304", "READ ONLY",             304},
    {ERROR305, "Error: 305", "USER AGENT",         305},
    {ERROR307, "Error: 307", "TEMPORARY REDICT",         307},    
    {ERROR400, "Error: 400", "BAD REQUEST",             400},
    {ERROR401, "Error: 401", "NOT AUTHORIZE",         401},
    {ERROR402, "Error: 402", "NECESSARY PAY",         402},
    {ERROR403, "Error: 403", "FORBIDDEN",             403},
    {ERROR404, "Error: 404", "NOT FOUND",             404},
    {ERROR405, "Error: 405", "NOT PERMISSION",     405},
    {ERROR406, "Error: 406", "NOT RECEIVED",             406},
    {ERROR407, "Error: 407", "NEED AGENT TO AUTHENTICATE",     407},
    {ERROR408, "Error: 408", "REQUEST TIMEOUT",         408},
    {ERROR409, "Error: 409", "CONFICT",             409},
    {ERROR410, "Error: 410", "STOP",             410},
    {ERROR411, "Error: 411", "REQUIRED LENGTH",         411},
    {ERROR412, "Error: 412", "PRETREAMENT FAIL",         412},
    {ERROR413, "Error: 413", "THE REQUESTED ENTITY IS TOO LARGE",     413},
    {ERROR414, "Error: 414", "THE REQUESTED URI IS TOO LARGE",     414},
    {ERROR415, "Error: 415", "UNSUPPORTED MEDIA TYPE",415},
    {ERROR416, "Error: 416", "THE REQUESTED SCOPE IS NOT SATISFIED",416},
    {ERROR417, "Error: 417", "EXPECT FAIL",         417},
    {ERROR500, "Error: 500", "INTERNAL SERVER ERROR", 500},
    {ERROR501, "Error: 501", "CAN'T REALIZE",         501},
    {ERROR502, "Error: 502", "BAD GATEWAT",             502},
    {ERROR503, "Error: 503", "SERVICE CANNOT BE REALIZED",     503},
    {ERROR504, "Error: 504", "GATEWAY TIMEOUT",         504},
    {ERROR505, "Error: 505", "HTTP VERSION NOT SUPPORTED", 505}
};

int my_GenerateErrorMine(struct worker_ctl* wctl) {
	struct error_mine* err = NULL;
	int i = 0;
	for (err = _error_http; err->status != wctl->conn.con_req.err; err++);
	if (err->status != wctl->conn.con_req.err) {
		err = &_error_http[0];
	}
    memset(wctl->conn.dres, 0, sizeof(wctl->conn.dres));
	snprintf(wctl->conn.dres, sizeof(wctl->conn.dres),\
    "HTTP/%lu.%lu %d %s\r\n\
    Content-Type:%s\r\n\
    Content-Length:%d\r\n\
    \r\n\
    %s", \
    wctl->conn.con_req.major, wctl->conn.con_req.minor, err->status, err->msg, \
    "text/plain", \
    (int)strlen(err->content),\
    err->content);
    
	wctl->conn.con_res.cl = strlen(err->content);
	wctl->conn.con_res.fd = -1;
	wctl->conn.con_res.status = err->status;
	return 0;
}

