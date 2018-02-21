// file server

// build command: gcc -o server server.c -lcrypto

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
#include <sys/stat.h>
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
	// Socket parameter
	int listenfd = 0;
	int connfd = 0;
	struct sockaddr_in serv_addr;
	int n = 0;
	int client_id = 0;

	// Buffer for send and receive
	char sendBuff[BUFFER_SIZE];
	char recvBuff[BUFFER_SIZE];
	char filename[1024];

	short int file_length;
	unsigned int file_size;
	unsigned char md5[MD5_DIGEST_LENGTH];
	unsigned char md5_compute[MD5_DIGEST_LENGTH];
	int send_bytes;
	off_t offset;
	int remain_data;
	int file_size_rcv;
	int i;
	int fd;
	int isLoggedOut;
	int cmd;

	FILE * received_file;

	// Create a socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(7000);

	bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	computeMd5("SmallFile.txt", md5_compute);
	printf("md5_compute: %s, len: %d\n", md5_compute, strlen(md5_compute));

	// Server is listening
	listen(listenfd, 10);
	printf("Server is waiting for connections..\n");
	while(1)
	{
		connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
		client_id++;
		isLoggedOut = 0;
		cmd = 0;

		while(!isLoggedOut)
		{
			printf("while loop\n");
			// Client reads cmd
			bzero(recvBuff,BUFFER_SIZE);
			n = read(connfd, recvBuff, BUFFER_SIZE - 1);

			bzero(sendBuff, BUFFER_SIZE);
		    snprintf(sendBuff, BUFFER_SIZE, "Receive cmd!");
			write(connfd, sendBuff, strlen(sendBuff));

			if (!strcmp(recvBuff, "REQ"))
			{
				cmd = 1;
			}
			else if (!strcmp(recvBuff, "UPL"))
			{
				cmd = 2;
			}
			else if (!strcmp(recvBuff, "EXIT"))
			{
				cmd = 3;
			}

			switch(cmd)
			{
				case 1:
					printf("Processing REQ\n");
					// Server reads file name & length
					bzero(recvBuff, sizeof(recvBuff));
					n = read(connfd, recvBuff, BUFFER_SIZE - 1);
					for (i = 0; i < strlen(recvBuff); i++)
					{
						if (recvBuff[i] != ',')
							filename[i] = recvBuff[i];
						else
							break;
					}
					filename[i] = '\0';
					i++;
					file_length = 0;
					while (i < strlen(recvBuff))
					{
						file_length = file_length * 10 + (recvBuff[i] - '0');
						i++;
					}

					// Check file name exist on server or NOT
					if(access(filename, F_OK ) != -1 ) {
				    // file exists
				    	file_size = fsize(filename);
				    	// Response to client file_size
						bzero(sendBuff, sizeof(sendBuff));
						snprintf(sendBuff, sizeof(sendBuff), "%d", file_size);
						write(connfd, sendBuff, strlen(sendBuff));

						// read client request MD5
						n = read(connfd, recvBuff, BUFFER_SIZE - 1);

						// Response to client fMD5
						bzero(sendBuff, sizeof(sendBuff));
						computeMd5(filename, md5);
						write(connfd, md5, strlen(md5));

						// read client request file
						n = read(connfd, recvBuff, BUFFER_SIZE - 1);

						fd = open(filename, O_RDONLY);
						if (fd == -1)
				        {
			                fprintf(stderr, "Error opening file --> %s", strerror(errno));
			                exit(EXIT_FAILURE);
				        }

				        // send file
				        remain_data = file_size;
				        printf("Sending %s\n", filename);
				        while(send_bytes = sendfile(connfd, fd, &offset, BUFFER_SIZE) && remain_data > 0)
				        {
				       		remain_data -= send_bytes;
				        }
				        printf("Sent!\n");
				        close(fd);
					} else {
					    // file doesn't exist
					    // Response to client file_size
						bzero(sendBuff, sizeof(sendBuff));
						snprintf(sendBuff, sizeof(sendBuff), "1");
						write(connfd, sendBuff, strlen(sendBuff));
					}
					break;

				case 2:
					printf("Processing UPL\n");
					// Server reads file name & length
					bzero(recvBuff, sizeof(recvBuff));
					n = read(connfd, recvBuff, BUFFER_SIZE - 1);
					for (i = 0; i < strlen(recvBuff); i++)
					{
						if (recvBuff[i] != ',')
							filename[i] = recvBuff[i];
						else
							break;
					}
					filename[i] = '\0';
					i++;
					file_length = 0;
					while (i < strlen(recvBuff))
					{
						file_length = file_length * 10 + (recvBuff[i] - '0');
						i++;
					}
					printf("File: %s\n", filename);

					// Server ack
					printf("Before Server ACK\n");
					bzero(sendBuff, sizeof(sendBuff));
					snprintf(sendBuff, sizeof(sendBuff), "ACK: Ready");
					printf("Before 2Server ACK\n");
					write(connfd, sendBuff, strlen(sendBuff));
					printf("Server ACK\n");

					// Server read file size
					bzero(recvBuff, sizeof(sendBuff));
					n = read(connfd, recvBuff, BUFFER_SIZE - 1);
					printf("Server read file size\n");

					// Request file
					snprintf(sendBuff, BUFFER_SIZE, "need file");
					write(connfd, sendBuff, strlen(sendBuff));
					printf("Server request file\n");

					received_file = fopen(filename, "w");
					if (received_file == NULL)
				    {
					    fprintf(stderr, "Failed to open file");
					    exit(EXIT_FAILURE);
				    }
				    printf("Sever open file to write\n");

				    printf("get file\n");
				    remain_data = file_size_rcv;
				    bzero(recvBuff, BUFFER_SIZE);
				    while((remain_data > 0) && (n = read(connfd, recvBuff, BUFFER_SIZE)) > 0)
				    {
				    	fwrite(recvBuff, sizeof(char), n, received_file);
				    	remain_data -= n;
				    	printf("remain_data: %d\n", remain_data);
				    }
				    fclose(received_file);
				    printf("Done!\n");

				    // Server requests md5
				    snprintf(sendBuff, BUFFER_SIZE, "need md5");
					write(connfd, sendBuff, strlen(sendBuff));
					printf("Server request md5\n");

					// Server read md5 from client
					bzero(md5, sizeof(md5));
					n = read(connfd, md5, BUFFER_SIZE);
					
					// MD5 checksum
					printf("BeforeServer check md5\n");
					printf("md5: %s, len: %d\n", md5, strlen(md5));
				    computeMd5(filename, md5_compute);
				    
				    printf("md5_compute: %s, len: %d\n", md5_compute, strlen(md5_compute));
				    if (md5_compare(md5, md5_compute))
				    {
				    	printf("File received successfully!\n");
				    }
				    printf("Server check md5\n");

				    // server well receive
				    snprintf(sendBuff, BUFFER_SIZE, "well receive");
					write(connfd, sendBuff, strlen(sendBuff));
				    break;

				case 3:
					printf("Processing EXIT\n");
					isLoggedOut = 1;
					close(connfd);
					sleep(1);
					break;
				default:
					;
			}
		}
	}
	close(listenfd);


	return 0;
}