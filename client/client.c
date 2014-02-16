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

#define PORT "3611" // the port client will be connecting to 

#define MAXDATASIZE 256 // max number of bytes we can get at once 

const int ERROR = -1;
const int FLAGS = 0;
const int MAX_CMD = 80;

char list_command[] = "list";
char quit_command[] = "quit";
char dl_command[] = "download";
char dsp_command[] = "display";
char chk_command[] = "check";
char help_command[] = "help";
char ack_response[] = "ack";
char nf_response[] = "nf";
char fo_response[] = "ff";

#define DSP_CASE  0
#define DL_CASE  1
#define CHK_CASE  2

void receivefile(int sock, char * filename);
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
         printf("You have entered the list command!\n");
         exec_list(sockfd); //Sends a list request to Server
         
         int list_not_rec = 1;
         int databytes;
         while(list_not_rec){
            databytes = recv(sockfd, buf, MAXDATASIZE, 0);
            if(databytes == ERROR){
               perror("recv");
               list_not_rec = 0;
            } else if (databytes == 0){
               list_not_rec = 0;
            } else {
               buf[databytes] = '\0';
               if(strcmp(buf, ack_response) == 0){
                  list_not_rec = 0;
                  break;
               }
               printf("%s", buf);
               list_not_rec = 1;
            }
         }
      } else if(strcmp(help_command, command) == 0) {
         while(getchar() != '\n');
         print_commands();
      } else if(strcmp(quit_command,command) == 0) {
          not_quit = 0;
          printf("Exiting client...\n");
      } else if(strcmp(chk_command,command) == 0) {
         scanf("%s", &sec_command);
         while(getchar() != '\n');
         exec_file(sockfd, CHK_CASE, sec_command);
      } else if(strcmp(dsp_command, command) == 0) {
         scanf("%s", &sec_command);
         while(getchar() != '\n');
         exec_file(sockfd, DSP_CASE, sec_command);
      } else if(strcmp(dl_command, command) == 0){
         scanf("%s", &sec_command);
         while(getchar() != '\n');
         exec_file(sockfd, DL_CASE, sec_command);
      } else {
         printf("Invalid command or missing parameters in command!\n");
         while(getchar() != '\n');
      }
      printf("\n");
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
   int length = strlen(data);
   if(send(socket, data, length, FLAGS) == ERROR){
       perror("send");
       printf("Error occured, or connection lost\n");
   }
   
   char buffer[MAXDATASIZE];
   int wait_ack = 1;
   int databytes;
   while(wait_ack){
      databytes = recv(socket, buffer, MAXDATASIZE, 0);
      if (databytes == ERROR){
         perror("recv");
         wait_ack = 0;
         return;
      } else if (databytes == 0) {
         printf("Server no response\n");
         wait_ack = 0;
      } else {
         buffer[databytes] = '\0';
         if(strcmp(buffer, ack_response) == 0){
            wait_ack = 0;
         } else {
            wait_ack = 1;
         }
      }
   }
  
   length = strlen(filename);
   if(send(socket, filename, length, FLAGS) == ERROR){
      perror("send");
      printf("Error occured, or connection lost\n");
   }
   
   wait_ack = 1;

   if(casenum == 1){
      receivefile(socket, filename);
   } else {
      while(wait_ack){
         databytes = recv(socket, buffer, MAXDATASIZE, 0);
         if(databytes == ERROR){
            perror("recv");
            wait_ack = 0;
         } else if (databytes == 0) {
            wait_ack = 0;
         } else {
            buffer[databytes] = '\0';
            if(strcmp(buffer, ack_response) == 0){
               wait_ack = 0;
               break;
            } else {
               wait_ack = 1;
            }
            printf("%s", buffer);
         }
      }
   }

}

void receivefile(int sock, char *filename)
{
   int rval;
   char buffer[MAXDATASIZE];
   int wait_ack = 1;
   int databytes;
   while(wait_ack){
      databytes = recv(sock, buffer, MAXDATASIZE, 0);
      if(databytes == ERROR){
         perror("recv");
         wait_ack = 0;
      } else if (databytes == 0) {
         wait_ack = 0;
      } else {
         buffer[databytes] = '\0';
         if(strcmp(buffer, nf_response) == 0){
            printf("File %s is not found\n", filename);
            wait_ack = 0;
            return;
         } else if (strcmp(buffer, fo_response) == 0){
            printf("File '%s' exists, downloading..\n", filename);
            wait_ack = 0;
            break;
         }
         wait_ack = 1;
      }
   }

   char buf[0x1000];
   char temp[0x1000];
   FILE *file = fopen(filename, "wb");
   if(!file){
      printf("Can't open file for writing\n");
      return;
   }
/*
   int run = 1;
   while(run){
     int bytes = recv(sock, buf, sizeof(buf), 0);
     if(bytes == 0){
         run = 0;
         printf("Download of '%s' complete.\n", filename);
         break;
     } else if (bytes < 0){
         printf("[ERROR] Connection terminated unexpectedly!\n");
         fclose(file);
         break;
     }

    strcpy(temp,buf);
    temp[bytes] = '\0';
    if(strcmp(temp, ack_response == 0)){
      printf("Download of '%s' complete.\n", filename);
      fclose(file);
      break;
    }


    void *p = buffer;
    while(bytes > 0) {
      int bytes = write(
    }
      
       
   }






*/



   do {
      rval = recv(sock, buf, sizeof(buf),0);
      strcpy(temp, buf);
      temp[rval] = '\0'; //Set last char to \0

      if(rval < 0){
         printf("Can't read from socket\n");
         fclose(file);
         return;
      }
      if(rval == 0) break;
      int off = 0;
      do{
         int written = fwrite(&buf[off], 1, rval - off, file);
         if(written < 1){
            printf("Can't write to file\n");
            fclose(file);
            return;
         }
         off += written;
      } while(off < rval);
   } while(strcmp(temp, ack_response)== -1);

   printf("Download of '%s' complete.\n", filename);

}
