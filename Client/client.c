// file client
// gcc -o client client.c -lcrypto

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <openssl/md5.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include "../lib/md5.h"

#define BUFFER_SIZE 1024

unsigned int fsize(char* file)
{
    FILE * f = fopen(file, "r");
    fseek(f, 0, SEEK_END);
    unsigned long len = (unsigned long)ftell(f);
    fclose(f);
    return len;
}

int main(int argc, char * argv[])
{
	int sockfd = 0;
	struct sockaddr_in serv_addr;
	int n = 0;

	char recvBuff[BUFFER_SIZE];
	char sendBuff[BUFFER_SIZE];

	short int file_length;
	char filename[1024];
	unsigned int file_size_rcv;
	unsigned int remain_data;
	unsigned char md5[MD5_DIGEST_LENGTH];
	unsigned char md5_compute[MD5_DIGEST_LENGTH];
	unsigned int file_size;

	int send_bytes;
	off_t offset;
	int i;
	int fd;

	int option = 0;
	int isLoggedOut = 0;

	FILE * received_file;

	if(argc != 2)
	{
		printf("\n Usage: %s <ip of server> \n", argv[0]);
		return 1;
	}

	// Create socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket\n");
		return 1;
	}
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(7000);
	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
	{
		printf("\n inet_pton error occured\n");
		return 1;
	}

	// Connect to server
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed\n");
		return 1;
	}

	printf("Logged into FILE Server\n");
	printf("Choose options:\n");
	printf("1. REQ (download a specific file from server)\n");
	printf("2. UPL (upload a specific file from server)\n");
	printf("3. EXIT (log out of FILE server)\n");

	while(!isLoggedOut)
	{
		printf("Option: ");
		scanf("%d", &option);
		switch (option)
		{
			case 1:
				// Client sends cmd:
				bzero(sendBuff, BUFFER_SIZE);
			    snprintf(sendBuff, BUFFER_SIZE, "REQ");
				write(sockfd, sendBuff, strlen(sendBuff));

			    bzero(recvBuff,BUFFER_SIZE);
				n = read(sockfd, recvBuff, BUFFER_SIZE - 1);

				// Client sends file name
				printf("File: ");
				bzero(filename, 1024);
			    scanf("%s", filename);
			    file_length = strlen(filename) - 1;
			    bzero(sendBuff, BUFFER_SIZE);
			    snprintf(sendBuff, BUFFER_SIZE, "%s,%d", filename, file_length);
				write(sockfd, sendBuff, strlen(sendBuff));

			    // Receive file name is existed or NOT from server
			    bzero(recvBuff,BUFFER_SIZE);
				n = read(sockfd, recvBuff, BUFFER_SIZE - 1);
				if (atoi(recvBuff) == 1)
				{
					printf("File is not existed on server!\n");
					break;
				}
				file_size_rcv = atoi(recvBuff);

				// Request md5
				snprintf(sendBuff, BUFFER_SIZE, "need MD5");
				write(sockfd, sendBuff, strlen(sendBuff));

				// Receive MD5
				bzero(md5,sizeof(md5));
				n = read(sockfd, md5, MD5_DIGEST_LENGTH);

				// Request file
				snprintf(sendBuff, BUFFER_SIZE, "need file");
				write(sockfd, sendBuff, strlen(sendBuff));

				received_file = fopen(filename, "w");
				if (received_file == NULL)
			    {
				    fprintf(stderr, "Failed to open file");
				    exit(EXIT_FAILURE);
			    }

			    remain_data = file_size_rcv;
			    bzero(recvBuff, BUFFER_SIZE);
			    while((remain_data > 0) && (n = read(sockfd, recvBuff, BUFFER_SIZE)) > 0)
			    {
			    	fwrite(recvBuff, sizeof(char), n, received_file);
			    	remain_data -= n;
			    	printf("remain_data: %d\n", remain_data);
			    }
			    fclose(received_file);
			    printf("Done!\n");

			    // MD5 checksum
			    computeMd5(filename, md5_compute);
			    printf("md5: %s\n", md5);
			    printf("md5_compute: %s\n", md5_compute);
			    if (md5_compare(md5, md5_compute))
			    {
			    	printf("File received successfully!\n");
			    }
				break;

			case 2:
				// Client sends cmd:
				bzero(sendBuff, BUFFER_SIZE);
			    snprintf(sendBuff, BUFFER_SIZE, "UPL");
				write(sockfd, sendBuff, strlen(sendBuff));

			    bzero(recvBuff,BUFFER_SIZE);
				n = read(sockfd, recvBuff, BUFFER_SIZE - 1);

				// Client sends file name
				printf("File: ");
				// Client sends file name
				bzero(filename, 1024);
			    scanf("%s", filename);
			    file_length = strlen(filename) - 1;
			    bzero(sendBuff, BUFFER_SIZE);
			    snprintf(sendBuff, BUFFER_SIZE, "%s,%d", filename, file_length);
			    printf("sendBuff: %s\n", sendBuff);
				write(sockfd, sendBuff, strlen(sendBuff));
				printf("2\n");

			    // Receive server's ACK
			    bzero(recvBuff, BUFFER_SIZE);
				n = read(sockfd, recvBuff, BUFFER_SIZE - 1);
				printf("Client receive ACK\n");

				// Client sends file size
				file_size = fsize(filename);
				if (file_size <= 0)
				{
					printf("File is not existed on client!\n");
					break;
				}
				bzero(sendBuff, BUFFER_SIZE);
			    snprintf(sendBuff, BUFFER_SIZE, "%d", file_size);
				write(sockfd, sendBuff, strlen(sendBuff));
				printf("Client  send file size\n");

				// read client request file
				n = read(sockfd, recvBuff, BUFFER_SIZE - 1);

				fd = open(filename, O_RDONLY);
				if (fd == -1)
		        {
	                fprintf(stderr, "Error opening file --> %s", strerror(errno));
	                exit(EXIT_FAILURE);
		        }
		        printf("Client open file to write\n");

		        // send file
		        remain_data = file_size;
		        printf("Sending %s\n", filename);
		        while((remain_data > 0) && (send_bytes = sendfile(sockfd, fd, &offset, BUFFER_SIZE)))
		        {
		        	printf("remain: %d, send_byte: %d\n", remain_data, send_bytes);
		       		remain_data -= send_bytes;
		        }
		        printf("Sent!\n");
		        close(fd);

		        // Client send md5 to server
				n = read(sockfd, recvBuff, BUFFER_SIZE - 1);
				printf("Client read md5 request\n");

				// Response to server MD5
				bzero(sendBuff, sizeof(sendBuff));
				computeMd5(filename, md5);
				write(sockfd, md5, strlen(md5));
				printf("Client response md5\n");

				// Client read server is well received
				n = read(sockfd, recvBuff, BUFFER_SIZE - 1);
				printf("Client read server is well received\n");

		        break;

			case 3:
				// Client sends cmd:
				bzero(sendBuff, BUFFER_SIZE);
			    snprintf(sendBuff, BUFFER_SIZE, "EXIT");
				write(sockfd, sendBuff, strlen(sendBuff));

				printf("Logged out of FILE Server..\n");
				isLoggedOut = 1;
				close(sockfd);
				break;

			default:
				;
		}
	}
	return 0;
}