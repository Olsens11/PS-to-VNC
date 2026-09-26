#include "audio/ps2_runtime.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_MAX_SEMAS 32
#define TEST_MAX_STATUS 16

typedef struct test_sema {
    int used;
    int count;
    int maximum;
} test_sema_t;

static test_sema_t g_semas[TEST_MAX_SEMAS];
static int g_next_sema_id;
static int g_create_sema_call_count;
static int g_create_sema_fail_call;
static int g_delete_sema_fail_id;
static int g_wait_sema_calls;
static int g_wait_sema_fail_once;
static int g_delay_calls;
static int g_delay_fail_once;

static ee_thread_t g_thread;
static int g_create_thread_fail;
static int g_start_thread_fail;
static int g_delete_thread_fail_once;
static int g_auto_run_thread;
static int g_thread_id;
static int g_exit_thread_calls;
static int g_worker_calls;

static int g_status_values[TEST_MAX_STATUS];
static int g_status_count;
static int g_status_index;
static int g_refer_fail_once;

static int g_load_module_calls;
static int g_exec_module_calls;
static int g_load_module_fail_once;
static int g_exec_module_fail_once;

char _gp;
unsigned char AUDSRV_irx[4] = {1u, 2u, 3u, 4u};
unsigned int size_AUDSRV_irx = sizeof(AUDSRV_irx);

static void reset_fakes(void)
{
    memset(g_semas, 0, sizeof(g_semas));
    memset(&g_thread, 0, sizeof(g_thread));
    memset(g_status_values, 0, sizeof(g_status_values));

    g_next_sema_id = 1;
    g_create_sema_call_count = 0;
    g_create_sema_fail_call = 0;
    g_delete_sema_fail_id = -1;
    g_wait_sema_calls = 0;
    g_wait_sema_fail_once = 0;
    g_delay_calls = 0;
    g_delay_fail_once = 0;

    g_create_thread_fail = 0;
    g_start_thread_fail = 0;
    g_delete_thread_fail_once = 0;
    g_auto_run_thread = 0;
    g_thread_id = 17;
    g_exit_thread_calls = 0;
    g_worker_calls = 0;

    g_status_count = 0;
    g_status_index = 0;
    g_refer_fail_once = 0;

    g_load_module_calls = 0;
    g_exec_module_calls = 0;
    g_load_module_fail_once = 0;
    g_exec_module_fail_once = 0;
}

int CreateSema(ee_sema_t *semaphore)
{
    int id;
    g_create_sema_call_count++;
    if (g_create_sema_fail_call == g_create_sema_call_count)
        return -1;
    id = g_next_sema_id++;
    assert(id < TEST_MAX_SEMAS);
    g_semas[id].used = 1;
    g_semas[id].count = semaphore->init_count;
    g_semas[id].maximum = semaphore->max_count;
    return id;
}

int DeleteSema(int semaphore_id)
{
    if (semaphore_id == g_delete_sema_fail_id) {
        g_delete_sema_fail_id = -1;
        return -1;
    }
    if (semaphore_id <= 0 || semaphore_id >= TEST_MAX_SEMAS ||
        !g_semas[semaphore_id].used)
        return -1;
    g_semas[semaphore_id].used = 0;
    return 0;
}

int WaitSema(int semaphore_id)
{
    g_wait_sema_calls++;
    if (g_wait_sema_fail_once) {
        g_wait_sema_fail_once = 0;
        return -1;
    }
    if (semaphore_id <= 0 || semaphore_id >= TEST_MAX_SEMAS ||
        !g_semas[semaphore_id].used || g_semas[semaphore_id].count <= 0)
        return -1;
    g_semas[semaphore_id].count--;
    return 0;
}

int SignalSema(int semaphore_id)
{
    if (semaphore_id <= 0 || semaphore_id >= TEST_MAX_SEMAS ||
        !g_semas[semaphore_id].used ||
        g_semas[semaphore_id].count >= g_semas[semaphore_id].maximum)
        return -1;
    g_semas[semaphore_id].count++;
    return 0;
}

int DelayThread(unsigned int microseconds)
{
    assert(microseconds == 1u);
    g_delay_calls++;
    if (g_delay_fail_once) {
        g_delay_fail_once = 0;
        return -1;
    }
    return 0;
}

int CreateThread(ee_thread_t *thread)
{
    if (g_create_thread_fail)
        return -1;
    g_thread = *thread;
    return g_thread_id;
}

int StartThread(int thread_id, void *argument)
{
    void (*entry)(void *);
    assert(thread_id == g_thread_id);
    if (g_start_thread_fail)
        return -1;
    if (g_auto_run_thread) {
        entry = (void (*)(void *))g_thread.func;
        entry(argument);
    }
    return 0;
}

int ReferThreadStatus(int thread_id, ee_thread_status_t *status)
{
    assert(thread_id == g_thread_id);
    if (g_refer_fail_once) {
        g_refer_fail_once = 0;
        return -1;
    }
    assert(g_status_index < g_status_count);
    status->status = g_status_values[g_status_index++];
    return 0;
}

int DeleteThread(int thread_id)
{
    assert(thread_id == g_thread_id);
    if (g_delete_thread_fail_once) {
        g_delete_thread_fail_once = 0;
        return -1;
    }
    return 0;
}

void ExitThread(void)
{
    g_exit_thread_calls++;
}

int SifLoadModule(const char *path, int argument_length, const char *arguments)
{
    g_load_module_calls++;
    assert(strcmp(path, "rom0:LIBSD") == 0);
    assert(argument_length == 0);
    assert(arguments == NULL);
    if (g_load_module_fail_once) {
        g_load_module_fail_once = 0;
        return -1;
    }
    return 0;
}

int SifExecModuleBuffer(
    void *module_buffer,
    unsigned int module_size,
    int argument_length,
    const char *arguments,
    int *result)
{
    g_exec_module_calls++;
    assert(module_buffer == AUDSRV_irx);
    assert(module_size == size_AUDSRV_irx);
    assert(argument_length == 0);
    assert(arguments == NULL);
    assert(result == NULL);
    if (g_exec_module_fail_once) {
        g_exec_module_fail_once = 0;
        return -1;
    }
    return 0;
}

static void worker_entry(void *argument)
{
    int *value = (int *)argument;
    g_worker_calls++;
    (*value)++;
}

static void configure_statuses(int first, int second, int third, int count)
{
    assert(count >= 1 && count <= 3);
    g_status_values[0] = first;
    g_status_values[1] = second;
    g_status_values[2] = third;
    g_status_count = count;
    g_status_index = 0;
}

static void init_runtime(
    pstvnc_audio_ps2_runtime_t *runtime,
    pstvnc_audio_session_memory_ops_t *memory_ops,
    pstvnc_audio_session_thread_ops_t *thread_ops,
    pstvnc_audio_session_sync_t *sync)
{
    assert(pstvnc_audio_ps2_runtime_init(runtime) == 0);
    assert(runtime->resources_owned == 1);
    assert(runtime->initialized == 1);
    assert(pstvnc_audio_ps2_runtime_operations(
        runtime, memory_ops, thread_ops, sync) == 0);
}

static void test_resident_preparation(void)
{
    pstvnc_audio_ps2_resident_t resident = {0, 0};

    reset_fakes();
    g_load_module_fail_once = 1;
    assert(pstvnc_audio_ps2_resident_prepare(&resident) == -1);
    assert(resident.libsd_loaded == 0);
    assert(resident.audsrv_loaded == 0);
    assert(g_load_module_calls == 1);
    assert(g_exec_module_calls == 0);

    assert(pstvnc_audio_ps2_resident_prepare(&resident) == 0);
    assert(resident.libsd_loaded == 1);
    assert(resident.audsrv_loaded == 1);
    assert(g_load_module_calls == 2);
    assert(g_exec_module_calls == 1);

    assert(pstvnc_audio_ps2_resident_prepare(&resident) == 0);
    assert(g_load_module_calls == 2);
    assert(g_exec_module_calls == 1);

    memset(&resident, 0, sizeof(resident));
    reset_fakes();
    g_exec_module_fail_once = 1;
    assert(pstvnc_audio_ps2_resident_prepare(&resident) == -1);
    assert(resident.libsd_loaded == 1);
    assert(resident.audsrv_loaded == 0);
    assert(g_load_module_calls == 1);
    assert(g_exec_module_calls == 1);

    assert(pstvnc_audio_ps2_resident_prepare(&resident) == 0);
    assert(resident.libsd_loaded == 1);
    assert(resident.audsrv_loaded == 1);
    assert(g_load_module_calls == 1);
    assert(g_exec_module_calls == 2);
}

static void test_memory_and_release(void)
{
    pstvnc_audio_ps2_runtime_t runtime;
    pstvnc_audio_session_memory_ops_t memory_ops;
    pstvnc_audio_session_thread_ops_t thread_ops;
    pstvnc_audio_session_sync_t sync;
    void *buffer;
    void *stack;

    reset_fakes();
    init_runtime(&runtime, &memory_ops, &thread_ops, &sync);

    buffer = memory_ops.allocate(memory_ops.context, 4096u, 1u);
    stack = memory_ops.allocate(memory_ops.context, 16384u, 16u);
    assert(buffer != NULL);
    assert(stack != NULL);
    assert(((uintptr_t)stack % 16u) == 0u);
    assert(runtime.live_allocations == 2u);
    assert(memory_ops.allocate(memory_ops.context, (size_t)-1, 16u) == NULL);
    assert(runtime.live_allocations == 2u);
    assert(pstvnc_audio_ps2_runtime_release(&runtime) == -1);

    memory_ops.release(memory_ops.context, stack);
    assert(runtime.live_allocations == 1u);
    memory_ops.release(memory_ops.context, stack);
    assert(runtime.live_allocations == 1u);
    memory_ops.release(memory_ops.context, buffer);
    assert(runtime.live_allocations == 0u);

    assert(sync.lock(sync.context) == 0);
    assert(sync.unlock(sync.context) == 0);
    assert(pstvnc_audio_ps2_runtime_release(&runtime) == 0);
}

static void test_start_failure_cleanup_is_retryable(void)
{
    pstvnc_audio_ps2_runtime_t runtime;
    pstvnc_audio_session_memory_ops_t memory_ops;
    pstvnc_audio_session_thread_ops_t thread_ops;
    pstvnc_audio_session_sync_t sync;
    int thread_id = -1;
    int value = 0;

    reset_fakes();
    init_runtime(&runtime, &memory_ops, &thread_ops, &sync);

    g_create_thread_fail = 1;
    assert(thread_ops.create(thread_ops.context, worker_entry, &value,
        (void *)(uintptr_t)0x1000u, 16384u, 65, &thread_id) == -1);
    assert(runtime.thread_slot_active == 0);

    g_create_thread_fail = 0;
    assert(thread_ops.create(thread_ops.context, worker_entry, &value,
        (void *)(uintptr_t)0x1000u, 16384u, 65, &thread_id) == 0);
    assert(runtime.thread_slot_active == 1);

    g_start_thread_fail = 1;
    assert(thread_ops.start(thread_ops.context, thread_id) == -1);
    assert(runtime.thread_started == 0);

    configure_statuses(THS_RUN, 0, 0, 1);
    assert(thread_ops.destroy(thread_ops.context, thread_id) == -1);
    assert(runtime.thread_slot_active == 1);

    configure_statuses(THS_DORMANT, 0, 0, 1);
    assert(thread_ops.destroy(thread_ops.context, thread_id) == 0);
    assert(runtime.thread_slot_active == 0);
    assert(pstvnc_audio_ps2_runtime_release(&runtime) == 0);
}

static void test_completion_dormancy_and_delete_retry(void)
{
    pstvnc_audio_ps2_runtime_t runtime;
    pstvnc_audio_session_memory_ops_t memory_ops;
    pstvnc_audio_session_thread_ops_t thread_ops;
    pstvnc_audio_session_sync_t sync;
    int thread_id = -1;
    int value = 0;

    reset_fakes();
    init_runtime(&runtime, &memory_ops, &thread_ops, &sync);

    assert(thread_ops.create(thread_ops.context, worker_entry, &value,
        (void *)(uintptr_t)0x1000u, 16384u, 65, &thread_id) == 0);

    g_auto_run_thread = 1;
    assert(thread_ops.start(thread_ops.context, thread_id) == 0);
    assert(g_worker_calls == 1);
    assert(value == 1);
    assert(g_exit_thread_calls == 1);
    assert(pstvnc_audio_ps2_runtime_release(&runtime) == -1);

    configure_statuses(THS_RUN, THS_RUN, THS_DORMANT, 3);
    assert(thread_ops.join(thread_ops.context, thread_id) == 0);
    assert(runtime.completion_observed == 1);
    assert(runtime.thread_dormant_proven == 1);
    assert(g_status_index == 3);
    assert(g_delay_calls == 2);

    g_delete_thread_fail_once = 1;
    assert(thread_ops.destroy(thread_ops.context, thread_id) == -1);
    assert(runtime.thread_slot_active == 1);
    assert(runtime.thread_dormant_proven == 1);
    assert(thread_ops.destroy(thread_ops.context, thread_id) == 0);
    assert(pstvnc_audio_ps2_runtime_release(&runtime) == 0);
}

static void test_completion_observation_survives_status_failure(void)
{
    pstvnc_audio_ps2_runtime_t runtime;
    pstvnc_audio_session_memory_ops_t memory_ops;
    pstvnc_audio_session_thread_ops_t thread_ops;
    pstvnc_audio_session_sync_t sync;
    int thread_id = -1;
    int value = 0;
    int wait_calls_after_failure;

    reset_fakes();
    init_runtime(&runtime, &memory_ops, &thread_ops, &sync);

    assert(thread_ops.create(thread_ops.context, worker_entry, &value,
        (void *)(uintptr_t)0x1000u, 16384u, 65, &thread_id) == 0);
    g_auto_run_thread = 1;
    assert(thread_ops.start(thread_ops.context, thread_id) == 0);

    g_refer_fail_once = 1;
    assert(thread_ops.join(thread_ops.context, thread_id) == -1);
    assert(runtime.completion_observed == 1);
    wait_calls_after_failure = g_wait_sema_calls;

    configure_statuses(THS_DORMANT, 0, 0, 1);
    assert(thread_ops.join(thread_ops.context, thread_id) == 0);
    assert(g_wait_sema_calls == wait_calls_after_failure);
    assert(runtime.thread_dormant_proven == 1);

    assert(thread_ops.destroy(thread_ops.context, thread_id) == 0);
    assert(pstvnc_audio_ps2_runtime_release(&runtime) == 0);
}

static void test_partial_runtime_release_retry(void)
{
    pstvnc_audio_ps2_runtime_t runtime;
    pstvnc_audio_session_memory_ops_t memory_ops;
    pstvnc_audio_session_thread_ops_t thread_ops;
    pstvnc_audio_session_sync_t sync;
    int retained_lock_id;

    reset_fakes();
    init_runtime(&runtime, &memory_ops, &thread_ops, &sync);
    retained_lock_id = runtime.session_lock_sema_id;
    g_delete_sema_fail_id = retained_lock_id;

    assert(pstvnc_audio_ps2_runtime_release(&runtime) == -1);
    assert(runtime.resources_owned == 1);
    assert(runtime.completion_sema_id == -1);
    assert(runtime.session_lock_sema_id == retained_lock_id);

    assert(pstvnc_audio_ps2_runtime_release(&runtime) == 0);
    assert(runtime.session_lock_sema_id == -1);
    assert(runtime.resources_owned == 0);
}

int main(void)
{
    test_resident_preparation();
    test_memory_and_release();
    test_start_failure_cleanup_is_retryable();
    test_completion_dormancy_and_delete_retry();
    test_completion_observation_survives_status_failure();
    test_partial_runtime_release_retry();

    puts("audio_ps2_runtime_test: PASS");
    return 0;
}
