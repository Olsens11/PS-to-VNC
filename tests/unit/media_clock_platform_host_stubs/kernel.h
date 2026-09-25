#ifndef PSTVNC_TEST_MEDIA_CLOCK_KERNEL_H
#define PSTVNC_TEST_MEDIA_CLOCK_KERNEL_H

typedef struct ee_sema {
    int init_count;
    int max_count;
    int option;
} ee_sema_t;

int CreateSema(ee_sema_t *semaphore);
int DeleteSema(int semaphore_id);
int WaitSema(int semaphore_id);
int SignalSema(int semaphore_id);

#endif
