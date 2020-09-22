#include "pintos_thread.h"

struct station {
	// number of people waiting on the platform
	int people_wait;
	//the number of people awake
	int people_awake;
	// the number of people aboard in this train
	int people_aboard;
	// whether the train is ready to go
	int train_ready;
	// the lock and condition of the train's movement
	struct lock *lock_train;
	struct condition *cond_train;
	// the lock and condition that controls the passenger's movement
	struct lock *lock_passenger;
	struct condition *cond_passenger;
};

// create
void station_init(struct station *station)
{
	// initialize the number of people waiting on the platform
	station->people_wait = 0;
	//initialize the lock of passenger
	station->lock_passenger = (struct lock*)malloc(sizeof(struct lock));
	lock_init(station->lock_passenger);
	// initialize the condition of passenger
	station->cond_passenger = (struct condition*)malloc(sizeof(struct condition));
	cond_init(station->cond_passenger);
	// initialize the lock of train
	station->lock_train = (struct lock*)malloc(sizeof(struct lock));
	lock_init(station->lock_train);
	// initialize the condition of train
	station->cond_train = (struct condition*)malloc(sizeof(struct condition));
	cond_init(station->cond_train);
}

// wake up certain amount of passengers waiting on station_wait_for_train
void station_load_train(struct station *station, int count)
{
	if(count==0)return;
	if(station->people_wait==0)return;
	// train cannot leave the station when loading passenger
	lock_acquire(station->lock_train);
	int i = 0;
	station->people_awake = 0;
	station->people_aboard = 0;
	station->train_ready = 0;
	while(i<count&&station->people_wait>0){
		lock_acquire(station->lock_passenger);
		cond_signal(station->cond_passenger, station->lock_passenger);
		station->people_wait--;
		station->people_awake++;
		i++;
		if(!(i<count&&station->people_wait>0))station->train_ready=1;
		//printf("ready!\n");
		lock_release(station->lock_passenger);
		//printf("waking number %d passenger\n", i);
	}
	// wait for all people aboard
	cond_wait(station->cond_train, station->lock_train);
	lock_release(station->lock_train);
}

// a passenger waits on the condition of moving
void station_wait_for_train(struct station *station)
{
	lock_acquire(station->lock_passenger);
	station->people_wait++;
	cond_wait(station->cond_passenger, station->lock_passenger);
	lock_release(station->lock_passenger);
}

void station_on_board(struct station *station)
{
	lock_acquire(station->lock_passenger);
	// really send one passenger on board
	station->people_aboard++;
	//printf("passenger No.%d get on board\n", station->people_aboard);
	lock_release(station->lock_passenger);
	if(station->people_aboard==station->people_awake&&station->train_ready==1){
		//printf("ready to go!\n");
		lock_acquire(station->lock_train);
		cond_signal(station->cond_train, station->lock_train);
		lock_release(station->lock_train);
	}
}
