#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

#define MAX   256
#define BLK  1024

int server_sock, client_sock;
char *serverIP = "127.0.0.1";      // hardcoded server IP address
int serverPORT = 1234;             // hardcoded server port number

struct sockaddr_in saddr, caddr;   // socket addr structs

int init()
{
    printf("1. create a socket\n");
    server_sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_sock < 0) { 
        printf("socket creation failed\n"); 
        exit(0); 
    }
    
    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
     saddr.sin_addr.s_addr = inet_addr(serverIP);
    saddr.sin_port = htons(serverPORT);
    
    printf("3. bind socket to server\n");
    if ((bind(server_sock, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
        printf("socket bind failed\n"); 
        exit(0); 
    }
    printf("4. server listen with queue size = 5\n");
    if ((listen(server_sock, 5)) != 0) { 
        printf("Listen failed\n"); 
        exit(0); 
    }
    printf("5. server at IP=%s port=%d\n", serverIP, serverPORT);
    char cwdBuf[256];
    int s = chroot("/");
    if(s==0)
    {
      printf("6. Change root to: %s\n", getcwd(cwdBuf, MAX));
    }
    else
    {
      perror("chroot /");
    }
    
    
}
  
int main() 
{   
    // cmd to process: get put ls cd pwd mkdir rmdir rm
    char *cmd, *pathname, *s;
    char cwdBuf[256];
    int n, length;
    char line[MAX];
    
    init();  

    while(1){
       printf("server: try to accept a new connection\n");
       length = sizeof(caddr);
       client_sock = accept(server_sock, (struct sockaddr *)&caddr, &length);
       if (client_sock < 0){
          printf("server: accept error\n");
          exit(1);
       }
 
       printf("server: accepted a client connection from\n");
       printf("-----------------------------------------------\n");
       printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
       printf("-----------------------------------------------\n");

       // Processing loop
       while(1){
         printf("server ready for next request ....\n");
         n = read(client_sock, line, MAX);

         if (n==0){
           printf("server: client died, server loops\n");
           close(client_sock);
           break;
         }
         else if(n == -1)
         {
            printf("server or client error. Exit server\n");
            close(client_sock);
            break;
         }
         line[n]=0;
         printf("server: read  line=[%s]\n", line);

         // handle input line from client:
         cmd = strtok(line, " ");
         printf("cmd= %s  \n", cmd);
         pathname = strtok(NULL, " ");
         printf("pathname= %s\n", pathname);
         // handle command
         if(!strcmp(cmd, "get"))
         {

         }
         else if(!strcmp(cmd, "put"))
         {

         }
         else if(!strcmp(cmd, "ls"))
         {
            printf("server: cmd = %s\n", cmd);
         }
         else if(!strcmp(cmd, "cd"))
         {
            int r = chdir(pathname);
            if (r != 0)
            {
               printf("errno=%d : %s\n", errno, strerror(errno));
            }
            s = getcwd(cwdBuf, 256);
            n = write(client_sock, s, MAX);
            printf("server: cmd = %s wrote  CWD=[%s]\n", cmd, s);
         }
         else if(!strcmp(cmd, "pwd"))
         {
            printf("Handling command: %s\n", cmd);
            s = getcwd(cwdBuf, 256);
            n = write(client_sock, s, MAX);
            printf("server: cmd = %s wrote  CWD=[%s]\n", cmd, s);
         }
         else if(!strcmp(cmd, "mkdir"))
         {
            int r;
            r = syscall(pathname, 0766);
            if (r < 0)
            {
               printf("errno=%d : %s\n", errno, strerror(errno));
            }
            r = chdir(pathname); // cd into newdir
            s = getcwd(cwdBuf, 256); // get CWD string into buf[ ]
            n = write(client_sock, s, MAX);
            printf("server: cmd = %s wrote  CWD=[%s]\n", cmd, s);
         }
         else if(!strcmp(cmd, "rmdir"))
         {
            int r = rmdir(pathname);
            if(r != 0)
            {
               printf("errno=%d : %s\n", errno, strerror(errno));
            }
            s =malloc(256);
            strcpy(s, "remove ");
            strcat(s, pathname);
            strcat(s, "\n");
            n = write(client_sock, s, MAX);
            s = getcwd(cwdBuf, 256);
            printf("server: cmd = %s wrote  CWD=[%s]\n", cmd, s);
         }
         else if(!strcmp(cmd, "rm"))
         {
            int r = unlink(pathname);
            if(r != 0)
            {
               printf("errno=%d : %s\n", errno, strerror(errno));
            }
            
            s =malloc(256);
            strcpy(s, "remove ");
            strcat(s, pathname);
            strcat(s, "\n");
            n = write(client_sock, s, MAX);
            s = getcwd(cwdBuf, 256);
            printf("server: cmd = %s wrote  CWD=[%s]\n", cmd, s);
         }
         //strcat(line, " ECHO");
         // send the echo line to client 
         // n = write(client_sock, line, MAX);

         // printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
       }
    }
}
