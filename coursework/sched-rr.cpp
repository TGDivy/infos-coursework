/*
 * Round-robin Scheduling Algorithm
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (2)
 */

/*
 * STUDENT NUMBER: s1885517
 */
#include <infos/kernel/sched.h>
#include <infos/kernel/thread.h>
#include <infos/kernel/log.h>
#include <infos/util/list.h>
#include <infos/util/lock.h>

using namespace infos::kernel;
using namespace infos::util;

/**
 * A round-robin scheduling algorithm
 */
class RoundRobinScheduler : public SchedulingAlgorithm
{
public:
	/**
	 * Returns the friendly name of the algorithm, for debugging and selection purposes.
	 */
	const char* name() const override { return "rr"; }

	/**
	 * Called when a scheduling entity becomes eligible for running.
	 * @param entity
	 */
	void add_to_runqueue(SchedulingEntity& entity) override
	{
		UniqueIRQLock l;
		runqueue.enqueue(&entity);
	}

	/**
	 * Called when a scheduling entity is no longer eligible for running.
	 * @param entity
	 */
	void remove_from_runqueue(SchedulingEntity& entity) override
	{
		UniqueIRQLock l;
		runqueue.remove(&entity);
	}

	/**
	 * Called every time a scheduling event occurs, to cause the next eligible entity
	 * to be chosen.  The next eligible entity might actually be the same entity, if
	 * e.g. its timeslice has not expired.
	 */
	SchedulingEntity *pick_next_entity() override
	{
		UniqueIRQLock l; // We will maniuplate the run que list.
		// if List is empty, nothing to return.
		if (runqueue.count() <= 0) 
			return NULL;
		else {
			// Uncomment the below comment to observer how the processess build up and are tackled.
			// syslog.messagef(LogLevel::DEBUG, "Number of processes in que %d", runqueue.count());
			const auto& entity = runqueue.pop(); // Get the first process.
			// syslog.messagef(LogLevel::DEBUG, "Execution time %d", entity->cpu_runtime());
			runqueue.append(entity); // Put this process at the back of the runque so others can be run as well. 
			return entity; // Return the first process.
		}

		// Round robin algorithm esssentially cycles between all the process in the run que, starting them, 
		// running them for a set quantum time, then putting them at the end of the que and running other processes.
		
		// ********************************
		// Bonus question (continued after fifo.) 
		// The reason round robin can terminate, is it cycles between all the runable/ running tasks.
		// As the enter key, or registering "\n" counts as a third process, the number of process in the que jumps to 3,
		// As it is quickly cycling through all the process, it encounters the third process, namely,
		// the enter key, which sets terminate to true. 
		// Hence, after it finishes the respective sleep cycles of 1.5, 1 second of each process it terminates.
	}

private:
	// A list containing the current runqueue.
	List<SchedulingEntity *> runqueue;
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

RegisterScheduler(RoundRobinScheduler);
