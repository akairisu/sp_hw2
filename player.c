#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr , "Error!\n");
	}
	int id = atoi(argv[1]);
	int money = id * 100;
	int count = 1;
	char buf[512];
	while(count != 10){
		printf("%d %d\n" , id , money);
		fflush(stdout);
		if(read(0 , buf , sizeof(buf)) > 0){
			count++;
		}
	}
	printf("%d %d\n" , id , money);
	fflush(stdout);
	return 0;
}