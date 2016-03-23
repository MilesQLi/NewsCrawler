#ifndef FUNCTION_H
#define FUNCTION_H
#include "constant.h"
#include "struct.h"


/************************************************************************/
/* TCPЭ��                                                                    */
/************************************************************************/

apr_status_t do_connect(apr_socket_t **sock,apr_pool_t *mp,char *host);
int do_client_task(apr_socket_t *sock, apr_pool_t *mp,apr_hash_t *urlht,apr_queue_t *urlqueue,char *host,char *file);

/************************************************************************/
/* ��Ҫ����                                                                    */
/************************************************************************/
apr_status_t begincraw(apr_pool_t *mp,apr_hash_t *urlht,apr_queue_t *urlqueue,char *enter,int control);
apr_status_t geturls(apr_pool_t *mp,apr_hash_t *urlht,apr_queue_t *urlqueue,char *enter);
apr_status_t getinarray(apr_pool_t *mp,apr_hash_t *urlht,apr_queue_t *urlqueue,apr_time_t t,char *url,urlinfo *father,char flag,char *ifgetchanged);
void* APR_THREAD_FUNC donewthread(apr_thread_t *thd, void *data);
void* APR_THREAD_FUNC geturlstoqueuefunction(apr_thread_t *thd, void *data);
void* APR_THREAD_FUNC writetofilefunction(apr_thread_t *thd, void *data);
char ifhasthischild(apr_pool_t *mp,char *childurl,urlinfo *father,char control,char flag,char *ifgetchanged);  //flag: 1��ʾ��childurl�������ھͼ��룬0��ʾû�в����� ����1��ʾ���ڣ�����0��ʾ������
/************************************************************************/
/* д���ļ�                                                                   */
/************************************************************************/
apr_status_t hashtablewritetofile(apr_pool_t *mp,apr_hash_t *urlht,char *datafilename);
apr_status_t hashtablereadfromfile(apr_pool_t *mp,apr_hash_t *urlht,apr_queue_t *urlqueue,char *datafilename);

/************************************************************************/
/* ���ڴ����ַ���                                                                     */
/************************************************************************/
apr_status_t getcontentstr(char *result,char **resource,char *begin,apr_size_t len,char *end,char *end2,apr_size_t num);
void cpstr(char *des,char *res,size_t len);
#endif
