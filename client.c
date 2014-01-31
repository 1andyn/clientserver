/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3511" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

const int ERROR = -1;
const int FLAGS = 0;
const int MAX_CMD = 80;

char list_command[] = "list";
char quit_command[] = "quit";
char dl_command[] = "download";
char dsp_command[] = "display";
char chk_command[] = "check";
char help_command[] = "help";

#define DSP_CASE  0
#define DL_CASE  1
#define CHK_CASE  2

void print_commands();
void exec_list(int socket);
void exec_file(int socket, int case_num, char *filename);

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);

   int not_quit = 1;
   
   char command[] = "";
   char sec_command[] = "";
   

   while(not_quit){
      printf("Enter a command, enter 'help' for a list of commands: ");
      scanf("%s", &command);
      if(strcmp(list_command,command) == 0) {
         while(getchar() != '\n');
         printf("You have entred the list command!\n");
         exec_list(sockfd);
      } else if(strcmp(help_command, command) == 0) {
         while(getchar() != '\n');
         print_commands();
      } else if(strcmp(quit_command,command) == 0) {
          not_quit = 0;
          printf("Exiting client...\n");
      } else if(strcmp(chk_command,command) == 0) {
         printf("Enter file name: \n");
         scanf("%s", &sec_command);
         while(getchar() != '\n');
         exec_file(sockfd, CHK_CASE, sec_command);
         printf("File name entered is '%s' \n", &sec_command);
      } else if(strcmp(dsp_command, command) == 0) {
         printf("Enter file name: \n");
         scanf("%s", &sec_command);
         while(getchar() != '\n');
         exec_file(sockfd, DSP_CASE, sec_command);
         printf("File name entered is '%s' \n", &sec_command);
      } else if(strcmp(dl_command, command) == 0){
         printf("Enter file name: \n");
         scanf("%s", &sec_command);
         while(getchar() != '\n');
         exec_file(sockfd, DL_CASE, sec_command);
         printf("File name entered is '%s' \n", &sec_command);
      } else {
         printf("Invalid command or missing parameters in command!\n");
         while(getchar() != '\n');
      }
   }
	close(sockfd);

	return 0;
}

void print_commands()
{
   printf("list - lists contents of server\n");
   printf("check <file name> - checks if server has file named <file name>\n");
   printf("display <file name> - displays content of <file name>\n");
   printf("download <file name> - downloads <file name> to client directory\n");
   printf("quit - exits the program\n");
}

void exec_list(int socket)
{
   int length = 4;
   if(send(socket, list_command, length, FLAGS) == ERROR){
       perror("send");
       printf("Error occured, or connection lost\n");
   }
}

void exec_file(int socket, int casenum, char *filename)
{
   char data[MAX_CMD];
   switch(casenum){
      case DSP_CASE:
         strcpy(data, dsp_command);
         break;
      case DL_CASE:
         strcpy(data, dl_command);
         break;
      case CHK_CASE:
         strcpy(data, chk_command);
         break;
      default:
         printf("Something went wrong..\n");
         return;
   }
   strcat(data, " ");
   strcat(data, filename);
   int length = strlen(data);
   if(send(socket, data, length, FLAGS) == ERROR){
       perror("send");
       printf("Error occured, or connection lost\n");
   }
}
