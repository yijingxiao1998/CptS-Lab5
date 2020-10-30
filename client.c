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
struct stat mystat, *sp;
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

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

int ls_file(char *fname)
{
	struct stat fstat, *sp;
	int r, i;
	char ftime[64], buf[BLK];
	sp = &fstat;
	if((r = lstat(fname, &fstat)) < 0)
	{
		printf("can't stat %s\n", fname);
		return -1;
		//exit(1);
	}
	if((sp->st_mode & 0xF000) == 0x8000)  // if(S_ISREG())
		printf("%c", '-');
	if((sp->st_mode & 0xF000) == 0x4000)  // if(S_ISDIR())
		printf("%c", 'd');
	if((sp->st_mode & 0xF000) == 0xA000)  // if(S_ISLINK())
		printf("%c", 'l');
	for(i = 8; i >= 0; i--)
	{
		if((sp->st_mode & (1<<i)))  // print r|w|x
			printf("%c", t1[i]);
		else
			printf("%c", t2[i]);  // or print -
	}
	printf("%4lu ", sp->st_nlink);  // link count
	printf("%4d ", sp->st_gid);    // gid
	printf("%4d ", sp->st_uid);    // uid
	printf("%8ld ", sp->st_size);   // file size
	// print time
	strcpy(ftime, ctime(&sp->st_ctime));  // print time in calendar
	ftime[strlen(ftime)-1] = 0;  // kill \n at end
	printf("%s  ", ftime);
	// print name
	printf("%s", basename(fname));  // print file basename
	// print -> linkname if symbolic file
	if((sp->st_mode & 0xF00) == 0xA000)
	{
		// use readlink() to read linkname
		char linkname[BLK];
		r = readlink(fname, linkname, BLK);
		printf(" -> %s", linkname);  // print linked name
	}
	printf("\n");
}

int main(int argc, char *argv[], char *env[]) 
{ 
    int  n, r, i=0;
    char line[MAX], ans[BLK];
    char *cmd;
    char *pathname;
    char buf[MAX], temp[MAX];
    char c;
    char *rec1, *rec2;

    init();
  
    printf("********  processing loop ********\n");
    while (1)
    {
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
      
      strcpy(temp, line);
      cmd = strtok(temp, " ");
      printf("cmd= %s  ", cmd);
      pathname = strtok(NULL, " ");
      if(pathname != 0)
      	 printf("pathname= %s", pathname);
      printf("\n");
      
      if(!strcmp(cmd, "exit"))
	  exit(0);
      else if(!strcmp(cmd, "lmkdir"))
      	  r = mkdir(pathname, 0755);
      else if(!strcmp(cmd, "lrmdir"))
      	  r = rmdir(pathname);
      else if(!strcmp(cmd, "lrm"))
    	  r = unlink(pathname);
      else if(!strcmp(cmd, "lcd"))
    	  r = chdir(pathname);
      else if(!strcmp(cmd, "lpwd"))
      {
      	  getcwd(buf, MAX);
      	  printf("%s\n", buf);
      }
      else if(!strcmp(cmd, "lls"))
      {
      	  int r = 0;
      	  DIR *dir;
      	  struct dirent *fileinfo;
      	  struct stat mystat, *sp = &mystat;
      	  char *filename, path[BLK], cwd[MAX];
      	  filename = "./";  // default to CWD
      	  
      	  if(pathname)
      	  	filename = pathname;
      	  if(r = lstat(filename, sp) < 0)
      	  	printf("no such file %s\n", filename);
      	  strcpy(path, filename);
      	  if(path[0] != '/')
      	  {
      	  	// filename is relative: get CWD path
      	  	getcwd(cwd, MAX);
      	  	strcpy(path, cwd);
      	  	strcat(path, "/");
      	  	strcat(path, filename);
      	  }
      	  //getcwd(cwd, MAX);
      	  if(S_ISDIR(sp->st_mode))
      	  {
      	  	bzero(temp, MAX);
      	  	dir = opendir(path);
      	  	while((fileinfo = readdir(dir)) != 0)
      	  	{
      	  		strcpy(temp, path);
      	  		strcat(temp, "/");
      	  		strcat(temp, fileinfo->d_name);
      	  		ls_file(temp);
      	  	}
      	  	closedir(dir);
      	  	printf("\n");
      	  }
      	  else
      	  {
      	  	ls_file(path);
      	  }	  	  	
      }
      else if(!strcmp(cmd, "lcat"))
      {
      	  FILE *file;
      	  if(pathname != 0)
      	  {
      	  	file = fopen(pathname, "r");
      	  	while((c = fgetc(file)) != EOF)
      	  		putchar(c);
      	  }
      	  else
      	  {
      	  	printf("please enter file name\n");
      	  }
      }
      else if(!strcmp(cmd, "get"))
      {
          int total = 0;
      	  FILE *file;
      	  printf("Client get %s\n",pathname);
      	  printf("(1): try to open %s for WRITE : filename=%s\n", pathname, pathname);

      	  n = write(sock, line, MAX);
      	  file = fopen(pathname, "w");
      	  if(file)
      	  {
      	  	printf("open OK\n");
      	  	n = read(sock, ans, MAX);
      	  	rec1 = strtok(ans, " ");
      	  	rec2 = strtok(NULL, " "); 
      	  	printf("(2): send get %s to Server and receive reply = %s\n", pathname, rec1);
      	  	printf("expecting %s bytes\n", rec2);
      	  	while(1)
      	  	{
      	  		bzero(ans, MAX);
      	  		n = read(sock, ans, BLK);
      	  		printf("n=%d", n);
      	  		total += n;
      	  		printf("total=%d\n", total);
      	  		fwrite(ans, n, 1, file);
      	  		//if(n < BLK)
			if(strncmp(ans, "END OF", 6) == 0)
      	  		{
      	  			printf("received %d bytes\n", total);
      	  			break;
      	  		}	
      	  	}
      	  	fclose(file);
      	  }
      }
      else if(!strcmp(cmd, "put"))
      {
          int total = 0;
          struct stat statbuf;
      	  FILE *file;
	  struct stat statBuf;
	  char fileSizeMsg[20];
      	  printf("Client put %s\n",pathname);
	  n = write(sock, line, MAX);
      	  file = fopen(pathname, "r"); 
      	  if(file)
      	  {
		printf("Successfully open %s and ready for read:\n", pathname);
		stat(pathname, &statBuf);
		int fileSize = statBuf.st_size;
		printf("Ready to read %d byte from client\n", fileSize);
		sprintf(fileSizeMsg, "OK %d", fileSize);
		write(sock, fileSizeMsg, MAX);
			
      	  	bzero(ans, MAX);
		n = write(sock, line, MAX);
      	  	while(total < fileSize)
      	  	{
      	  		n = fread(ans, 1, BLK, file);
      	  		if(n<0)
				break;
      	  		n = write(sock, ans, BLK);
      	  		printf("n=%d  ", n);
      	  		total += n;
      	  		printf("total=%d\n", total);
      	  	}
      	  	printf("sent %d bytes\n", total);	
      	  	fclose(file);
      	  }
      }
      else if(!strcmp(cmd, "ls"))
      {
      	  // Send ENTIRE line to server
          n = write(sock, line, MAX);
          printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
          
          // Read a line from sock and show it
          while(1)
          {
          	bzero(ans, MAX);
          	n = read(sock, ans, MAX);
                printf("%s", ans);
          	//if(!strcmp(ans, "END OF ls \n"))
          	if(strncmp(ans, "END OF ls", 9) == 0)
          		break;
          }
      }
      else if(!strcmp(cmd, "cd") || !strcmp(cmd, "pwd") || !strcmp(cmd, "mkdir") || !strcmp(cmd, "rmdir") || !strcmp(cmd, "rm"))
      { 	    
      	  // Send ENTIRE line to server
          n = write(sock, line, MAX);
          printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
          
          // Read a line from sock and show it
          n = read(sock, ans, MAX);
          printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
      }
      else
      {
      	  printf("invalid comment %s\n", line);
      }
      printf("%s Done\n", line); 
      bzero(line, MAX);
      bzero(ans, MAX);
      printf("\n");
    }
}
