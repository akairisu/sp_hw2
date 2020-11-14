#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

int distribute(int competition[3030][8] , int playernum){
	int competitionnum = 0;
	for(int i = 1 ; i <= playernum - 7 ; i++){
		for(int j = i + 1 ; j <= playernum - 6 ; j++){
			for(int k = j + 1 ; k <= playernum - 5 ; k++){
				for(int l = k + 1 ; l <= playernum - 4; l++){
					for(int m = l + 1 ; m <= playernum - 3 ; m++){
						for(int n = m + 1 ; n <= playernum - 2 ; n++){
							for(int o = n + 1 ; o <= playernum - 1 ; o++){
								for(int p = o + 1 ; p <= playernum ; p++){
									competition[competitionnum][0] = i;
									competition[competitionnum][1] = j;
									competition[competitionnum][2] = k;
									competition[competitionnum][3] = l;
									competition[competitionnum][4] = m;
									competition[competitionnum][5] = n;
									competition[competitionnum][6] = o;
									competition[competitionnum][7] = p;
									competitionnum++;
								}
							}
						}
					}
				}
			}
		}
	}
	return competitionnum;
}

void swap(int rank[] , int index){
	int temp;
	temp = rank[index];
	rank[index] = rank[index + 1];
	rank[index + 1] = temp;
	return;
}

void ranking(int point[] , int rank[] , int playernum){
	for(int i = 1 ; i < playernum ; i++){
		for(int j = 1 ; j <= playernum - i ; j++){
			if(point[rank[j]] < point[rank[j + 1]]){
				swap(rank , j);
			}
		}
	}
	return;
}

void score(int result[9][2] , int point[]){
	for(int i = 1 ; i <= 8 ; i++){
		point[result[i][0]] += (8 - result[i][1]);
	}
	return;
}

int main(int argc, char *argv[]){
	int hostnum;
	int playernum;
	char buf[512];
	FILE *fp;
	int file_fd[11];
	int point[15] = {0};
	int rank[15];
	if(argc != 3){
		printf("Error!\n");
	}
	hostnum = atoi(argv[1]);
	playernum = atoi(argv[2]);
	for(int i = 1 ; i <= playernum ; i++){
		rank[i] = i;
	}
	int competition[3030][8];
	int competitionnum = distribute(competition , playernum);
	char FIFO[20][20];
	for(int i = 1 ; i <= hostnum ; i++){
		sprintf(FIFO[i] , "./Host%d.FIFO" , i);
	}
	if(mkfifo("./Host.FIFO" , 0777) < 0){
		sprintf(buf , "mkfifo error!\n");
		write(2 , buf , strlen(buf));
	}
	for(int i = 1 ; i <= hostnum ; i++){
		if(mkfifo(FIFO[i] , 0777) < 0){
			sprintf(buf , "mkfifo error!\n");
			write(2 , buf , strlen(buf));
		}
	}
	for(int i = 1 ; i <= hostnum ; i++){
		if(fork() == 0){
			char argvlist[3][5];
			sprintf(argvlist[0] , "%d" , i);
			sprintf(argvlist[1] , "%d" , i);
			sprintf(argvlist[2] , "0");
			execl("./host" , "./host" , argvlist[0] , argvlist[1] , argvlist[2] , NULL);
		}
	}
	sleep(1);
	fp = fopen("Host.FIFO" , "r+");
	for(int i = 1 ; i <= hostnum ; i++){
		file_fd[i] = open(FIFO[i] , O_RDWR);
	}
	int count = 0;
	int distribute_count = 0;
	char ending_signal[50] = "-1 -1 -1 -1 -1 -1 -1 -1\n";
	for(int i = 1 ; i <= competitionnum && i <= hostnum ; i++){
		sprintf(buf , "%d %d %d %d %d %d %d %d\n" , competition[distribute_count][0] , competition[distribute_count][1]
				 , competition[distribute_count][2] , competition[distribute_count][3] , competition[distribute_count][4] , 
				 		competition[distribute_count][5] , competition[distribute_count][6] , competition[distribute_count][7]);
		write(file_fd[i] , buf , strlen(buf));
		distribute_count++;
	}
	int randomkey;
	int result[9][2];
	while(fscanf(fp , "%d" , &randomkey) > 0){
		for(int i = 1 ; i <= 8 ; i++){
			fscanf(fp , "%d" , &result[i][0]);
			fscanf(fp , "%d" , &result[i][1]);
		}
		score(result , point);
		count++;
		if(count == competitionnum){
			break;
		}
		if(distribute_count < competitionnum){
			sprintf(buf , "%d %d %d %d %d %d %d %d\n" , competition[distribute_count][0] , competition[distribute_count][1]
				 , competition[distribute_count][2] , competition[distribute_count][3] , competition[distribute_count][4] , 
				 		competition[distribute_count][5] , competition[distribute_count][6] , competition[distribute_count][7]);
			write(file_fd[randomkey] , buf , strlen(buf));
			distribute_count++;
		}
	}
	for(int i = 1 ; i <= hostnum ; i++){
		write(file_fd[i] , ending_signal , strlen(ending_signal));
	}
	wait(0);
	fclose(fp);
	unlink("Host.FIFO");
	for(int i = 1 ; i <= hostnum ; i++){
		close(file_fd[i]);
		unlink(FIFO[i]);
	}
	ranking(point , rank , playernum);
	int finalrank[15];
	for(int i = 1 ; i <= playernum ; i++){
		if(point[rank[i]] == point[rank[i - 1]]){
			finalrank[rank[i]] = finalrank[rank[i - 1]];
		}
		else{
			finalrank[rank[i]] = i;
		}
	}
	for(int i = 1 ; i <= playernum ; i++){
		fprintf(stdout , "%d %d\n", i , finalrank[i]);
	}
	return 0;
}