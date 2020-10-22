#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>     // for dirname()/basename()
#include <time.h> 

#define MAX 256
#define BLK 1024

struct sockaddr_in saddr; 
char *serverIP   = "127.0.0.1";
int   serverPORT = 1234;
int   sock;

int init()
{
    int n; 

    printf("1. create a socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP=%s, port number=%d\n", serverIP, serverPORT);
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = inet_addr(serverIP); 
    saddr.sin_port = htons(serverPORT); 
  
    printf("3. connect to server\n");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    }
    printf("4. connected to server OK\n");
}

void execute(char *cmd, char *pathname)
{
    int r;
    char buf[MAX];
    if(!strcmp(cmd, "exit"))
	  exit(0);
    else if(!strcmp(cmd, "mkdir"))
      	  r = mkdir(pathname, 0755);
    else if(!strcmp(cmd, "rmdir"))
      	  r = rmdir(pathname);
    else if(!strcmp(cmd, "rm"))
    	  r = unlink(pathname);
    else if(!strcmp(cmd, "cd"))
    	  r = chdir(pathname);
    else if(!strcmp(cmd, "pwd"))
      	  getcwd(buf, MAX);
      	  printf("%s\n", buf);
}

int main(int argc, char *argv[], char *env[]) 
{ 
    int  n, r;
    char line[MAX], ans[MAX];
    char *cmd;
    char *pathname;
    char buf[MAX];

    init();
  
    printf("********  processing loop ********\n");
    while (1){
      printf("********************** menu *********************\n");
      printf("* get  put  ls   cd   pwd   mkdir   rmdir   rm  *\n");
      printf("* lcat     lls  lcd  lpwd  lmkdir  lrmdir  lrm  *\n");
      printf("*************************************************\n");
      printf("input a line : ");
      bzero(line, MAX);                // zero out line[]
      fgets(line, MAX, stdin);         // get a line from stdin
      line[strlen(line)-1] = 0;        // kill <CR> at end
      if (line[0]==0)                  // exit if NULL line
         exit(0);
      
      cmd = strtok(line, " ");
      printf("cmd= %s  ", cmd);
      pathname = strtok(NULL, " ");
      if(pathname != 0)
      	 printf("pathname= %s", pathname);
      printf("\n");
      
      execute(cmd, pathname);
      /*if(!strcmp(cmd, "exit"))
	  exit(0);
      else if(!strcmp(cmd, "mkdir"))
      	  r = mkdir(pathname, 0755);
      else if(!strcmp(cmd, "rmdir"))
      	  r = rmdir(pathname);
      else if(!strcmp(cmd, "pwd"))
      	  getcwd(buf, MAX);
      	  printf("%s\n", buf);*/
	  
      /*// Send ENTIRE line to server
      n = write(sock, line, MAX);
      printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

      // Read a line from sock and show it
      bzero(ans, MAX);
      n = read(sock, ans, MAX);
      printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);*/
    }
}