/** @file libpriqueue.h
 */

#ifndef LIBPRIQUEUE_H_
#define LIBPRIQUEUE_H_

#define INITIAL_ARRAY_SIZE 32

/**
  Priqueue Data Structure
*/
typedef struct _priqueue_t
{
    void** m_array;
    int (*m_comparer) (const void* arg1, const void* arg2);
    int m_size;
    int m_used;
} priqueue_t;


void   priqueue_init     (priqueue_t *q, int(*comparer)(const void *, const void *));

int    priqueue_offer    (priqueue_t *q, void *ptr);
void * priqueue_peek     (priqueue_t *q);
void * priqueue_poll     (priqueue_t *q);
void * priqueue_at       (priqueue_t *q, int index);
int    priqueue_remove   (priqueue_t *q, void *ptr);
void * priqueue_remove_at(priqueue_t *q, int index);
int    priqueue_size     (priqueue_t *q);
void   priqueue_resort   (priqueue_t *q);
void   priqueue_movetoback (priqueue_t *q, void *ptr);

void   priqueue_destroy  (priqueue_t *q);

#endif /* LIBPQUEUE_H_ */
