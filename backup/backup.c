#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdint.h>


void cpr(const char*, const char*);

int main(int argc, char * argv[]) {
	if(argc != 3) {
		puts("Wrong number of args.");
		exit(-1);
	}


	mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    cpr(argv[1], argv[2]);

    while(wait(NULL) != -1);

    return 0;
}

int cmp(const char * inpath, const char * topath) {
    int fp1 = open(inpath, O_RDONLY);
    if(fp1 == -1) {
        printf("Can't open %s\n", inpath);
        return -1;
    }
    int fp2 = open(topath, O_WRONLY | O_CREAT);
    if(fp2 == -1) {
        printf("Can't open %s\n", topath);
        close(fp1);
        return -1;
    }
    uint8_t * buf1 = calloc(4096, 1), * buf2 = calloc(4096, 1);
    size_t read_bytes1, read_bytes2;
    int diff = 0;
    while ((read_bytes1 = read(fp1, buf1, 4096)) > 0 || (read_bytes2 = read(fp2, buf2, 4096)) > 0) {
        if(read_bytes1 != read_bytes2) {
            diff = 1;
            break;
        }
        if(memcmp(buf1, buf2, 4096) != 0) {
            diff = 1;
            break;
        }
    }
    if(read_bytes1 == -1 || read_bytes2 == -1) {
        printf("Error on reading %s\n", read_bytes1 == -1 ? topath : inpath);
        diff = -1;
    }
    close(fp1);
    close(fp2);
    return diff;
}

int cp(char * inpath, char * topath){
    int fp1 = open(inpath, O_RDONLY);
    if(fp1 == -1) {
        printf("Can't open %s\n", inpath);
        return -1;
    }
    int fp2 = open(topath, O_WRONLY | O_CREAT, 0666);
    if(fp2 == -1) {
        printf("Can't open %s\n", topath);
        close(fp1);
        return -1;
    }

    uint8_t  * buf = calloc(4096, sizeof(uint8_t));
    size_t read_size, wrote_size = 0;
    while ((read_size = (read(fp1, buf, 4096))) > 0) {
        wrote_size = write(fp2, buf, read_size);
        if(wrote_size != read_size) {
            printf("Error on copying %s to %s: read %zu bytes, wrote %zu\n", inpath, topath, read_size, wrote_size);
            close(fp1);
            close(fp2);
            return (int)(wrote_size - read_size);
        }
    }
    close(fp1);
    close(fp2);
    if(read_size == -1)
        printf("Error on reading %s\n", inpath);
    else if(wrote_size == -1)
        printf("Error on writing %s", topath);
    return 0;
}

void cpr(const char * inpath, const char * topath) {
    DIR * dp;
    struct dirent * dirp;
    if((dp = opendir(inpath)) == NULL) {
        printf("Unable to open %s\n", inpath);
        return;
    }


    while((dirp = readdir(dp)) != NULL) {
        char pname[FILENAME_MAX];
        strcpy(pname, dirp->d_name);
        char pt2[FILENAME_MAX];
        strcpy(pt2, inpath);
        strcat(pt2, "/");
        strcat(pt2, pname);
        char pt[FILENAME_MAX];
        strcpy(pt, topath);
        strcat(pt, "/");
        strcat(pt, pname);

        if(dirp->d_type == 4) {
            if(strcmp(pname, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
                continue;

            if(mkdir(pt, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
                if (errno != EEXIST)
                    printf("Unable to make dir: %s\n", strerror(errno));
            }

            cpr(pt2, pt);
        } else {
            char ptgz[FILENAME_MAX];

            strcpy(ptgz, pt);
            strcat(ptgz, ".gz");
            if(access(ptgz, F_OK)) {

                pid_t pid = fork();

                if(pid < 0) {
                    puts("Fork failure");
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
                        puts("Second fork failure");
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
