/**
 ******************************************************************************
 * @file    os_thread.c
 * @author
 * @version V1.0.0
 * @date
 * @brief   RTOS thread control demo.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2017 Beken Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************
 */
#include <rtthread.h>
#include "include.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bk_rtos_pub.h"
#include "uart_pub.h"
#include "error.h"
#include "portmacro.h"

#define	 OS_THREAD_DEMO		0
#define	 OS_MUTEX_DEMO		0
#define	 OS_SEM_DEMO		0
#define  OS_QUEUE_DEMO 		0
#define  OS_TIMER_DEMO 		0

#if OS_THREAD_DEMO
static void thread_0( beken_thread_arg_t arg )
{
    (void)( arg );

    os_printf( "This is thread 0\r\n");
    bk_rtos_delay_milliseconds((TickType_t)1000 );

    /* Make with terminate state and IDLE thread will clean resources */
    bk_rtos_delete_thread(NULL);
}

static void thread_1( beken_thread_arg_t arg )
{
    (void)( arg );
    OSStatus err = kNoErr;
    beken_thread_t t_handler = NULL;

    while ( 1 )
    {
        /* Create a new thread, and this thread will delete its self and clean its resource */
        err = bk_rtos_create_thread( &t_handler,
                                  BEKEN_APPLICATION_PRIORITY,
                                  "Thread 0",
                                  thread_0,
                                  0x400,
                                  0);
        if(err != kNoErr)
        {
            os_printf("ERROR: Unable to start the thread 1.\r\n" );
        }
        /* wait thread 0 delete it's self */
        bk_rtos_thread_join( &t_handler );
    }
}

static void thread_2( beken_thread_arg_t arg )
{
    (void)( arg );

    while ( 1 )
    {
        os_printf( "This is thread 2\r\n" );
        bk_rtos_delay_milliseconds((TickType_t)600);
    }
}

/***************************************************************
   This function make two thread for Os_thread application.
***************************************************************/
static int thread_demo_start( void )
{
    OSStatus err = kNoErr;
    beken_thread_t t_handler1 = NULL, t_handler2 = NULL;

    os_printf("\r\n\r\noperating system thread demo............\r\n" );

    err = bk_rtos_create_thread( &t_handler1, BEKEN_APPLICATION_PRIORITY,
                              "Thread 1",
                              thread_1,
                              0x400,
                              0);
    if(err != kNoErr)
    {
        os_printf("ERROR: Unable to start the thread 1.\r\n" );
        goto exit;
    }

    err = bk_rtos_create_thread( &t_handler2, BEKEN_APPLICATION_PRIORITY,
                              "Thread 2",
                              thread_2,
                              0x400,
                              0);
    if(err != kNoErr)
    {
        os_printf("ERROR: Unable to start the thread 2.\r\n" );
        goto exit;
    }

exit:
    if ( err != kNoErr )
    {
        os_printf( "Thread exit with err: %d", err );

        if(t_handler1 != NULL)
        {
            bk_rtos_delete_thread(t_handler1);
        }

        if(t_handler2 != NULL)
        {
            bk_rtos_delete_thread(t_handler2);
        }
    }

    return err;
}

#endif

#if OS_MUTEX_DEMO
static beken_mutex_t os_mutex = NULL;

static OSStatus mutex_printf_msg(char *s)
{
    OSStatus err = kNoErr;
    if(os_mutex == NULL)
    {
        return -1;
    }
    err = bk_rtos_lock_mutex(&os_mutex);
    if(err != kNoErr)
    {
        return err;
    }
    os_printf( "%s\r\n", s);
    err = bk_rtos_unlock_mutex(&os_mutex);
    if(err != kNoErr)
    {
        return err;
    }
    return err;
}

static void os_mutex_sender_thread( beken_thread_arg_t arg )
{
    OSStatus err = kNoErr;
    char *taskname = (char *)arg;
    char strprt[100];
    int rd;
    while ( 1 )
    {
        rd = rand() & 0x1FF;
        sprintf(strprt, "%s , Rand:%d", taskname, rd);
        err = mutex_printf_msg(strprt);
        if(err != kNoErr)
        {
            os_printf( "%s printf_msg error!\r\n", taskname);
            goto exit;
        }
        bk_rtos_delay_milliseconds( rd );
    }

exit:
    if ( err != kNoErr )
    {
        os_printf( "Sender exit with err: %d\r\n", err );
    }
    if(os_mutex != NULL)
    {
        bk_rtos_deinit_mutex(&os_mutex);
    }
    bk_rtos_delete_thread( NULL );
}

static int mutex_demo_start( void )
{
    OSStatus err = kNoErr;
    beken_thread_t t_handler1 = NULL, t_handler2 = NULL;

    err = bk_rtos_init_mutex( &os_mutex );

    if(err != kNoErr)
    {
        os_printf( "bk_rtos_init_mutex err: %d\r\n", err );
        goto exit;
    }

    err = bk_rtos_create_thread( &t_handler1,
                              BEKEN_APPLICATION_PRIORITY,
                              "sender1",
                              os_mutex_sender_thread,
                              0x400,
                              "my name is thread1");
    if(err != kNoErr)
    {
        goto exit;
    }

    err = bk_rtos_create_thread( &t_handler2,
                              BEKEN_APPLICATION_PRIORITY,
                              "sender2",
                              os_mutex_sender_thread,
                              0x400,
                              "I'm is task!" );
    if(err != kNoErr)
    {
        goto exit;
    }
exit:
    if ( err != kNoErr )
    {
        os_printf( "Thread exit with err: %d\r\n", err );
    }
    return err;
}
#endif

#if OS_QUEUE_DEMO
typedef struct _msg
{
    int value;
} msg_t;

static beken_queue_t os_queue = NULL;

static void receiver_thread( beken_thread_arg_t arg )
{
    OSStatus err;
    msg_t received = { 0 };

    while ( 1 )
    {
        /*Wait until queue has data*/
        err = bk_rtos_pop_from_queue( &os_queue, &received, BEKEN_NEVER_TIMEOUT);
        if(err == kNoErr)
        {
            os_printf( "Received data from queue:value = %d\r\n", received.value );
        }
        else
        {
            os_printf("Received data from queue failed:Err = %d\r\n", err);
            goto exit;
        }
    }

exit:
    if ( err != kNoErr )
        os_printf( "Receiver exit with err: %d\r\n", err );

    bk_rtos_delete_thread( NULL );
}

static void sender_thread( beken_thread_arg_t arg )
{
    OSStatus err = kNoErr;

    msg_t my_message = { 0 };

    while ( 1 )
    {
        my_message.value++;
        err = bk_rtos_push_to_queue(&os_queue, &my_message, BEKEN_NEVER_TIMEOUT);
        if(err == kNoErr)
        {
            os_printf( "send data to queue\r\n" );
        }
        else
        {
            os_printf("send data to queue failed:Err = %d\r\n", err);
        }
        bk_rtos_delay_milliseconds( 100 );
    }

exit:
    if ( err != kNoErr )
    {
        os_printf( "Sender exit with err: %d\r\n", err );
    }

    bk_rtos_delete_thread( NULL );
}

static int queue_demo_start( void )
{
    OSStatus err = kNoErr;
    beken_thread_t t_handler1 = NULL, t_handler2 = NULL;
	
    err = bk_rtos_init_queue( &os_queue, "queue", sizeof(msg_t), 3 );

    if(err != kNoErr)
    {
        goto exit;
    }
    err = bk_rtos_create_thread( &t_handler1,
                              BEKEN_APPLICATION_PRIORITY,
                              "sender",
                              sender_thread,
                              0x500,
                              0 );
    if(err != kNoErr)
    {
        goto exit;
    }
    err = bk_rtos_create_thread( &t_handler2,
                              BEKEN_APPLICATION_PRIORITY,
                              "receiver",
                              receiver_thread,
                              0x500,
                              0 );
    if(err != kNoErr)
    {
        goto exit;
    }

exit:
    if ( err != kNoErr )
    {
        os_printf( "Thread exit with err: %d\r\n", err );
    }
    return err;
}
#endif

#if OS_SEM_DEMO
static beken_semaphore_t os_sem = NULL;

static void set_semaphore_thread( beken_thread_arg_t arg )
{
    while ( 1 )
    {
        os_printf( "release semaphore!\r\n" );
        bk_rtos_set_semaphore( &os_sem );
        bk_rtos_delay_milliseconds( 500 );
    }

exit:
    if(os_sem)
    {
        bk_rtos_deinit_semaphore(&os_sem);
    }
    bk_rtos_delete_thread( NULL );
}

static void get_semaphore_thread( beken_thread_arg_t arg )
{
    OSStatus err;

    while(1)
    {
        err = bk_rtos_get_semaphore(&os_sem, BEKEN_NEVER_TIMEOUT);
        if(err == kNoErr)
        {
            os_printf("Get_Sem Succend!\r\n");
        }
        else
        {
            os_printf("Get_Sem Err:%d\r\n", err);
            goto exit;
        }
    }

exit:
    if(os_sem)
    {
        bk_rtos_deinit_semaphore(&os_sem);
    }
    bk_rtos_delete_thread( NULL );
}

static int sem_demo_start( void )
{
    OSStatus err = kNoErr;
    beken_thread_t t_handler1 = NULL, t_handler2 = NULL;
	
    os_printf( "test binary semaphore\r\n" );

    err = bk_rtos_init_semaphore( &os_sem, 1 ); //0/1 binary semaphore || 0/N semaphore

    if(err != kNoErr)
    {
        goto exit;
    }

    err = bk_rtos_create_thread( &t_handler1,
                              BEKEN_APPLICATION_PRIORITY,
                              "get_sem",
                              get_semaphore_thread,
                              0x500,
                              0 );
    if(err != kNoErr)
    {
        goto exit;
    }
    err = bk_rtos_create_thread( &t_handler2,
                              BEKEN_APPLICATION_PRIORITY,
                              "set_sem",
                              set_semaphore_thread,
                              0x500,
                              0 );
    if(err != kNoErr)
    {
        goto exit;
    }

    return err;
exit:
    if ( err != kNoErr )
    {
        os_printf( "Thread exit with err: %d\r\n", err );
    }
    return err;
}
#endif

#if OS_TIMER_DEMO
beken_timer_t timer_handle, timer_handle2;

static void destroy_timer( void )
{
    /* Stop software timer(timer_handle) */
    bk_rtos_stop_timer( &timer_handle );
    /* delete software timer(timer_handle) */
    bk_rtos_deinit_timer( &timer_handle );
    /* Stop software timer(timer_handle2) */
    bk_rtos_stop_timer( &timer_handle2 );
    /* delete software timer(timer_handle2) */
    bk_rtos_deinit_timer( &timer_handle2 );
}

static void timer_alarm( void *arg )
{
    os_printf("I'm timer_handle1\r\n");
}

static void timer2_alarm( void *arg )
{
    os_printf("I'm timer_handle2,destroy timer!\r\n");

    destroy_timer();
}

static int timer_demo_start( void )
{
    OSStatus err = kNoErr;

    os_printf("timer demo\r\n");

    /* Create a new software timer,software is AutoReload */
    err = bk_rtos_init_timer(&timer_handle, 500, timer_alarm, 0);  ///500mS
    if(kNoErr != err)
        goto exit;

    /* Create a new software timer,software is AutoReload */
    err = bk_rtos_init_timer(&timer_handle2, 2600, timer2_alarm, 0);  ///2.6S
    if(kNoErr != err)
        goto exit;

    /* start (timer_handle) timer */
    err = bk_rtos_start_timer(&timer_handle);
    if(kNoErr != err)
        goto exit;

    /* start (timer_handle2) timer */
    err = bk_rtos_start_timer(&timer_handle2);
    if(kNoErr != err)
        goto exit;

    return err;

exit:
    if( err != kNoErr )
        os_printf( "os timer exit with err: %d", err );

    return err;
}
#endif

static int os_demo(int argc, char **argv)
{
	
    if(strcmp(argv[1], "thread") == 0)
    {
#if OS_THREAD_DEMO
		thread_demo_start();
#endif
    }
	else if(strcmp(argv[1], "mutex") == 0)
	{
#if OS_MUTEX_DEMO
		mutex_demo_start();
#endif
	}
	else if(strcmp(argv[1], "queue") == 0)
	{
#if OS_QUEUE_DEMO
		queue_demo_start();
#endif
	}
	else if(strcmp(argv[1], "semaphore") == 0)
	{
#if OS_SEM_DEMO
		sem_demo_start();
#endif
	}
	else if(strcmp(argv[1], "timer") == 0)
	{
#if OS_TIMER_DEMO
		timer_demo_start();
#endif
	}
	else
	{
		os_printf("os demo %s dosn't support.\n", argv[1]);
	}
}

MSH_CMD_EXPORT(os_demo, os_demo command);
// eof

