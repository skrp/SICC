#include <stdio.h>
#include <sha256.h>
#define SIZE 10000

int main(int argc, *argv[])
{
	FILE *fp1, *fp2;
	long size = fsize(argv[1]);
	int total=size/10000+1, count=0;
	char fname[120]
	char data[SIZE+1];	
	fp1 = fopen(argv1, r);
		while(fgets(data, SIZE, fp1) && count <= SIZE)
		{
			char *chunk_sha = SHA256_FileChunk(argv[1], NULL, position, SIZE);
			fputs(data, fp2);
			position += SIZE;
		}
		fclose(fp2);
	}
	return 0;
}
char sha()
{
	return;
}
void usage()
	{ printf("ARG1 File_List ARG2 Dump Dir"); exit(1); }
long fsize(char *name)
{
	FILE *fp = fopen(name, "rb");
	long size;
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		size = ftell(fp)+1;
		fclose(fp);
	}
	return size;
}
