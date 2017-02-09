/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"

// Keeps a running total for each of the times and divides by num_jobs in the calculations at the end;
int num_jobs = 0;
float total_waiting_time = 0;
float total_turnaround_time = 0;
float total_response_time = 0;

/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements.
*/
// TODO: some elements may need to be added we'll see
// running_time = total amount of time that the job needs to run; remaining_time = time left needed to run
typedef struct _job_t
{
	int job_id, arrival_time, start_executing, stop_executing, running_time, remaining_time, priority, core_id, first_time_scheduled;
} job_t;

// Function pointer type for the m_comparerFunc; didn't need to define a type but hell y not
typedef int (*comparerPointer) (const void*, const void*);

// TODO: Not sure if you want this to be a struct or not?
// Struct
// typedef struct _scheduler_t
// {
// 	priqueue_t m_queue;
// 	int m_coreNum, m_scheme;
//  comparerPointer m_comparerFunc;
//  int* m_cores;
// } scheduler_t;

// Or not
priqueue_t m_queue;
int m_coreNum, m_scheme;
comparerPointer m_comparerFunc;
job_t** m_cores;

// Place for all comparison functions used by the priority queuetest
// Compare the arrival_time values of the jobs; Sort in ascending order
int comparerFCFS (const void* a, const void* b)
{
	job_t* temp_a = *((job_t**)a);
	job_t* temp_b = *((job_t**)b);
	return (temp_a->arrival_time - temp_b->arrival_time);
}

// Compare the remaining_time values of the jobs; Sort in ascending order
// For preemptive: we will just call priqueue_resort on the queue whenever a new job is added
// For preemptive and non-preemptive: removing a job when it is finished will resort the array
int comparerSJF (const void* a, const void* b)
{
	// TODO: If two jobs have the same remaining_time values then how to sort?
	// Currently I am sorting by their arrival_time on conflict...thoughts?
	// UPDATE: It actually does say to do it this way in the instructions
	job_t* temp_a = *((job_t**)a);
	job_t* temp_b = *((job_t**)b);
	int comp = temp_a->remaining_time - temp_b->remaining_time;
	return (comp == 0 ? temp_a->arrival_time - temp_b->arrival_time : comp);
}

// Compare the priority values of the jobs; Sort in ascending order
// For preemptive: we will just call priqueue_resort on the queue whenever a new job is added
// For preemptive and non-preemptive: removing a job when it is finished will resort the array
int comparerPRI (const void* a, const void* b)
{
	// If the priorities are the same then sort by arrival_time; this is the procedure defined in the instructions
	job_t* temp_a = *((job_t**)a);
	job_t* temp_b = *((job_t**)b);
	int comp = temp_a->priority - temp_b->priority;
	return (comp == 0 ? temp_a->arrival_time - temp_b->arrival_time : comp);
}

// For round robin we will just need to make sure when a quantum has expired that we move the job to the end of the array
// Since we don't want the round robin function to rearrange the array, but still want to maintain the comparison function Structure, I am simply returning 1
int comparerRR (const void* a, const void* b)
{
	return -1;
}


// Using this to assign the m_comparerFunc pointer based on the scheme provided in the start_up function
void assignFunction (scheme_t scheme)
{
	switch (scheme)
	{
		case FCFS: m_comparerFunc = &comparerFCFS; break;
		case SJF:
		case PSJF: m_comparerFunc = &comparerSJF; break;
		case PRI:
		case PPRI: m_comparerFunc = &comparerPRI; break;
		case RR: m_comparerFunc = &comparerRR; break;
		default: break;
	}
}

// This returns the id of the first available core; -1 if none are available
int coreAvail()
{
	int i;
	for (i = 0; i < m_coreNum; i++)
	{
		if (m_cores[i] == NULL) return i;
	}
	return -1;
}

// Preempt a job and assign the correct core numbers to the jobs and the correct job number to the core
// Req: old_job must be non NULL; new_job may be NULL
// Returns the cores whose value has changed
int preemptJob (job_t* old_job, job_t* new_job, int time)
{
	int i, temp_core;

	// If we are preempting a job that was just set to run for the first time this same cycle, then decrement total_response_time by the number just added and reset the jobs first_time_scheduled value.
	// Fixes case when job is schedule for the first time because another job finished, but then is scheduled over before it can execute.
	if (old_job->first_time_scheduled == time)
	{
		total_response_time -= time - old_job->arrival_time;
		old_job->first_time_scheduled = -1;
	}

	// Preempt the old job
	old_job->remaining_time -= time - old_job->start_executing;
	temp_core = old_job->core_id;
	old_job->core_id = -1;
	old_job->stop_executing = time;

	// Update the total_turnaround_time if the old job just finished
	if (old_job->remaining_time == 0)
		total_turnaround_time += time - old_job->arrival_time;

	// Start the new job
	if (new_job != NULL)
	{
		// Set the core it is on and the time it starts executing
		new_job->core_id = temp_core;
		new_job->start_executing = time;

		// Update the total_waiting_time for the new job;
		total_waiting_time += time - new_job->stop_executing;

		// If the job has not been scheduled before then increment the total_response_time
		if (new_job->first_time_scheduled == -1)
		{
			new_job->first_time_scheduled = time;
			total_response_time += time - new_job->arrival_time;
		}
	}

	// Update the core array
	m_cores[temp_core] = new_job;

	return temp_core;
}

/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
	// Ensure that these are all initialized to zero on scheduler_start_up
	num_jobs = 0;
	total_waiting_time = 0;
	total_turnaround_time = 0;
	total_response_time = 0;

	int i;
	// Set initial values for the scheduler
	m_scheme = scheme;
	m_coreNum = cores;	// This just stores the number of cores that we are using
	m_cores = malloc(cores * sizeof(job_t*));
	assignFunction (scheme);
	priqueue_init(&m_queue, m_comparerFunc);

	// Set all m_core values to -1; -1 = empty, any other number = job_id of the job running on it
	for (i = 0; i < m_coreNum; i++)
	{
		m_cores[i] = NULL;
	}
}


/**
  Called when a new job arrives.

  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made.

 */
 // TODO: still need to uptdate the remaining_time
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
	job_t* temp_job, preempted_job;
	int i, job_index;
	int core_available = coreAvail();

	// Incremment the number of jobs we have by one
	num_jobs++;

	// Update the remaining_time and change the start_executing for each of the jobs being run
	for (i = 0; i < m_coreNum; i++)
	{
		if (m_cores[i] != NULL)
		{
			m_cores[i]->remaining_time -= time - m_cores[i]->start_executing;
			m_cores[i]->start_executing = time;
		}
	}

	// Create job and add it to m_queue
	job_t* temp = malloc(sizeof(job_t));
	temp->job_id = job_number;
	temp->arrival_time = time;
	temp->stop_executing = time;
	temp->start_executing = -1;
	temp->running_time = running_time;
	temp->remaining_time = running_time;
	temp->priority = priority;
	temp->core_id = core_available;
	temp->first_time_scheduled = -1;

	priqueue_offer (&m_queue, temp);

	// If there is an available core then return that core number
	if (core_available != -1)
	{
		// Set the job number for the core to the job just added
		m_cores[core_available] = temp;

		// Mark the job as having been scheduled
		temp->first_time_scheduled = time;

		// Set the core and start_executing time for the job
		temp->core_id = core_available;
		temp->start_executing = time;
		return core_available;
	}

	// Check to see if a job should be preempted; if so call the preemption function
	if (m_scheme == PSJF || m_scheme == PPRI)
	{
		for (i = 0; i < m_coreNum; i++)
		{
			temp_job = (job_t*)priqueue_at(&m_queue, i);

			// If the new job is far up enough in the queue that it should preempt another job
			if (temp_job == temp)
			{
				return preemptJob((job_t*)priqueue_at(&m_queue, m_coreNum), temp, time);
			}
		}
	}

	return -1;
}


/**
  Called when a job has completed execution.

  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.

  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
	int i, j, mark;
	// Pointers to both the old and new jobs
	job_t* next_job = NULL;
	job_t* old_job = m_cores[core_id];

	// Find the nearest available item in the queue not already being executed
	for (i = 0; i < priqueue_size(&m_queue); i++)
	{
		mark = 0;
		for (j = 0; j < m_coreNum; j++)
		{
			if (m_cores[j] == (job_t*)priqueue_at(&m_queue, i))
				mark = 1;
		}

		if (mark == 0)
		{
			next_job = (job_t*)priqueue_at(&m_queue, i);
			break;
		}
	}

	// Preempt the old_job with the new_job
	preemptJob(old_job, next_job, time);

	// Calculate any values that need to be calculated before removing
	priqueue_remove(&m_queue, old_job);

	return (next_job == NULL ? -1 : next_job->job_id);
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.

  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
	int i, j, mark;

	// Find the next job to be scheduled; and the job currently scheduled
	job_t* next_job = NULL;
	job_t* old_job = m_cores[core_id];

	// Move the job to the back of the queuetest
	priqueue_movetoback (&m_queue, old_job);

	// Free the core that was executing the old_job; thus if the old_job is to be executed again it will be found in the for loops
	m_cores[core_id] = NULL;

	// Find the nearest available item in the queue not already being executed
	for (i = 0; i < priqueue_size(&m_queue); i++)
	{
		mark = 0;
		for (j = 0; j < m_coreNum; j++)
		{
			if (m_cores[j] == (job_t*)priqueue_at(&m_queue, i))
				mark = 1;
		}

		if (mark == 0)
		{
			next_job = (job_t*)priqueue_at(&m_queue, i);
			break;
		}
	}

	// Preempt the old_job with the appropriate new one
	preemptJob(old_job, next_job, time);

	return (next_job != NULL ? next_job->job_id : -1);
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	return (num_jobs != 0 ? total_waiting_time / num_jobs : 0.0);
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	return (num_jobs != 0 ? total_turnaround_time / num_jobs : 0.0);
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	return (num_jobs != 0 ? total_response_time / num_jobs : 0.0);
}


/**
  Free any memory associated with your scheduler.

  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
	// Free m_cores and jobs in the queue
	free(m_cores);
	priqueue_destroy(&m_queue);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{

}
