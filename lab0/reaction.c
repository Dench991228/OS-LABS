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

	// the condition where hydrogen atoms are waiting for reaction
	struct condition *hydrogen_cond;

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
	// initialize the condition of hydrogen
	reaction->hydrogen_cond = (struct condition*)malloc(sizeof(struct condition));
	cond_init(reaction->hydrogen_cond);
	// initialize the condition of oxygen
	reaction->oxygen_cond = (struct condition*)malloc(sizeof(struct condition));
	cond_init(reaction->oxygen_cond);
}

// add a hydrogen atom into the container
// added when no reaction happens
void reaction_h(struct reaction *reaction)
{
	lock_acquire(reaction->reaction_lock);
	if(reaction->hydrogen_number >= 1 && reaction->oxygen_number >= 1){
		// consume a hydrogen atom and an oxygen atom
		cond_signal(reaction->hydrogen_cond, reaction->reaction_lock);
		reaction->hydrogen_number--;
		cond_signal(reaction->oxygen_cond, reaction->reaction_lock);
		reaction->oxygen_number--;

		make_water();
	}
	else{// no reaction
		reaction->hydrogen_number++;
		cond_wait(reaction->hydrogen_cond, reaction->reaction_lock);
	}
	lock_release(reaction->reaction_lock);
}

void reaction_o(struct reaction *reaction)
{
	// reaction takes place
	lock_acquire(reaction->reaction_lock);
	if(reaction->hydrogen_number>=2){
		//consume two hydrogen atoms
		cond_signal(reaction->hydrogen_cond, reaction->reaction_lock);
		cond_signal(reaction->hydrogen_cond, reaction->reaction_lock);
		reaction->hydrogen_number-=2;
		make_water();
	}
	else{// no reaction
		reaction->oxygen_number++;
		cond_wait(reaction->oxygen_cond, reaction->reaction_lock);
	}
	lock_release(reaction->reaction_lock);
}
