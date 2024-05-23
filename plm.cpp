#include <iostream>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include "task_heap.h"
#include "task_worker_mapping.h"
#include "plm.h"

#define SHM_KEY_TASK_HEAP 12345
#define SHM_KEY_TASK_WORKER_MAPPING 12346

void remove_shared_memory(key_t key) {
    int shm_id = shmget(key, 0, 0666);
    if (shm_id >= 0) {
        shmctl(shm_id, IPC_RMID, nullptr);
    }
}

SharedMemory* initialize_shared_memory() {
    // Remove existing shared memory segment
    remove_shared_memory(SHM_KEY_TASK_HEAP);

    // Calculate the size of the shared memory segment
    size_t shm_size = sizeof(SharedMemory);
    std::cout << "Shared memory size for heap: " << shm_size << " bytes" << std::endl;

    int shm_id = shmget(SHM_KEY_TASK_HEAP, shm_size, IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget");
        exit(1);
    }

    void* shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    SharedMemory* shared_memory = (SharedMemory*)shm_addr;

    // Initialize mutex if this is the first process to create the shared memory
    static bool initialized = false;
    if (!initialized) {
        pthread_mutexattr_t mutex_attr;
        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&shared_memory->mutex, &mutex_attr);
        initialized = true;
    }

    // Initialize the task heap
    initialize_task_heap(&shared_memory->heap);
    return shared_memory;
}

SharedMemoryMapping* initialize_shared_memory_mapping() {
    // Remove existing shared memory segment
    remove_shared_memory(SHM_KEY_TASK_WORKER_MAPPING);

    // Calculate the size of the shared memory segment
    size_t shm_size = sizeof(SharedMemoryMapping);
    std::cout << "Shared memory size for task-worker mapping: " << shm_size << " bytes" << std::endl;

    int shm_id = shmget(SHM_KEY_TASK_WORKER_MAPPING, shm_size, IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget");
        exit(1);
    }

    void* shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    SharedMemoryMapping* shared_memory_mapping = (SharedMemoryMapping*)shm_addr;

    // Initialize mutex if this is the first process to create the shared memory
    static bool initialized = false;
    if (!initialized) {
        pthread_mutexattr_t mutex_attr;
        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&shared_memory_mapping->mutex, &mutex_attr);
        initialized = true;
    }

    // Initialize the task-worker mapping list
    shared_memory_mapping->mapping_list.size = 0;
    return shared_memory_mapping;
}

void add_task_to_heap(SharedMemory* shared_memory, const char* taskId, const char* timestamp, int32_t priority, int32_t execution_time, const char* clientIp) {
    pthread_mutex_lock(&shared_memory->mutex);

    std::cout << "Adding task to heap: " << taskId << std::endl;
    task_heap_push(&shared_memory->heap, taskId, timestamp, priority, execution_time, clientIp);

    pthread_mutex_unlock(&shared_memory->mutex);
}

void assign_task_to_worker(SharedMemory* shared_memory, SharedMemoryMapping* shared_memory_mapping, const char* workerId) {
    pthread_mutex_lock(&shared_memory->mutex);
    if (shared_memory->heap.size == 0) {
        pthread_mutex_unlock(&shared_memory->mutex);
        std::cerr << "Heap is empty, no task to assign" << std::endl;
        return;
    }

    TaskElement task = task_heap_pop(&shared_memory->heap);
    pthread_mutex_unlock(&shared_memory->mutex);

    pthread_mutex_lock(&shared_memory_mapping->mutex);

    if (shared_memory_mapping->mapping_list.size == MAX_MAPPINGS) {
        pthread_mutex_unlock(&shared_memory_mapping->mutex);
        std::cerr << "Mapping list is full" << std::endl;
        return;
    }

    TaskWorkerMapping& mapping = shared_memory_mapping->mapping_list.mappings[shared_memory_mapping->mapping_list.size++];
    strncpy(mapping.taskId, task.taskId, sizeof(mapping.taskId) - 1);
    mapping.taskId[sizeof(mapping.taskId) - 1] = '\0';
    strncpy(mapping.workerId, workerId, sizeof(mapping.workerId) - 1);
    mapping.workerId[sizeof(mapping.workerId) - 1] = '\0';
    strncpy(mapping.timestamp, task.timestamp, sizeof(mapping.timestamp) - 1);
    mapping.timestamp[sizeof(mapping.timestamp) - 1] = '\0';
    mapping.priority = task.priority;
    mapping.execution_time = task.execution_time;
    strncpy(mapping.clientIp, task.clientIp, sizeof(mapping.clientIp) - 1);
    mapping.clientIp[sizeof(mapping.clientIp) - 1] = '\0';
    strncpy(mapping.status, "inprogress", sizeof(mapping.status) - 1);
    mapping.status[sizeof(mapping.status) - 1] = '\0';

    std::cout << "Assigned task " << mapping.taskId << " to worker " << mapping.workerId << std::endl;

    pthread_mutex_unlock(&shared_memory_mapping->mutex);
}

void update_task_status(SharedMemory* shared_memory, SharedMemoryMapping* shared_memory_mapping, const char* taskId, const char* new_status) {
    pthread_mutex_lock(&shared_memory_mapping->mutex);

    for (size_t i = 0; i < shared_memory_mapping->mapping_list.size; ++i) {
        if (strcmp(shared_memory_mapping->mapping_list.mappings[i].taskId, taskId) == 0) {
            std::cout << "Updating task " << taskId << " status to " << new_status << std::endl;
            if (strcmp(new_status, "completed") == 0) {
                // Remove task from mapping
                shared_memory_mapping->mapping_list.mappings[i] = shared_memory_mapping->mapping_list.mappings[--shared_memory_mapping->mapping_list.size];
            } else if (strcmp(new_status, "reassign") == 0) {
                // Get task details and remove from mapping
                TaskElement task;
                strncpy(task.taskId, shared_memory_mapping->mapping_list.mappings[i].taskId, sizeof(task.taskId) - 1);
                task.taskId[sizeof(task.taskId) - 1] = '\0';
                strncpy(task.timestamp, shared_memory_mapping->mapping_list.mappings[i].timestamp, sizeof(task.timestamp) - 1);
                task.timestamp[sizeof(task.timestamp) - 1] = '\0';
                task.priority = shared_memory_mapping->mapping_list.mappings[i].priority + 1;
                task.execution_time = shared_memory_mapping->mapping_list.mappings[i].execution_time;
                strncpy(task.clientIp, shared_memory_mapping->mapping_list.mappings[i].clientIp, sizeof(task.clientIp) - 1);
                task.clientIp[sizeof(task.clientIp) - 1] = '\0';

                shared_memory_mapping->mapping_list.mappings[i] = shared_memory_mapping->mapping_list.mappings[--shared_memory_mapping->mapping_list.size];

                // Add task back to heap with increased priority
                pthread_mutex_lock(&shared_memory->mutex);
                task_heap_push(&shared_memory->heap, task.taskId, task.timestamp, task.priority, task.execution_time, task.clientIp);
                pthread_mutex_unlock(&shared_memory->mutex);
            } else {
                // Update task status
                strncpy(shared_memory_mapping->mapping_list.mappings[i].status, new_status, sizeof(shared_memory_mapping->mapping_list.mappings[i].status) - 1);
                shared_memory_mapping->mapping_list.mappings[i].status[sizeof(shared_memory_mapping->mapping_list.mappings[i].status) - 1] = '\0';
            }
            break;
        }
    }

    pthread_mutex_unlock(&shared_memory_mapping->mutex);
}

void print_heap(SharedMemory* shared_memory) {
    pthread_mutex_lock(&shared_memory->mutex);
    std::cout << "Current heap state:" << std::endl;
    for (size_t i = 0; i < shared_memory->heap.size; ++i) {
        std::cout << "Task ID: " << shared_memory->heap.data[i].taskId
                  << ", Priority: " << shared_memory->heap.data[i].priority
                  << ", Timestamp: " << shared_memory->heap.data[i].timestamp
                  << ", Execution Time: " << shared_memory->heap.data[i].execution_time
                  << ", Client IP: " << shared_memory->heap.data[i].clientIp << std::endl;
    }
    pthread_mutex_unlock(&shared_memory->mutex);
}

int main() {
    SharedMemory* shared_memory = initialize_shared_memory();
    SharedMemoryMapping* shared_memory_mapping = initialize_shared_memory_mapping();

    // Example usage: add tasks to heap
    add_task_to_heap(shared_memory, "task1", "2024-05-16T12:00:00Z", 1, 100, "192.168.1.1");
    add_task_to_heap(shared_memory, "task2", "2024-05-16T12:05:00Z", 2, 200, "192.168.1.2");

    // Scheduler: assign tasks to workers
    assign_task_to_worker(shared_memory, shared_memory_mapping, "worker1");
    assign_task_to_worker(shared_memory, shared_memory_mapping, "worker2");

    // Registry: update task status
    update_task_status(shared_memory, shared_memory_mapping, "task1", "completed");
    update_task_status(shared_memory, shared_memory_mapping, "task2", "reassign");

    // Print the task-worker mappings to verify
    pthread_mutex_lock(&shared_memory_mapping->mutex);
    std::cout << "Current task-worker mappings:" << std::endl;
    if (shared_memory_mapping->mapping_list.size == 0) {
        std::cout << "No tasks are currently assigned to workers." << std::endl;
    } else {
        for (size_t i = 0; i < shared_memory_mapping->mapping_list.size; ++i) {
            std::cout << "Task ID: " << shared_memory_mapping->mapping_list.mappings[i].taskId
                      << ", Worker ID: " << shared_memory_mapping->mapping_list.mappings[i].workerId
                      << ", Timestamp: " << shared_memory_mapping->mapping_list.mappings[i].timestamp
                      << ", Priority: " << shared_memory_mapping->mapping_list.mappings[i].priority
                      << ", Execution Time: " << shared_memory_mapping->mapping_list.mappings[i].execution_time
                      << ", Client IP: " << shared_memory_mapping->mapping_list.mappings[i].clientIp
                      << ", Status: " << shared_memory_mapping->mapping_list.mappings[i].status << std::endl;
        }
    }
    pthread_mutex_unlock(&shared_memory_mapping->mutex);

    // Print the heap state to verify reassignments
    print_heap(shared_memory);

    return 0;
}
