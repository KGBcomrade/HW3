#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>


void cpr(const char*, const char*, const char*);

int main(int argc, char * argv[]) {
	if(argc != 3) {
		puts("Wrong number of args.");
		exit(-1);
	}


	mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);


    regex_t regex;

    if(regcomp(&regex, "(\\/\\w+)$", REG_EXTENDED)) {
        puts("Illegal regex");
        exit(-1);
    }

    regmatch_t a;
    if(regexec(&regex, argv[1], 1, &a, 0)) {
        puts("Incorrect input format");
        exit(-1);
    }

    cpr(argv[1], argv[2], &(argv[1][a.rm_so]));

    return 0;
}

int cmp(const char * inpath, const char * topath) {
    FILE * fp1 = fopen(inpath, "r");
    if(fp1 == NULL) {
        printf("Can't open %s\n", inpath);
        return -1;
    }
    FILE * fp2 = fopen(topath, "w");
    if(fp2 == NULL) {
        printf("Can't open %s\n", topath);
        fclose(fp1);
        return -1;
    }
    int c1, c2;
    while((c1 = fgetc(fp1)) != EOF && (c2 = fgetc(fp2)) != EOF) {
        if(c1 != c2)
            break;
    }
    fclose(fp1);
    fclose(fp2);
    if(c1 != EOF || c2 != EOF) {
        return 1;
    }
    return 0;
}

int cp(char * inpath, char * topath){
    FILE * fp1 = fopen(inpath, "r");
    if(fp1 == NULL) {
        printf("Can't open %s\n", inpath);
        return -1;
    }
    FILE * fp2 = fopen(topath, "w");
    if(fp2 == NULL) {
        printf("Can't open %s\n", topath);
        fclose(fp1);
        return -1;
    }

    int c;
    while((c = fgetc(fp1)) != EOF) {
        fputc(c, fp2);
    }
    fclose(fp1);
    fclose(fp2);
    return 0;
}

void cpr(const char * inpath, const char * topath, const char * cdir) {
    DIR * dp;
    struct dirent * dirp;
    if((dp = opendir(inpath)) == NULL) {
        printf("Unable to open %s\n", inpath);
        return;
    }


    while((dirp = readdir(dp)) != NULL) {
        char * pt2 = malloc(strlen(inpath) + strlen(cdir) + 2);
        strcpy(pt2, inpath);
        strcat(pt2, "/");
        strcat(pt2, dirp->d_name);
        char * pt = malloc(strlen(topath) + strlen(dirp->d_name) + 2);
        strcpy(pt, topath);
        strcat(pt, "/");
        strcat(pt, dirp->d_name);
        if(dirp->d_type == 4) {
            if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
                continue;

            if(mkdir(pt, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
                if (errno != EEXIST)
                    printf("Mda, %s\n", strerror(errno));
            }

            cpr(pt2, pt, cdir);
        } else {
            char * ptgz = malloc(strlen(pt) + 4);
            strcpy(ptgz, pt);
            strcat(ptgz, ".gz");
            if(access(ptgz, F_OK)) {
                pid_t pid = fork();

                if(pid < 0) {
                    puts("Fork failure (avtor dolbaeb)");
                    return;
                } else if (pid == 0) {
                    if(cp(pt2, pt) == 0) {
                        execlp("gzip", "gzip", pt, NULL);

                    }
                } else
                    wait(NULL);
            } else {
                pid_t pid = fork();
                if(pid < 0) {
                    puts("Fork failure!");
                    return;
                } else if(pid == 0) {
                    pid_t pid1 = fork();

                    if(pid1 < 0) {
                        puts("Fork failure (mda)");
                        return;
                    } else if (pid1 == 0) {
                        execlp("gunzip", "gunzip", ptgz, NULL);

                    } else {
                        wait(NULL);
                        if(cmp(pt, pt2)) {
                            if (cp(pt2, pt) == 0) {
                                execlp("gzip", "gzip", pt, NULL);
                            }
                        } else
                            execlp("gzip", "gzip", pt, NULL);
                    }
                } else
                    wait(NULL);
            }
        }
    }
    closedir(dp);

}
