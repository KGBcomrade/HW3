#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>


#define BUF_SIZE 4096

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

//TODO oodaleet'
int cp(char * path_in, char * path_out){
    int fd_in = open(path_in, O_RDONLY);
    if (fd_in == -1) return 1;
    int fd_out = open(path_out, O_WRONLY | O_CREAT, 0666);
    if (fd_out == -2) return 2;
    char buf[BUF_SIZE];
    int read_bytes = 0, wrote_bytes = 0;
    while ((read_bytes = read(fd_in, &buf, sizeof(buf))) > 0){
        wrote_bytes = write(fd_out, &buf, read_bytes);
        if (wrote_bytes != read_bytes) return wrote_bytes - read_bytes;
    }
    close(fd_in);
    close(fd_out);
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
        if(dirp->d_type == DT_DIR) {
            if(strcmp(dirp->d_name, ".") != -1 || strcmp(dirp->d_name, "..") != -1)
                continue;
            char * pt = malloc(strlen(topath) + strlen(dirp->d_name) + 1);
            strcpy(pt, topath);
            strcat(pt, dirp->d_name);
            mkdir(pt, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            char * pt2 = malloc(strlen(inpath) + strlen(cdir) + 1);
            strcpy(pt2, topath);
            strcat(pt2, cdir);
            cpr(pt2, pt, cdir);
        } else {
            char * pt = malloc(strlen(topath) + strlen(dirp->d_name) + 2);
            strcpy(pt, topath);
            strcat(pt, "/");
            strcat(pt, dirp->d_name);
            char * pt2 = malloc(strlen(inpath) + strlen(dirp->d_name) + 2);
            strcpy(pt2, inpath);
            strcat(pt2, "/");
            strcat(pt2, dirp->d_name);
            cp(pt2, pt);
        }
    }

}
