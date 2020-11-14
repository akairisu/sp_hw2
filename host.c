#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

void swap(int rank[] , int index){
	int temp;
	temp = rank[index];
	rank[index] = rank[index + 1];
	rank[index + 1] = temp;
	return;
}

void ranking(int id[] , int wins[] , int rank[] , int finalrank[][2]){
	for(int i = 0 ; i <= 7 ; i++){
		rank[i] = id[i];
	}
	for(int i = 1 ; i < 8 ; i++){
		for(int j = 0 ; j <= 7 - i ; j++){
			if(wins[rank[j]] < wins[rank[j + 1]]){
				swap(rank , j);
			}
		}
	}
	for(int i = 0 ; i < 8 ; i++){
		for(int j = 0 ; j < 8 ; j++){
			if(rank[j] == id[i]){
				finalrank[i][0] = id[i];
				finalrank[i][1] = j + 1;
				int compare = j - 1;
				while((j != 0) && (compare >= 0) && (wins[id[i]] == wins[rank[compare]])){
					finalrank[i][1] = compare + 1;
					compare--;
				}
				break;
			}
		}
	}
	return;
}

void initial(int rank[] , int wins[] , int finalrank[][2]){
	for(int i = 0 ; i < 8 ; i++){
		rank[i] = 0;
	}
	for(int i = 0 ; i < 15 ; i++){
		wins[i] = 0;
	}
	for(int i = 0 ; i < 8 ; i++){
		finalrank[i][0] = 0;
		finalrank[i][1] = 0;
	}
}

int main(int argc, char *argv[]){
	int fd = open("Host.FIFO" , O_RDWR);
	char FIFO[20][20];
	for(int i = 1 ; i <= 10 ; i++){
		sprintf(FIFO[i] , "./Host%d.FIFO" , i);
	}
	if(argc != 4){
		fprintf(stderr , "Error!\n");
	}
	int id[8];
	char buf[512];
	int rank[8] = {0};
	int wins[15] = {0};
	int finalrank[8][2] = {{0}};
	int randomkey = atoi(argv[2]);
	if(atoi(argv[3]) == 0){
		int hostid = atoi(argv[1]);
		int writetochild1[2];
		int writetoparent1[2];
		if(pipe(writetochild1) < 0){
			fprintf(stderr , "pipe error\n");
		}
		if(pipe(writetoparent1) < 0){
			fprintf(stderr , "pipe error\n");
		}
		int pid;
		if(fork()  == 0){
			char argvlist[3][5];
			sprintf(argvlist[0] , "%d" , hostid);
			sprintf(argvlist[1] , "%d" , randomkey);
			sprintf(argvlist[2] , "1");
			dup2(writetochild1[0] , 0);
			dup2(writetoparent1[1] , 1);
			close(writetochild1[1]);
			close(writetoparent1[0]);
			execl("./host" , "./host" , argvlist[0] , argvlist[1] , argvlist[2] , NULL);
		}
		else{
			close(writetochild1[0]);
			close(writetoparent1[1]);
		}
		int writetochild2[2];
		int writetoparent2[2];
		if(pipe(writetochild2) < 0){
			fprintf(stderr , "pipe error\n");
		}
		if(pipe(writetoparent2) < 0){
			fprintf(stderr , "pipe error\n");
		}
		if(fork()  == 0){
			char argvlist[3][5];
			sprintf(argvlist[0] , "%d" , hostid);
			sprintf(argvlist[1] , "%d" , randomkey);
			sprintf(argvlist[2] , "1");
			dup2(writetochild2[0] , 0);
			dup2(writetoparent2[1] , 1);
			close(writetochild2[1]);
			close(writetoparent2[0]);
			execl("./host" , "./host" , argvlist[0] , argvlist[1] , argvlist[2] , NULL);
		}
		else{
			close(writetochild2[0]);
			close(writetoparent2[1]);
		}
		FILE *fp = fopen(FIFO[hostid] , "r+");
		while(fscanf(fp , "%d%d%d%d%d%d%d%d" , &id[0] , &id[1] , &id[2] , &id[3] , &id[4] , &id[5] , &id[6] , &id[7]) > 0){
			initial(rank , wins , finalrank);
			sprintf(buf , "%d %d %d %d\n" , id[0] , id[1] , id[2] , id[3]);
			write(writetochild1[1] , buf , strlen(buf));
			sprintf(buf , "%d %d %d %d\n" , id[4] , id[5] , id[6] , id[7]);
			write(writetochild2[1] , buf , strlen(buf));
			if(id[0] == -1){
				for(int i = 0 ; i < 2 ; i++){
					wait(0);
				}
				close(fd);
				exit(0);
			}
			int player[2];
			int playermoney[2];
			for(int i = 1 ; i <= 10 ; i++){
				if(read(writetoparent1[0] , buf , sizeof(buf)) > 0){
					char *start = strtok(buf , " ");
					player[0] = atoi(start);
					start = strtok(NULL , "\n");
					playermoney[0] = atoi(start);
				}
				if(read(writetoparent2[0] , buf , sizeof(buf)) > 0){
					char *start = strtok(buf , " ");
					player[1] = atoi(start);
					start = strtok(NULL , "\n");
					playermoney[1] = atoi(start);
				}
				//fprintf(stderr , "%d %d %d %d\n" , player[0] , playermoney[0] , player[1] , playermoney[1]);
				int winnerid = (playermoney[0] > playermoney[1]) ? player[0] : player[1];
				wins[winnerid]++;
				if(i != 10){
					sprintf(buf , "%d\n" , winnerid);
					write(writetochild1[1] , buf , strlen(buf));
					sprintf(buf , "%d\n" , winnerid);
					write(writetochild2[1] , buf , strlen(buf));
				}
				else{
					//fprintf(stderr , "pass!!\n");
					ranking(id , wins , rank , finalrank);
					char buf2[10];
					sprintf(buf , "%d\n" , randomkey);
					for(int j = 0 ; j < 8 ; j++){
						sprintf(buf2 , "%d %d\n" , finalrank[j][0] , finalrank[j][1]);
						strcat(buf , buf2);
					}
					write(fd , buf , strlen(buf));
				}
			}
			//fprintf(stderr , "pass!!!\n");
		}
	}
	else if(atoi(argv[3]) == 1){
		int parentid = atoi(argv[1]);
		int writetoleaf1[2];
		int writetochildren1[2];
		if(pipe(writetoleaf1) < 0){
			fprintf(stderr , "pipe error\n");
		}
		if(pipe(writetochildren1) < 0){
			fprintf(stderr , "pipe error\n");
		}
		int cid;
		if(fork()  == 0){
			char argvlist[3][5];
			sprintf(argvlist[0] , "%d" , parentid);
			sprintf(argvlist[1] , "%d" , randomkey);
			sprintf(argvlist[2] , "2");
			dup2(writetoleaf1[0] , 0);
			dup2(writetochildren1[1] , 1);
			close(writetoleaf1[1]);
			close(writetochildren1[0]);
			execl("./host" , "./host" , argvlist[0] , argvlist[1] , argvlist[2] , NULL);
		}
		else{
			close(writetoleaf1[0]);
			close(writetochildren1[1]);
		}
		int writetoleaf2[2];
		int writetochildren2[2];
		if(pipe(writetoleaf2) < 0){
			fprintf(stderr , "pipe error\n");
		}
		if(pipe(writetochildren2) < 0){
			fprintf(stderr , "pipe error\n");
		}
		if(fork()  == 0){
			char argvlist[3][5];
			sprintf(argvlist[0] , "%d" , parentid);
			sprintf(argvlist[1] , "%d" , randomkey);
			sprintf(argvlist[2] , "2");
			dup2(writetoleaf2[0] , 0);
			dup2(writetochildren2[1] , 1);
			close(writetoleaf2[1]);
			close(writetochildren2[0]);
			execl("./host" , "./host" , argvlist[0] , argvlist[1] , argvlist[2] , NULL);
		}
		else{
			close(writetoleaf2[0]);
			close(writetochildren2[1]);
		}
		int childid[4];
		char childbuf[512];
		while(scanf("%d%d%d%d" , &childid[0] , &childid[1] , &childid[2] , &childid[3]) > 0){
			//fprintf(stderr , "%d %d %d %d\n" , childid[0] , childid[1] , childid[2] , childid[3]);
			sprintf(childbuf , "%d %d\n" , childid[0] , childid[1]);
			write(writetoleaf1[1] , childbuf , strlen(childbuf));
			sprintf(childbuf , "%d %d\n" , childid[2] , childid[3]);
			write(writetoleaf2[1] , childbuf , strlen(childbuf));
			if(childid[0] == -1){
				for(int i = 0 ; i < 2 ; i++){
					wait(0);
				}
				exit(0);
			}
			int childplayer[2];
			int childplayermoney[2];
			for(int i = 1 ; i <= 10 ; i++){
				if(read(writetochildren1[0] , childbuf , sizeof(int) * 2) > 0){
					char *start = strtok(childbuf , " ");
					childplayer[0] = atoi(start);
					start = strtok(NULL , "\n");
					childplayermoney[0] = atoi(start);
				}
				if(read(writetochildren2[0] , childbuf , sizeof(int) * 2) > 0){
					char *start = strtok(childbuf , " ");
					childplayer[1] = atoi(start);
					start = strtok(NULL , "\n");
					childplayermoney[1] = atoi(start);
				}
				int childwinnerid = (childplayermoney[0] > childplayermoney[1]) ? childplayer[0] : childplayer[1];
				int childwinnermoney = (childplayermoney[0] > childplayermoney[1]) ? childplayermoney[0] : childplayermoney[1];
				printf("%d %d\n" , childwinnerid , childwinnermoney);
				//fprintf(stderr , "child %d %d\n" , childwinnerid , childwinnermoney);
				fflush(stdout);
				int returnwinnerid;
				if(i != 10){
					if(scanf("%d" , &returnwinnerid) > 0){
						sprintf(childbuf , "%d\n" , returnwinnerid);
						write(writetoleaf1[1] , childbuf , strlen(childbuf));
						write(writetoleaf2[1] , childbuf , strlen(childbuf));
					}
				}
			}
		}
	}
	else if(atoi(argv[3]) == 2){
		int leafid[2];
		int writetoplayer1[2];
		int writetoleafhost1[2];
		while(scanf("%d %d" , &leafid[0] , &leafid[1]) > 0){
			//fprintf(stderr , "%d %d\n" , leafid[0] , leafid[1]);
			if(leafid[0] == -1){
				exit(0);
			}
			if(pipe(writetoplayer1) < 0){
				fprintf(stderr , "pipe error\n");
			}
			if(pipe(writetoleafhost1) < 0){
				fprintf(stderr , "pipe error\n");
			}
			int writetoplayer2[2];
			int writetoleafhost2[2];
			if(pipe(writetoplayer2) < 0){
				fprintf(stderr , "pipe error\n");
			}
			if(pipe(writetoleafhost2) < 0){
				fprintf(stderr , "pipe error\n");
			}
			int lid;
			if(fork() == 0){
				char playerid[10];
				sprintf(playerid , "%d" , leafid[0]);
				dup2(writetoplayer1[0] , 0);
				dup2(writetoleafhost1[1] , 1);
				close(writetoplayer1[1]);
				close(writetoleafhost1[0]);
				execl("./player" , "./player" , playerid, NULL);
			}
			else{
				close(writetoplayer1[0]);
				close(writetoleafhost1[1]);
			}
			if(fork() == 0){
				char playerid[10];
				sprintf(playerid , "%d" , leafid[1]);
				dup2(writetoplayer2[0] , 0);
				dup2(writetoleafhost2[1] , 1);
				close(writetoplayer2[1]);
				close(writetoleafhost2[0]);
				execl("./player" , "./player" , playerid, NULL);
			}
			else{
				close(writetoplayer2[0]);
				close(writetoleafhost2[1]);
			}
			char leafbuf[512];
			int leafplayer[2];
			int leafplayermoney[2];
			for(int i = 1 ; i <= 10 ; i++){
				if(read(writetoleafhost1[0] , leafbuf , sizeof(int) * 2) > 0){
					char *start = strtok(leafbuf , " ");
					leafplayer[0] = atoi(start);
					start = strtok(NULL , "\n");
					leafplayermoney[0] = atoi(start);		
				}
				if(read(writetoleafhost2[0] , leafbuf, sizeof(int) * 2) > 0){
					char *start = strtok(leafbuf , " ");
					leafplayer[1] = atoi(start);
					start = strtok(NULL , "\n");
					leafplayermoney[1] = atoi(start);
				}
				int leafwinnerid = (leafplayermoney[0] > leafplayermoney[1]) ? leafplayer[0] : leafplayer[1];
				int leafwinnermoney =  (leafplayermoney[0] > leafplayermoney[1]) ? leafplayermoney[0] : leafplayermoney[1];
				printf("%d %d\n" , leafwinnerid , leafwinnermoney);
				//fprintf(stderr , "leaf %d %d\n" , leafwinnerid , leafwinnermoney);
				fflush(stdout);
				if(i != 10){
					int returnleafwinnerid;
					if(scanf("%d" , &returnleafwinnerid) > 0){
						sprintf(leafbuf , "%d" , returnleafwinnerid);
						write(writetoplayer1[1] , leafbuf , strlen(leafbuf));
						write(writetoplayer2[1] , leafbuf , strlen(leafbuf));
					}
				}
				else{
					for(int i = 0 ; i < 2 ; i++){
						wait(0);
						for(int i = 0 ; i < 2 ; i++){
							close(writetoplayer1[i]);
							close(writetoplayer2[i]);
							close(writetoleafhost1[i]);
							close(writetoleafhost2[i]);
						}
					}
				}
			}
		}
	}
	return 0;
}