#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

#define DATABASE "my.db"

#define SQL 500
#define N 32
#define MAN 0
#define WOMAN 1
#define R 1  //注册
#define L 2  //登录
#define A 3  //添加
#define D 4  //删除
#define M 5  //修改
#define Q 6  //查询
#define QA 7  //查询所有
#define EIXT 8 //退出

//定义通信结构体
typedef struct {
	int types;
	char uid[N];
	char password[N];
	char name[N];
	char sno[N];
	int sex;
	char data[N];
}__attribute__((packed)) MSG;

sqlite3 *db;



void sig_child_handle(int signo)
{
	if(signo == SIGCHLD)
		waitpid(-1,NULL,WNOHANG);
}


//选项
int do_client(int acceptfd,sqlite3 *db)
{
	MSG msg;
	while(recv(acceptfd,&msg,sizeof(msg),0)>0)
	{
		printf("types:%d\n",msg.types);
		switch(msg.types)
		{
		case R:
			do_register(acceptfd,&msg,db);
			break;
		case L:
			do_login(acceptfd,&msg,db);
			break;
		case A:
			do_add(acceptfd,&msg,db);
			break;
		case D:
			do_delete(acceptfd,&msg,db);
			break;
		case M:
			do_modify(acceptfd,&msg,db);
			break;
		case Q:
			do_query(acceptfd,&msg,db);
			break;
		case QA:
			do_query_all(acceptfd,db);
			break;
		default:
			printf("Invalid data msg.\n");
		}
	}
	printf("client exit.\n");
	close(acceptfd);
	exit(0);
	return 0;
}

//注册
int do_register(int acceptfd,MSG *msg,sqlite3 *db)
{
	char *errmsg = NULL;
	char sql[SQL] = {};
	int ret;
	sprintf(sql,"insert into usr values('%s', '%s');",msg->uid,msg->password);
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("%s\n",errmsg);
		strcpy(msg->data,"usr name already exist.");
	}else{
		printf("client register success\n");
		strcpy(msg->data,"OK");
	}
	do{
		ret = send(acceptfd,msg,sizeof(MSG),0);
	}while(ret<0 && errno==EINTR);
	if(ret<0){
		perror("fail to send:");
		return -1;
	}
	return 0;
}

//登录
int do_login(int acceptfd,MSG *msg,sqlite3 *db)
{
	char sql[SQL] = {};
	char *errmsg;
	int nrow;
	int ncloumn;
	char **resultp;
	sprintf(sql,"select * from usr where uid = '%s' and password = '%s';",msg->uid,msg->password);
	printf("%s\n",sql);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
		return -1;
	}else{
		printf("get_table ok\n");
	}
	//有此用户
	if(nrow == 1){
		strcpy(msg->data,"OK");
		send(acceptfd,msg,sizeof(MSG),0);
		return 1;
	}
	//密码或用户名错误
	if(nrow == 0){
		strcpy(msg->data,"usr or passwd wrong.");
		send(acceptfd,msg,sizeof(MSG),0);
	}
	
	return 0;
}

//添加
int do_add(int acceptfd,MSG *msg,sqlite3 *db)
{
	char sql[SQL] = {};
	char *errmsg; 
	char **resultp;
	int nrow;
	int ncloumn;
	//检查学号是否存在
	sprintf(sql,"select * from std_info where sno = %s;",msg->sno);
	printf("%s\n",sql);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!= SQLITE_OK){
		printf("%s\n",errmsg);
	}else{
		if(nrow>0){
			strcpy(msg->data,"student exist.");
			if(send(acceptfd,msg,sizeof(MSG),0)<0){
				perror("Fail to send:");
			}
			return -1;
		}
	}
	//插入学生信息
	sprintf(sql,"insert into std_info values('%s','%s',%d);",msg->name,msg->sno,msg->sex);
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK){
		printf("%s\n",errmsg);
		strcpy(msg->data,"insert std_info error.");
	}else{
		printf("insert info success.\n");
		strcpy(msg->data,"OK");
	}
	if(send(acceptfd,msg,sizeof(MSG),0)<0){
		perror("fail to send:");
	}

	return 0;
}

//删除
int do_delete(int acceptfd,MSG *msg,sqlite3 *db)
{
	char sql[SQL] = {};
	char *errmsg;
	char **resultp;
	int nrow;
	int ncloumn;
	//删除学生信息
	sprintf(sql,"delete from std_info where sno = %s;",msg->sno);
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK){
		strcpy(msg->data,"delete error.");
		printf("%s\n",errmsg);
		return -1;
	}
	strcpy(msg->data,"OK");
	if(send(acceptfd,msg,sizeof(MSG),0)<0){
		perror("Fail to send:");
	}
	return 0;
}

//修改
int do_modify(int acceptfd,MSG *msg,sqlite3 *db)
{
	char sql[SQL] = {};
	char *errmsg;
	sprintf(sql,"update std_info set name = '%s',sex = %d where sno = '%s';",msg->name,msg->sex,msg->sno);
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK){
		strcpy(msg->data,"update error.");
		printf("%s\n",errmsg);
		return -1;
	}
	strcpy(msg->data,"OK");
	if(send(acceptfd,msg,sizeof(MSG),0)<0){
		perror("Fail to send:");
	}
	return 0;
}

//按学号查询
int do_query(int acceptfd,MSG *msg,sqlite3 *db)
{
	int i;
	MSG recvmsg;
	char sql[SQL] = {};
	char *errmsg;
	char **resultp;
	int nrow;
	int ncloumn;
	sprintf(sql,"select * from std_info where sno = '%s';",msg->sno);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!= SQLITE_OK){
		printf("sql:%s\n",errmsg);
		return -1;
	}else{
		if(nrow == 0){
			strcpy(msg->data,"student not exist."); 
		}else{
			strcpy(msg->data,"OK");
			strcpy(msg->name,resultp[3]);
			strcpy(msg->sno,resultp[4]);
			msg->sex = atoi(resultp[5]);
		}
		if(send(acceptfd,msg,sizeof(MSG),0)<0){
			perror("Fail to send:");
		}
	}
	return 0;
}


//创建数据库
int do_database()
{
	char sql[SQL]={};
	char *errmsg;
	char **resultp;
	int nrow;
	int ncloumn;
	printf("create database.\n");
	if(sqlite3_open(DATABASE,&db)!=SQLITE_OK){
		printf("%s\n",errmsg);
		return -1;
	}
	sprintf(sql,"create table if not exists usr(uid text primary key,password text);");
	printf("%s\n",sql);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK){
		printf("%s\n",errmsg);
		return -1;
	}
	printf("create database.\n");
	sprintf(sql,"create table if not exists std_info(name text,sno text primary key,sex integer);");
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK){
		printf("%s\n",errmsg);
		return -1;
	}
	printf("create or open database success.\n");
	return 0;
}

//查询所有
int do_query_all(int acceptfd,sqlite3 *db)
{
	int i=0,j=3;
	MSG recvmsg[500]={0};
	char sql[SQL]={};
	char *errmsg;
	char **resultp;
	int nrow;
	int ncloumn;
	printf("1..%d",sizeof(recvmsg[500]));
	sprintf(sql,"select * from std_info;");
	printf("%s\n",sql);
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg)!=SQLITE_OK){
		printf("sql:%s\n",errmsg);
		return -1;
	}
	printf("2..%d",sizeof(recvmsg[500]));
	if(nrow == 0){
		strcpy(recvmsg[0].data,"table is empty.");
	}else{
		strcpy(recvmsg[0].data,"OK");
		for(i=0;i<nrow;i++){
			strcpy(recvmsg[i].name,resultp[j++]);
			strcpy(recvmsg[i].sno,resultp[j++]);
			recvmsg[i].sex = atoi(resultp[j++]);
			printf("%s\n",recvmsg[i].name);
			printf("%s\n",recvmsg[i].sno);
			printf("%d\n",recvmsg[i].sex);
		}
	}
	if(send(acceptfd,recvmsg,sizeof(recvmsg[500])*500,0)<0){
		perror("Fail to send:");
	}
	return 0;
}


//主函数
int main(int argc,const char *argv[])
{
	int sockfd;
	int acceptfd;
	MSG msg;
	char *errmsg = NULL;
	pid_t pid;
	int n;
	if(argc !=3){
		printf("Usage:%s serverip port.\n",argv[0]);
		return -1;
	}
	do_database();
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

	if(bind(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr))<0){
		perror("bind");
		return -1;
	}

	if(listen(sockfd,5)<0){
		perror("listen");
		return -1;
	}

	signal(SIGCHLD,sig_child_handle);

	while(1){
		if((acceptfd = accept(sockfd,NULL,NULL))<0){
			perror("accept");
			return -1;
		}
		if((pid = fork())<0){
			perror("fork");
			return -1;
		}else if(pid == 0){
			close(sockfd);
			do_client(acceptfd,db);
		}else{
			close(acceptfd);
		}
	}
	return 0;
}
