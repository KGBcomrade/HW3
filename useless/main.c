#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>




#define err(msg) {puts(msg); \
	exit(-1); \
}



int main(int argc, char ** argv) {
	if(argc < 2) 
		err("No file name given!");
	FILE *f;
	f = fopen(argv[1], "r");

	char * str;
	int d = 0, c;
	while((c = fscanf(f, "%d %255s", &d, str)) == 2) {

		printf("%d %s\n", d, str);
		if(d < 0)
			err("Wrong delay format!");

		pid_t pid = fork();

		switch(pid) {
			case -1:
				err("Fork failure!");
				break;
			case 0:
				sleep(d);
				execlp(str, str, NULL); //Executing given program
				err("Failure!");
				break;
		}
	}


	while(wait(NULL) != -1);

	if(!feof(f))
		puts("Incorrect input file format!"); 
	fclose(f);
	
	return 0;
}

	
