#include "sys_rtos.h"
#include "error.h"
#include "bk_rtos_pub.h"

#include "finsh.h"

#define THREAD_TIMESLICE 5

#define RTOS_DEBUG   0
#if RTOS_DEBUG
#define RTOS_DBG(...)     rt_kprintf("[RTOS]"),rt_kprintf(__VA_ARGS__)
#else
#define RTOS_DBG(...)
#endif


/******************************************************
 *               Function Definitions
 ******************************************************/
OSStatus bk_rtos_create_thread( beken_thread_t* thread, uint8_t priority, const char* name, 
                        beken_thread_function_t function, uint32_t stack_size, beken_thread_arg_t arg )
{
    if(thread == RT_NULL)
    {
        RTOS_DBG("can not create null thread\n");
        return kGeneralErr;
    }

    *thread = rt_thread_create(name, function, arg, stack_size, priority, THREAD_TIMESLICE);
    if(*thread != RT_NULL)
    {
        rt_thread_startup(*thread);
    }
    else
    {
        RTOS_DBG("create thread fail\n");
        return kGeneralErr;
    }

    RTOS_DBG("create thread %s\n", name);

    return kNoErr;
}

OSStatus bk_rtos_delete_thread( beken_thread_t* thread )
{
    if(thread != RT_NULL)
    {
        rt_thread_delete(*thread);
    }
    else
    {
        RTOS_DBG("thread == NULL, %s delete self NC\r\n", __FUNCTION__);
        return kGeneralErr;
    }

    RTOS_DBG("deleate thread\n");

    return kNoErr;

}

void bk_rtos_thread_sleep(uint32_t seconds)
{
    rt_thread_delay(rt_tick_from_millisecond(seconds * 1000));
}

static rt_uint32_t rtos_sem_cnt = 0;
static rt_uint32_t rtos_mutex_cnt = 0;

OSStatus bk_rtos_init_semaphore( beken_semaphore_t* semaphore, int maxCount )
{
    *semaphore = rt_sem_create("rtos_sem", 0,  RT_IPC_FLAG_FIFO);
    RTOS_DBG("bk_rtos_init_semaphore:%8x\n", *semaphore);
    rtos_sem_cnt++;

    return (*semaphore != RT_NULL) ? kNoErr : kGeneralErr;
}

OSStatus bk_rtos_get_semaphore(beken_semaphore_t* semaphore, uint32_t timeout_ms )
{
    rt_err_t result;

    result = rt_sem_take(*semaphore, rt_tick_from_millisecond(timeout_ms));

    if(result == RT_EOK)
    {
        return kNoErr;
    }
    else
    {
        rt_kprintf("take semaphore failed %d \n", result);
        return kTimeoutErr;
    }
}

int bk_rtos_get_sema_count(beken_semaphore_t* semaphore )
{
    RTOS_DBG("rtos_get_sema_count\n");
    rt_sem_t sem = *semaphore;
    
    return sem->value;
}

int bk_rtos_set_semaphore( beken_semaphore_t* semaphore)
{
    rt_err_t result;

    result = rt_sem_release(*semaphore);
    if(result != RT_EOK)
    {
        RTOS_DBG("release semaphore failed %d \n", result);
        return kGeneralErr;
    }

    return kNoErr;    
}

OSStatus bk_rtos_deinit_semaphore( beken_semaphore_t* semaphore )
{
    RTOS_DBG("bk_rtos_deinit_semaphore:%8x\n", *semaphore);
    if(semaphore != RT_NULL)
    {
        RTOS_DBG("rtos_deinit_sucess:%8x\n");
        rt_sem_delete(*semaphore);
        rtos_sem_cnt--;        
        *semaphore = RT_NULL;
        return kNoErr;

    }

    return kGeneralErr;
}

OSStatus bk_rtos_init_mutex( beken_mutex_t* mutex )
{
    RTOS_DBG("bk_rtos_init_mutex\n");

    /* Mutex uses priority inheritance */
    *mutex = rt_mutex_create("rtos_mutex", RT_IPC_FLAG_PRIO);
    if ( *mutex == RT_NULL )
    {
        return kGeneralErr;
    }

    rtos_mutex_cnt++;

    return kNoErr;
}

OSStatus bk_rtos_lock_mutex( beken_mutex_t* mutex)
{
    RTOS_DBG("bk_rtos_lock_mutex\n");
    rt_mutex_take(*mutex, BEKEN_WAIT_FOREVER);

    return RT_EOK;
}

OSStatus bk_rtos_unlock_mutex( beken_mutex_t* mutex)
{
    RTOS_DBG("bk_rtos_unlock_mutex\n");
    rt_mutex_release(*mutex);

    return RT_EOK;
}

OSStatus bk_rtos_deinit_mutex( beken_mutex_t* mutex)
{
    if(*mutex != RT_NULL)
    {
        rt_mutex_delete(*mutex);
        rtos_mutex_cnt--;
        *mutex = RT_NULL;
    }

    return RT_EOK;
}

#if 1
OSStatus bk_rtos_init_queue(beken_queue_t* queue, const char* name, uint32_t message_size, uint32_t number_of_messages)
{
    beken_queue_t mq = RT_NULL;

    mq = rt_malloc(sizeof(struct beken_queue));
    rt_memset(mq, 0, sizeof(struct beken_queue));
    *queue = mq;

    RTOS_DBG("bk_rtos_init_queue!\r\n");
    mq->handle = rt_mb_create("rtos_queue", number_of_messages, RT_IPC_FLAG_FIFO);
	if (mq->handle == RT_NULL)
	{
		RTOS_DBG("rt_mailbox_create: 0x%08X!\r\n", mq->handle);
		goto _exit;
	}
    RTOS_DBG("mq->handle:%08p!\r\n", mq->handle);
    RTOS_DBG("rt_mp_create!\r\n");
	mq->mp = rt_mp_create("rtos_queue", number_of_messages, message_size);
	if (mq->mp == RT_NULL)
	{
		RTOS_DBG("rt_mempool_create: 0x%08X!\r\n", mq->mp);
		goto _exit;
	}
    
    RTOS_DBG("rt_mp_create done!\r\n");
    
	mq->message_size       = message_size;
    mq->number_of_messages = number_of_messages;

    return kNoErr;

_exit:
    if(mq->handle != RT_NULL)
    {
        rt_mb_delete(mq->handle);
        mq->handle =RT_NULL;
    }

    if(mq->mp != RT_NULL)
    {
        rt_mp_delete(mq->mp);
        mq->mp = RT_NULL;
    }

	return kGeneralErr;
}
OSStatus bk_rtos_push_to_queue(beken_queue_t* queue, void* message, uint32_t timeout_ms)
{
    rt_err_t ret;
    void *msg_tmp = RT_NULL;
    beken_queue_t mq = *queue;

	msg_tmp = rt_mp_alloc(mq->mp, rt_tick_from_millisecond(timeout_ms));
	if(msg_tmp)
	{
		memcpy(msg_tmp, message, mq->message_size);
		ret = rt_mb_send_wait(mq->handle, (rt_uint32_t)msg_tmp, rt_tick_from_millisecond(timeout_ms));
		if(ret != RT_EOK)
		{
			RTOS_DBG("%s rt_mb_send_wait ret:%d!\r\n", __FUNCTION__, ret);
			return kGeneralErr;
		}
	}
	else
	{
		RTOS_DBG("%s rt_mp_alloc failed! queue->message_size: %d\r\n", __FUNCTION__, mq->message_size);
		return kGeneralErr;
	}

	return kNoErr;
}

OSStatus bk_rtos_push_to_queue_front(beken_queue_t* queue, void* message, uint32_t timeout_ms)
{
    beken_queue_t mq = *queue;
    
    rt_kprintf("\nrtos_push_to_queue_front not implement!!!\n");

    return kGeneralErr;
    // return rt_mq_urgent(mq, message, mq->msg_size);
}

OSStatus bk_rtos_pop_from_queue(beken_queue_t* queue, void* message, uint32_t timeout_ms)
{
    void *msg_tmp = RT_NULL;
    beken_queue_t mq = *queue;
    rt_err_t result;

	result = rt_mb_recv(mq->handle, (rt_uint32_t *)&msg_tmp, rt_tick_from_millisecond(timeout_ms));
	if(result != RT_EOK)
	{
		RTOS_DBG("%s rt_mb_recv ret:%d, ms:=%d!\r\n", __FUNCTION__, result, rt_tick_from_millisecond(timeout_ms));
        // RTOS_DBG("mq->handle:%08p!\r\n", mq->handle);
		return kGeneralErr;
	}

	if(msg_tmp)
	{
		memcpy(message, msg_tmp, mq->message_size);
		rt_mp_free(msg_tmp);
	}
	else
	{
		RTOS_DBG("%s rt_mb_recv item:0x%08X!\r\n", __FUNCTION__, msg_tmp);
		return kGeneralErr;
	}

	return kNoErr;
}

OSStatus bk_rtos_deinit_queue(beken_queue_t* queue)
{
    beken_queue_t mq = *queue;


    if(mq->handle != RT_NULL)
    {
        rt_mb_delete(mq->handle);
        mq->handle =RT_NULL;
    }

    if(mq->mp != RT_NULL)
    {
        rt_mp_delete(mq->mp);
        mq->mp = RT_NULL;
    }

    mq->message_size       = 0;
    mq->number_of_messages = 0;

    rt_free(mq);

    return kNoErr;
}

BOOL bk_rtos_is_queue_empty(beken_queue_t* queue )
{
    rt_uint32_t level;
    beken_queue_t mq = *queue;

    rt_enter_critical();
    if(mq->handle->entry == 0)
    {
        rt_exit_critical();
        
        return true;
    }
    
    rt_exit_critical();

    return false;
}

BOOL bk_rtos_is_queue_full(beken_queue_t* queue)
{
    rt_uint32_t level;
    beken_queue_t mq = *queue;

    rt_enter_critical();
    if(mq->handle->entry == mq->handle->size)
    {
        rt_exit_critical();

        return true;
    }

    rt_exit_critical();

    return false;
}
#else
OSStatus rtos_init_queue(beken_queue_t* queue, const char* name, uint32_t message_size, uint32_t number_of_messages)
{
    *queue = rt_mq_create(name, message_size, number_of_messages, RT_IPC_FLAG_FIFO);
    
    if(*queue == RT_NULL)
    {
        rt_kprintf("create queue failed\n");
        return RT_ERROR;
    }

    return RT_EOK;
}

OSStatus rtos_push_to_queue(beken_queue_t* queue, void* message, uint32_t timeout_ms)
{
    rt_mq_t mq = *queue;
    rt_err_t result;
    
    result = rt_mq_send(mq, message, mq->msg_size);
    if(result != RT_EOK)
    {
        rt_kprintf("mq send failed %d \n", result);
        return kGeneralErr;
    }

    return kNoErr;
}

OSStatus rtos_push_to_queue_front( beken_queue_t* queue, void* message, uint32_t timeout_ms )
{
    rt_mq_t mq = *queue;
    
    return rt_mq_urgent(mq, message, mq->msg_size);
}

OSStatus rtos_pop_from_queue( beken_queue_t* queue, void* message, uint32_t timeout_ms )
{
    rt_mq_t mq = *queue;
    rt_err_t result;

    result = rt_mq_recv(mq, message, mq->msg_size, rt_tick_from_millisecond(timeout_ms));
    if(result != RT_EOK)
    {
        rt_kprintf("%s rt_mq_recv ret:%d, ms:=%d!\r\n", __FUNCTION__, result, rt_tick_from_millisecond(timeout_ms));
        return kGeneralErr;
    }

    return kNoErr;
}

OSStatus rtos_deinit_queue(beken_queue_t* queue)
{
    if(queue != RT_NULL)
    {
        rt_mq_delete(*queue);
        *queue = RT_NULL;

        return RT_EOK;
    }

    return RT_ERROR;
}

BOOL rtos_is_queue_empty(beken_queue_t* queue )
{
    rt_uint32_t level;
    rt_bool_t result;
    rt_mq_t mq = *queue;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();
    if(mq->entry == 0)
    {
        /* enable interrupt */
        rt_hw_interrupt_enable(level);
        
        return true;
    }
    
    rt_hw_interrupt_enable(level);

    return false;
}

BOOL rtos_is_queue_full(beken_queue_t* queue )
{
    rt_uint32_t level;
    struct rt_mq_message *msg;
    rt_mq_t mq = *queue;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    /* get a free list, there must be an empty item */
    msg = mq->msg_queue_free;

    /* message queue is full */
    if (msg == RT_NULL)
    {
        /* enable interrupt */
        rt_hw_interrupt_enable(level);

        return false;
    }

    rt_hw_interrupt_enable(level);

    return true;
}
#endif
OSStatus bk_rtos_delay_milliseconds( uint32_t num_ms)
{
    rt_thread_delay(rt_tick_from_millisecond(num_ms));
} 

static void timer_oneshot_callback(void* parameter)
{
    beken2_timer_t *timer = (beken2_timer_t*)parameter;

    RTOS_DBG("one shot callback\n");

	if(BEKEN_MAGIC_WORD != timer->beken_magic)
	{
		return;
	}
    if (timer->function)
    {
        timer->function(timer->left_arg, timer->right_arg );
    }
}

OSStatus bk_rtos_start_oneshot_timer( beken2_timer_t* timer)
{
    RTOS_DBG("oneshot_timer start \n");

    if(timer->handle != RT_NULL)
    {
        rt_timer_start(timer->handle);
        return RT_EOK;
    }

    return RT_ERROR;
}

OSStatus bk_rtos_stop_oneshot_timer(beken2_timer_t* timer)
{
    RTOS_DBG("oneshot_timer stop \n");

    if(timer->handle != RT_NULL)
    {
        rt_timer_stop(timer->handle);
        return kNoErr;
    }

    return kGeneralErr;
}

BOOL bk_rtos_is_oneshot_timer_init(beken2_timer_t* timer)
{
    RTOS_DBG("oneshot_timer is init \n");
    return timer->handle ? true : false;
}

BOOL bk_rtos_is_oneshot_timer_running(beken2_timer_t* timer)
{
    rt_timer_t os_timer = (rt_timer_t)timer;

    RTOS_DBG("oneshot_timer is runing \n");
    return os_timer->parent.flag & RT_TIMER_FLAG_ACTIVATED ? true : false;
}

OSStatus bk_rtos_oneshot_reload_timer( beken2_timer_t* timer)
{
    RTOS_DBG("reload oneshot timer\n");

    if(bk_rtos_is_oneshot_timer_running(timer))
    {
        rt_timer_start(timer->handle);
        RTOS_DBG("timer is runing, set tick to 0 \r\n");
    }
    else
    {
        rt_timer_start(timer->handle);
        RTOS_DBG("timer is stop, start timer \r\n");
    }
    return kNoErr;
}

OSStatus bk_rtos_init_oneshot_timer( beken2_timer_t *timer, 
									uint32_t time_ms, 
									timer_2handler_t function,
									void* larg, 
									void* rarg )
{
	OSStatus ret = kNoErr;

    RTOS_DBG("create oneshot_timer \n");
    timer->function = function;
    timer->left_arg = larg;
    timer->right_arg = rarg;	
	timer->beken_magic = BEKEN_MAGIC_WORD;

    timer->handle = rt_timer_create("rtos_oneshot_time",
                                    timer_oneshot_callback,
                                    timer,
                                    rt_tick_from_millisecond(time_ms),
                                    RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);
    if ( timer->handle == NULL )
    {
        ret = kGeneralErr;
    }

    return ret;
}

OSStatus bk_rtos_deinit_oneshot_timer( beken2_timer_t* timer )
{
	OSStatus ret = kNoErr;

    RTOS_DBG("delete oneshot_timer \n");
    if(rt_timer_delete((rt_timer_t)timer->handle) != RT_EOK)
    {
        ret = RT_ERROR;
    }
	else
	{
		timer->handle = 0;
		timer->function = 0;
		timer->left_arg = 0;
		timer->right_arg = 0;
		timer->beken_magic = 0;
	}

    return ret;
}

static void bk_timer_period_callback(void* parameter)
{
    beken_timer_t *timer = (beken_timer_t*)parameter;

    RTOS_DBG("period timer callback\n");

    if (timer->function)
    {
        timer->function(timer->arg);
    }
}

OSStatus bk_rtos_start_timer( beken_timer_t* timer)
{
    RTOS_DBG("period_timer start \n");
    if(timer->handle != RT_NULL)
    {
        rt_timer_start(timer->handle);
        return RT_EOK;
    }

    return RT_ERROR;
}

OSStatus bk_rtos_stop_timer(beken_timer_t* timer)
{
    RTOS_DBG("period_timer stop \n");

    if(timer->handle != RT_NULL)
    {
        rt_timer_stop(timer->handle);
        return RT_EOK;
    }

    return RT_ERROR;
}

BOOL bk_rtos_is_timer_init(beken_timer_t* timer)
{
    RTOS_DBG("period_timer is init \n");
    return timer->handle ? true : false;
}

BOOL bk_rtos_is_timer_running(beken_timer_t* timer)
{
    rt_timer_t os_timer = (rt_timer_t)timer;

    RTOS_DBG("period_timer is runing \n");
    return os_timer->parent.flag & RT_TIMER_FLAG_ACTIVATED ? true : false;
}

OSStatus bk_rtos_reload_timer( beken_timer_t* timer)
{
    RTOS_DBG("reload period timer\n");

    if(bk_rtos_is_timer_running(timer))
    {
        rt_timer_start(timer->handle);
        rt_kprintf("timer is runing, set tick to 0 \r\n");
    }
    else
    {
        rt_timer_start(timer->handle);
        // rt_kprintf("timer is stop, start timer \r\n");
    }
    return kNoErr;
}

OSStatus bk_rtos_change_period( beken_timer_t* timer, uint32_t time_ms)
{
    rt_uint32_t timeout_value;

    timeout_value = rt_tick_from_millisecond(time_ms);
    rt_timer_control(timer->handle, RT_TIMER_CTRL_SET_TIME, (void *)&timeout_value);
    rt_timer_start(timer->handle);

    return kNoErr;
}

OSStatus bk_rtos_init_timer( beken_timer_t* timer, uint32_t time_ms, timer_handler_t function, void* arg)
{
	OSStatus ret = kNoErr;

    RTOS_DBG("create period_timer \n");
    timer->function = function;
    timer->arg      = arg;

    timer->handle = rt_timer_create("rtos_period_time",
                                    bk_timer_period_callback,
                                    timer,
                                    rt_tick_from_millisecond(time_ms),
                                    RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_SOFT_TIMER);
    if ( timer->handle == NULL )
    {
        ret = kNoErr;
    }

    return ret;
}

OSStatus bk_rtos_deinit_timer( beken_timer_t* timer )
{
	OSStatus ret = kNoErr;

    RTOS_DBG("delete period_timer \n");
    if(rt_timer_delete((rt_timer_t)timer->handle) != RT_EOK)
    {
        ret = kGeneralErr;
    }
	else
	{
		timer->handle = 0;
		timer->function = 0;
		timer->arg      = 0;
	}

    return ret;
}

uint32_t bk_rtos_get_timer_expiry_time( beken_timer_t* timer )
{
    // TODO:
    rt_kprintf("[TODO]:Note, bk_rtos_get_timer_expiry_time is not implement \n");
    return 0;
}

uint32_t bk_rtos_get_time(void)
{
    return rt_tick_get();
}

OSStatus bk_rtos_thread_join( beken_thread_t* thread )
{
	rt_thread_t thread_p = *thread;
	
	while(rt_thread_find(thread_p->name))
	{
        bk_rtos_delay_milliseconds(10);
	}

	return kNoErr;
}

