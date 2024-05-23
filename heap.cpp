#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include "task_heap.h"

#define SHM_KEY 12345 // Use a unique integer key for shared memory

// Function definitions for task heap

TaskHeapData* initialize_task_heap(void* shm_addr) {
    TaskHeapData* heap = (TaskHeapData*)shm_addr;
    heap->size = 0;
    return heap;
}

void swap(TaskElement& a, TaskElement& b) {
    TaskElement temp = a;
    a = b;
    b = temp;
}

void task_heapify_up(TaskHeapData* heap, size_t index) {
    while (index != 0 && heap->data[(index - 1) / 2].priority_difference() < heap->data[index].priority_difference()) {
        swap(heap->data[index], heap->data[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}

void task_heapify_down(TaskHeapData* heap, size_t index) {
    size_t largest = index;
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;

    if (left < heap->size && heap->data[left].priority_difference() > heap->data[largest].priority_difference()) {
        largest = left;
    }
    if (right < heap->size && heap->data[right].priority_difference() > heap->data[largest].priority_difference()) {
        largest = right;
    }
    if (largest != index) {
        swap(heap->data[index], heap->data[largest]);
        task_heapify_down(heap, largest);
    }
}

void task_heap_push(TaskHeapData* heap, const char* taskId, const char* timestamp, int32_t priority, int32_t execution_time, const char* clientIp) {
    if (heap->size == MAX_TASKS) {
        fprintf(stderr, "Heap is full\n");
        return;
    }

    strncpy(heap->data[heap->size].taskId, taskId, sizeof(heap->data[heap->size].taskId) - 1);
    heap->data[heap->size].taskId[sizeof(heap->data[heap->size].taskId) - 1] = '\0';
    strncpy(heap->data[heap->size].timestamp, timestamp, sizeof(heap->data[heap->size].timestamp) - 1);
    heap->data[heap->size].timestamp[sizeof(heap->data[heap->size].timestamp) - 1] = '\0';
    heap->data[heap->size].priority = priority;
    heap->data[heap->size].execution_time = execution_time;
    strncpy(heap->data[heap->size].clientIp, clientIp, sizeof(heap->data[heap->size].clientIp) - 1);
    heap->data[heap->size].clientIp[sizeof(heap->data[heap->size].clientIp) - 1] = '\0';
    task_heapify_up(heap, heap->size);
    heap->size++;
}

TaskElement task_heap_pop(TaskHeapData* heap) {
    if (heap->size == 0) {
        fprintf(stderr, "Heap is empty\n");
        exit(1);
    }

    TaskElement root = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    task_heapify_down(heap, 0);

    return root;
}

ssize_t find_task_index_by_id(TaskHeapData* heap, const char* taskId) {
    for (size_t i = 0; i < heap->size; ++i) {
        if (strncmp(heap->data[i].taskId, taskId, sizeof(heap->data[i].taskId)) == 0) {
            return i;
        }
    }
    return -1;
}

bool remove_task_by_id(TaskHeapData* heap, const char* taskId) {
    ssize_t index = find_task_index_by_id(heap, taskId);
    if (index == -1) {
        fprintf(stderr, "Task ID not found\n");
        return false;
    }

    heap->data[index] = heap->data[heap->size - 1];
    heap->size--;

    if (index < heap->size) {
        task_heapify_down(heap, index);
        task_heapify_up(heap, index);
    }

    return true;
}
