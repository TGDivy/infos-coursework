/*
 * FIFO Scheduling Algorithm
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
 * A FIFO scheduling algorithm
 */
class FIFOScheduler : public SchedulingAlgorithm
{
public:
	/**
	 * Returns the friendly name of the algorithm, for debugging and selection purposes.
	 */
	const char* name() const override { return "fifo"; }

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
	 * e.g. its timeslice has not expired, or the algorithm determines it's not time to change.
	 */
	SchedulingEntity *pick_next_entity() override
	{
		// syslog.messagef(LogLevel::DEBUG, "Number of processes in que %d", runqueue.count());
		// If there is no process running there is nothing to return
		if (runqueue.count() == 0)
			return NULL; 
		// otherwise just return the first task to be run.
		else {
			syslog.messagef(LogLevel::DEBUG, "Number of processes in que %d", runqueue.count());
			return runqueue.first();
		}
			
		// Always return the first task in the que, until it has been completed, or cannot be run.
		// This would imply that it's always the same task that is returned, unless it's no longer in the que, 
		// i.e. removed by the remove from runque function.

		// This makes it always wait for a given task to complete.
		// When a task runs infinetly, it has no choice but to be stuck there.
		// hence, for the sched 2 test, this is what happens causing it to always be running the ticker 1.
		// Similarly, as it is always focused on this process it can no longer register for the "\n" or enter key
		// as that would require engaging with a differet process, and the variable terminate is always false.

		// The reason round robin can terminate is it cycles between all the runable/ running tasks, eventually 
		// registering the "\n".
		
	}

private:
	// A list containing the current runqueue.
	List<SchedulingEntity *> runqueue;
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

RegisterScheduler(FIFOScheduler);
