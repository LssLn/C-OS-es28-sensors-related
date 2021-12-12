/*
Scrivere un programma in C che permetta la raccolta dati da una serie di sensori (3 sensori)

Ogni sensore è rappresentato da un processo/thread.
-Il primo sensore, periodicamente e ogni secondo, monitora la temperatura di una stanza.
-Il secondo sensore, periodicamente e ogni secondo, monitora l'umidità di una stanza.
-Il terzo sensore, periodicamente e ogni secondo, il consumo energetico.

tutti i sensori inviano i propri valori al main thread/processo padre che si occuperà di stamparli a video i valori dei tre sensori ogni 2 secondi

Dopo 20 secondi di esecuzione tutti i processi/thread sensori imposteranno il valore del sensore a -1 per comunicare al main thread7processo padre che hanno terminato.

Il processo padre/main thread, quando leggerà che il valore di tutti e tre i sensori è uguale a -1 termina.

Utilizzare thread/mutex oppure processi/shared memory/semafori per l'implementazione del programma. 
*/

//REALIZZAZIONE CON SEMAFORI E PROCESSI



#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>

#define TIPO_TEMPERATURA 0
#define TIPO_UMIDITA 1
#define TIPO_ENERGIA 2

int run = 1;


struct dati {
	int temperatura;
	int umidita;
	int energia;
};

union semun {
    int val;
    struct semid_ds *buf;   
    unsigned short  *array;  
    struct seminfo  *__buf;  
};


int semset(int semid, int val);
int semdown(int semid);
int semup(int semid);

void alarm_handler(int sig); 
void sensore (int tipo);


int semset(int semid, int val){
	union semun un;
	un.val = val;
	return semctl(semid, 0, SETVAL, un);

}
int semdown(int semid){
	struct sembuf buf;
	buf.sem_num = 0;
	buf.sem_flg = SEM_UNDO;
	buf.sem_op = -1;
	return semop(semid, &buf, 1);
}
int semup(int semid){
	struct sembuf buf;
	buf.sem_num = 0;
	buf.sem_flg = SEM_UNDO;
	buf.sem_op = 1;
	return semop(semid, &buf, 1);
}


int main() {
	pid_t pid;
	int i;
	struct dati *sensori_ptr; //punterà alla shared memory
	int semid, shmid;
	
	semid = semget((key_t)7777, 1, 0666 | IPC_CREAT);
	shmid = shmget((key_t)7778, sizeof(struct dati), 0666 | IPC_CREAT);

	semset(semid, 1);

	sensori_ptr = (struct dati *)shmat(shmid, NULL, 0);	
	
	for(i=0; i<3; i++){
		pid=fork();
		if(pid==0){
			sensore(i);
			exit(0);		
		}
	}

	while(1){
		semdown(semid);
		if(sensori_ptr->temperatura == -1 && sensori_ptr->umidita == -1 && sensori_ptr->energia == -1) {
			break;
		}
		printf("Temp: %d\nUmid: %d\nEnergia: %d\n\n", sensori_ptr->temperatura, sensori_ptr->umidita, sensori_ptr->energia);
		semup(semid);
		sleep(2);
	}
	shmdt(sensori_ptr);
	shmctl(shmid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID, NULL);
}


void sensore (int tipo){
	struct dati *sensori_ptr; //punterà alla shared memory
	int semid, shmid;
	time_t s_time;	
	
	semid = semget((key_t)7777, 1, 0666 | IPC_CREAT);
	shmid = shmget((key_t)7778, sizeof(struct dati), 0666 | IPC_CREAT);

	sensori_ptr = (struct dati *)shmat(shmid, NULL, 0);	

	signal(SIGALRM, alarm_handler);

	srand((unsigned int)getpid());
	
	alarm(20);
	while(run){
		semdown(semid);
		switch(tipo) {
			case TIPO_TEMPERATURA:
				sensori_ptr->temperatura = (rand()%30)+10;
				break;
			case TIPO_UMIDITA:
				sensori_ptr->umidita = (rand()%100)+1;
				break;
			case TIPO_ENERGIA:
				sensori_ptr->energia = (rand()%3000)+1;
				break;
			default:
				break;
		}
		semup(semid);
		s_time=time(NULL);
		while((time(NULL) - s_time) < 1);
	}
	
		semdown(semid);
		switch(tipo) {
			case TIPO_TEMPERATURA:
				sensori_ptr->temperatura = -1;
				break;
			case TIPO_UMIDITA:
				sensori_ptr->umidita = -1;
				break;
			case TIPO_ENERGIA:
				sensori_ptr->energia = -1;
				break;
			default:
				break;
		}
		semup(semid);
		printf("FIglio %d - Termino\n", tipo);
		shmdt(sensori_ptr);
	
}

void alarm_handler(int sig){
	run=0;
}
