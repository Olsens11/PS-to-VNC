#ifndef PSTVNC_INPUT_RUNTIME_PRODUCT_ACTION_TEST_KERNEL_H
#define PSTVNC_INPUT_RUNTIME_PRODUCT_ACTION_TEST_KERNEL_H

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

#define THS_DORMANT 1

extern unsigned char _gp;

int CreateSema(ee_sema_t *semaphore);
int DeleteSema(int semaphore_id);
int WaitSema(int semaphore_id);
int SignalSema(int semaphore_id);

int CreateThread(ee_thread_t *thread);
int StartThread(int thread_id, void *argument);
int ReferThreadStatus(int thread_id, ee_thread_status_t *status);
int DeleteThread(int thread_id);
void ExitThread(void);

#endif
