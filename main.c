
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

// gcc -o main main.c -lcrypto

// void computeMd5(char * filename, unsigned char * md5)
// {
// 	unsigned char c[MD5_DIGEST_LENGTH];
// 	int i;
//     FILE *inFile;
//     MD5_CTX mdContext;
//     int bytes;
//     unsigned char data[1024];
//     char temp_str[3];

//     inFile = fopen (filename, "rb");
// 	if (inFile == NULL) {
//         printf ("%s can't be opened.\n", filename);
//         return;
//     }
//     MD5_Init (&mdContext);
//     while ((bytes = fread (data, 1, 1024, inFile)) != 0)
//     MD5_Update (&mdContext, data, bytes);
//     MD5_Final (c,&mdContext);
//     c[MD5_DIGEST_LENGTH] = '\0';
//     for(i = 0; i < MD5_DIGEST_LENGTH; i++)
//     {
//     	snprintf(temp_str, sizeof(temp_str), "%02x", c[i]);
//     	md5[2*i] = temp_str[0];
//     	md5[2*i + 1] = temp_str[1];
//     }
//     fclose (inFile);
// }

void computeMd5(char * filename, unsigned char * md5)
{
	unsigned char c[MD5_DIGEST_LENGTH];
	int i;
    FILE *inFile;
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];
    char temp_str[3];

    inFile = fopen (filename, "rb");
	if (inFile == NULL) {
        printf ("%s can't be opened.\n", filename);
        return;
    }
    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
    MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);
    c[MD5_DIGEST_LENGTH] = '\0';
    for(i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
    	snprintf(temp_str, sizeof(temp_str), "%02x", c[i]);
    	md5[2*i] = temp_str[0];
    	md5[2*i + 1] = temp_str[1];
    }
    fclose (inFile);
}

int main()
{
	char buffer[1024];
	char filename[1024];
	int n;
	unsigned char md5[MD5_DIGEST_LENGTH * 2 + 1];
	unsigned char md5_2[MD5_DIGEST_LENGTH * 2 + 1];
	int i;

	n = 10;
	bzero(buffer, 1024);
    // fgets(filename, 1024, stdin);
    scanf ("%[^\n]%*c", filename);
    printf("Filename: %s\n", filename);
    // snprintf(buffer, 1024, "%s,%d", filename, n);
    // printf(" buffer: %s\n", buffer);

    if( access( filename, F_OK ) != -1 ) {
    // file exists
    	printf("YES\n");
	} else {
	    // file doesn't exist
	    printf("NO\n");
	}
	computeMd5("data.txt", md5);
	printf("%s, len: %d\n", md5, strlen(md5));

	computeMd5("data.txt", md5_2);
	printf("%s, len: %d\n", md5_2, strlen(md5_2));

	printf("%d\n", strcmp(md5, md5_2));


	return 0;
}