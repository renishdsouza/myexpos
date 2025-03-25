#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_PROCESSES 10
#define TIME_QUANTUM 5

// Process structure
typedef struct {
    char id;                 // Process identifier (A, B, C, etc.)
    int total_cpu_time;      // Total CPU time required
    int remaining_cpu_time;  // Remaining CPU time
    int arrival_time;        // Arrival time (set to 0 for all processes)
    int completion_time;     // Completion time
    int turnaround_time;     // Completion time - arrival time
    bool is_completed;       // Process completion flag
    pthread_t thread;        // Thread for simulating process concurrency
} Process;

// Global mutex for scheduling synchronization
pthread_mutex_t lock;

// Helper thread function that simply waits until the process is marked completed.
void* process_thread(void* arg) {
    Process* proc = (Process*)arg;
    while (!proc->is_completed) {
        usleep(1000);   // Sleep for 10 ms
    }
    return NULL;
}

// The MLFQ scheduler simulates execution over 4 queues:
// Q0: Round Robin, Q1: FCFS, Q2: Priority (inverse of remaining time),
// Q3: Shortest Job First.
void mlfq_scheduler(Process* processes, int n) {
    int current_time = 0;
    int completed = 0;

    // Allocate arrays to represent each queue.
    Process* q0 = (Process*)malloc(n * sizeof(Process));
    Process* q1 = (Process*)malloc(n * sizeof(Process));
    Process* q2 = (Process*)malloc(n * sizeof(Process));
    Process* q3 = (Process*)malloc(n * sizeof(Process));
    int q0_count = n, q1_count = 0, q2_count = 0, q3_count = 0;
    
    // Initially, all processes are placed in Q0.
    for (int i = 0; i < n; i++) {
        q0[i] = processes[i];
    }
    
    // Main scheduler loop continues until all processes are complete.
    while (completed < n) {
        // Q0: Round Robin Scheduling
        if (q0_count > 0) {
            //printf("\n[Q0] Round Robin at time %d with %d processes\n", current_time, q0_count);
            int temp_q0 = q0_count;
            for (int i = 0; i < temp_q0; i++) {
                Process* p = &q0[i];
                int exe_time = (p->remaining_cpu_time < TIME_QUANTUM) ? p->remaining_cpu_time : TIME_QUANTUM;
                //printf("Executing Process %c for %d ms\n", p->id, exe_time);
                usleep(exe_time * 1000);  // simulate execution
                current_time += exe_time;
                p->remaining_cpu_time -= exe_time;
                if (p->remaining_cpu_time <= 0) {
                    p->is_completed = true;
                    p->completion_time = current_time;
                    p->turnaround_time = p->completion_time - p->arrival_time;
                    printf("Process %c completed at time %d (Turnaround: %d ms)\n",
                           p->id, p->completion_time, p->turnaround_time);
                    // Update the original process array.
                    for (int j = 0; j < n; j++) {
                        if (processes[j].id == p->id) {
                            processes[j] = *p;
                            break;
                        }
                    }
                    completed++;
                } else {
                    // Not finished: move process to Queue 1.
                    q1[q1_count++] = *p;
                }
            }
            q0_count = 0;
        }
        
        // Q1: FCFS Scheduling
        if (q1_count > 0) {
            //printf("\n[Q1] FCFS at time %d with %d processes\n", current_time, q1_count);
            int temp_q1 = q1_count;
            for (int i = 0; i < temp_q1; i++) {
                Process* p = &q1[i];
                int exe_time = (p->remaining_cpu_time < TIME_QUANTUM) ? p->remaining_cpu_time : TIME_QUANTUM;
                //printf("Executing Process %c for %d ms\n", p->id, exe_time);
                usleep(exe_time * 1000);
                current_time += exe_time;
                p->remaining_cpu_time -= exe_time;
                if (p->remaining_cpu_time <= 0) {
                    p->is_completed = true;
                    p->completion_time = current_time;
                    p->turnaround_time = p->completion_time - p->arrival_time;
                    printf("Process %c completed at time %d (Turnaround: %d ms)\n",
                           p->id, p->completion_time, p->turnaround_time);
                    for (int j = 0; j < n; j++) {
                        if (processes[j].id == p->id) {
                            processes[j] = *p;
                            break;
                        }
                    }
                    completed++;
                } else {
                    // Move process to Queue 2.
                    q2[q2_count++] = *p;
                }
            }
            q1_count = 0;
        }
        
        // Q2: Priority Scheduling (order by remaining_cpu_time ascending)
        if (q2_count > 0) {
            //printf("\n[Q2] Priority Scheduling at time %d with %d processes\n", current_time, q2_count);
            // Bubble sort the queue based on remaining CPU time.
            for (int i = 0; i < q2_count - 1; i++) {
                for (int j = 0; j < q2_count - i - 1; j++) {
                    if (q2[j].remaining_cpu_time > q2[j + 1].remaining_cpu_time) {
                        Process temp = q2[j];
                        q2[j] = q2[j + 1];
                        q2[j + 1] = temp;
                    }
                }
            }
            int temp_q2 = q2_count;
            for (int i = 0; i < temp_q2; i++) {
                Process* p = &q2[i];
                int exe_time = (p->remaining_cpu_time < TIME_QUANTUM) ? p->remaining_cpu_time : TIME_QUANTUM;
                //printf("Executing Process %c for %d ms\n", p->id, exe_time);
                usleep(exe_time * 1000);
                current_time += exe_time;
                p->remaining_cpu_time -= exe_time;
                if (p->remaining_cpu_time <= 0) {
                    p->is_completed = true;
                    p->completion_time = current_time;
                    p->turnaround_time = p->completion_time - p->arrival_time;
                    printf("Process %c completed at time %d (Turnaround: %d ms)\n",
                           p->id, p->completion_time, p->turnaround_time);
                    for (int j = 0; j < n; j++) {
                        if (processes[j].id == p->id) {
                            processes[j] = *p;
                            break;
                        }
                    }
                    completed++;
                } else {
                    // Move process to Queue 3.
                    q3[q3_count++] = *p;
                }
            }
            q2_count = 0;
        }
        
        // Q3: Shortest Job First Scheduling.
        if (q3_count > 0) {
            //printf("\n[Q3] Shortest Job First at time %d with %d processes\n", current_time, q3_count);
            // Bubble sort based on total_cpu_time.
            for (int i = 0; i < q3_count - 1; i++) {
                for (int j = 0; j < q3_count - i - 1; j++) {
                    if (q3[j].remaining_cpu_time > q3[j + 1].remaining_cpu_time) {
                        Process temp = q3[j];
                        q3[j] = q3[j + 1];
                        q3[j + 1] = temp;
                    }
                }
            }
            int temp_q3 = q3_count;
            for (int i = 0; i < temp_q3; i++) {
                Process* p = &q3[i];
                int exe_time = (p->remaining_cpu_time < TIME_QUANTUM) ? p->remaining_cpu_time : TIME_QUANTUM;
                //printf("Executing Process %c for %d ms\n", p->id, exe_time);
                usleep(exe_time * 1000);  // simulate execution
                current_time += exe_time;
                p->remaining_cpu_time -= exe_time;
                if (p->remaining_cpu_time <= 0) {
                    p->is_completed = true;
                    p->completion_time = current_time;
                    p->turnaround_time = p->completion_time - p->arrival_time;
                    printf("Process %c completed at time %d (Turnaround: %d ms)\n",
                           p->id, p->completion_time, p->turnaround_time);
                    // Update the original process array.
                    for (int j = 0; j < n; j++) {
                        if (processes[j].id == p->id) {
                            processes[j] = *p;
                            break;
                        }
                    }
                    completed++;
                    // Remove the completed process from Queue 3.
                    for (int j = i; j < temp_q3 - 1; j++) {
                        q3[j] = q3[j + 1];
                    }
                    temp_q3--;
                    i--; // Adjust index to re-evaluate the next process
                } else {
                    // Move process to Queue 0.
                    q0[q0_count++] = *p;
                }
            }
            q3_count = 0;
        }
        
        // If not all processes are done and all queues are empty,
        // gather the unfinished processes and reset them into Q0.
        if (completed < n && q0_count == 0 && q1_count == 0 && q2_count == 0 && q3_count == 0) {
            for (int i = 0; i < n; i++) {
                if (!processes[i].is_completed)
                    q0[q0_count++] = processes[i];
            }
        }
    }
    
    // Free allocated memory for queues.
    free(q0);
    free(q1);
    free(q2);
    free(q3);
    
    // Compute and display average turnaround time.
    int total_turnaround = 0;
    for (int i = 0; i < n; i++) {
        total_turnaround += processes[i].turnaround_time;
    }
    printf("\nAverage Turnaround Time: %d ms\n", total_turnaround / n);
}

int main() {
    pthread_mutex_init(&lock, NULL);
    
    // Set up the processes.
    Process processes[] = {
        {'A', 16, 16, 0, 0, 0, false, 0},
        {'B', 12, 12, 0, 0, 0, false, 0},
        {'C', 20, 20, 0, 0, 0, false, 0},
        {'D', 8, 8, 0, 0, 0, false, 0},
    };
    int n = sizeof(processes) / sizeof(processes[0]);
    
    // Create a thread for each process (to simulate concurrency).
    for (int i = 0; i < n; i++) {
        pthread_create(&processes[i].thread, NULL, process_thread, &processes[i]);
    }
    
    // Run the multilevel feedback queue scheduler.
    mlfq_scheduler(processes, n);
    
    // Join all process threads.
    for (int i = 0; i < n; i++) {
        pthread_join(processes[i].thread, NULL);
    }
    
    pthread_mutex_destroy(&lock);
    return 0;
}
