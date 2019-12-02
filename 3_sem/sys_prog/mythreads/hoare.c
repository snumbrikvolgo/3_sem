#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

struct monitor {

	int total;
	int occupied;
	int searching;
	int cur_number;


	pthread_mutex_t lock;
	pthread_cond_t is_smb_inside;

};
void* EnterParking(void* arg);
void InitMon(struct monitor* data, int cars);

void* EnterParking(void* arg)
{
	struct monitor* data = (struct monitor*) arg;
	pthread_mutex_lock(&data->lock);

	printf("Car: wants to find a place\n");
	while(data -> searching){
		pthread_cond_wait(&data -> is_smb_inside, &data -> lock);
	}
	data -> searching = 1;
	if (data -> occupied < data -> total){
		printf("Car: found a place!\n");

		data -> searching = 0;
		(data -> occupied++);
		printf("Current sutiation: %d/%d\n", data ->occupied, data ->total);
	}
	else {
		printf("Car: no place for me :(\n");
		printf("Current sutiation: %d/%d\n", data ->occupied, data ->total);
		data -> searching = 0;
	}

	pthread_cond_signal(&data -> is_smb_inside);
	pthread_mutex_unlock(&data->lock);

}

int main(int argc, char** argv)
{
	int TotalPlace = atoi(argv[1]);
	int Cars = atoi(argv[2]);

	struct monitor data;
	InitMon(&data, TotalPlace);

	pthread_t cars[Cars];

	for (int i = 0; i < Cars; i++)
		if (pthread_create(&cars[i], NULL, EnterParking, &data) < 0){
			perror("Error in creating threads\n");
			return errno;
		}

	for (int i = 0; i < Cars; i++)
		pthread_join(cars[i], NULL);

	pthread_mutex_destroy(&data.lock);
	return 0;
}

void InitMon(struct monitor* data, int cars)
{
	assert(data);
	if(pthread_mutex_init(&data->lock, NULL) != 0){
        perror("Error in mutex initialization\n");
        return errno;
    }
	pthread_cond_init(&(data->is_smb_inside), NULL);

	data -> total = cars;
	data -> searching = 0;
	data -> occupied = 0;
	data -> cur_number = 0;
}
