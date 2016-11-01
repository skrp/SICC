/* slicr.c
 *
 * Shred a file into random sized parts
 * Store the complete file key as a qui file
 *
 * ARG2 - FILE
 * ARG3 - DUMP DIR
 * ARG4 - QUI DIR
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sha256.h>
#include <sys/stat.h>
// GLOBAL
#define SHA256_DIGEST_LENGTH 32
#define FILE_STEP 1024
// USAGE
static void usage()
{
    fprintf(stderr, "usage: slicr file path_to_dump path_to_qui_dump\n");
    exit(1);
}

// RECOMPILE FROM QUI = dp
// CHUNK_FILE = sp
int push(char * chunk_file, const char * dest_file)
{
    FILE *sp, *dp;
// OPEN FILES
    sp = fopen(chunk_file, "rb");
    dp = fopen(dest_file, "a");
// VERIFY FILES
    if (sp == NULL)
    {
        printf("Problem with opening source file\n");
        return 1;
    }
    if (dp == NULL)
    {
        printf("Problem with opening dist file\n");
        return 1;
    }

// CHUNK SIZE = f_size
    fseek(sp, 0, SEEK_END);
    long f_size = ftell(sp);
    fseek(sp, 0, SEEK_SET);

    long position = 0;

////////////////////////////////////////////////////////////
// ITERATE BY FILE_STEP = size
    while (position + 1 < f_size)
    {
        long size = FILE_STEP;
        if (position + size >= f_size)
        {
            size = f_size - position;
        }

// BUFFER
        char * buffer = (char*) malloc(sizeof(char) * size);
        if (buffer == NULL)
        {
            fprintf(stderr, "memory error\n");
            fclose(sp);
            fclose(dp);
            return 1;
        }

        position += size;

// TEST SIZE INTEGRITY = read_size
        size_t read_size = fread(buffer, 1, (size_t) size, sp);
        if (read_size != size)
        {
            fprintf(stderr, "read error\n");
            fclose(sp);
            fclose(dp);
            return 1;
        }

// CONCATENATE
        fwrite(buffer, 1, (size_t) size, dp);
        free(buffer);
    }
/////////////////////////////////////////////////////////////////////

    fclose(sp);
    fclose(dp);
    return 0;
}

// BEGIN
int main(int argc, char *argv[])
{
    int i = 0;
// TEST ARGS
    if (argc != 4)
    {
        usage();
    }
// OPEN & CHECK FILE = fp
    FILE *fp, *mfp;
    fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        printf("Problem with opening file\n");
        exit(1);
    }

// TEST DIRECTORIES
    struct stat st_dump;
    if (stat(argv[2], &st_dump) != 0)
    {
        printf("Problem with dump dir\n");
        fclose(fp);
        fclose(mfp);
        exit(1);
    }
    if (stat(argv[3], &st_dump) != 0)
    {
        printf("Problem with qui dir\n");
        fclose(fp);
        fclose(mfp);
        exit(1);
    }

// SEED RAND
    srand((unsigned int) time(NULL));

// FILE SIZE = f_size
    fseek(fp, 0, SEEK_END);
    long f_size = ftell(fp);
// RESET FILE
    fseek(fp, 0, SEEK_SET);
    long position = 0;
// COMPLETE FILE SHA = file_hash
    char * file_hash = SHA256_File(argv[1], NULL);

// QUI FILE PATH = qui_file_path
    char * qui_file_path = (char *) malloc(sizeof(char) * (strlen(argv[3]) + strlen(file_hash) + 1));
    strcpy(qui_file_path, argv[3]);
    strcat(qui_file_path, file_hash);

// CREATE QUI FILE
    mfp = fopen(qui_file_path, "w");
    if (mfp == NULL)
    {
        printf("Problem with opening qui file\n");
        fclose(fp);
        exit(1);
    }
////////////////////////////////////////////////////////////////////////////
// SLICR
    while (position + 1 < f_size)
    {
// RAND NUMB = size
        long size = rand() % 1000000;
// TEST RAND NUMB COMPATIBILITY
        if (position + size >= f_size)
        {
            size = f_size - position;
        }
// BUFFER
        char * buffer = (char*) malloc(sizeof(char) * size);
        if (buffer == NULL)
        {
            fprintf(stderr, "memory error\n");
            fclose(fp);
            fclose(mfp);
            exit(1);
        }
        position += size;
// TEST SIZE INTEGRITY = read_size
        size_t read_size = fread(buffer, 1, (size_t) size, fp);
        if (read_size != size)
        {
            fprintf(stderr, "read error\n");
            fclose(fp);
            fclose(mfp);
            exit(1);
        }
// PARTIAL FILE SHA = chunk_hash
        char * chunk_hash = SHA256_FileChunk(argv[1], NULL, position, size);
// SET DUMP PATH
        char * dump_file_path = (char *) malloc(sizeof(char) * (strlen(argv[2]) + strlen(chunk_hash) + 2));
        strcpy(dump_file_path, argv[2]);
        if (dump_file_path[strlen(argv[2]) - 1] != '/')
        {
            strcat(dump_file_path, "/");
        }
        strcat(dump_file_path, chunk_hash);

        FILE *wfp;
        wfp = fopen(dump_file_path, "w");
        free(dump_file_path);

        if (wfp == NULL)
        {
            printf("Problem with opening dump file\n");
            fclose(fp);
            fclose(mfp);
            exit(1);
        }

        fwrite(buffer, 1, (size_t) size, wfp);
        free(buffer);
        fclose(wfp);

        fwrite(chunk_hash, 1, 2 * SHA256_DIGEST_LENGTH, mfp);
        fwrite("\n", 1, 1, mfp);
        free(chunk_hash);
    }
////////////////////////////////////////////////////////////////////////////
    fclose(fp);
    fclose(mfp);

// OPERATION INTEGRITY CHECK
    mfp = fopen(qui_file_path, "rb");
    free(qui_file_path);
    if (mfp == NULL)
    {
        printf("Problem with opening qui file\n");
        exit(1);
    }
// EXPERIMENT = tmp_file
    char * tmp_file = (char *) malloc(sizeof(char) * (strlen(argv[2]) + 10));
    strcpy(tmp_file, argv[2]);
    if (tmp_file[strlen(argv[2]) - 1] != '/')
    {
        strcat(tmp_file, "/");
    }
    strcat(tmp_file, "tmp_copy");
// CREATE EXPERIMENT FILE
    fp = fopen(tmp_file, "w");
    if (fp == NULL)
    {
        printf("Problem with opening tmp file\n");
        exit(1);
    }
    fclose(fp); 

    size_t len;
    char *chunk_fname;
////////////////////////////////////////////////////////////////////////////
// RECOMPILE FILE
    while ((chunk_fname = fgetln(mfp, &len)) != NULL)
    {
// SANITIZED DATA = chunk_fname
        if (chunk_fname[len - 1] == '\n')
        {
            chunk_fname[len - 1] = '\0';
        }
        char * chunk_file = (char *) malloc(sizeof(char) * (strlen(argv[2]) + strlen(chunk_fname) + 2));

        strcpy(chunk_file, argv[2]);
        if (chunk_file[strlen(argv[2]) - 1] != '/')
        {
            strcat(chunk_file, "/");
        }
// CONCATENATE
        strcat(chunk_file, chunk_fname);
// PUSH FN
        if (push(chunk_file, tmp_file) != 0)
        {
            exit(1);
        }
        free(chunk_file);
    }
///////////////////////////////////////////////////////////////////////////
    fclose(mfp);

// VERIFY FINAL RESULT OF OPERATIONS = tmp_hash
    char *tmp_hash = SHA256_File(tmp_file, NULL);
    if (strcmp(tmp_hash, file_hash) != 0)
    {
        printf("Integrity problem\n");
    }
// CLEAN UP
    free(tmp_hash);
    remove(tmp_file);
    free(tmp_file);
    free(chunk_fname);
    free(file_hash);
    return 0;
}
/* 
 * MKRX skrp
 * Feat. Mikhail Usvyatsov
 */
