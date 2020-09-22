#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define size 256
#define ORG 0
#define PIPE 1
#define REDIR1 2
#define REDIR2 3
#define REDIR3 4
typedef struct pt{
	char str[size];
    struct pt *next;
}node;
node *head;
int type;
char* STR_TOK(char*str);
void to_tok(char t_buf[],char *t_arg[])
{
	char *p=0;
	int i=0;

	p=STR_TOK(t_buf);

    while(p)
	{
		t_arg[i]=p;
        i++;
        p=STR_TOK(NULL);
    }
	t_arg[i]=NULL;
}
char* STR_TOK(char ck_str[])
{
	static char* str;
	char *res=0;

	if(ck_str!=NULL)
	{
		str=ck_str;
	}

	if(*str=='\0')
		return 0;

    while(*str){
        if(*str==' ')
		{
            str++;
            continue;
        }
        else
		{
            res=str;
            break;
        }
    }

	while(*str)
	{
		if(*str=='\n')
		{
			*str='\0';
			break;
		}

		else if(*str==' '||*str=='|'||*str=='>'||*str=='<')
		{
			*str='\0';
			str++;

			if(*str==' '||*str=='>')
				continue;
	
			else
				return res;
		}
		str++;
	}
	return res;
}
void add_path(char *path_[],int ck_)
{
	node *ne;
	node *last;
	int i=0;

	if(ck_<0){
		ne=(node*)malloc(sizeof(node));
		strcpy(ne->str,path_[1]);
		ne->next=0;

		if(!head)
        {
            head=ne;
        }

		else{
        	for(last=head;last->next!=0;last=last->next)
         		;
      		last->next=ne;
		}
	}
	else
	{
		for(i=0;path_[i]!=0;i++){
			ne=(node*)malloc(sizeof(node));
			strcpy(ne->str,path_[i]);
			ne->next=0;
	
			if(!head)
			{
				head=ne;
			}

			else{
				for(last=head;last->next!=0;last=last->next)
					;
				last->next=ne;
			}
		}
	}
}
int ACS(char *acs_arg[])
{
	node *a;
	char p[size];

	for(a=head;a!=0;a=a->next)
	{
		strcpy(p,a->str);
		strcat(p,"/");
		strcat(p,acs_arg[0]);
		if(access(p,F_OK)==0)
			return 1;
	}
	return -1;
}
void fork_(char f_buf[],char *f_arg[])
{
	int pid;
	int a_ck=0;
	
	switch(pid=fork())
	{
		case -1:
			printf("fork error\n");
			break;
		case 0:
			a_ck=ACS(f_arg);
			if(a_ck>0)
			{
				execvp(f_arg[0],f_arg);
     	  		printf("command not found\n");
       			exit(1);
			}
			else
     	  		printf("directory not found\n");
    }
	wait(NULL);
}
void cd_(char *cd_arg[])
{
	if(cd_arg[2]!=0)
		printf("input error\n");
	else if(cd_arg[1]==0||!strcmp(cd_arg[1],"~"))
		chdir("/home/kii96");
	else
		chdir(cd_arg[1]);
}
void sig_(int sig)
{
	if(sig==SIGINT)
		printf("\nSIGINT receive\n");
	else if(sig==SIGQUIT)
		printf("\nSIGQUIT receive\n");
}
void file_()
{
	int fd=0,n=0;
	char fb[size];
    char *fa[size];

    if((fd=open("mysh",O_RDONLY))<0)
	{
		perror("open():");
    }

	while((n=read(fd,fb,size))>0)
		to_tok(fb,fa);

	add_path(fa,1);
}
void ck_type(char ck_t_buf[])
{
	int i=0;

	while(ck_t_buf[i]!='\n')
	{
		if(ck_t_buf[i]=='|')
		{
			type=PIPE;
			break;
		}
		else if(ck_t_buf[i]=='>')
		{
			if(ck_t_buf[i+1]=='>')
				type=REDIR2;
			else
				type=REDIR1;
			break;
		}
		else if(ck_t_buf[i]=='<')
		{
			type=REDIR3;
			break;
		}
		else
			i++;
	}
}
void pipe_(char p_buf[],char *p_arg[])
{
	int fd[2],pid=0;
    char *c1[size]={0};
    char *c2[size]={0};

	to_tok(p_buf,p_arg);

    c1[0]=p_arg[0];
    c2[0]=p_arg[1];

    if(pipe(fd)==-1)
       printf("pipe error\n");

	switch(pid=fork())
	{
       case -1:
          printf("fork error\n");
          break;

       case 0:
          dup2(fd[1],1);
          close(fd[0]); close(fd[1]);
          execvp(c1[0],c1);
	}
	switch(pid=fork())
	{
		case -1:
          printf("fork error\n");
		  break;

		case 0:
          dup2(fd[0],0);
          close(fd[0]); close(fd[1]);
          execvp(c2[0],c2);
	}
	close(fd[0]); close(fd[1]);
}
void redir_o(char r_buf[],char *r_arg[])
{
	int fd,pid;
	to_tok(r_buf,r_arg);

	if((pid=fork())==-1)
		printf("fork error\n");

	if(pid==0){
		switch(type){
			case REDIR1:
				if((fd=open(r_arg[1],O_RDWR|O_CREAT|O_TRUNC|S_IROTH,0644))==-1)
				{
					printf("open error\n");
				}
				break;
			case REDIR2:
				if((fd=open(r_arg[1],O_RDWR|O_CREAT|O_APPEND|S_IROTH,0644))==-1)
				{
					printf("open error\n");
				}
				break;
		}
		dup2(fd,1);
		close(fd);

		r_arg[1]=NULL;
		execvp(r_arg[0],r_arg);
	}

	wait(NULL);
}
void redir_i(char r_buf[],char *r_arg[])
{
	int fd,pid;
	to_tok(r_buf,r_arg);

	if((pid=fork())==-1)
		printf("fork error\n");

	if(pid==0){
		if((fd=open(r_arg[1],O_RDONLY))==-1)
		{
			printf("open error\n");
		}
		dup2(fd,0);
		close(fd);

		r_arg[1]=NULL;
		execvp(r_arg[0],r_arg);
	}

	wait(NULL);
}
void runc(int c_type,char c_buf[],char *c_argv[])
{
	switch(type){
		case PIPE:
			pipe_(c_buf,c_argv);
			break;

		case REDIR1:
		case REDIR2:
			redir_o(c_buf,c_argv);
			break;

		case REDIR3:
			redir_i(c_buf,c_argv);
			break;

		case ORG:
			to_tok(c_buf,c_argv);

        	if(!strcmp(c_argv[0],"cd"))
				cd_(c_argv);

			else if(!strcmp(c_argv[0],"setpath"))
				add_path(c_argv,-1);

			else
				fork_(c_buf,c_argv);
			break;
	}
}
int main()
{
    char *argv[size];
    char buf[size];

	signal(SIGINT,sig_);
	signal(SIGQUIT,sig_);

	file_();
    while(1){
        printf("~$");

        if(fgets(buf,size,stdin)!=NULL)
        {
			 if(!strcmp(buf,"exit\n"))
			 {
                printf("Shell Exit\n");
                exit(0);
			 }
		
			type=ORG;
			ck_type(buf);

			runc(type,buf,argv);
		}
	}
}
