#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#define IPC_KEY_SM 0x20021203
#define IPC_KEY_DM 0x30021201
struct message{
    long msg_type;
    char msgtext[100];
};

extern int errno;
int main(int argc , char * argv[])
{
    int i;
    int ret = 0,flag = 1;
    if(argc < 2){
        return -1;
    }
    
    if(argc > 2){
        flag = strncmp(argv[2], "dm", 2);
    }
    int msgid = msgget((flag ? IPC_KEY_SM:IPC_KEY_DM), IPC_CREAT|0666);

    struct message msgq;
    msgq.msg_type = 9527;
    strcpy(msgq.msgtext, argv[1]);
#if 0
    for(i=0; i< argc; i++){
       printf(" %s ", argv[i]);
    }
    printf("argc =%d  len of cmd %d, msgid=%d\n",argc,strlen(argv[1]), msgid);
#endif
    ret = msgsnd(msgid, &msgq, 100 , 0);
    if(ret != 0){
        printf("msgsend failed %s\n",strerror(errno)); 
	return -1;
    }
    if(argc > 2){
        msgq.msg_type = 1234;
        msgrcv(msgid, &msgq, 100, 1234, 0); 
    }
    return 0;
}

