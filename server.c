/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define PORT "3611"  // the port users will be connecting to
#define MAXDATASIZE 100
#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXLINESIZE 256
#define STDEER_FILENO 2
#define STDOUT_FILENO 1
#define CMD_MAX 20
#define FILE_NAME_MAX 50

const int ERROR = -1;
const int FLAGS = 0;

char list_command[] = "list";
char dl_command[] = "download";
char dsp_command[] = "display";
char chk_command[] = "check";
char ack_response[] = "ack";

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void exec_commands(char *data_received);
void exec_list(int *pid);
void run_list_pipe(int pip[]);
void run_dl_pipe(int pip[]);
void run_dsp_pipe(int pip[]);
void run_chk_pipe(int pip[]);
void error(char *s);

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("Server: Waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("Server: Got connection from %s\n", s);

      char buf[MAXDATASIZE];
   	
      if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			if (send(new_fd, "Connection established!", 23, 0) == -1){
				perror("send");
         }
         /* Variables for Pipes */
         int waiting = 1;
         int numbytes, nbytes;

         while(waiting){
           numbytes = recv(new_fd, buf, MAXDATASIZE-1, FLAGS);
           if(numbytes == ERROR){
              perror("recv");
              waiting = 0;
           } else if (numbytes == 0) {
              printf("Lost connection\n");
              waiting = 0;
           } else {
              buf[numbytes] = '\0';
              printf("Data received: '%s'\n", buf);
              
              if(strcmp(buf, list_command) == 0){
                  int pid = 0;
                  int pipefd[2];
                  FILE* output;
                  char line[MAXLINESIZE];     
                  int status;         
                  
                  pipe(pipefd); // create pipe
                  pid = fork();
                  
                  if (pid == 0){
                     close(pipefd[0]);
                     dup2(pipefd[1], STDOUT_FILENO);
                     dup2(pipefd[1], STDERR_FILENO); 
                     execl("/bin/ls", "ls", "-l", (char*) NULL);
                  }

                  close(pipefd[1]);
                  output = fdopen(pipefd[0], "r");

                  while(fgets(line, sizeof(line), output)) {
                     if(send(new_fd, line, sizeof(line), 0) == -1){
                        perror("send");
                     }
                  }

                  waitpid(pid, &status, 0);
                  printf("Transcation Complete \n"); 
                  exit(0);

             } else {
               
               char file_name[FILE_NAME_MAX];
               int command = 0;

               if(strcmp(buf, dsp_command) == 0){
                  command = 1;
                  printf("Display command inputted \n"); 
               } else if (strcmp(buf, dl_command) == 0) {
                  command = 2;
                  printf("Download command inputted \n");
               } else if (strcmp(buf, chk_command) == 0) {
                  command = 3;
                  printf("Check command inputted \n");
               }
                  
               /* Send Acknowledge meant and request filename*/
               if(send(new_fd, ack_response, sizeof(ack_response), 0) == -1){
                  perror("send");
               }

               char buffer[MAXDATASIZE];
               int wait_file = 1;
               int databytes;
               while(wait_file){
                  databytes = recv(new_fd, buffer, MAXDATASIZE, 0);
                  if(databytes == ERROR){
                     perror("recv");
                     wait_file = 0;
                     return;
                  } else if (databytes == 0){
                     printf("File name received.\n");
                     wait_file = 0;
                  } else {
                     buffer[databytes] = '\0';
                     strcpy(file_name, buffer);
                     printf("File name is: %s, \n", file_name);
                     wait_file = 0;
                  }
               
               }

             }
       
       
       
        }
      }
         printf("Closing connection.\n");
         close(new_fd);
         exit(0);
      
      } else {
		   close(new_fd);  // parent doesn't need this
      }
	}

	return 0;
}

char *listcmd[] = {"/bin/ls", "-l", 0};
char *chkcmd[] = {};
char *dspcmd[] = {};
char *dlcmd[] = {};

void exec_commands(char *data_received)
{
   if(strcmp(data_received, list_command) == 0){
    
   }
}

void exec_list(int *pid)
{

}

void error(char *s)
{
   perror(s);
   exit(1);
}
