/*
Name: Kevin Jara
Course: CSCI 340 Operating Systems
Instructor: Pavel Shostak
Date: Spring 2025
Project Name: OS Simulation
*/

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>

struct FileReadRequest{
    int  PID{0};
    std::string fileName{""};
};

struct Disk{
    FileReadRequest currentReadRequest; // process that has access to disk
    std::deque <FileReadRequest> IO_Queue; // io queue for the disk
};

struct MemoryItem{
    unsigned long long itemAddress;
    unsigned long long itemSize;
    int PID; // PID of the process using this chunk of memory
};


using MemoryUse = std::vector<MemoryItem>;


constexpr int NO_PROCESS{-1};

struct PCB {
    int PID; // the pid of the process
    int PriorityNo; // the priority number of the process
    std::vector<int> child_processes; // vector that stores the pids of the processes children
    int parentPID = NO_PROCESS; // the parent id of the process if it is a child
    bool WaitStatus = false; // the wait status of the process that called SimWait()
    bool isZombie = false; // the zombie status of the process

    /**
     * @post : operator overloading function used by std::sort to sort the PCB objects by
     * their priority number in the ReadyQueue
     * @param : another process 
    */
    bool operator<(const PCB& other_process);
};

class SimOS {
    public:
    /**
     * @param : the parameters specify number of hard disks in the simulated computer and 
     * amount of memory
     * @pre : Disks enumeration starts from 0
     * @param : sizeOfOS specifies the size of the OS process. It has the PID of 1, priority
     * of 0, and resides in the very beginninng of memory
     */
    SimOS( int numberOfDisks, unsigned long long amountOfRAM, unsigned long long sizeOfOS );

    /**
     * @post : Creates a new process with the specified priority in the simulated system. The
     * new process takes place in the ready-queue or immediatley starts using the CPU.
     * @pre : Every process in the simulated system has a PID. The simulation assigns PIDs to
     * new processes starting from 2 (1 is reserved for the OS) and increments it by one for each
     * new process. Do not reuse PIDs of the terminated processes
     * @example : command NewProcess(1000, 5) means that a new process with priority level 5 should
     * be created and it requires 1000 bytes of memory
     * @post: NewProcess returns true if a new process was successfully created and false if otherwise.
     * One of the reasons a proces wasn't created is insufficent free memory in the system.
     */
    bool NewProcess( unsigned long long size, int priority );

    /**
     * @post : The currently running process forks a child. The child's priority and size are inherited
     * from the parent. The child is placed in the end of the ready-queue
     * @post : SimFork() returns ture if a new process was successfully created and false if otherwise.
     * One of the reasons a process wasn't created is insuffiecient free memory in the system. 
     * @pre : The OS process (PID = 1) ignores SimFork instruction (does nothing)
     */
    bool SimFork();

    /**
     * @post : The process that is currently using the CPU. Make sure you release the memory used by
     * this process immediately. If its parent is already waiting, the process terminates immediatley and
     * the parent becomes runnable (goes to the ready-queue or CPU). If its parent hasn't called wait yet,
     * the process turns into zombie.
     * @post: To avoid the apperance of the orphans, the system implements the cascading termination.
     * The OS process (PID = 1) ignores SimExit instruction (does nothing)
     */
    void SimExit();

    /**
     * @post : The process wants to pause and wait for any of its child processes to terminate. Once the
     * wait is over, the process goes to the end of the ready-queue or the CPU. If the zombie-child already
     * exists, the process proceeds right away(keeps using the CPU) and the zombie-child disappears. If more
     * then one zombie child exists, the system uses on of them (any) to immediately restart the parent and
     * other zombies keep waiting for the next wait from the parent
     * @post : The OS process (with PID 1) ignores SimWait instruction (does nothing)
     */
    void SimWait();

    /**
     * @post : Currently running process requests to read the specified file fomr the disk with a given number
     * The process issuing disk reading requests immediatley stops using the CPU, even if the ready-queue is empty
     * @post : The OS proces (with PID 1) ignores DiskReadRequest instruction (does nothing)
     */
    void DiskReadRequest(int diskNumber, std::string fileName );

    /**
     * @post : A disk with a specified number reports that a single job is completed. The served process should
     * return to the ready-queue or immediately start using the CPU (depending on the priority)
     */
    void DiskJobCompleted( int diskNumber );

    /**
     * @post : returns the PID of the process currently using the cpu
     */
    int GetCPU();

    /**
     * @post : returns the vector with PIDs of processes in the ready-queue in any order
     */
    std::vector<int> GetReadyQueue();

    /**
     * @post : GetMemory returns MemoryUsage vector describing locations of all processes in memory.
     * Terminated "zombie" processes don't use memory, so they don't contribute to memory usage.
     * Processes appear in the MemoryUsage vector in the same order they appear in memory (from low
     * addresses to high)
     */
    MemoryUse GetMemory();

    /**
     * @post : returns an object with PID of the process served by specified disk and the name of the
     * file read for that process. If the disk is idle, GetDisk returns the default FileReadRequest object
     * (with PID 0 and empty string in fileName)
     */
    FileReadRequest GetDisk(int diskNumber);

    /**
     * @post : returns the I/O queue of the specified disk starting from the "next to be served" process
     */
    std::queue<FileReadRequest> GetDiskQueue(int diskNumber);

    /**
     * @param: two MemoryItems to compare and determine their order
     * @post : used by std::sort as a helper function in order to sort their order by address in the Memory
     * data structure
     */
    static bool sortByItemAddress(const MemoryItem& a, const MemoryItem& b);


    /**
     * @param : the pid of the process
     * @post : recursively removes process and its descendents from memory, ready queue and io queue
     */
    void recursiveCascadingTermination(int pid);

    
    private:
        int numberOfDisks; // the number of disks on the system
        int currentAvailablePID; // stores the current PID that is available to use
        unsigned long long amountOfRAM; // the amount of RAM in the OS
        PCB current_process; // the process that is currently using the CPU
        std::vector<PCB> ReadyQueue; // the ready queue of the OS
        MemoryUse Memory; // the memory of the OS
        std::unordered_map<int, PCB> processPidTracker; //single source of truth for processes information
        std::vector<Disk> Disks; // stores the disk objects to be used

};