#ifndef CONSTANT_H
#define CONSTANT_H
#define DEF_REMOTE_PORT		80

#define DEF_SOCK_TIMEOUT	(APR_USEC_PER_SEC * 30)

#define BUFSIZE			10000

#define CRLF_STR		"\r\n"

#define ARRAY_INIT_SZ           100

#define TIME_ORIGIN_DALTA	 APR_TIME_C(1000)*1000*30    //APR_TIME_C(1000)*1000��1����

#define SHORTEST_DALTA	 APR_TIME_C(1000)*1000*60

#endif
