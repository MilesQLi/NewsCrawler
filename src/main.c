#include"function.h"

int main(int argc, const char *argv[])
{
    apr_status_t rv;
    apr_pool_t *mp;
	apr_hash_index_t *hi;
	apr_hash_t *urlht;
	apr_queue_t *urlqueue;
	char choose;
	apr_initialize();
	apr_pool_create(&mp, NULL);
	urlht = apr_hash_make(mp);
	apr_queue_create(&urlqueue,1000000, mp);
	printf("�Ƿ������й������򣿣����Ƿ���data.txt�ļ����Ѻ���һ���������������ݣ�\n1.�� 2.��");
	scanf("%d",&choose);
	if(choose!=2)
	{
	hashtablereadfromfile(mp,urlht,urlqueue,"data.txt");
    begincraw(mp,urlht,urlqueue,"www.pku.edu.cn/schools/yxsz.jsp",1);
	}
	else
    begincraw(mp,urlht,urlqueue,"www.pku.edu.cn/schools/yxsz.jsp",2);
	system("pause");
    apr_terminate();
    return 0;

 error:
    {
        char errbuf[256];
        apr_strerror(rv, errbuf, sizeof(errbuf));
        printf("error: %d, %s\n", rv, errbuf);
    }
    apr_terminate();
    system("pause");
    return -1;
}

