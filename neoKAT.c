#include <stdio.h>
#include <sha256.h>
#define SIZE 10000

int main(int argc, *argv[])
{
	FILE *fpl, *tfp, *bfp;
	int total=size/10000+1, position=0, i=0;
	char fname[120]
	char data[SIZE+1];	
	fpl = fopen(argv[1], r);
	char list[SIZE];
	while (list[i] != NULL)
		long size = fsize(list[i]);
		tfp = fopen(list[i], rb);
		while(fgets(data, SIZE, tfp) && position <= SIZE)
		{
			char *b_sha = SHA256_FileChunk(list[i], NULL, position, SIZE);
			fputs(data, bfp);
			char *bpath[250] = path(&argv[2], &bfp);
			bfp = fopen(bpath, "w");
			fputs(data, bfp);
			position += SIZE;
		}
		fclose(bfp); i++;
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
void path(char, char)
{
	strcat(path, b_sha);
	p
}
