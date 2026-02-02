#include <iostream>
#include "SimOS.h"


int main() {
    // Initialize SimOS with 2 disks, 1024 bytes of RAM, and 128 bytes OS
    SimOS sim(2, 1024, 128);

    std::cout << "Initial CPU PID: " << sim.GetCPU() << "\n";

    // Test: Create processes until memory is full
    int p2 = sim.NewProcess(200, 3); // PID 2
    int p3 = sim.NewProcess(300, 5); // PID 3
    int p4 = sim.NewProcess(300, 2); // PID 4
    int p5 = sim.NewProcess(150, 1); // PID 5 — should fail if memory too tight

    std::cout << "Memory After Creation:\n";
    for (const auto& m : sim.GetMemory()) {
        std::cout << "PID " << m.PID << ": [" << m.itemAddress << ", " << m.itemSize << "]\n";
    }

    std::cout << "Current CPU PID: " << sim.GetCPU() << "\n";

    // Test: Disk I/O and preemption
    sim.DiskReadRequest(0, "file_A.txt"); // Current process off CPU

    std::cout << "CPU After Disk I/O: " << sim.GetCPU() << "\n";

    // Test: Disk job completion with higher priority
    sim.DiskJobCompleted(0);
    std::cout << "CPU After Disk Completion: " << sim.GetCPU() << "\n";

    // Test: Fork a process
    sim.SimFork(); // Should create a child of current process
    auto readyQueue = sim.GetReadyQueue();
    std::cout << "Ready Queue After Fork: ";
    for (int pid : readyQueue) std::cout << pid << " ";
    std::cout << "\n";

    // Test: Wait on child
    sim.SimWait(); // Moves parent to waiting, highest priority child runs

    std::cout << "CPU After Wait: " << sim.GetCPU() << "\n";

    // Test: Exit child — should wake up parent if waiting
    sim.SimExit();
    std::cout << "CPU After Child Exit (should be parent again): " << sim.GetCPU() << "\n";

    // Test: Recursive cascading termination with a process tree
    sim.NewProcess(100, 4); // PID 6
    sim.SimFork(); // PID 7
    sim.SimFork(); // PID 8

    std::cout << "Before Cascade Termination:\n";
    for (const auto& m : sim.GetMemory()) {
        std::cout << "PID " << m.PID << ": [" << m.itemAddress << ", " << m.itemSize << "]\n";
    }

    sim.SimExit(); // Terminates PID 6 → cascades to 7 and 8

    std::cout << "After Cascade Termination:\n";
    for (const auto& m : sim.GetMemory()) {
        std::cout << "PID " << m.PID << ": [" << m.itemAddress << ", " << m.itemSize << "]\n";
    }

    // Final: Test GetDiskQueue
    sim.NewProcess(50, 1);
    sim.DiskReadRequest(1, "logfile.log");
    auto queueCopy = sim.GetDiskQueue(1);
    std::cout << "Disk Queue (copy) contains:\n";
    while (!queueCopy.empty()) {
        auto req = queueCopy.front();
        std::cout << "PID " << req.PID << " requesting " << req.fileName << "\n";
        queueCopy.pop();
    }

    return 0;
}
