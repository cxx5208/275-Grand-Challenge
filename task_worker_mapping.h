#ifndef TASK_WORKER_MAPPING_H
#define TASK_WORKER_MAPPING_H

#include <stdint.h>
#include "task_heap.h" // Assuming task_heap.h defines TaskElement

#define MAX_MAPPINGS 256

struct TaskWorkerMapping {
    char taskId[32];
    char workerId[32];
    char timestamp[32];
    int32_t priority;
    int32_t execution_time;
    char clientIp[64];  // Added clientIp field
    char status[16]; // Status field added
};

struct TaskWorkerMappingList {
    TaskWorkerMapping mappings[MAX_MAPPINGS];
    size_t size;
};

// Forward declaration of SharedMemory and SharedMemoryMapping structures
struct SharedMemory;
struct SharedMemoryMapping;

// Function declarations for task-worker mapping
TaskWorkerMappingList* initialize_task_worker_mapping(void* shm_addr);
void add_task_worker_mapping(TaskWorkerMappingList* list, const TaskElement& task, const char* workerId);
void update_task_status(SharedMemory* shared_memory, SharedMemoryMapping* shared_memory_mapping, const char* taskId, const char* new_status);

#endif // TASK_WORKER_MAPPING_H
