/**
 * @file process_scheduler.c
 * @brief A simple Round Robin process scheduler simulation.
 *
 * This program simulates a basic process scheduler using the Round Robin
 * algorithm. It manages a list of processes, each with a Process Control Block (PCB),
 * and simulates their execution in time slices (quanta).
 *
 * Concepts Demonstrated:
 * - Process Control Block (PCB) structure.
 * - Ready queue (implemented as a simple circular array-based queue).
 * - Round Robin scheduling logic.
 * - Simulation of process states (simplified to NEW, READY, RUNNING, TERMINATED).
 * - Basic time quantum management.
 *
 * Limitations for simplicity:
 * - No actual multi-threading or multi-processing; it's a simulation.
 * - No I/O blocking or other complex states (WAITING, SUSPENDED).
 * - Processes have a predefined burst time and don't perform real work.
 * - Error handling is minimal.
 */

#include <stdio.h>
#include <stdlib.h> // For malloc, free, exit
#include <string.h> // For strcpy
#include <stdbool.h> // For bool type

#define MAX_PROCESSES 10 // Maximum number of processes the scheduler can handle
#define TIME_QUANTUM 3   // Time slice for Round Robin (in arbitrary time units)
#define MAX_QUEUE_SIZE MAX_PROCESSES

// Process States
typedef enum {
    STATE_NEW,
    STATE_READY,
    STATE_RUNNING,
    STATE_TERMINATED
} ProcessState;

const char* get_state_string(ProcessState state) {
    switch (state) {
        case STATE_NEW: return "NEW";
        case STATE_READY: return "READY";
        case STATE_RUNNING: return "RUNNING";
        case STATE_TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

// Process Control Block (PCB)
typedef struct {
    int process_id;
    char process_name[50];
    ProcessState state;
    int burst_time;      // Total CPU time required by the process
    int remaining_time;  // Remaining CPU time
    int arrival_time;    // Time the process arrives in the system (not used actively in RR here, but good for future)
    // Add other fields like priority, memory usage, etc. for more complex schedulers
} PCB;

// Simple Circular Queue for Ready Processes
typedef struct {
    PCB* buffer[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int count;
} ReadyQueue;

// --- Queue Operations ---
void init_queue(ReadyQueue *q) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

bool is_queue_empty(ReadyQueue *q) {
    return q->count == 0;
}

bool is_queue_full(ReadyQueue *q) {
    return q->count == MAX_QUEUE_SIZE;
}

void enqueue(ReadyQueue *q, PCB *process) {
    if (is_queue_full(q)) {
        // fprintf(stderr, "Error: Ready queue is full. Cannot enqueue process %d.\n", process->process_id);
        return;
    }
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->buffer[q->rear] = process;
    q->count++;
    process->state = STATE_READY;
    // printf("Process %d (%s) enqueued. State: %s\n", process->process_id, process->process_name, get_state_string(process->state));
}

PCB* dequeue(ReadyQueue *q) {
    if (is_queue_empty(q)) {
        // fprintf(stderr, "Error: Ready queue is empty. Cannot dequeue.\n");
        return NULL;
    }
    PCB *process = q->buffer[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->count--;
    // printf("Process %d (%s) dequeued.\n", process->process_id, process->process_name);
    return process;
}
// --- End Queue Operations ---

// Global array to store all processes (simplifies management for this example)
PCB all_processes[MAX_PROCESSES];
int num_processes = 0;
int global_time = 0; // Simulation time

/**
 * @brief Creates a new process and adds it to the system.
 * @param id Process ID.
 * @param name Process name.
 * @param burst_time Total execution time required.
 * @return Pointer to the created PCB, or NULL on failure.
 */
PCB* create_process(int id, const char *name, int burst_time, int arrival_time_sim) {
    if (num_processes >= MAX_PROCESSES) {
        // fprintf(stderr, "Cannot create more processes. Limit reached.\n");
        return NULL;
    }
    PCB *p = &all_processes[num_processes];
    p->process_id = id;
    strncpy(p->process_name, name, sizeof(p->process_name) - 1);
    p->process_name[sizeof(p->process_name) - 1] = '\0'; // Ensure null termination
    p->burst_time = burst_time;
    p->remaining_time = burst_time;
    p->state = STATE_NEW;
    p->arrival_time = arrival_time_sim; // For this simulation, we add them based on call order

    num_processes++;
    // printf("Created Process: ID=%d, Name=%s, Burst=%d, Arrival=%d, State=%s\n",
    //        p->process_id, p->process_name, p->burst_time, p->arrival_time, get_state_string(p->state));
    return p;
}

/**
 * @brief Simulates the Round Robin scheduler.
 * @param ready_q Pointer to the ready queue.
 */
void run_scheduler(ReadyQueue *ready_q) {
    PCB *current_process = NULL;
    int processes_terminated = 0;

    printf("\n--- Starting Round Robin Scheduler (Time Quantum: %d units) ---\n", TIME_QUANTUM);
    printf("Global Time | Process ID | Action         | Remaining Time | Queue Size\n");
    printf("------------|------------|----------------|----------------|------------\n");

    // Initial population of the ready queue (assuming all processes arrive at or before time 0 for this simple start)
    // A more complex simulation would add processes to ready_q as global_time >= p->arrival_time
    for (int i = 0; i < num_processes; ++i) {
         if(all_processes[i].state == STATE_NEW && all_processes[i].arrival_time <= global_time) {
            enqueue(ready_q, &all_processes[i]);
         }
    }


    while (processes_terminated < num_processes) {
        if (is_queue_empty(ready_q)) {
            // No processes ready to run, advance time (e.g., if waiting for new arrivals)
            // For this simulation, if queue is empty and not all terminated, it means some processes haven't "arrived"
            // or there's a logic gap. We'll assume for now this means we just advance time if nothing to do.
            bool new_arrivals = false;
            for (int i = 0; i < num_processes; ++i) {
                if (all_processes[i].state == STATE_NEW && all_processes[i].arrival_time <= global_time) {
                    enqueue(ready_q, &all_processes[i]);
                    new_arrivals = true;
                }
            }
            if (!new_arrivals && processes_terminated < num_processes) {
                 // printf("%11d | ---        | IDLE           | ---            | %d\n", global_time, ready_q->count);
                 global_time++; // Advance time if idle and waiting for future arrivals
                 // Check again for arrivals after advancing time
                 for (int i = 0; i < num_processes; ++i) {
                    if (all_processes[i].state == STATE_NEW && all_processes[i].arrival_time <= global_time) {
                        enqueue(ready_q, &all_processes[i]);
                    }
                }
                if (is_queue_empty(ready_q) && processes_terminated == num_processes) break; // All done
                if (is_queue_empty(ready_q) && !new_arrivals) continue; // Still idle, loop
            }
             if (is_queue_empty(ready_q)) { // if still empty after checking arrivals
                if (processes_terminated == num_processes) break;
                printf("%11d | ---        | IDLE           | ---            | %d\n", global_time, ready_q->count);
                global_time++;
                continue;
            }
        }

        current_process = dequeue(ready_q);
        if (current_process == NULL) continue; // Should not happen if not empty

        current_process->state = STATE_RUNNING;
        printf("%11d | %-10d | RUNNING        | %-14d | %d\n",
               global_time, current_process->process_id, current_process->remaining_time, ready_q->count);

        int time_to_run = (current_process->remaining_time < TIME_QUANTUM) ?
                          current_process->remaining_time : TIME_QUANTUM;

        // Simulate execution for the time slice
        // In a real OS, this would involve setting a timer interrupt.
        for (int t = 0; t < time_to_run; ++t) {
            global_time++;
            current_process->remaining_time--;
            // printf("    (t=%d) Process %d running, remaining: %d\n", global_time, current_process->process_id, current_process->remaining_time);

            // Check for new arrivals during this time slice
            for (int i = 0; i < num_processes; ++i) {
                if (all_processes[i].state == STATE_NEW && all_processes[i].arrival_time == global_time) {
                    enqueue(ready_q, &all_processes[i]);
                    printf("%11d | %-10d | ARRIVED        | (Burst: %-5d) | %d\n",
                           global_time, all_processes[i].process_id, all_processes[i].burst_time, ready_q->count);
                }
            }
        }


        if (current_process->remaining_time <= 0) {
            current_process->state = STATE_TERMINATED;
            processes_terminated++;
            printf("%11d | %-10d | TERMINATED     | 0              | %d\n",
                   global_time, current_process->process_id, ready_q->count);
        } else {
            current_process->state = STATE_READY;
            enqueue(ready_q, current_process); // Add back to the queue
            printf("%11d | %-10d | PREEMPTED      | %-14d | %d\n",
                   global_time, current_process->process_id, current_process->remaining_time, ready_q->count);
        }
        current_process = NULL; // Relinquish CPU
    }
    printf("------------|------------|----------------|----------------|------------\n");
    printf("All processes terminated at Global Time: %d\n", global_time);
}

// Main function to set up processes and run the scheduler
int main() {
    ReadyQueue ready_q;
    init_queue(&ready_q);

    printf("--- Process Creation ---\n");
    // Create some sample processes
    // PCB* create_process(int id, const char *name, int burst_time, int arrival_time_sim)
    create_process(1, "P1", 10, 0);
    create_process(2, "P2", 5,  1); // Arrives a bit later
    create_process(3, "P3", 8,  0);
    create_process(4, "P4", 2,  3); // Arrives later
    create_process(5, "P5", 6,  1);

    // The scheduler will pick up processes based on arrival time if they are NEW
    // For this simple run_scheduler, it initially enqueues all that have arrived by global_time=0.
    // Then, during execution, it checks for new arrivals.

    run_scheduler(&ready_q);

    // Optional: Print final state of all processes (should all be TERMINATED)
    printf("\n--- Final Process States ---\n");
    for (int i = 0; i < num_processes; i++) {
        printf("Process ID: %d, Name: %s, State: %s, Burst: %d, Remaining: %d\n",
               all_processes[i].process_id,
               all_processes[i].process_name,
               get_state_string(all_processes[i].state),
               all_processes[i].burst_time,
               all_processes[i].remaining_time);
    }

    return 0;
}

/**
 * Potential Enhancements:
 * 1. Different Scheduling Algorithms: Implement FCFS, SJF (Preemptive/Non-Preemptive), Priority Scheduling.
 * 2. I/O Operations: Introduce an I/O wait queue and simulate processes blocking for I/O.
 * This would require more states (e.g., WAITING) and logic for I/O completion.
 * 3. Dynamic Process Arrival: Processes arrive at different times during the simulation dynamically,
 * not just added at the start. (Partially implemented with arrival_time).
 * 4. Priority Inversion & Aging: For priority scheduling, handle these concepts.
 * 5. Memory Management: Simulate basic memory allocation/deallocation for processes.
 * 6. More Detailed Statistics: Calculate average waiting time, turnaround time, CPU utilization.
 * 7. GUI: A simple graphical interface to visualize the scheduling process.
 */
