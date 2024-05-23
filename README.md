# High-Performance Distributed Task Scheduler Using gRPC

## Overview
A high-performance distributed task scheduler leveraging gRPC to facilitate efficient communication across multiple nodes. The system optimizes task distribution and execution in a scalable, fault-tolerant, and dynamic manner.

## Features
- **Low Latency and High Throughput**: Using gRPC and shared memory for efficient communication and data sharing.
- **Scalability**: Horizontal scaling by adding worker nodes.
- **Advanced Scheduling Algorithms**: Ensuring optimal task allocation and resource utilization.
- **Fault Tolerance**: Robust mechanisms for fault detection and task reassignment.

## Architecture
The system architecture includes the following components:
- **Client**: Submits tasks and handles asynchronous task responses.
- **Worker**: Executes tasks and sends heartbeat signals to the Register.
- **Process Lifecycle Manager (PLM)**: Manages task queue and task-worker mappings.
- **Main Scheduler**: Allocates tasks using advanced scheduling algorithms.
- **Register**: Maintains a registry of clients and workers, monitoring system health.

![System Architecture](<img width="1352" alt="Screenshot 2024-05-23 at 1 07 34 AM" src="https://github.com/cxx5208/275-Grand-Challenge/assets/76988460/ade5d2c3-3661-4dd6-90c2-734dbf097e4c">)


## Communication Protocol and Data Flow
1. **Task Submission**: Client submits tasks to PLM using `submitTask` gRPC method.
2. **Task Assignment**: Main Scheduler assigns tasks to workers.
3. **Task Execution and Result Reporting**: Workers execute tasks and report results via heartbeat messages.
4. **Result Delivery**: PLM sends task results to the client.
5. **Failure Handling**: Register detects worker failures and informs PLM for task reassignment.

## Performance and Scalability
- **Shared Memory**: Used for task and worker heaps, enabling quick access and updates.
- **gRPC Communication**: Provides efficient, low-latency communication between components.

## Implementation Details
### Shared Memory
- **Initialization**: Using POSIX system calls.
- **Data Structures**: Priority queues for task and worker heaps.
- **Synchronization**: Mutexes and semaphores ensure data consistency.
- **Memory Barriers**: Prevent CPU memory operation reordering.

### gRPC Services
- **Multiple Services**: Dedicated to specific communication pathways.

![Shared Memory and gRPC Communication](https://lucid.app/lucidchart/ca5e9169-b038-400b-8eb4-76e449eccb24/edit?viewport_loc=-1047%2C138%2C4419%2C2392%2C0_0&invitationId=inv_58301903-06af-4a2a-83be-d0cdd904016b)

## Phases and Key Improvements
### Phase 1: Initial Planning and Architecture Design
- Designed initial architecture and implemented basic task submission and worker assignment using gRPC.

### Phase 2: Refinement of PLM and Scheduler Integration
- Enhanced task ID assignment and queue management, integrated advanced scheduling algorithms.

### Phase 3: Optimization and Concurrency Management
- Replaced MPI with shared memory reading, introduced centralized management, enhanced locking mechanisms.

### Phase 4: Consolidation and Integration of PLM into gRPC
- Moved task assignment to Main Scheduler, optimized shared memory usage.

### Phase 5: Final Optimization and Full Communication Establishment
- Implemented comprehensive gRPC communication, real-time monitoring, and sophisticated task reassignment strategies.

## Execution
### Compilation:
```sh
g++ -o plm plm.cpp heap.cpp -lpthread
```
### Execution:
```sh
./plm
```

## References
1. Tanenbaum, A. S., & Van Steen, M. (2007). Distributed Systems: Principles and Paradigms (2nd ed.). Prentice Hall.
2. The gRPC Authors. gRPC: A High-Performance, Open-Source Universal RPC Framework. Google. Available: [gRPC](https://grpc.io/)
3. Stevens, I. (1999). UNIX Network Programming, Volume 2: Interprocess Communications (2nd ed.). Prentice Hall.
4. Kerrisk, M. (2010). The Linux Programming Interface: A Linux and UNIX System Programming Handbook. No Starch Press.
5. Google Inc. gRPC Core Concepts, Architecture and API. Available: [gRPC Documentation](https://grpc.io/docs/guides/)
6. Love, R. (2013). Linux System Programming: Talking Directly to the Kernel and C Library (2nd ed.). O'Reilly Media.

---

For further details, refer to the full project report and codebase.
