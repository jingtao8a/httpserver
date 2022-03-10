/*************************************************************************
	> File Name: shttpd.c
	> Author: yuxintao
	> Mail: 1921056015@qq.com 
	> Created Time: 2022年03月10日 星期四 16时09分59秒
 ************************************************************************/

#include <stdio.h>
#include "datatype.h"
#include "cmdparse.h"
#include "fileparse.h"

struct conf_opts conf_para = {
	"/usr/local/var/www/cgi-bin",	//CGIRoot
	"index.html",	//DefalutFile
	"/usr/local/var/www",	//DocumetRoot
	"/etc/SHTTPD.conf",	//ConfiFile
	8080,	//port
	4,	//MaxClient
	3,	//timeout
	2	//threadnum
};



int main(int argc, char** argv){
	Para_CmdParse(argc, argv);
	Para_FileParse(conf_para.ConfigFile);
	return 0;
}
