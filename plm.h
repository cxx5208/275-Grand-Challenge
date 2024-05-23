#ifndef PLM_H
#define PLM_H

#include <pthread.h>
#include "task_heap.h"
#include "task_worker_mapping.h"

struct SharedMemory {
    pthread_mutex_t mutex;
    TaskHeapData heap;
};

struct SharedMemoryMapping {
    pthread_mutex_t mutex;
    TaskWorkerMappingList mapping_list;
};

void remove_shared_memory(key_t key);

SharedMemory* initialize_shared_memory();
SharedMemoryMapping* initialize_shared_memory_mapping();

void add_task_to_heap(SharedMemory* shared_memory, const char* taskId, const char* timestamp, int32_t priority, int32_t execution_time, const char* clientIp);
void assign_task_to_worker(SharedMemory* shared_memory, SharedMemoryMapping* shared_memory_mapping, const char* workerId);
void update_task_status(SharedMemory* shared_memory, SharedMemoryMapping* shared_memory_mapping, const char* taskId, const char* new_status);

void print_heap(SharedMemory* shared_memory);

#endif // PLM_H