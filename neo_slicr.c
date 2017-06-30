/////////////////////////////////
// SLICR - file shred obfuscation
// chunk: partial file renamed by its sha
// dump : dir of file chunks
// qui  : reciepe to recompile file 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sha256.h>
#include <sys/stat.h>
#include <sys/types.h>
// GLOBAL
#define SHA_LEN 65
#define STEP 1024
#define LIM 9999 // chunk lim 10KB
static void usage();
int build(char *chunk_file, const char *dest_file);
int main(int argc, char *argv[])
{
// INITIALIZE ////////////////////
	int i=0;
	long position=0, f_size=0;
	if (argc != 4) usage();
	FILE *fp; // TARGET FILE
	FILE *qfp; // QUI FILE
	char *sha = SHA256_File(argv[1], NULL);
	char *qui_path = malloc(strlen(argv[3]) + SHA_LEN + 1); 
	strcpy(qui_path, argv[3]);
	if (qui_path[strlen(qui_path) - 1] != '/')
		strcat(qui_path, "/");
	strcat(qui_path, sha);
	fp = fopen(argv[1], "r");
	qfp = fopen(qui_path, "w");
	if (fp == NULL) 
		{ printf("ERR open %s\n", argv[1]); exit(1); }
	if (qfp == NULL) 
		{ printf("ERR qui creation at %s\n", argv[1]); exit(1); }
	struct stat st_dump; 
	struct stat st_qui;
	if ((stat(argv[2], &st_dump) != 0) && S_ISDIR(st_dump.st_mode))
		{ printf("ERR dump dir %s\n", argv[2]); exit(1); }
	if ((stat(argv[3], &st_qui) != 0) && S_ISDIR(st_qui.st_mode))
		{ printf("ERR qui dir %s\n", argv[3]); exit(1); }
// FILE SIZE
	fseek(fp, 0, SEEK_END); 
	f_size = ftell(fp); 
	fseek(fp, 0, SEEK_SET);
// SLICR /////////////////////////////
	int outs=0;
	while (position < f_size)
	{
// ERROR TOLERANCE
		if (outs > 100)
			{ printf("ERR %s @ %ld STRIKEOUT!!!\n", argv[1], position); exit(1); }
// CHUNK SIZE
		uint32_t chunk_size = arc4random_uniform(LIM);
		if (chunk_size < 100)
			continue;
		if (position + chunk_size >= f_size)
			chunk_size = f_size - position;
		char *chunk_sha = SHA256_FileChunk(argv[1], NULL, position, chunk_size);
// CHUNK PATH
		char *chunk_path = malloc(strlen(argv[3]) + SHA_LEN + 1);
		strcpy(chunk_path, argv[2]);
		if (chunk_path[strlen(chunk_path) -1] != '/')
			strcat(chunk_path, "/");
		strcat(chunk_path, chunk_sha);
// BUFFER
		char *chunk_buf =  malloc((size_t) LIM);
		if (chunk_buf == NULL)
			{ printf("ERR chunk_buf %s mem @ %ld\n", argv[1], position); exit(1); }
		size_t read_size = fread(chunk_buf, 1, (size_t) chunk_size, fp);
		if (read_size != chunk_size)
			{ printf("ERR %s read_size: %ld - chunk_size: %u\n", argv[1], read_size, chunk_size); exit(1); }
// CHUNK FILE
		FILE *cfp = fopen(chunk_path, "w");
		if (cfp == NULL)
			{ printf("ERR chunk %s - position %ld\n", argv[1], position); exit(1); }
		fwrite(chunk_buf, 1, (size_t) chunk_size, cfp);
		fclose(cfp);
// VERIFY CHUNK
		char *ver_sha = SHA256_File(chunk_path, NULL);
		if (strcmp(ver_sha, chunk_sha) != 0)
		{ // OUT
			printf("ERR %d OUT %s @ %u\n", outs, argv[1], chunk_size);
			printf("%s %s\n", chunk_sha, ver_sha);
			printf("%s\n", chunk_path);
			outs++;
			remove(chunk_path); free(chunk_buf);
			continue;
		}
// ADD QUI
		fwrite(chunk_sha, 1, SHA_LEN, qfp);
		fwrite("\n", 1, 1, qfp); 
		position += chunk_size; 
	}
	fclose(fp); 
	fclose(qfp);
// SETUP VERIFY
	FILE *vqfp; // reopen qui file
	vqfp = fopen(qui_path, "r"); 
	free(qui_path);
	if (vqfp == NULL) { printf("ERR qui reread at %s\n", argv[1]); exit(1); }
	char *tmp = malloc(strlen(argv[2]) + 10);
	strcpy(tmp, argv[2]);
	if (tmp[strlen(tmp) - 1] != '/')
		strcat(tmp, "/");
	strcat(tmp, "tmp_copy");
// VERIFY CHUNKS
	FILE *vfp; // VERFIED FILE
	vfp = fopen(tmp, "w");
	if (vfp == NULL)
		{ printf("ERR cant open %s\n", tmp); exit(1); }
	size_t len;
	char *chunk_fname;
	while ((chunk_fname = fgetln(vqfp, &len)) != NULL) 
	{
		if (chunk_fname[len - 1] == '\n')
			chunk_fname[len-1] = '\0';
		char *chunk_file = malloc(strlen(argv[2]) + strlen(chunk_fname) + 1);
		strcpy(chunk_file, argv[2]);
		if (chunk_file[strlen(chunk_file) - 1] != '/')
			strcat(chunk_file, "/");
		strcat(chunk_file, chunk_fname);
		if (build(chunk_file, tmp) != 0)
			{ printf("ERR with chunk_file & tmp\n"); exit(1); }
		free(chunk_file);
	}
	fclose(vqfp);
// INTEGRITY CHK
	char *tmp_sha = SHA256_File(tmp, NULL);
	if (strcmp(tmp_sha, sha) != 0)
		{ printf("INTEGRITY ISSUE: %s\n", argv[1]); exit(1); }
	free(tmp_sha); remove(tmp); free(tmp);
	free(chunk_fname); free(sha);
	return 0;
}
// FNS ///////////////////////////////
static void usage() 
	{ printf("usage: slicr file dump_path qui_path\n"); exit(1); }
int build(char *chunk_file, const char *dest_file)
{
	FILE *sp; // CHUNK FILE
	FILE *dp; // NEW FULL FILE
	sp = fopen(chunk_file, "r");
	dp = fopen(dest_file, "a");
	if (sp == NULL || dp == NULL)
		{ printf("Problem opening files %s or %s\n", chunk_file, dest_file); exit(1); }
	fseek(sp, 0, SEEK_END);
	long f_size = ftell(sp);
	fseek(sp, 0, SEEK_SET);
	long position = 0;
// STEP CHUNK FILE TO BUFFER
	while (position + 1 < f_size)
	{
		long size = STEP;
		if (position + size >= f_size)
			size = f_size - position;
		char *buffer = malloc(size);
		if (buffer == NULL)
			{ printf("memory error at %s %s\n", chunk_file, dest_file); exit(1); }
		position += size;
		size_t read_size = fread(buffer, 1, size, sp);
		if (read_size != size)
			{ printf("read buffer error at %s %s\n", chunk_file, dest_file); exit(1); }
// CONCATENATE
		fwrite(buffer, 1, size, dp);
		free(buffer);
	}
	fclose(sp); fclose(dp); return 0;
}
/*
logic -> random-size slice -> slice sha = $presha -> copy # of bytes to new_file -> reverify slice sha with $presha

<dead> well i would start by factoring out a lot of this into small functions
<dead> why does your build function while() loop condition have position + 1 < f_size?
<dead> why the +1
<dead> i would also replace your fseek() SEEK_SET calls with just rewind()
<dead> as it's clearer intent
*/
