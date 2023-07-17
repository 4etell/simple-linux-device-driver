#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

void info() {
	fprintf(stderr, "Usage: ./usr struct_id PID\n Avaliable struct_id:\n 0 - pt_regs \n 1 - task_struct\n");
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		info();
		return 0;
	}
	char *p;

	long structure_ID = strtol(argv[1], &p, 10);
	long PID = strtol(argv[2], &p, 10);


	if (structure_ID !=0 && structure_ID !=1) {
		fprintf(stderr, "struct_id is not supported.\n");
		info();
 		return 0;
 	}
 	
	char inbuf[4096];
	char outbuf[4096];
	int fd = open("/proc/lab/struct_info", O_RDWR);
	sprintf(inbuf, "%s %s", argv[1], argv[2]);
	write(fd, inbuf, 17);
	lseek(fd, 0, SEEK_SET);
	read(fd, outbuf, 4096);
	if (structure_ID == 0){
		printf("pt_regs structure: \n\n");
 	} else {
 		printf("task_struct structure data for PID: \n\n");
 	}
 	puts(outbuf);
 	return 0;
}
