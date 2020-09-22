#include "pintos_thread.h"

// Forward declaration. This function is implemented in reaction-runner.c,
// but you needn't care what it does. Just be sure it's called when
// appropriate within reaction_o()/reaction_h().
void make_water();

struct reaction {
	// the number of hydrogen atom in the container
	int hydrogen_number;

	// the number of oxygen atom in the container
	int oxygen_number;

	// the lock of the reaction
	struct lock *reaction_lock;

	// the lock of modifying the number of hydrogen atom
	struct lock *hydrogen_lock;

	// the condition where hydrogen atoms are waiting for reaction
	struct condition *hydrogen_cond;

	// the lock of modifying the number oxygen
	struct lock *oxygen_lock;

	// the condition where oxygen atoms are waiting for reaction
	struct condition *oxygen_cond;
};

// initialize the container of reaction
void reaction_init(struct reaction *reaction)
{
	// initialize the number of hydrogen atoms in the container
	reaction->hydrogen_number = 0;
	// initialize the number of oxygen atoms in the container
	reaction->oxygen_number = 0;
	// initialize the lock of reaction
	reaction->reaction_lock = (struct lock*)malloc(sizeof(struct lock));
	lock_init(reaction->reaction_lock);
	// initialize the lock and condition of hydrogen
	reaction->hydrogen_lock = (struct lock*)malloc(sizeof(struct lock));
	lock_init(reaction->hydrogen_lock);
	reaction->hydrogen_cond = (struct condition*)malloc(sizeof(struct condition));
	cond_init(reaction->hydrogen_cond);
	// initialize the lock and condition of oxygen
	reaction->oxygen_lock = (struct lock*)malloc(sizeof(struct lock));
	lock_init(reaction->oxygen_lock);
	reaction->oxygen_cond = (struct condition*)malloc(sizeof(struct condition));
	cond_init(reaction->oxygen_cond);
}

// add a hydrogen atom into the container
// added when no reaction happens
void reaction_h(struct reaction *reaction)
{
	lock_acquire(reaction->reaction_lock);
	// reaction can takes places
	if(reaction->hydrogen_number >= 1 && reaction->oxygen_number >= 1){
		// consume a hydrogen atom
		lock_acquire(reaction->hydrogen_lock);
		cond_signal(reaction->hydrogen_cond, reaction->hydrogen_lock);
		reaction->hydrogen_number--;
		lock_release(reaction->hydrogen_lock);

		// consume an oxygen atom
		lock_acquire(reaction->oxygen_lock);
		cond_signal(reaction->oxygen_cond, reaction->oxygen_lock);
		reaction->oxygen_number--;
		lock_release(reaction->oxygen_lock);

		// forms a water molecule
		make_water();

		lock_release(reaction->reaction_lock);
	}
	else{// no reaction
		lock_release(reaction->reaction_lock);
		lock_acquire(reaction->hydrogen_lock);
		reaction->hydrogen_number++;
		cond_wait(reaction->hydrogen_cond, reaction->hydrogen_lock);
		lock_release(reaction->hydrogen_lock);
	}
}

void reaction_o(struct reaction *reaction)
{
	// reaction takes place
	lock_acquire(reaction->reaction_lock);
	if(reaction->hydrogen_number>=2){
		// consume two hydrogen atoms
		lock_acquire(reaction->hydrogen_lock);
		cond_signal(reaction->hydrogen_cond, reaction->hydrogen_lock);
		cond_signal(reaction->hydrogen_cond, reaction->hydrogen_lock);
		reaction->hydrogen_number-=2;
		lock_release(reaction->hydrogen_lock);

		// forms a water molecule
		make_water();

		lock_release(reaction->reaction_lock);
	}
	else{// no reaction
		lock_release(reaction->reaction_lock);
		lock_acquire(reaction->oxygen_lock);
		reaction->oxygen_number++;
		cond_wait(reaction->oxygen_cond, reaction->oxygen_lock);
		lock_release(reaction->oxygen_lock);
	}
}
