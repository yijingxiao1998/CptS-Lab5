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
struct stat mystat, *sp;
char *t1 = "xwrxwrxwr---------";
char *t2 = "------------------";
char cwdBuf[BLK];
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
    printf("6. Change root to: %s\n", getcwd(cwdBuf, MAX));
    getcwd(cwdBuf, MAX);
    int s = chroot(cwdBuf);
    if(s!=0)
    {
      perror("chroot /");
    }
}

char* ls_file(char* filename)
{
   struct stat fstat, *sp;
   int r,i;
   char ftime[64], *message = malloc(MAX), linkBuf[256];
   sp = &fstat;
   char tempStr[MAX];

   if ( (r = lstat(filename, &fstat))<0)
   {
      printf("Can't stat %s\n", filename);
      exit(1);
   }
   printf("%s", message);
   if((sp->st_mode & 0xF000) == 0x8000)
   {
      strcat(message, "-");
   }
   if((sp->st_mode & 0xF000) == 0x4000)
   {
      strcat(message, "d");
   }
   if((sp->st_mode & 0xF000) == 0xA000)
   {
      strcat(message, "l");
   }
   for (i = 8; i >=0; i--)
   {
      if(sp->st_mode & (1 << i))
      {
         sprintf(tempStr, "%c", t1[i]);
         strcat(message, tempStr);
      }
      else
      {
         sprintf(tempStr, "%c", t2[i]);
         strcat(message, tempStr);
      }
   }
   
   sprintf(tempStr, " %4d %4d %4d %8d ", sp->st_nlink, sp->st_gid, sp->st_uid, sp->st_size);
   strcat(message, tempStr);

   // print time
   strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form
   ftime[strlen(ftime)-1] = 0; // kill \n at end
   sprintf(tempStr, " %s ", ftime);
   strcat(message, tempStr);

   // print name
   sprintf(tempStr, " %s ", basename(filename));// print file basename
   strcat(message, tempStr);
   // print -> linkname if symbolic file
   if ((sp->st_mode & 0xF000)== 0xA000)
   {
      // use readlink() to read linkname
      sprintf(tempStr, " -> %s \n", readlink(filename, linkBuf, MAX));// print linked name
      strcat(message, tempStr);
   }
   strcat(message, "\n");

   return message;
}

int ls_dir(char* pathname, char *lsList)
{
   printf("Enter ls_dir\n");
   DIR* curDir = opendir(pathname);
   struct dirent *dp;
   if (curDir != NULL)
   {
      while((dp = readdir(curDir))!= NULL)
      {
         ls_file(dp->d_name);
      }
   }
}

int main() 
{   
    // cmd to process: get put ls cd pwd mkdir rmdir rm
    char *cmd, *pathname, *s = malloc(1024);
    char cwdBuf[1024], fileContent[MAX];
    int n, length;
    char line[MAX];
    char *rec1, *rec2;
    
    init();  

    while(1)
    {
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
       while(1)
       {
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
            int total = 0;
            char fileBuf[4096];
            FILE* file = fopen(pathname, 'r');
            struct stat statBuf;
            char fileSizeMsg[20];

            printf("server: cmd = %s\n", cmd);
            printf("(1): try to open %s for READ : filename=%s\n", pathname, pathname);
            if(file)
            {
               printf("Successfully open %s and ready for read:\n", pathname);
               stat(pathname, &statBuf);
               int fileSize = statBuf.st_size;
               printf("Ready to write %d byte to client\n", fileSize);
               sprintf(fileSizeMsg, "OK %d", fileSize);
               write(client_sock, fileSizeMsg, MAX);
               bzero(fileContent, MAX);

               while(1)
               {
                  n = fread(fileContent, 1, BLK, file);
                  if(n < 0)
                  {
                     break;
                  }
                  n = write(client_sock, fileContent, BLK);
                  printf("n = %d ", n);
                  total += n;
                  printf("total = %d\n", total);
               }
               printf("Sent %d bytes in total\n", total);
      	  	   fclose(file);
            }
            write(client_sock, "END OF get", MAX);
            printf("server: cmd = %s\n", cmd);
            s = getcwd(cwdBuf, 256);
            printf("server: cmd = %s wrote  CWD=[%s]\n", cmd, s);
         }
         else if(!strcmp(cmd, "put"))
         {
            int total = 0;
            char fileBuf[4096];
            printf("server: cmd = %s\n", cmd);
            printf("(1): try to open %s for WRITE : filename=%s\n", pathname, pathname);
            FILE* file = fopen(pathname, "w");
            if(file)
            {
               printf("Successfully open %s and ready for write:\n", pathname);
               n = read(client_sock, fileContent, MAX);
               rec1 = strtok(fileContent, " ");
               rec2 = strtok(NULL, " ");
               printf("(2): Send get %s to Client and receive file size reply = %s\n", pathname, rec1);
               printf("expecting %s bytes\n", rec2);
               while(1)
               {
                  bzero(fileContent, MAX);
                  n = read(client_sock, fileContent, BLK);
                  printf("n=%d ", n);
                  total += n;
                  printf("total=%d\n", total);
                  fwrite(fileContent, n, 1, file);
                  if(n < BLK)
                  {
                     printf("received %d bytes\n", total);
                     break;
                  }	
      	  	   }
      	  	   fclose(file);
            }
            s = getcwd(cwdBuf, 256);
            printf("server: cmd = %s wrote  CWD=[%s]\n", cmd, s);
         }
         else if(!strcmp(cmd, "ls"))
         {
            int r;
            char path[1024];
            char *lsList = malloc(MAX);
            struct stat mystat, *sp = &mystat;
            char *endMessage = malloc(30);
            DIR *dir;
      	   struct dirent *file;
            
            printf("server: cmd = %s\n", cmd);

            if(pathname == 0)
            {
               dir = opendir(getcwd(cwdBuf, MAX));
            }
            else
            {
               dir = opendir(pathname);
            }

            n=write(client_sock, "Displaying file:\n", MAX);
            printf("Reading File:\n");

            while((file = readdir(dir)) != 0)
            {
               printf("lsFile: %s\n", file->d_name);
               write(client_sock, ls_file(file->d_name), MAX);             
            }
            closedir(dir);
            strcpy(endMessage, "END OF ls");
            write(client_sock, endMessage, MAX);
            printf("server: cmd = %s wrote  CWD=[%s]\n", cmd, getcwd(cwdBuf,MAX));
         }
         else if(!strcmp(cmd, "cd"))
         {
            int r = chdir(pathname);
            if (r != 0)
            {
               printf("errno=%d : %s\n", errno, strerror(errno));
               n = write(client_sock, "CD failed\n", MAX);
               continue;
            }
            s = getcwd(cwdBuf, 256);
            n = write(client_sock, s, MAX);
            printf("server: cmd = %s wrote  CWD=[%s]\n", cmd, s);
         }
         else if(!strcmp(cmd, "pwd"))
         {
            printf("Handling command: %s\n", cmd);
            s = getcwd(cwdBuf, 256);
            printf("CWD:%s\n", s);
            n = write(client_sock, s, MAX);
            printf("server: cmd = %s wrote  CWD=[%s]\n", cmd, s);
         }
         else if(!strcmp(cmd, "mkdir"))
         {
            int r;
            r = mkdir(pathname, 0755);
            if (r < 0)
            {
               printf("errno=%d : %s\n", errno, strerror(errno));
               strcpy(s, "Make new dir failed\n");
               n = write(client_sock, s, MAX);
               continue;
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
               strcpy(s, "remove dir failed\n");
               n = write(client_sock, s, MAX);
               continue;
            }
            
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
         // bzero(line, MAX);
         // bzero(s, 1024);
       }
       
    }
}
