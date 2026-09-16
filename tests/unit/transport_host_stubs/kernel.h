/*
 * File synopsis:
 * Minimal host-only PS2 kernel declarations used by the direct A001 Transport
 * behavior fixtures. Implementations live in each fixture so failure and
 * synchronization behavior stays explicit and deterministic.
 */

#ifndef PSTVNC_TEST_TRANSPORT_HOST_KERNEL_H
#define PSTVNC_TEST_TRANSPORT_HOST_KERNEL_H

typedef struct ee_sema {
    int init_count;
    int max_count;
    int option;
} ee_sema_t;

typedef struct ee_thread {
    void *func;
    void *stack;
    int stack_size;
    void *gp_reg;
    int initial_priority;
    int attr;
    int option;
} ee_thread_t;

typedef struct ee_thread_status {
    int status;
} ee_thread_status_t;

#define THS_DORMANT 0
#define THS_RUNNING 1

extern unsigned char _gp;

int CreateSema(ee_sema_t *semaphore);
int DeleteSema(int semaphore_id);
int WaitSema(int semaphore_id);
int SignalSema(int semaphore_id);

int CreateThread(ee_thread_t *thread);
int StartThread(int thread_id, void *argument);
int DeleteThread(int thread_id);
int ReferThreadStatus(int thread_id, ee_thread_status_t *status);
int TerminateThread(int thread_id);
void ExitThread(void);

#endif
