///////////////////////////////////////
// SLICR  
// split file into random chunks
// each chunk is named after their sha256 digest
// QUI - file of ordered chunk listings
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sha256.h>
#include <sys/stat.h>
#include <sys/types.h>
// GLOBAL
#define DIGEST_LEN 32
#define STEP 1024
#define LIM 99999 // chunk lim .1 MB
static void usage();
int build(char *chunk_file, const char *dest_file);
int main(int argc, char *argv[])
{
	int i=0;
	long position=0, f_size=0;
	uint32_t size=0;
	if (argc != 4) usage();
	FILE *fp; // TARGET FILE
	FILE *qfp; // QUI FILE
	fp = fopen(argv[1], "rb");
	if (fp == NULL) { printf("ERR open %s\n", argv[1]); exit(1); }
	struct stat st_dump;
	if ((stat(argv[2], &st_dump) != 0) && S_ISDIR(st_dump.st_mode))
		{ printf("ERR dump dir %s\n", argv[2]); exit(1); }
	fseek(fp, 0, SEEK_END); f_size = ftell(fp); fseek(fp, 0, SEEK_SET);
// DIGEST
	char *digest = SHA256_File(argv[1], NULL);
// QUI PATH
	char *qui_path = malloc(strlen(argv[3]) + DIGEST_LEN + 1); 
	strcpy(qui_path, argv[3]);
	if (qui_path[strlen(qui_path) - 1] != '/')
		strcat(qui_path, "/");
	strcat(qui_path, digest);
// QUI
	qfp = fopen(qui_path, "w");
	if (qfp == NULL) { printf("ERR qui creation at %s\n", argv[1]); exit(1); }
// SLICR /////////////////////////////
	while (position + 1 < f_size)
	{
		size = arc4random_uniform(LIM);
		if (position + size >= f_size)
			size = f_size - position;
// BUFFER
		char *buffer = malloc(size);
		if (buffer == NULL) 
			{ printf("ERR slicr mem %s: position %ld @ %u\n", argv[1], position, size); exit(1); }
		size_t read_size = fread(buffer, 1, (size_t) size, fp);
		if (read_size != size) 
			{ printf("ERR %s inequality size: %u != read_size: %zu\n", argv[1], size, read_size); exit(1); }
// CHUNK ALLOCATION
		char *chunk_digest = SHA256_FileChunk(argv[1], NULL, position, size);
		char *chunk_path = malloc(strlen(argv[3]) + strlen(chunk_digest) + 1);
		strcpy(chunk_path, argv[2]);
		if (chunk_path[strlen(chunk_path) - 1] != '/')
			strcat(chunk_path, "/");
		strcat(chunk_path, chunk_digest);
// CHUNK 
		FILE *cfp;
		cfp = fopen(chunk_path, "w");
		if (cfp == NULL)
			{ printf("ERR chunk file %s : %s\n", argv[1], chunk_path); exit(1); }
		free(chunk_path);
		fwrite(buffer, 1, (size_t) size, cfp);
		free(buffer); fclose(cfp);
		fwrite(chunk_digest, 1, 2 * DIGEST_LEN, qfp);
		fwrite("\n", 1, 1, qfp); 
		position += size; free(chunk_digest);
	}
	fclose(fp); fclose(qfp);
// SETUP VERIFY
	qfp = fopen(qui_path, "rb"); free(qui_path);
	if (qfp == NULL) { printf("ERR qui reread at %s\n", argv[1]); exit(1); }
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
	while ((chunk_fname = fgetln(qfp, &len)) != NULL) 
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
	fclose(qfp);
// INTEGRITY CHK
	char *tmp_digest = SHA256_File(tmp, NULL);
	if (strcmp(tmp_digest, digest) != 0)
		{ printf("INTEGRITY ISSUE: %s\n", argv[1]); exit(1); }
	free(tmp_digest); remove(tmp); free(tmp);
	free(chunk_fname); free(digest);
	return 0;
}
// FNS ///////////////////////////////
static void usage() 
	{ printf("usage: slicr file dump_path qui_path\n"); exit(1); }
int build(char *chunk_file, const char *dest_file)
{
	FILE *sp; // CHUNK FILE
	FILE *dp; // NEW FULL FILE
	sp = fopen(chunk_file, "rb");
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
