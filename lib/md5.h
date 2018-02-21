#include <openssl/md5.h>

void computeMd5(char *, unsigned char *);
int md5_compare(unsigned char *, unsigned char *);

void computeMd5(char * filename, unsigned char * md5)
{
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
    MD5_Final (md5,&mdContext);
    md5[MD5_DIGEST_LENGTH] = '\0';
    fclose (inFile);
}

int md5_compare(unsigned char * md5_1, unsigned char * md5_2)
{
	int i = 0;
	int isMatch = 1;
	for (i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		if(md5_1[i] != md5_2[i])
		{
			isMatch = 0;
			break;
		}
	}
	return isMatch;
}

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