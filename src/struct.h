#ifndef STRUCT_H
#define STRUCT_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "constant.h"
#include <apr_md5.h>
#include <apr_base64.h>
#include <stdio.h>
#include <assert.h>
#include <apr_general.h>
#include <apr_network_io.h>
#include <apr_strings.h>
#include <apr_file_io.h>
#include <apr_hash.h>
#include <apr_thread_proc.h>
#include <apr_hash.h>
#include <apr_queue.h>
#include <apr_tables.h>
#include <apr_time.h>
#include <apr_date.h>


typedef struct childurlstruct{ 
	char *url;
	struct childurlstruct *next;
} childurlstruct;

typedef struct {  
	const char *url;
	int depth;
	childurlstruct *childurl;
	int childnumber;
	char flag;  //0则表示这个url还从来未检索过，所以第一次把它页面的url搜索出来时不能增加它的times，1表示检索过
	int times;  //更新次数
	char *md5footprint;
	apr_time_t firstvisittime;
	apr_time_t lastvisittime;
	apr_time_t nextvisittime;
} urlinfo;



typedef struct { 
	apr_pool_t *mp;
	apr_hash_t *urlht;
	char *enter;
	apr_queue_t *urlqueue;
} threaddata;

typedef struct { 
	const char *schoolname;
	const char *creattime;
	int newnum;
	int halflife;

} updatenums;

#endif