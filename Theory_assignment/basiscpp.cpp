#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <chrono>

using namespace std;

struct Process {
    char id;
    int total_cpu_time;
    int remaining_cpu_time;
    int arrival_time;
    int completion_time;
    int turnaround_time;
    bool is_completed;

    Process(char id, int total_cpu_time) 
        : id(id), total_cpu_time(total_cpu_time), remaining_cpu_time(total_cpu_time), 
          arrival_time(0), completion_time(0), turnaround_time(0), is_completed(false) {}
};

mutex mtx;  // Mutex for synchronization
condition_variable cv;  // Condition variable for thread coordination

// Round Robin Scheduling (Queue 0)
void round_robin(queue<Process*>& q, int time_quantum, int& current_time) {
    while (!q.empty()) {
        Process* p = q.front();
        q.pop();

        lock_guard<mutex> lock(mtx);
        int execution_time = min(p->remaining_cpu_time, time_quantum);
        this_thread::sleep_for(chrono::milliseconds(execution_time));
        current_time += execution_time;
        p->remaining_cpu_time -= execution_time;

        if (p->remaining_cpu_time <= 0) {
            p->is_completed = true;
            p->completion_time = current_time;
            p->turnaround_time = p->completion_time - p->arrival_time;
            cout << "Process " << p->id << " completed at time " << p->completion_time 
                 << ", Turnaround time: " << p->turnaround_time << endl;
        } else {
            q.push(p);  // Reinsert into queue for further processing
        }
    }
}

// First-Come, First-Served Scheduling (Queue 1)
void fcfs(queue<Process*>& q, int time_quantum, int& current_time) {
    while (!q.empty()) {
        Process* p = q.front();
        q.pop();

        lock_guard<mutex> lock(mtx);
        int execution_time = min(p->remaining_cpu_time, time_quantum);
        this_thread::sleep_for(chrono::milliseconds(execution_time));
        current_time += execution_time;
        p->remaining_cpu_time -= execution_time;

        if (p->remaining_cpu_time <= 0) {
            p->is_completed = true;
            p->completion_time = current_time;
            p->turnaround_time = p->completion_time - p->arrival_time;
            cout << "Process " << p->id << " completed at time " << p->completion_time 
                 << ", Turnaround time: " << p->turnaround_time << endl;
        } else {
            q.push(p);  // Reinsert into queue for further processing
        }
    }
}

// Priority Scheduling (Queue 2)
void priority_scheduling(vector<Process*>& v, int time_quantum, int& current_time) {
    sort(v.begin(), v.end(), [](Process* a, Process* b) {
        return a->remaining_cpu_time < b->remaining_cpu_time;  // Inverse priority
    });

    for (auto& p : v) {
        lock_guard<mutex> lock(mtx);
        int execution_time = min(p->remaining_cpu_time, time_quantum);
        this_thread::sleep_for(chrono::milliseconds(execution_time));
        current_time += execution_time;
        p->remaining_cpu_time -= execution_time;

        if (p->remaining_cpu_time <= 0) {
            p->is_completed = true;
            p->completion_time = current_time;
            p->turnaround_time = p->completion_time - p->arrival_time;
            cout << "Process " << p->id << " completed at time " << p->completion_time 
                 << ", Turnaround time: " << p->turnaround_time << endl;
        }
    }
}

// Shortest Job First Scheduling (Queue 3)
void sjf(vector<Process*>& v, int& current_time) {
    sort(v.begin(), v.end(), [](Process* a, Process* b) {
        return a->total_cpu_time < b->total_cpu_time;  // Shortest job first
    });

    for (auto& p : v) {
        lock_guard<mutex> lock(mtx);
        this_thread::sleep_for(chrono::milliseconds(p->total_cpu_time));
        current_time += p->total_cpu_time;
        p->remaining_cpu_time = 0;  // Job completed
        p->is_completed = true;
        p->completion_time = current_time;
        p->turnaround_time = p->completion_time - p->arrival_time;

        cout << "Process " << p->id << " completed at time " << p->completion_time 
             << ", Turnaround time: " << p->turnaround_time << endl;
    }
}

int main() {
    vector<Process> processes = {
        {'A', 300},
        {'B', 150},
        {'C', 250},
        {'D', 350},
        {'E', 450}
    };

    queue<Process*> q0;  // Queue for Round Robin
    queue<Process*> q1;  // Queue for FCFS
    vector<Process*> q2; // Vector for Priority Scheduling
    vector<Process*> q3; // Vector for SJF

    for (auto& process : processes) {
        q0.push(&process);
    }

    int current_time = 0; 
    const int time_quantum = 5;

    cout << "\nStarting Round Robin Scheduling...\n";
    round_robin(q0, time_quantum, current_time);

    cout << "\nStarting FCFS Scheduling...\n";
    fcfs(q1, time_quantum, current_time);

    cout << "\nStarting Priority Scheduling...\n";
    priority_scheduling(q2, time_quantum, current_time);

    cout << "\nStarting Shortest Job First Scheduling...\n";
    sjf(q3, current_time);

    return 0;
}
