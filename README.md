# High-Performance Distributed Task Scheduler Using gRPC

## Table of Contents
1. [Project Overview](#project-overview)
2. [System Architecture](#system-architecture)
3. [Communication Protocol and Data Flow](#communication-protocol-and-data-flow)
4. [Performance and Scalability](#performance-and-scalability)
5. [Implementation Details](#implementation-details)
6. [Phase Details and Key Improvements](#phase-details-and-key-improvements)
7. [Execution Results](#execution-results)
8. [References](#references)

---

## Project Overview
This project involves the development of a high-performance distributed task scheduler leveraging gRPC (Google Remote Procedure Call) for inter-process communication. The primary objective is to optimize task distribution and execution across multiple nodes, ensuring scalability, fault tolerance, and efficiency in modern distributed computing environments.

---

## System Architecture
### Core Components
- **Client**: Interface for task submission via gRPC. Handles asynchronous task responses.
- **Worker**: Executes assigned tasks, transmits heartbeat signals to the Register.
- **Process Lifecycle Manager (PLM)**: Manages task queue, assigns task IDs, maintains mappings between tasks and workers.
- **Main Scheduler**: Allocates tasks based on advanced scheduling algorithms, utilizing task and worker heaps stored in shared memory.
- **Register**: Monitors system health through heartbeat signals, updates worker status, manages fault detection and recovery.

### Architecture Diagram
![System Architecture](https://lucid.app/lucidchart/ca5e9169-b038-400b-8eb4-76e449eccb24/edit?viewport_loc=-1047%2C138%2C4419%2C2392%2C0_0&invitationId=inv_58301903-06af-4a2a-83be-d0cdd904016b)

---

## Communication Protocol and Data Flow
1. **Task Submission**: The Client submits tasks to the PLM using the `submitTask` gRPC method. The PLM assigns a unique task ID and enqueues the task in the task heap.
2. **Task Assignment**: The Main Scheduler dequeues tasks from the task heap, selects appropriate workers from the worker heap, updates the task-worker mapping, and dispatches tasks to workers via gRPC.
3. **Task Execution and Result Reporting**: Workers execute tasks and report results within heartbeat messages to the Register. The Register updates the task status and notifies the PLM.
4. **Result Delivery**: The PLM retrieves task results from the task-worker mapping and sends them back to the Client using gRPC.
5. **Failure Handling**: The Register detects worker failures through missing heartbeats, marks workers as dead, updates the worker heap, and informs the PLM for task reassignment.

---

## Performance and Scalability
### Key Features
- **Low Latency and High Throughput**: Achieved using gRPC for inter-process communication and shared memory for data storage and access.
- **Horizontal Scalability**: System can scale by adding additional worker nodes, dynamically distributing load.
- **Optimal Load Balancing**: Advanced scheduling algorithms ensure efficient task allocation and resource utilization.

### Performance Enhancements
- **Shared Memory**: Utilized for task and worker heaps, providing quick access and updates, reducing inter-process communication overhead.
- **gRPC Communication**: Ensures efficient, low-latency communication between system components.

---

## Implementation Details
### Shared Memory Utilization
- **Initialization**: Shared memory segments initialized using POSIX system calls (`shmget`, `shmat`, `shmctl`). Memory allocation planned to ensure contiguous space for task and worker heaps.
- **Data Structures**: Task heaps implemented as priority queues for efficient task management. Worker heaps managed availability and load using similar priority queue structures.
- **Synchronization**: Mutexes and semaphores used to synchronize access to shared memory. Atomic operations employed for critical sections to enable lock-free, thread-safe modifications.
- **Memory Barriers**: Prevent CPU memory operation reordering, ensuring immediate visibility of changes across processes.
- **Error Handling**: Robust mechanisms to manage memory allocation failures and access violations, with fallback procedures for graceful degradation or reinitialization of shared memory segments.

### gRPC Services
- **Multiple gRPC Services**: Implemented to facilitate specific communication pathways for task submissions, task assignment, heartbeat signals, and worker status updates.

### Shared Memory and gRPC Communication Diagram
![Shared Memory and gRPC Communication](https://lucid.app/lucidchart/ca5e9169-b038-400b-8eb4-76e449eccb24/edit?viewport_loc=-1047%2C138%2C4419%2C2392%2C0_0&invitationId=inv_58301903-06af-4a2a-83be-d0cdd904016b)

---

## Phase Details and Key Improvements
### Phase 1: Initial Planning and Architecture Design
- **Initial Architecture**: Designed with Client, Worker, PLM, Main Scheduler, and Register components.
- **Basic Implementation**: Task submission and worker assignment using gRPC.

### Phase 2: Refinement of PLM and Scheduler Integration
- **Enhancements**: Improved task ID assignment, queue management, integrated advanced scheduling algorithms.

### Phase 3: Optimization and Concurrency Management
- **Concurrency Improvements**: Replaced MPI with shared memory reading, introduced a singleton class for centralized management, enhanced locking mechanisms.

### Phase 4: Consolidation and Integration of PLM into gRPC
- **Integration**: Moved task assignment to Main Scheduler, optimized shared memory usage, eliminated the need for separate singleton classes.

### Phase 5: Final Optimization and Full Communication Establishment
- **Comprehensive gRPC Communication**: Implemented across all components.
- **Real-Time Monitoring**: Enhanced with periodic heartbeat signals, developed sophisticated task reassignment strategies.

---

## Execution Results
### Command:
```sh
g++ -o plm plm.cpp heap.cpp -lpthread
./plm
```

### Outcome
- **Efficiency**: Demonstrated efficient task scheduling and execution across multiple nodes.
- **Real-Time Monitoring**: Achieved real-time status updates and task reassignment.
- **Fault Tolerance**: Ensured robust fault detection and recovery mechanisms.

---

## References
1. Tanenbaum, A. S., & Van Steen, M. (2007). Distributed Systems: Principles and Paradigms (2nd ed.). Prentice Hall.
2. The gRPC Authors. gRPC: A High-Performance, Open-Source Universal RPC Framework. Google. Available: [gRPC](https://grpc.io/)
3. Stevens, I. (1999). UNIX Network Programming, Volume 2: Interprocess Communications (2nd ed.). Prentice Hall.
4. Kerrisk, M. (2010). The Linux Programming Interface: A Linux and UNIX System Programming Handbook. No Starch Press.
5. Google Inc. gRPC Core Concepts, Architecture and API. Available: [gRPC Documentation](https://grpc.io/docs/guides/)
6. Love, R. (2013). Linux System Programming: Talking Directly to the Kernel and C Library (2nd ed.). O'Reilly Media.

---

