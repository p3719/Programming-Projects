/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
    int i;
    q->m_array = (void*)malloc(INITIAL_ARRAY_SIZE*sizeof(void*)); //TODO: or just 4 idk whats clearer also it returns a void* and I'm casting idk if that is clear or confusing -> makes no difference either way

    //Initialize all the pointers to NULL
    for(i = 0; i < INITIAL_ARRAY_SIZE; i++)
    {
        q->m_array[i] = NULL;
    }

    q->m_size = INITIAL_ARRAY_SIZE;
    q->m_used = 0;
    q->m_comparer = comparer;
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
    int i, j;

    //check to see if we need to grow array
    if(q->m_used == q->m_size)
    {
        q->m_size = q->m_size*2;
        q->m_array = realloc(q->m_array, q->m_size*sizeof(void*));
				//TODO: note to self - check if new elements are Initialized to null - UPDATE: they are not, they're just wutever was previously occupying that space in memory
				// CHANGED: Initialize new values to null;
				for (j = q->m_size / 2; j < q->m_size; j++)
				{
					q->m_array[j] = NULL;
				}
		}

    //add ptr to end of array and increment used
    q->m_array[q->m_used] = ptr;
    q->m_used++;		//CHANGED: was "q->m_used = q->m_used++;" ->  waatwaa :)

    //do a qsort with m_comparer
    qsort(q->m_array, q->m_used, sizeof(void*), q->m_comparer);

    //find the thing we put into the queue and return it's index
    for(i = 0; i < q->m_used; i++)
    {
        if(ptr == q->m_array[i])
        {
            return i;
        }
    }

    //error
	return -1;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
    if(q->m_used != 0)
    {
        return q->m_array[0];
    }
	return NULL;
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
		int i = 0;
    //TODO: We could assume that when the queue gets emptied that there should be a NULL in first spot or we can trust that m_used is correct value. Your call first approach looks a little cleaner but yeah
		// COMMENT: I like how you have it with the NULL's

    //Grab with temp*
    void* temp = q->m_array[0];

    //move everything down; decrement used
    //TODO: also not confident this stuff works ; CHANGED: added conditional and decremented m_used
		if (temp != NULL)
		{
			for(i = 0; i < (q->m_used) - 1; i++)
			{
					q->m_array[i] = q->m_array[i+1];
			}
			q->m_used--;

			//set m_used - 1 (hopefully) last index to null
			q->m_array[i] = NULL;
		}

    //TODO: I am not sure the kind of sorts we are doing for the different scheduler types. we may need to resort? Idk
		// CHANGED: we want to return the head of the queue not just NULL; u already grabbed the value above
		return temp;
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
    //assuming it is still zero indexed, check effective size vs index
    if(q->m_used > index)
    {
        return q->m_array[index];
    }
	return NULL;
}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
    int skips = 0;
    int i;

    //go through whole array set pointers to null? or move down as you go like in 560 hw
		// COMMENT: I like the way you have it implemented now
    for(i = 0; i < q->m_used; i++)
    {
        if(q->m_array[i] == ptr)
        {
            q->m_array[i] = NULL;
            skips++;
        }
        else
        {
            q->m_array[i-skips] = q->m_array[i];
						if (skips > 0)
						{
							q->m_array[i] = NULL;
						}
        }
    }

    //less m_used by number of removed elements
    q->m_used = (q->m_used) - skips;

		// CHANGED: added a qsort to ensure that the array is correctly sorted when we remove
		qsort(q->m_array, q->m_used, sizeof(void*), q->m_comparer);

    // CHANGED: We should not free memory that we did not allocate
    //TODO: Do i actually need to free up this memory I have no clue
		// COMMENT: I would assume so otherwise it would just be chillin there;
		// It was allocated in the testing file and if we don't free it then it will never be freed
		// free(ptr);


	return skips;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
    void* temp = NULL;
		int i;

    //check
    if(q->m_used > index)
    {
        //grab
        temp = q->m_array[index];

        //Start at next index and move left one
				// CHANGED: Made this code consistent with the for loop from priqueue_poll; exact same functionality though
        for(i = index; i < q->m_used - 1; i++)
        {
            q->m_array[i] = q->m_array[i+1];
        }

        //set last thing in array to null so we don't have a duplicate
        q->m_array[i] = NULL;

        //reduce effective size
        q->m_used--;
    }

		// CHANGED: added a qsort to ensure that the array is correctly sorted when we remove
		qsort(q->m_array, q->m_used, sizeof(void*), q->m_comparer);

	return temp;
}


/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
    //return effective size
	return q->m_used;
}

// Resorts the array; figured this might be helpful have as a function as to not need to call qsort...maybe
void priqueue_resort(priqueue_t *q)
{
	qsort(q->m_array, q->m_used, sizeof(void*), q->m_comparer);
}

// Takes the index of the element that you would like to be moved to the back
void priqueue_movetoback (priqueue_t *q, void *ptr)
{
	int i, index;

	// Find the index that the old job was at in the queue
	for (i = 0; i < q->m_used; i++)
	{
		if (q->m_array[i] == ptr)
		{
			index = i;
		}
	}

	// Move the old job to the back of the queue
	void* temp = priqueue_remove_at(q, index);
	priqueue_offer (q, temp);
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
    int i;
    // CHANGED: we should only free memory that we allocated
    free(q->m_array);

    //TODO: reset everything else?? I have no clue if this object will be reused
		// COMMENT: I reset the rest in the case that they do reuse it.
		// It isn't reused in the test code, but hey better safe than sorry :)
		q->m_comparer = NULL;
		q->m_size = INITIAL_ARRAY_SIZE;
		q->m_used = 0;
}
