# CPU Scheduling & Memory Simulator

![C++](https://img.shields.io/badge/Language-C++17-blue.svg) ![License](https://img.shields.io/badge/License-MIT-green.svg)

A robust C++ simulation of an Operating System kernel, focusing on **Priority-Based CPU Scheduling**, **Worst-Fit Memory Management**, and complex **Process Lifecycle** handling (Fork, Wait, Exit, Zombie states).

This project simulates the low-level interactions between the CPU, RAM, and Disk I/O subsystems, offering a detailed view of how an OS manages resources and processes.

## Key Features

### Process Management
* **Priority Scheduling:** Implements a preemptive scheduler where the highest priority process always preempts lower priority ones.
* **Process Lifecycle:** Full implementation of `NewProcess`, `SimFork`, `SimWait`, and `SimExit`.
* **Zombie & Orphan Handling:** Correctly manages zombie processes when children exit before parents, and prevents orphans via **Recursive Cascading Termination**.
* **O(1) Lookup:** Utilizes `std::unordered_map` to track Process Control Blocks (PCBs) for constant-time access.

### Memory Management
* **Worst-Fit Allocation:** Implements the Worst-Fit algorithm to reduce fragmentation by searching for the largest available memory hole for new processes.
* **Dynamic Deallocation:** Automatically reclaims memory and merges gaps when processes terminate.
* **Fragmentation Detection:** Tracks memory usage and identifying gaps between contiguous blocks.

### I/O Subsystem
* **Disk Queues:** Simulates multiple hard disks with independent I/O queues using `std::deque`.
* **Asynchronous Processing:** Processes requesting I/O yield the CPU immediately and enter a blocked state until the disk operation completes.

## Technical Implementation

The system is built using modern C++ features and the Standard Template Library (STL):

* **PCB (Process Control Block):** A custom struct managing PID, priority, parent/child relationships, and execution state.
* **Sorting & Overloading:** Overloads the `<` operator to facilitate sorting of the Ready Queue based on priority.
* **Recursion:** `recursiveCascadingTermination` ensures that killing a parent process safely eliminates all descendant processes to prevent memory leaks.

## Installation & Usage

### Prerequisites
* A C++ compiler (GCC, Clang, or MSVC)
* Make (optional)

### Compilation
You can compile the simulation using `g++`:

```bash
g++ -std=c++11 main.cpp SimOS.cpp -o os_sim
