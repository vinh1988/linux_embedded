# POSIX Shared Memory and Semaphore Example

This project demonstrates inter-process communication (IPC) using POSIX shared memory and semaphores.

## Overview

This example consists of two processes: a writer and a reader. The writer process writes data to a shared memory segment, and the reader process reads data from the same shared memory segment. Semaphores are used to synchronize access to the shared memory segment, ensuring that the writer and reader processes do not interfere with each other.

## Youtube Demo
[Click here ](https://youtu.be/3fawAVk4yV8)

## General Graph structure
![Workflow in General](semaphore.png)

## Mermaid graph structure
![Workflow Detail](semaphore_workflow.png)

## Building the example

To build the example, run the following command:

```
make
```

This will create two executable files: `reader` and `writer`.

## Semaphores

A semaphore is a synchronization primitive that can be used to control access to a shared resource. In this example, semaphores are used to synchronize access to the shared memory segment, ensuring that the writer and reader processes do not interfere with each other.

### Types of Semaphores

There are two types of semaphores:

*   **Named semaphores:** These semaphores are identified by a name and can be shared between unrelated processes.
*   **Unnamed semaphores:** These semaphores are not identified by a name and can only be shared between related processes (e.g., processes created by `fork()`).

This example uses unnamed semaphores because the writer and reader processes are related (they are both created by the same parent process).

### Semaphore Operations

The two main operations that can be performed on a semaphore are:

*   **wait()**: This operation decrements the semaphore value. If the semaphore value is zero, the process will block until the semaphore value becomes greater than zero.
*   **post()**: This operation increments the semaphore value. If there are any processes blocked on the semaphore, one of them will be unblocked.

In this example, the writer process calls `wait()` before writing to the shared memory segment, and `post()` after writing to the shared memory segment. The reader process calls `wait()` before reading from the shared memory segment, and `post()` after reading from the shared memory segment.

### Aspects of Semaphores

| Aspect                  | Description                                                                    | Example                                                                 |
| ----------------------- | ------------------------------------------------------------------------------ | ----------------------------------------------------------------------- |
| Mutual Exclusion (Mutex) | Make sure only one process or thread is in the critical section at a time.     | Lock shared memory before writing.                                      |
| Synchronization         | Make processes coordinate their execution (i.e., one must happen after another). | Producer waits until consumer reads.                                    |
| Counting resource       | Track availability of multiple identical resources.                            | 5 printer slots, 10 database connections.                               |
| Blocking and Waking     | If resource unavailable, wait (block); when available, wake processes.          | Reader blocks until writer posts.                                       |
| Fairness / Deadlock     | If misused, semaphores can cause deadlocks or starvation (one process waits forever). | Careful coding required!                                              |

## Running the example

To run the example, first run the `writer` process in one terminal:

```
./writer
```

Then, run the `reader` process in another terminal:

```
./reader
```

The `writer` process will write data to the shared memory segment, and the `reader` process will read data from the shared memory segment and print it to the console.

## Source files

*   `src/writer.c`: This file contains the source code for the writer process.
*   `src/reader.c`: This file contains the source code for the reader process.
*   `include/shared.h`: This file contains the definitions for the shared memory segment and the semaphores.
*   `test/test_shared.c`: This file contains unit tests for the shared memory and semaphore functionality.
*   `Makefile`: This file contains the build instructions for the example.
