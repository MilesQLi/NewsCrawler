#include"function.h"


apr_status_t begincraw(apr_pool_t *mp,apr_hash_t *urlht,apr_queue_t *urlqueue,char *enter,int control)
{
	apr_status_t rv;
	char host[50],*file,*temppchar;
	apr_hash_index_t *hi;
	apr_time_t t;
	urlinfo *newurlinfo;
	apr_thread_t *geturlstoqueue, *writetofile;
	threaddata *geturlstoqueuedata=(threaddata *)malloc(sizeof(threaddata)),*writetofiledata=(threaddata *)malloc(sizeof(threaddata));
	if(control==2)
	{
	file=strchr(enter+8,'/');
	if(file==NULL)
	{
		strcpy(host,enter);
		file="/";
	}
	else
		cpstr(host,enter,file-enter);
	apr_queue_push(urlqueue,enter);
	t = apr_time_now();
	newurlinfo= malloc(sizeof(urlinfo));
	newurlinfo->url=apr_pstrdup(mp,enter);
	newurlinfo->firstvisittime=t;
	newurlinfo->lastvisittime=t;
	newurlinfo->nextvisittime=t;  //入口下次访问时间要改
	newurlinfo->depth=0;
	newurlinfo->times=1;
	newurlinfo->flag=0;
	newurlinfo->md5footprint=apr_pstrdup(mp,"aaa");
	newurlinfo->childurl= NULL;
	newurlinfo->childnumber=0;
	printf("url is:%s\n",newurlinfo->url);
	apr_hash_set(urlht,enter,APR_HASH_KEY_STRING,newurlinfo);
/*
	while( apr_queue_size(urlqueue)!=0)
	{
		apr_thread_t *thd_arr;
		threaddata *newdata=(threaddata *)malloc(sizeof(threaddata));
		apr_queue_pop(urlqueue,&temppchar);
		newdata->enter=temppchar;
		newdata->mp=mp;
		newdata->urlqueue=urlqueue;
		newdata->urlht=urlht;
		rv = apr_thread_create(&thd_arr, NULL, donewthread, newdata, mp);
		rv = apr_thread_join(&rv, thd_arr);
		//geturls(mp,urlht,urlqueue,temppchar);
		//Sleep(3000L);
	}*/
	/*for (hi = apr_hash_first(NULL, urlht); hi; hi = apr_hash_next(hi)) {
		const char *key,*content;
		urlinfo *urlinfovalue;
		apr_hash_this(hi, (const void**)&key, NULL, (void**)&urlinfovalue);
	}*/
	}
	geturlstoqueuedata=(threaddata *)malloc(sizeof(threaddata));
	geturlstoqueuedata->mp=mp;
	geturlstoqueuedata->urlqueue=urlqueue;
	geturlstoqueuedata->urlht=urlht;
	rv = apr_thread_create(&geturlstoqueue, NULL, geturlstoqueuefunction, geturlstoqueuedata, mp);

	writetofiledata=(threaddata *)malloc(sizeof(threaddata));
	writetofiledata->mp=mp;
	writetofiledata->urlqueue=urlqueue;
	writetofiledata->urlht=urlht;
	rv = apr_thread_create(&writetofile, NULL, writetofilefunction, writetofiledata, mp);
	while(1)
	{
		printf("apr_queue_size:%d\n",apr_queue_size(urlqueue));
	while( apr_queue_size(urlqueue)!=0)
	{
		apr_thread_t *thd_arr;
		threaddata *newdata=(threaddata *)malloc(sizeof(threaddata));
		urlinfo *newurlinfo;
		apr_queue_pop(urlqueue,&temppchar);
		newurlinfo= apr_hash_get(urlht, apr_pstrdup(mp,temppchar), APR_HASH_KEY_STRING);
        newdata->enter=temppchar;
		newdata->mp=mp;
		newdata->urlqueue=urlqueue;
		newdata->urlht=urlht;
		//printf("firsturl:%s\n",*temppchar);
		//rv = apr_thread_create(&thd_arr, NULL, donewthread, newdata, mp);
		//rv = apr_thread_join(&rv, thd_arr);
		geturls(mp,urlht,urlqueue,temppchar);
	}
	Sleep(2000L);
	//system("pause");
	}
	return APR_SUCCESS;
}
void* APR_THREAD_FUNC writetofilefunction(apr_thread_t *thd, void *data)
{
	threaddata *newdata=(threaddata*)data;
	while(1)
	{
		Sleep(30000L);
		hashtablewritetofile(newdata->mp,newdata->urlht,"data.txt");
	}
}
void* APR_THREAD_FUNC geturlstoqueuefunction(apr_thread_t *thd, void *data)
{
	apr_hash_index_t *hi;
	threaddata *newdata=(threaddata*)data;
	while(1)
	{
		Sleep(5000L);
		for (hi = apr_hash_first(NULL, newdata->urlht); hi; hi = apr_hash_next(hi)) {
			const char *key,*content;
			urlinfo *urlinfovalue;
			apr_time_t t = apr_time_now();
			apr_hash_this(hi, (const void**)&key, NULL, (void**)&urlinfovalue);
			printf("%s 's next visit time last:%ds\n",urlinfovalue->url,(urlinfovalue->nextvisittime-t)/(APR_TIME_C(1000)*1000));
			if((urlinfovalue->nextvisittime-t)/(APR_TIME_C(1000)*1000)<5)
			{
				apr_queue_push(newdata->urlqueue,apr_pstrdup(newdata->mp,urlinfovalue->url));
				printf("pushtoqueue:%s\n",urlinfovalue->url);
			}
		}
	}
	apr_thread_exit(thd, APR_SUCCESS);
	return NULL;
}
void* APR_THREAD_FUNC donewthread(apr_thread_t *thd, void *data)
{
	threaddata *newdata=(threaddata*)data;
	apr_status_t rv;
	rv=geturls(newdata->mp,newdata->urlht,newdata->urlqueue,newdata->enter);
	if (rv != APR_SUCCESS) {
		apr_thread_exit(thd, rv);
	}
	free(newdata);
	apr_thread_exit(thd, APR_SUCCESS);
	return NULL;
}

apr_status_t geturls(apr_pool_t *mp,apr_hash_t *urlht,apr_queue_t *urlqueue,char *enter)
{
	apr_status_t rv;
	apr_socket_t *s;
	char host[50],*file,**temppchar;
	file=strchr(enter+8,'/');
	if(file==NULL)
	{
		strcpy(host,enter);
		file="/";
	}
	else
		cpstr(host,enter,file-enter);
	rv=do_connect(&s,mp,host);
	if(rv!=APR_SUCCESS)
		return rv;
	rv=do_client_task(s,mp,urlht,urlqueue,host,file);
	return rv;
}


apr_status_t do_connect(apr_socket_t **sock,apr_pool_t *mp,char *host)
{
    apr_sockaddr_t *sa;
    apr_socket_t *s;
    apr_status_t rv;
    rv = apr_sockaddr_info_get(&sa, host, APR_INET, DEF_REMOTE_PORT, 0, mp);
    if (rv != APR_SUCCESS) {
	return rv;
    }
    
    rv = apr_socket_create(&s, sa->family, SOCK_STREAM, APR_PROTO_TCP, mp);
    if (rv != APR_SUCCESS) {
	return rv;
    }

/*
    apr_socket_opt_set(s, APR_SO_NONBLOCK, 1);
    apr_socket_timeout_set(s, DEF_SOCK_TIMEOUT);*/

    rv = apr_socket_connect(s, sa);
    if (rv != APR_SUCCESS) {
        return rv;
    }

    apr_socket_opt_set(s, APR_SO_NONBLOCK, 0);
    apr_socket_timeout_set(s, DEF_SOCK_TIMEOUT);


    *sock = s;
    return APR_SUCCESS;
}

apr_status_t do_client_task(apr_socket_t *sock,apr_pool_t *mp,apr_hash_t *urlht,apr_queue_t *urlqueue,char *host,char *file)
{
    apr_status_t rv;
	//char *req_hdr = apr_pstrcat(mp, "GET ",file, " HTTP/1.0" CRLF_STR "Accept: text/html, application/xhtml+xml, */*\nAccept-Encoding: gzip, deflate\nAccept-Language: zh-Hans-CN,zh-Hans;q=0.8,en-US;q=0.5,en;q=0.3\nConnection: Keep-Alive\nDNT: 1\nHost: ",host,CRLF_STR CRLF_STR, NULL);
	char *req_hdr = apr_pstrcat(mp, "GET ",file, " HTTP/1.1" CRLF_STR "Accept: text/html, application/xhtml+xml, */*\nHost: ",host,CRLF_STR CRLF_STR, NULL);
	apr_size_t len,templen=0,urllen,/*forsavetext,*/size;
	char *info="",url[1000],*tempp,*tempp2,*fatherurl,*backupurl,ifgetchanged;
	urlinfo *urlinfo;
	apr_time_t t;
	char bsalt[64]="ir";/*number(binary) for salt */
	char salt[128];
	char digest[128];
    len= strlen(req_hdr);
	//printf("%s",req_hdr);
	if(strcmp(file,"/")!=0)
	fatherurl=apr_pstrcat(mp,host,file,NULL);
	else
	{
		fatherurl=apr_pstrcat(mp,host,NULL);
	}
	urlinfo= apr_hash_get(urlht, apr_pstrdup(mp,fatherurl), APR_HASH_KEY_STRING);
	if(urlinfo->depth>3)
		return APR_SUCCESS;
	printf("\nfather:%s\n",fatherurl);
	rv = apr_socket_send(sock, req_hdr, &len);
	if (rv != APR_SUCCESS) {
        return rv;
    }
//	printf("3\n");
        while (1) {
            char buf[BUFSIZE];
            apr_size_t len = sizeof(buf)-1;
            apr_status_t rv;
			rv = apr_socket_recv(sock, buf, &len);
		//	printf("size:%d",len);
			buf[len]='\0';
			/*printf("%s\n",buf);
			system("pause");*/
			info=apr_pstrcat(mp,info,buf, NULL);
            if (rv == APR_EOF || len == 0) {
                break;
            }
		}
	//	printf("5\n");
		tempp=apr_pstrndup(mp, info+9, 3);
		if(apr_strnatcmp(tempp, "200")==0)
		{
		/*printf("%s\n",info);
		system("pause");*/
		t = apr_time_now();
	//	sprintf(date,"%"APR_TIME_T_FMT "", apr_time_sec(t));
		tempp=strstr(info,"\r\n\r\n");
		apr_base64_encode(salt, bsalt, sizeof(bsalt));
		apr_md5_encode(tempp, salt, digest, sizeof(digest));
		//printf("salt:%s\n",digest);
		//printf("footprintf:%s\n",urlinfo->md5footprint);
		if(strcmp(urlinfo->md5footprint,digest)==0)
		{
		//	printf("网页内容未曾改变\n");
			urlinfo->lastvisittime=t;		
			urlinfo->nextvisittime=t+(t-urlinfo->firstvisittime)/(urlinfo->times-1)/5;  //urlinfo->times初始值为1所以-1，为更新次数，扫描频率为更新周期的1/5
			return APR_SUCCESS;
		}
		else
		{
		 urlinfo->md5footprint=apr_pstrdup(mp,digest);
		 urlinfo->times++;
		 urlinfo->lastvisittime=t;
		 urlinfo->nextvisittime=t+(t-urlinfo->firstvisittime)/(urlinfo->times-1)/5;  //urlinfo->times初始值为1所以-1，为更新次数，扫描频率为更新周期的1/5
		}
		while(tempp=strstr(tempp,"href="))
		{
			tempp+=5;
            tempp2=strstr(tempp,"\"http://");
			if(tempp2==tempp)
			{
			getcontentstr(url,&tempp,"\"http://",8,"\"","\'",10000);
		//	printf("now url is:%s\n",url);
			getinarray(mp,urlht,urlqueue,t,url,urlinfo,urlinfo->flag,&ifgetchanged);
		    }
			else
			{
				tempp2=strstr(tempp,"\'http://");
				if(tempp2==tempp)
				{
				getcontentstr(url,&tempp,"\'http://",8,"\'","\"",10000);
			//	printf("now url is:%s\n",url);
				getinarray(mp,urlht,urlqueue,t,url,urlinfo,urlinfo->flag,&ifgetchanged);
				}
				else
				{
					tempp2=strstr(tempp,"\"/");
					if(tempp2==tempp)
					{
						getcontentstr(url,&tempp,"\"/",2,"\"","\'",10000);
						backupurl=apr_pstrcat(mp,host,"/",url,NULL);
			//			printf("now url is:%s\n",backupurl);
						getinarray(mp,urlht,urlqueue,t,backupurl,urlinfo,urlinfo->flag,&ifgetchanged);
					}
					else
					{
						tempp2=strstr(tempp,"\'/");
						if(tempp2==tempp)
						{
							getcontentstr(url,&tempp,"\'/",2,"\'","\"",10000);
							backupurl=apr_pstrcat(mp,host,"/",url,NULL);
			//				printf("now url is:%s\n",backupurl);
							getinarray(mp,urlht,urlqueue,t,backupurl,urlinfo,urlinfo->flag,&ifgetchanged);
						}
						else
						{
                            tempp2=strstr(tempp,"\"");
							if(tempp2==tempp)
							{
								getcontentstr(url,&tempp,"\"",1,"\"","\'",10000);
								backupurl=apr_pstrcat(mp,host,"/",url,NULL);
					//			printf("now url is:%s\n",backupurl);
								getinarray(mp,urlht,urlqueue,t,backupurl,urlinfo,urlinfo->flag,&ifgetchanged);
							}
							else
							{
								tempp2=strstr(tempp,"\'");
								if(tempp2==tempp)
								{
									getcontentstr(url,&tempp,"\'",1,"\'","\"",10000);
									backupurl=apr_pstrcat(mp,host,"/",url,NULL);
						//			printf("now url is:%s\n",backupurl);
									getinarray(mp,urlht,urlqueue,t,backupurl,urlinfo,urlinfo->flag,&ifgetchanged);
								}
							}
						}
					}
				}
			}
		
		
		}
		urlinfo->flag=1;
		if(ifgetchanged==1)
		urlinfo->nextvisittime+=(urlinfo->nextvisittime-t)/2;
		}
		else
		{
			printf("%s\n",apr_pstrcat(mp, host, "访问失败，状态码：",tempp, NULL));
			return rv;
		}
	 return rv;
}

apr_status_t getcontentstr(char *result,char **resource,char *begin,apr_size_t len,char *end,char *end2,apr_size_t num)
{
	char *temp;
	int templen;
	*resource=strstr(*resource,begin);
	if(*resource==NULL)
		return 0;
	*resource+=len;
	temp=strstr(*resource,end);
	templen=temp-*resource;
	if(templen>num||templen<0)
	{
		temp=strstr(*resource,end2);
		templen = temp - *resource;
		if(templen>num||templen<0)
		{
			temp=strstr(*resource,">");
			templen = temp - *resource;
		}
	}
	cpstr(result, *resource, templen); //到此获取了url的id
	return 1;
}

void cpstr(char *des,char *res,size_t len)
{
	memcpy(des, res, len);
	des[len] = '\0';
}

apr_status_t getinarray(apr_pool_t *mp,apr_hash_t *urlht,apr_queue_t *urlqueue,apr_time_t t,char *url,urlinfo *father,char flag,char *ifgetchanged)
{
	urlinfo *newurlinfo;
	*ifgetchanged=0; 
	if(ifhasthischild(mp,url,father,1,flag,ifgetchanged))
		return APR_SUCCESS;
	newurlinfo= apr_hash_get(urlht, apr_pstrdup(mp,url), APR_HASH_KEY_STRING);	
	if(newurlinfo==NULL) 
	{
		apr_queue_push(urlqueue,apr_pstrdup(mp,url));
		newurlinfo= malloc(sizeof(urlinfo));
		newurlinfo->url=apr_pstrdup(mp,url);
		newurlinfo->firstvisittime=t;
		newurlinfo->lastvisittime=t;
		newurlinfo->nextvisittime=t+TIME_ORIGIN_DALTA;
		newurlinfo->times=1;
		newurlinfo->flag=0;
		newurlinfo->md5footprint=apr_pstrdup(mp,"aaa");
		newurlinfo->depth=father->depth+1;
		newurlinfo->childnumber=0;
		newurlinfo->childurl=NULL;
		printf("url is:%s\n",newurlinfo->url);
		apr_hash_set(urlht,newurlinfo->url,APR_HASH_KEY_STRING,newurlinfo);
	}
	return APR_SUCCESS;
}


apr_status_t hashtablewritetofile(apr_pool_t *mp,apr_hash_t *urlht,char *datafilename)
{
	apr_status_t rv;
	apr_file_t *fileobj = NULL;
	apr_hash_index_t *hi;
	apr_size_t size;
	if ((rv = apr_file_open(&fileobj,datafilename, APR_CREATE|APR_WRITE|APR_BINARY|APR_TRUNCATE, APR_OS_DEFAULT, mp)) != APR_SUCCESS) {
		goto done;
	}
	for (hi = apr_hash_first(NULL, urlht); hi; hi = apr_hash_next(hi)) {
		char *key,*content,firsttime[APR_RFC822_DATE_LEN + 1],lasttime[APR_RFC822_DATE_LEN + 1],nexttime[APR_RFC822_DATE_LEN + 1],depth[5],times[5],childnumber[5],*childurls="";
		const urlinfo *urlinfovalue;
		int num;
		childurlstruct *childsruct;
		apr_hash_this(hi, (const void**)&key, NULL, (void**)&urlinfovalue);
		apr_rfc822_date(firsttime,urlinfovalue->firstvisittime);
		apr_rfc822_date(lasttime,urlinfovalue->lastvisittime);
		apr_rfc822_date(nexttime,urlinfovalue->nextvisittime);
		sprintf(times,"%d\0", urlinfovalue->times);
		sprintf(depth,"%d\0", urlinfovalue->depth);
		num=urlinfovalue->childnumber;
		childsruct=urlinfovalue->childurl;
		while(num--)
		{
			char *temp;
			temp=apr_pstrcat(mp,childurls,childsruct->url,"\n",NULL);
			childurls=apr_pstrdup(mp,temp);
			childsruct=childsruct->next;
		}
		sprintf(childnumber,"%d\0", urlinfovalue->childnumber);
		content=apr_pstrcat(mp,"本条url信息：\n",urlinfovalue->url,"\n",childnumber,"\n",childurls
			,times,"\n",depth,"\n",firsttime,"\n",lasttime,"\n",nexttime,"\n",urlinfovalue->md5footprint,"\n", NULL);
		size=strlen(content);
		//printf("content=%s\n",content);
		rv = apr_file_write(fileobj, content, &size);if (rv != APR_SUCCESS) {
			goto done;
		}

	}
	apr_file_close(fileobj);
	return rv;

done:
	if (fileobj) {
		apr_file_close(fileobj);
	}
	return rv;
}


apr_status_t hashtablereadfromfile(apr_pool_t *mp,apr_hash_t *urlht,apr_queue_t *urlqueue,char *datafilename)
{
	apr_status_t rv;
	apr_file_t *fileobj = NULL;
	apr_size_t size;
	char buf[1000];
	if ((rv = apr_file_open(&fileobj,datafilename, APR_CREATE|APR_READ|APR_BINARY|APR_BUFFERED, APR_OS_DEFAULT, mp)) != APR_SUCCESS) {
		goto done;
	}
	while(rv =apr_file_gets(buf,1000,fileobj)==APR_SUCCESS)
	{
		childurlstruct *childsruct;
		urlinfo *newurlinfo;
		char *num,*childnum,*time,*depth,test[APR_RFC822_DATE_LEN + 1];
		int childnumber;
		rv =apr_file_gets(buf,1000,fileobj);
		newurlinfo= malloc(sizeof(urlinfo));
		size=strlen(buf);
		buf[size-1]='\0';
		newurlinfo->url=apr_pstrdup(mp,buf);
		apr_queue_push(urlqueue,apr_pstrdup(mp,newurlinfo->url));
	//	printf("url:%s\n",newurlinfo->url);
		rv =apr_file_gets(buf,100,fileobj);
		size=strlen(buf);
		buf[size-1]='\0';
		childnum=apr_pstrdup(mp,buf);
		newurlinfo->childnumber=atoi(childnum);
		childnumber=newurlinfo->childnumber;
		if(childnumber!=0)
		{
		rv =apr_file_gets(buf,1000,fileobj);
		size=strlen(buf);
		buf[size-1]='\0';
		newurlinfo->childurl=malloc(sizeof(struct childurlstruct));
		newurlinfo->childurl->url=apr_pstrdup(mp,buf);
		newurlinfo->childurl->next=NULL;
	//	printf("%s's childurl:%s\n",newurlinfo->url,newurlinfo->childurl->url);
		childsruct=newurlinfo->childurl;
	//	printf("childnumber:%d\n",childnumber);
		while(--childnumber)
		{
	//		printf("childnumber:%d\n",childnumber);
		rv =apr_file_gets(buf,1000,fileobj);
		size=strlen(buf);
		buf[size-1]='\0';
		childsruct->next=malloc(sizeof(struct childurlstruct));
		childsruct->next->url=apr_pstrdup(mp,buf);
		childsruct->next->next=NULL;
	//	printf("%s's childurl:%s\n",newurlinfo->url,childsruct->next->url);
		childsruct=childsruct->next;
		}
		}
		else
		{
			newurlinfo->childurl=NULL;
		}
		rv =apr_file_gets(buf,100,fileobj);
		size=strlen(buf);
		buf[size-1]='\0';
		num=apr_pstrdup(mp,buf);
		newurlinfo->times=atoi(num);
	//	printf("time:%d\n",newurlinfo->times);
		rv =apr_file_gets(buf,100,fileobj);
		size=strlen(buf);
		buf[size-1]='\0';
		depth=apr_pstrdup(mp,buf);
		newurlinfo->depth=atoi(depth);
		//	printf("depth:%d\n",newurlinfo->depth);
		rv =apr_file_gets(buf,100,fileobj);
		size=strlen(buf);
		buf[size-1]='\0';
		time=apr_pstrdup(mp,buf);
		newurlinfo->firstvisittime=apr_date_parse_rfc(time);
	//	apr_rfc822_date(test, newurlinfo->firstvisittime);
	//	printf("firsttime:%s\n",test);
		rv =apr_file_gets(buf,100,fileobj);
		size=strlen(buf);
		buf[size-1]='\0';
		time=apr_pstrdup(mp,buf);
		newurlinfo->lastvisittime=apr_date_parse_rfc(time);
	//	apr_rfc822_date(test, newurlinfo->lastvisittime);
	//	printf("lasttime:%s\n",test);
		rv =apr_file_gets(buf,100,fileobj);
		size=strlen(buf);
		buf[size-1]='\0';
		time=apr_pstrdup(mp,buf);
		newurlinfo->nextvisittime=apr_date_parse_rfc(time);
	//	apr_rfc822_date(test, newurlinfo->nextvisittime);
	//	printf("nexttime:%s\n",test);
		rv =apr_file_gets(buf,200,fileobj);
		size=strlen(buf);
		buf[size-1]='\0';
		newurlinfo->md5footprint=apr_pstrdup(mp,buf);
		apr_hash_set(urlht, newurlinfo->url, APR_HASH_KEY_STRING, newurlinfo);
	}
	apr_file_close(fileobj);
	return rv;
done:
	if (fileobj) {
		apr_file_close(fileobj);
	}
	return rv;
}

char ifhasthischild(apr_pool_t *mp,char *childurl,urlinfo *father,char control,char flag,char *ifgetchanged)
{
	char result;
	childurlstruct *allchildurl,*temp;
	allchildurl=father->childurl;
	//printf("childurl=%s\n",childurl);
	if(father->childurl!=NULL)
	{
	while(allchildurl!=NULL)
	{

	//	printf("allchildurl=%s\n",allchildurl->url);
		if(strcmp(allchildurl->url,childurl)==0)
		break;
		else
		{
			temp=allchildurl;
			allchildurl=allchildurl->next;
		}
	}
	if(allchildurl==NULL&&control==1)
	{
      temp->next=malloc(sizeof(struct childurlstruct));
	  temp->next->url=apr_pstrdup(mp,childurl);
	  temp->next->next=NULL;
	//  printf("%s add childurl=%s\n",father->url,temp->next->url);
	  father->childnumber++;
	  if(flag==1)
	  *ifgetchanged=1;
	return 0;
	}
	else if(allchildurl==NULL&&control==0)
		return 0;
	else
	{
	//	printf("%s 's childurl已存在%s\n",father->url,allchildurl->url);
		return 1;
	}
	}
	else
	{
		father->childurl=malloc(sizeof(struct childurlstruct));
		father->childnumber=1;
		father->childurl->url=apr_pstrdup(mp,childurl);
		father->childurl->next=NULL;
	//	printf("%s add childurl=%s\n",father->url,father->childurl->url);
		return 0;
	}

}