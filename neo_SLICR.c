//#####################################
// SLICR - shred file into random sizes
// FreeBSD

// ISSUES: HELP PLZ
// clang --analyze
//    SLICR.c:66:3: warning function call argument is an uninitialized value
//    strcat(f_key, key_path);

// ln_49:  (size_t) len;
// ln_128: while ((k_line = fgetln(kfp, &len)) != NULL)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sha256.h>
#include <sys/stat.h>
// GLOBAL
#define SHALEN 65
#define SIZE 1000000 // 1B -> 1MB
// USAGE
static void usage()
  { printf("usage: SLICR target_file dump_path key_path\n"); exit(1); }
int build(char *f_block, char *v_file);
// BEGIN ##############################
int main(int argc, char *argv[])
{
// ARG CHK
  if (argc != 4)
    { usage(); }
  struct stat st_dump;

  if (stat(argv[2], &st_dump) != 0)
    { printf("FAIL dump_path %s", argv[2]); exit(1); }
  if (stat(argv[3], &st_dump) != 0)
    { printf("FAIL key_path %s", argv[3]); exit(1); }
// SEED RAND
  srand((unsigned int) time(NULL));
// DECLARE
  FILE *fp, *kfp, *vfp;

  unsigned long long int f_size, position = 0;

  char *key_path, *dump_path, *v_path;
  char *f_sha, *b_sha, *v_sha;
  char *f_key, *f_block, *v_file, *k_line;

  size_t len;
// SANITIZE
  dump_path = malloc(strlen(argv[2] + SHALEN + 1));
  if (dump_path[strlen(dump_path) - 1] != '/')
    { strcat(dump_path, "/"); }
  key_path = malloc(strlen(argv[3] + SHALEN + 1));
  strcat[]
  if (key_path[strlen(key_path) - 1] != '/')
    { strcat(key_path, "/"); }
// TARGET FILE
  if ((fp = fopen(argv[1], "rb")) < 0)
    { printf("FAIL fopen(fp) at%s\n", argv[1]); }

  fseek(fp, 0, SEEK_END);
  f_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  f_sha = SHA256_File(argv[1], NULL);
// KEY FILE
  strcat(f_key, key_path);
  strcat(f_key, f_sha);

  if ((kfp = fopen(f_key, "wb")) < 0)
    { printf("FAIL fopen(f_key) at: %s\n", argv[3]); exit(1); }
// SLICE //////////////////////////////
  while (position < f_size)
  {
// DECLARE
    FILE *bfp;
    unsigned long int size, read_size;
    char *buf, *b_sha, *f_block;
// block SIZE
    size = rand() % SIZE;

    if (size == 0)
      { continue; }

    if (position + size >= f_size)
      { size = f_size - position; }

    if ((buf = malloc(size)) == NULL)
      { printf("FAIL memory buf pos: %llu\n", position); exit(1); }
// write block
    if ((read_size = fread(buf, 1, (size_t) size, fp)) != size)
      { printf("FAIL read mismatch size: %lu read_size: %lu\n", size, read_size); exit(1); }
// sha block
    b_sha = SHA256_FileChunk(argv[1], NULL, (off_t) position, (off_t) size);
    strcat(f_block, dump_path);
    strcat(f_block, b_sha);

    if ((bfp = fopen(f_block, "wb")) < 0)
      { printf("FAIL f_block open pos: %llu\n", position); exit(1); }

    if ((fwrite(buf, 1, (size_t) size, bfp)) != size)
      { printf("FAIL write block: %llu \n", position); exit(1); }
// write key
    fwrite(b_sha, 1, SHALEN, kfp);
    fwrite("\n", 1, 1, kfp);

    position += size;
///////////////////////////////////////
// cleanup
//    fclose(bfp);
//    free(size); free(read_size);
//    free(buf); free(b_sha); free(f_block);
  }
// VERIFICATION BUILD &&&&&&&&&&&&&&&&&
  strcat(v_path, key_path);
  strcat(v_path, "tmp");

  if ((vfp = fopen(v_path, "wb")) < 0)
    { printf("FAIL fopen(v_file) at: %s\n",v_path); exit(1); }

  if ((kfp = fopen(f_key, "rb")) < 0)
    { printf("FAIL fopen(f_key) at: %s\n", f_key); exit(1); }
// build verification-file
  while ((k_line = fgetln(kfp, &len)) != NULL)
  {
    strcat(f_block, dump_path);
    strcat(f_block, k_line);
    if ((build(f_block, v_file)) < 0)
      { printf("FAIL push(v_file) at: %s\n", f_key); exit(1); }
  }
// INTEGRITY CHK
  v_sha = SHA256_File(v_file, NULL);
  if ((strcmp(f_sha, v_sha)) < 0)
    { printf("FAIL VERIFICATION with %s\n", f_key); exit(1); }
// cleanup
//  fclose(fp); fclose(kfp); fclose(vfp);
//  free(f_size); free(position);
//  free(key_path); free(dump_path); free(v_path);
//  free(f_sha); free(b_sha); free(v_sha);
//  free(f_key); free(f_block); free(v_file);

  return 0;
}
// FN #################################
int build(char *f_block, char *v_file)
{
  FILE *vfp, *bfp;
  unsigned long int b_size, writ_size;
  char *buf;
// verification file
  if ((vfp = fopen(v_file, "wb")) < 0)
    { printf("FAIL fopen(v_file) at: %s\n",v_file); exit(1); }
// block file
  if ((bfp = fopen(f_block, "rb")) < 0)
    { printf("FAIL fopen(f_block) at: %s\n",f_block); exit(1); }

  fseek(bfp, 0, SEEK_END);
  b_size = ftell(bfp);
  fseek(bfp, 0, SEEK_SET);

  if ((buf = malloc(b_size)) == NULL)
    { printf("FAIL out of memory buf b_size: %lu\n", b_size); exit(1); }
// read block-file
  if ((writ_size = fread(buf, 1, (size_t) b_size, bfp)) != b_size)
    { printf("FAIL write size mismatch b_size: %lu writ_size: %lu\n", b_size, writ_size); exit(1); }

// write verification-file
  if ((fwrite(buf, 1, (size_t) b_size, vfp)) != b_size)
    { printf("FAIL write v_fp: %s \n", v_file); exit(1); }

  return 0;
}
// cleanup
//  fclose(vfp); fclose(bfp);
//  free(b_size); free(writ_size);
//  free(buf);
//  free(f_block); free(v_file);
