/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"

#if !NO_SYS

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#if defined(LWIP_PROVIDE_ERRNO)
int errno;
#endif

/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	*mbox = xQueueCreate(size, sizeof(void *));
	if (*mbox == NULL) {
#if SYS_STATS
		++lwip_stats.sys.mbox.err;
#endif /* SYS_STATS */
		return ERR_MEM;
	}

#if SYS_STATS
	++lwip_stats.sys.mbox.used;
	if (lwip_stats.sys.mbox.max < lwip_stats.sys.mbox.used) {
		lwip_stats.sys.mbox.max = lwip_stats.sys.mbox.used;
	}
#endif /* SYS_STATS */

	return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
	if (uxQueueMessagesWaiting(*mbox)) {
		/* Line for breakpoint.  Should never break here! */
		portNOP();
#if SYS_STATS
		lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
	}
	// Delete the queue
	vQueueDelete(*mbox);
#if SYS_STATS
	--lwip_stats.sys.mbox.used;
#endif /* SYS_STATS */
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void sys_mbox_post(sys_mbox_t *mbox, void *data)
{
	// Block until the message is posted successfully
	while (xQueueSend(*mbox, &data, portMAX_DELAY) != pdPASS) {
	}
}

/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	// Try to post the message to the mailbox without blocking
	if (xQueueSend(*mbox, &msg, 0) == pdPASS) {
		return ERR_OK;
	} else {
		// could not post, queue must be full
#if SYS_STATS
		lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
		return ERR_MEM;
	}
}

/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if (xQueueSendFromISR(*mbox, &msg, &xHigherPriorityTaskWoken) == pdPASS) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		return ERR_OK;
	} else {
		// could not post, queue must be full
#if SYS_STATS
		lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */
		return ERR_MEM;
	}
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP.
*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	TickType_t starttime = xTaskGetTickCount();
	if (timeout != 0) {
		if (xQueueReceive(*mbox, msg, pdMS_TO_TICKS(timeout)) == pdPASS) {
			return (xTaskGetTickCount() - starttime);
		} else {
			return SYS_ARCH_TIMEOUT;
		}
	} else {
		while (xQueueReceive(*mbox, msg, portMAX_DELAY) != pdPASS) {
		}
		return (xTaskGetTickCount() - starttime);
	}
}

/*-----------------------------------------------------------------------------------*/
/*
  Similar to sys_arch_mbox_fetch, but if message is not ready immediately, we'll
  return with SYS_MBOX_EMPTY.  On success, 0 is returned.
*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	if (xQueueReceive(*mbox, msg, 0) == pdPASS) {
		return ERR_OK;
	} else {
		return SYS_MBOX_EMPTY;
	}
}

/*----------------------------------------------------------------------------------*/
int sys_mbox_valid(sys_mbox_t *mbox)
{
	if (*mbox == SYS_MBOX_NULL) {
		return 0;
	} else {
		return 1;
	}
}

/*-----------------------------------------------------------------------------------*/
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	*mbox = NULL;
}

/*-----------------------------------------------------------------------------------*/
//  Creates a new semaphore. The "count" argument specifies
//  the initial state of the semaphore.
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	// Create a counting semaphore
	*sem = xSemaphoreCreateCounting(UINT16_MAX, count);
	if (*sem == NULL) {
#if SYS_STATS
		++lwip_stats.sys.sem.err;
#endif /* SYS_STATS */
		return ERR_MEM;
	}

	// If the initial count is zero, take the semaphore to set it to the correct state
	if (count == 0) {
		xSemaphoreTake(*sem, 0);
	}

#if SYS_STATS
	++lwip_stats.sys.sem.used;
	if (lwip_stats.sys.sem.max < lwip_stats.sys.sem.used) {
		lwip_stats.sys.sem.max = lwip_stats.sys.sem.used;
	}
#endif /* SYS_STATS */

	return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	TickType_t starttime = xTaskGetTickCount();
	if (timeout != 0) {
		if (xSemaphoreTake(*sem, pdMS_TO_TICKS(timeout)) == pdPASS) {
			return (xTaskGetTickCount() - starttime);
		} else {
			return SYS_ARCH_TIMEOUT;
		}
	} else {
		while (xSemaphoreTake(*sem, portMAX_DELAY) != pdPASS)
			;
		return (xTaskGetTickCount() - starttime);
	}
}

/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
void sys_sem_signal(sys_sem_t *sem)
{
	xSemaphoreGive(*sem);
}

/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
void sys_sem_free(sys_sem_t *sem)
{
#if SYS_STATS
	--lwip_stats.sys.sem.used;
#endif /* SYS_STATS */
	vSemaphoreDelete(*sem);
}

/*-----------------------------------------------------------------------------------*/
int sys_sem_valid(sys_sem_t *sem)
{
	if (*sem == SYS_SEM_NULL) {
		return 0;
	} else {
		return 1;
	}
}

/*-----------------------------------------------------------------------------------*/
void sys_sem_set_invalid(sys_sem_t *sem)
{
	*sem = SYS_SEM_NULL;
}

/*-----------------------------------------------------------------------------------*/
SemaphoreHandle_t lwip_sys_mutex;

// Initialize sys arch
void sys_init(void)
{
	// Create a mutex
	lwip_sys_mutex = xSemaphoreCreateMutex();
}

/*-----------------------------------------------------------------------------------*/
/* Mutexes*/
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
#if LWIP_COMPAT_MUTEX == 0
/* Create a new mutex*/
err_t sys_mutex_new(sys_mutex_t *mutex)
{
	// Create a mutex
	*mutex = xSemaphoreCreateMutex();
	if (*mutex == NULL) {
#if SYS_STATS
		++lwip_stats.sys.mutex.err;
#endif /* SYS_STATS */
		return ERR_MEM;
	}

#if SYS_STATS
	++lwip_stats.sys.mutex.used;
	if (lwip_stats.sys.mutex.max < lwip_stats.sys.mutex.used) {
		lwip_stats.sys.mutex.max = lwip_stats.sys.mutex.used;
	}
#endif /* SYS_STATS */
	return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/* Deallocate a mutex*/
void sys_mutex_free(sys_mutex_t *mutex)
{
#if SYS_STATS
	--lwip_stats.sys.mutex.used;
#endif /* SYS_STATS */
	vSemaphoreDelete(*mutex);
}

/*-----------------------------------------------------------------------------------*/
/* Lock a mutex*/
void sys_mutex_lock(sys_mutex_t *mutex)
{
	xSemaphoreTake(*mutex, portMAX_DELAY);
}

/*-----------------------------------------------------------------------------------*/
/* Unlock a mutex*/
void sys_mutex_unlock(sys_mutex_t *mutex)
{
	xSemaphoreGive(*mutex);
}
#endif /*LWIP_COMPAT_MUTEX*/

/*
  Starts a new thread with priority "prio" that will begin its execution in the
  function "thread()". The "arg" argument will be passed as an argument to the
  thread() function. The id of the new thread is returned. Both the id and
  the priority are system dependent.
*/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize,
			    int prio)
{
	sys_thread_t task;
	xTaskCreate(thread, name, stacksize, arg, prio, &task);
}

/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.

  Note: This function is based on FreeRTOS API, because no equivalent CMSIS-RTOS
	API is available
*/
sys_prot_t sys_arch_protect(void)
{
	taskENTER_CRITICAL();
	return (sys_prot_t)1;
}

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.

  Note: This function is based on FreeRTOS API, because no equivalent CMSIS-RTOS
	API is available
*/
void sys_arch_unprotect(sys_prot_t pval)
{
	(void)pval;
	taskEXIT_CRITICAL();
}

u32_t sys_now(void)
{
	return xTaskGetTickCount();
}

#endif /* !NO_SYS */
