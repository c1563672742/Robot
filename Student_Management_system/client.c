#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SQL 500
#define N 32
#define MAN 1
#define WOMAN 2
#define R 1  //注册
#define L 2  //登录
#define A 3  //添加
#define D 4  //删除
#define M 5  //修改
#define Q 6  //查询
#define QA 7  //查询所有
#define RERUEN 8 //返回

typedef struct {
	int types;
	char uid[N];
	char password[N];
	char name[N];
	char sno[N];
	int sex;
	char data[N];
}__attribute__((packed)) MSG;

//注册
int do_register(int sockfd,MSG *msg)
{
	msg->types= R;
	printf("Input uid: ");
	scanf("%s",msg->uid);
	getchar();
	printf("Input password: ");
	scanf("%s",msg->password);
	getchar();
	if(send(sockfd,msg,sizeof(MSG),0)<0){
		printf("Fail to send.\n");
		return -1;
	}
	if(recv(sockfd,msg,sizeof(MSG),0)<0){
		printf("Fail to recv.\n");
		return -1;
	}
	printf("%s\n",msg->data);

	return 0;
}

//登录
int do_login(int sockfd,MSG *msg)
{
	msg->types = L;
	printf("Input uid: ");
	scanf("%s",msg->uid);
	getchar();
	printf("Input password: ");
	scanf("%s",msg->password);
	getchar();

	if(send(sockfd,msg,sizeof(MSG),0)<0){
		printf("Fail to send.\n");
		return -1;
	}
	if(recv(sockfd,msg,sizeof(MSG),0)<0){
		printf("Fail to recv.\n");
		return -1;
	}
	if(strncmp(msg->data,"OK",3) == 0){
		printf("login\n");
		return 1;
	}else{
		printf("%s\n",msg->data);
	}

	return 0;
}

//登录选项
int do_login_select(int sockfd,MSG *msg)
{
	int n;
	while(1){
		printf("*************************************************************\n");
		printf("******    1.add    2.query    3.query_all    4.exit    ******\n");
		printf("*************************************************************\n");
		printf("Please choose: ");
		if(scanf("%d",&n)!=1){
			printf("Input error.\n");
			continue;
		}
		while(getchar() !='\n' );
		switch (n){
		case 1:
			do_add(sockfd,msg);
			break;
		case 2:
			do_query(sockfd,msg);
			break;
		case 3:
			do_query_all(sockfd,msg);
			break;
		case 4:
			return -1;
			break;
		default:
			printf("Invalid input.\n");
		}
	}
	return 0;
}

//添加
int do_add(int sockfd,MSG *msg)
{
	while(1){
		msg->types = A;
		printf("Input student num(return:Input #):\n");
		if(scanf("%s",msg->sno)!=1){
			printf("Invalid operation.\n");
			continue;
		}
		if(strncmp(msg->sno,"#",1)==0) break; 
		getchar();
		printf("Input student name:\n");
		if(scanf("%s",msg->name)!=1){
			printf("Invalid operation.\n");
			continue;
		}
		getchar();
		printf("Input student sex:(Input num: 1.man 2.woman)\n");
		if(scanf("%d",&(msg->sex))!=1){
			printf("Invalid operation.\n");
			continue;
		}
		if((msg->sex!=1)&&(msg->sex!=2)){
			printf("sex input error.\n");
			continue;
		}
		getchar();
		if(send(sockfd,msg,sizeof(MSG),0)<0)
		{
			printf("Fail to send.\n");
			return -1;
		}
		if(recv(sockfd,msg,sizeof(MSG),0)<0)
		{
			printf("Fail to recv.\n");
			return -1;
		}
		printf("%s\n",msg->data);
		if(strncmp(msg->data,"OK",2)==0){
			printf("%s\n",msg->data);
		}else{
			printf("%s\n",msg->data);
		}
	}
	printf("add over.\n");
	return 0;
}

//查询所有
int do_query_all(int sockfd,MSG *msg)
{
	int i,n=500;
	MSG recvmsg[500]={0};
	//清空结构体
	//bzero(&recvmsg,sizeof(recvmsg[500])*500);
	//指定类型
	msg->types = QA;
	if(send(sockfd,msg,sizeof(MSG),0)<0)
	{
		printf("Fail to send.\n");
		return -1;
	}
	if(recv(sockfd,&recvmsg,sizeof(recvmsg[500])*500,0)<0){
		printf("Fail to recv.\n");
		return -1;
	}
	printf("%s\n",recvmsg[0].data);
	if(strncmp(recvmsg[0].data,"OK",2)==0){
		for(i=0;i<n;i++){
			if(recvmsg[i].sex==0) break;
			printf("%d:\n",i);
			printf("name: %s\n",recvmsg[i].name);
			printf("sno: %s\n",recvmsg[i].sno);
			if(recvmsg[i].sex==1){
				printf("sex: man\n");
			}else if(recvmsg[i].sex==2){
				printf("sex: woman\n");
			}
		}
	}
	return 0;
}

//查询
int do_query(int sockfd,MSG *msg)
{
	int num;
	char c;
	while(1){
		msg->types=Q;
		printf("Please input student num(return: input #).\n");
		if(scanf("%s",msg->sno)!=1){
			printf("Invalid input.\n");
			continue;
		}
		getchar();
		if(strncmp(msg->sno,"#",1)==0) break;
		if(send(sockfd,msg,sizeof(MSG),0)<0)
		{
			printf("Fail to send.\n");
			return -1;
		}
		if(recv(sockfd,msg,sizeof(MSG),0)<0){
			printf("Fail to recv.\n");
			return -1;
		}
		if(strncmp(msg->data,"OK",2)==0){
			printf("%s\n",msg->data);
			printf("name: %s\n",msg->name);
			printf("sno: %s\n",msg->sno);
			if(msg->sex==1){
				printf("sex: man\n");
			}else if(msg->sex==2){
				printf("sex: woman\n");
			}
		}else{
			printf("%s\n",msg->data);
			continue;
		}
		while(1){
			printf("Manage this data?(y or n)");
			if(scanf("%c",&c)!=1){
				printf("Invalid input.\n");
				continue;
			}else{
				if(c=='n'||c=='#') break;
				if(c=='y'){
					while(1){
						printf("****************************************************\n");
						printf("******    1.modify    2.delete    3.return    ******\n");
						printf("****************************************************\n");
						printf("Please choose: ");
						if(scanf("%d",&num)!=1){
							printf("Input error.\n");
							continue;
						}
						while(getchar() !='\n' );
						switch (num){
						case 1:
							do_modify(sockfd,msg);
							break;
						case 2:
							do_delete(sockfd,msg);
							break;
						case 3:
							msg->types=RERUEN;
							break;
						default:
							printf("Invalid input.\n");
						}
						if(msg->types==D) break;
						if(msg->types==RERUEN) break;
					}
				}
			}
			if(msg->types==D) break;
			if(msg->types==RERUEN) break;
		}
		if(msg->types==D) break;
		if(msg->types==RERUEN) break;
	}
	return 0;
}

//修改
int do_modify(int sockfd,MSG *msg)
{
	while(1){
		msg->types=M;
		printf("Input new name(return:Input #): ");
		if(scanf("%s",msg->name)!=1){
			printf("Invalid input.\n");
			continue;
		}
		if(strncmp(msg->name,"#",1)==0) break;
		printf("Input new sex:(Input num: 1.man 2.woman) ");
		if(scanf("%d",&(msg->sex))!=1){
			printf("Invalid input.\n");
			continue;
		}
		if((msg->sex!=1)&&(msg->sex!=2)){
			printf("sex input error.\n");
			continue;
		}
		if(send(sockfd,msg,sizeof(MSG),0)<0)
		{
			printf("Fail to send.\n");
			return -1;
		}
		if(recv(sockfd,msg,sizeof(MSG),0)<0){
			printf("Fail to recv.\n");
			return -1;
		}
		printf("%s\n",msg->data);
		break;
	}
	return 0;
}

//删除
int do_delete(int sockfd,MSG *msg)
{
	msg->types = D;
	int ret =msg->types;
	if(send(sockfd,msg,sizeof(MSG),0)<0)
	{
		printf("Fail to send.\n");
		return -1;
	}
	if(recv(sockfd,msg,sizeof(MSG),0)<0){
		printf("Fail to recv.\n");
		return -1;
	}
	printf("%s\n",msg->data);
	return ret;
}


//主函数 
int main(int argc,const char *argv[])
{
	int sockfd;
	int n;
	MSG msg;
	if(argc != 3)
	{
		printf("Usage:%s serverip port.\n",argv[0]);
		return -1;
	}
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in serveraddr;
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));

	if(connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr))<0){
		perror("connect");
		return -1;
	}
	char buf[100]={};
	while(1){
		printf("***************************************************\n");
		printf("******    1.register    2.login    3.exit    ******\n");
		printf("***************************************************\n");
		printf("Please choose: ");
		if(scanf("%d",&n)!=1){
			while(getchar() != '\n');
			printf("Input error.\n");
			continue;
		}
		while(getchar() != '\n');
		switch(n){
		case 1:
			do_register(sockfd,&msg);
			break;
		case 2:
			if(do_login(sockfd,&msg)==1){
				do_login_select(sockfd,&msg);
			}
			break;
		case 3:
			close(sockfd);
			return 0;
			break;
		default:
			printf("Invalid operation.\n");
		}
	}
	return 0;
}
