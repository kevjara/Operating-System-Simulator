/*
Name: Kevin Jara
Course: CSCI 340 Operating Systems
Instructor: Pavel Shostak
Date: Spring 2025
Project Name: OS Simulation
*/

#include "SimOS.h"

/**
 * @post : operator overloading function used by std::sort to sort the PCB objects by
 * their priority number in the ReadyQueue
 * @param : another process 
 */
bool PCB::operator<(const PCB& other_process){
    if(this->PriorityNo < other_process.PriorityNo){
        return true;
    }

    return false;
};


/**
 * @param : the parameters specify number of hard disks in the simulated computer and 
 * amount of memory
 * @pre : Disks enumeration starts from 0
 * @param : sizeOfOS specifies the size of the OS process. It has the PID of 1, priority
 * of 0, and resides in the very beginninng of memory
 */
SimOS::SimOS( int numberOfDisks, unsigned long long amountOfRAM, unsigned long long sizeOfOS ){
    this->numberOfDisks = numberOfDisks;
    this->amountOfRAM = amountOfRAM;

    Disks.resize(numberOfDisks);
    
    if(sizeOfOS < amountOfRAM){
        MemoryItem os_memory;
        os_memory.itemAddress = 0;
        os_memory.itemSize = sizeOfOS;
        os_memory.PID = 1;
        Memory.push_back(os_memory);

        PCB OS;
        OS.PID = 1;
        OS.PriorityNo = 0;
        this->currentAvailablePID = 2;

        current_process = OS;

        processPidTracker[0] = OS;
    }
};


/**
 * @post : Creates a new process with the specified priority in the simulated system. The
 * new process takes place in the ready-queue or immediatley starts using the CPU.
 * @pre : Every process in the simulated system has a PID. The simulation assigns PIDs to
 * new processes starting from 2 (1 is reserved for the OS) and increments it by one for each
 * new process. Do not reuse PIDs of the terminated processes
 * @example : command NewProcess(1000, 5) means that a new process with priority level 5 should
 * be created and it requires 1000 bytes of memory
 * @post: NewProcess returns true if a new process was successfully created and false if otherwise.
 * One of the reasons a process wasn't created is insufficent free memory in the system.
 */
bool SimOS::NewProcess( unsigned long long size, int priority ){
    if(priority == 0){
        return false;
    }

    std::sort(Memory.begin(), Memory.end(), sortByItemAddress);

    unsigned long long iterator = Memory[0].itemSize;
    unsigned long long memoryHoleStart = 0;
    unsigned long long memoryHoleEnd = 0;
    unsigned long long memoryHoleSize = 0;

    // check for memory holes in between processes and saves the largest one with the lowest address
    for(int i = 1; i < Memory.size(); i++){
        if(iterator < Memory[i].itemAddress){
            memoryHoleEnd = Memory[i].itemAddress;

            int tempHoleSize = memoryHoleEnd - iterator;
            if(tempHoleSize > memoryHoleSize || (tempHoleSize == memoryHoleSize && iterator < memoryHoleStart)){
                memoryHoleSize = tempHoleSize;
                memoryHoleStart = iterator;
            }
        }

        iterator = Memory[i].itemAddress + Memory[i].itemSize;
    };

    // check memory hole between final process and end of memory if the previous hole is the same size
    // as this one choose the one with the smallest address
    unsigned long long finalHoleSize = amountOfRAM - iterator;
    if(finalHoleSize > memoryHoleSize || (finalHoleSize == memoryHoleSize && iterator < memoryHoleStart)){
        memoryHoleSize = finalHoleSize;
        memoryHoleStart = iterator;
    }

    // if process is too big return false
    if(size > memoryHoleSize){
        return false;
    }

    MemoryItem processMemory{memoryHoleStart, size, currentAvailablePID};

    Memory.push_back(processMemory);

    
    PCB process;

    process.PriorityNo = priority;
    process.PID = currentAvailablePID;
    currentAvailablePID++;

    processPidTracker[process.PID] = process; 

    if(priority > this->current_process.PriorityNo){
        ReadyQueue.push_back(current_process);
        current_process = process;
    }
    else{
        ReadyQueue.push_back(process);
    }

    return true;
};

/**
 * @post : The currently running process forks a child. The child's priority and size are inherited
 * from the parent. The child is placed in the end of the ready-queue
 * @post : SimFork() returns true if a new process was successfully created and false if otherwise.
 * One of the reasons a process wasn't created is insuffiecient free memory in the system. 
 * @pre : The OS process (PID = 1) ignores SimFork instruction (does nothing)
 */
bool SimOS::SimFork(){
    if(GetCPU() == 1){
        return false;
    }
    
    // sort processes in memory by their address
    std::sort(Memory.begin(), Memory.end(), sortByItemAddress);

    // initialze size to be memory of parent process if not found return false
    bool found = false;
    unsigned long long size = 0;
    for(const MemoryItem& mem_item : Memory){
        if(mem_item.PID == GetCPU()){
            size = mem_item.itemSize;
            found = true;
            break;
        }
    };

    if(!found){
        return false;
    }


    unsigned long long iterator = Memory[0].itemSize;
    unsigned long long memoryHoleStart = 0;
    unsigned long long memoryHoleEnd = 0;
    unsigned long long memoryHoleSize = 0;

    // check for memory holes in between processes and saves the largest one with the lowest address
    // in case of memory hole being same size choose the one with smaller address
    for(int i = 1; i < Memory.size(); i++){
        if(iterator < Memory[i].itemAddress){
            memoryHoleEnd = Memory[i].itemAddress;

            int tempHoleSize = memoryHoleEnd - iterator;
            if(tempHoleSize > memoryHoleSize || (tempHoleSize == memoryHoleSize && iterator < memoryHoleStart)){
                memoryHoleSize = tempHoleSize;
                memoryHoleStart = iterator;
            }
        }

        iterator = Memory[i].itemAddress + Memory[i].itemSize;
    };

    // check memory hole between final process and end of memory if the previous hole is the same size
    // as this one choose the one with the smallest address
    unsigned long long finalHoleSize = amountOfRAM - iterator;
    if(finalHoleSize > memoryHoleSize || (finalHoleSize == memoryHoleSize && iterator < memoryHoleStart)){
        memoryHoleSize = finalHoleSize;
        memoryHoleStart = iterator;
    }

    // if process too big return false
    if(size > memoryHoleSize){
        return false;
    }

    MemoryItem forked_processMemory{memoryHoleStart, size, currentAvailablePID};

    Memory.push_back(forked_processMemory);

    PCB forked_process;

    forked_process.PID = currentAvailablePID;
    currentAvailablePID++;

    forked_process.PriorityNo = current_process.PriorityNo;
    forked_process.parentPID = current_process.PID;
    ReadyQueue.push_back(forked_process);

    processPidTracker[forked_process.PID] = forked_process;
    processPidTracker[current_process.PID].child_processes.push_back(forked_process.PID);

    return true;
};


/**
 * @post : The process that is currently using the CPU. Make sure you release the memory used by
 * this process immediately. If its parent is already waiting, the process terminates immediatley and
 * the parent becomes runnable (goes to the ready-queue or CPU). If its parent hasn't called wait yet,
 * the process turns into zombie.
 * @post: To avoid the apperance of the orphans, the system implements the cascading termination.
 * The OS process (PID = 1) ignores SimExit instruction (does nothing)
 */
void SimOS::SimExit(){
    if(GetCPU() == 1){
        return;
    }

    // freeing memory of process first
    int pid;
    for(auto i = Memory.begin(); i != Memory.end(); ++i){
        if(i->PID == current_process.PID){
            pid = i->PID;
            Memory.erase(i);
            break;
        }
    };

    //if child check if parent called wait 
    if(current_process.parentPID != NO_PROCESS){
        int parentPID = processPidTracker[current_process.PID].parentPID;
        if(processPidTracker[parentPID].WaitStatus == true){
            // parent wait status set to false since child exited and parent is
            // no longer waiting
            processPidTracker[parentPID].WaitStatus = false;

            // push parent into ready queue
            PCB parent_process = processPidTracker[parentPID];
            ReadyQueue.push_back(parent_process);

            //sort by priority number so that process with higher priority is at the end of the list
            //uses operator overloading to achieve this
            std::sort(ReadyQueue.begin(), ReadyQueue.end());

            //set current process to highest priority process
            current_process = ReadyQueue[ReadyQueue.size()-1];
            ReadyQueue.pop_back();

            
            //since process is not zombie remove record from system
            processPidTracker.erase(current_process.PID);
        }
        else{
            // if parent did not call wait mark process as zombie
            processPidTracker[current_process.PID].isZombie = true;
        }
    }
    // if parent cascading termination
    else{
        recursiveCascadingTermination(GetCPU());
        
        //then queue up next process with highest priority
        std::sort(ReadyQueue.begin(), ReadyQueue.end());
        current_process = ReadyQueue[ReadyQueue.size()- 1];
        ReadyQueue.pop_back();
    }
};

/**
 * @post : The currently running process wants to pause and wait for any of its child processes to terminate. Once the
 * wait is over, the process goes to the end of the ready-queue or the CPU. If the zombie-child already
 * exists, the process proceeds right away(keeps using the CPU) and the zombie-child disappears. If more
 * then one zombie child exists, the system uses one of them (any) to immediately restart the parent and
 * other zombies keep waiting for the next wait from the parent
 * @post : The OS process (with PID 1) ignores SimWait instruction (does nothing)
 */
void SimOS::SimWait(){
    if(current_process.PID == 1 ){
        return;
    }

    // check how many zombie children the process has
    int zombieCount = 0;
    int zombieChildPID = -1;
    for(int childPID : processPidTracker[current_process.PID].child_processes){
        if(processPidTracker[childPID].isZombie == true){
            zombieCount++;
            if(zombieChildPID == -1){
                zombieChildPID = childPID;
            }
        }
    }

    // if no zombie children then set process wait status true and replace
    // current process to next process with higher priority
    if(zombieCount == 0){
        processPidTracker[current_process.PID].WaitStatus = true;

        std::sort(ReadyQueue.begin(), ReadyQueue.end());
        current_process = ReadyQueue[ReadyQueue.size()- 1];
        ReadyQueue.pop_back();
    }
    // if only one zombie child get rid of it from record and let parent process run
    else if(zombieCount == 1){
        processPidTracker.erase(zombieChildPID);
    }
    // if more than zombie child get rid of first zombie child and leave rest alone
    // we let user call wait to get rid of rest also let parent continue to run
    else if (zombieCount > 1){
        processPidTracker.erase(zombieChildPID);
    }

};

/**
 * @post : Currently running process requests to read the specified file form the disk with a given number
 * The process issuing disk reading requests immediatley stops using the CPU, even if the ready-queue is empty
 * @post : The OS proces (with PID 1) ignores DiskReadRequest instruction (does nothing)
 */
void SimOS::DiskReadRequest(int diskNumber, std::string fileName ){
    if(current_process.PID == 1){
        return;
    }

    if(diskNumber < 0 || diskNumber >= Disks.size()){
        return;
    }

    FileReadRequest request{current_process.PID, fileName};

    //if disk is currently idle create request
    if(Disks[diskNumber].currentReadRequest.PID == 0){
        Disks[diskNumber].currentReadRequest = request;
    }
    else{
        Disks[diskNumber].IO_Queue.push_back(request);
    }

    std::sort(ReadyQueue.begin(), ReadyQueue.end());
    current_process = ReadyQueue.back();
    ReadyQueue.pop_back();
};

/**
 * @post : A disk with a specified number reports that a single job is completed. The served process should
 * return to the ready-queue or immediately start using the CPU (depending on the priority)
 */
void SimOS::DiskJobCompleted( int diskNumber ){
    if(diskNumber < 0 || diskNumber >= Disks.size()){
        return;
    }

    // retrieve entire process from processPidTracker
    PCB completedProcess;
    if(Disks[diskNumber].currentReadRequest.PID != 0){
        completedProcess = processPidTracker[Disks[diskNumber].currentReadRequest.PID];
    }

    // check io queue if not empty set next process in queue as current job else set
    // to default
    if(!Disks[diskNumber].IO_Queue.empty()){
        Disks[diskNumber].currentReadRequest = Disks[diskNumber].IO_Queue.front();
        Disks[diskNumber].IO_Queue.pop_front();
    }
    else{
        Disks[diskNumber].currentReadRequest = FileReadRequest{0, ""};
    }
    
    // check if completed process priority is greater than current_proces if it is
    // send it to cpu else send to ready queue
    if(current_process.PriorityNo < completedProcess.PriorityNo){
        ReadyQueue.push_back(current_process);
        current_process = completedProcess;
    }
    else{
        ReadyQueue.push_back(completedProcess);
    }
};

/**
 * @post : returns the PID of the process currently using the cpu
 */
int SimOS::GetCPU(){
    return current_process.PID;
};

/**
 * @post : returns the vector with PIDs of processes in the ready-queue in any order
 */
std::vector<int> SimOS::GetReadyQueue(){
    std::vector<int> pids;
    for(auto i = ReadyQueue.begin(); i != ReadyQueue.end(); ++i){
        pids.push_back(i->PID);
    }

    return pids;
};

/**
 * @post : GetMemory returns MemoryUsage vector describing locations of all processes in memory.
 * Terminated "zombie" processes don't use memory, so they don't contribute to memory usage.
 * Processes appear in the MemoryUsage vector in the same order they appear in memory (from low
 * addresses to high)
 */
MemoryUse SimOS::GetMemory(){
    std::sort(Memory.begin(), Memory.end(), sortByItemAddress);
    return Memory;
};

/**
 * @post : returns an object with PID of the process served by specified disk and the name of the
 * file read for that process. If the disk is idle, GetDisk returns the default FileReadRequest object
 * (with PID 0 and empty string in fileName)
 */
FileReadRequest SimOS::GetDisk(int diskNumber){
    if(diskNumber < 0 || diskNumber >= Disks.size()){
        return FileReadRequest {0, ""};
    }
    return Disks[diskNumber].currentReadRequest;
};

/**
 * @post : returns the I/O queue of the specified disk starting from the "next to be served" process
 */
std::queue<FileReadRequest> SimOS::GetDiskQueue(int diskNumber){
    if(diskNumber < 0 || diskNumber >= Disks.size()){
        return std::queue<FileReadRequest>();
    }
    return std::queue<FileReadRequest>(Disks[diskNumber].IO_Queue);
};

/**
 * @param: two MemoryItems to compare to determine their order
 * @post : used by std::sort as a helper function in order to sort their order by address in the Memory
 * data structure
 */
bool SimOS::sortByItemAddress(const MemoryItem& a, const MemoryItem& b){
    return a.itemAddress < b.itemAddress;
};

/**
 * @param : the pid of the process
 * @post : recursively deallocates memory and removes from ready queue and io queue the descendents of 
 * the original parent process in which this function is called on
 */
void SimOS::recursiveCascadingTermination(int pid){
    // create copy of child processes due to case where we remove process from processPidTracker mid
    // execution would cause undefined behavior
    std::vector<int> children = processPidTracker[pid].child_processes;
    for(int child : children ){
        recursiveCascadingTermination(child);
    }

    // remove from memory
    for(auto i = Memory.begin(); i != Memory.end(); ++i){
        if(i->PID == pid){
            Memory.erase(i);
            break;
        }
    };

    // remove from ready-queue
    for(auto it = ReadyQueue.begin(); it != ReadyQueue.end(); ++it){
        if(it->PID == pid){
            ReadyQueue.erase(it);
            break;
        }
    }

    // remove from io queue or currentDiskRequest depending on where it is
    for(Disk& disk : Disks){
        for(auto it = disk.IO_Queue.begin(); it != disk.IO_Queue.end(); ++it){
            if(it->PID == pid){
                disk.IO_Queue.erase(it);
                break;
            }
        }

        if(disk.currentReadRequest.PID == pid){
            // if io queue not empty set currentReadRequest to next process in queue
            // otherwise set currentReadRequest to default
            if (!disk.IO_Queue.empty()) {
                disk.currentReadRequest = disk.IO_Queue.front();
                disk.IO_Queue.pop_front();
            } 
            else {
                disk.currentReadRequest = FileReadRequest{0, ""};
            }
        }
    }
    
    
    // if parent called wait get rid of record if not mark as zombie
    int parent_pid = processPidTracker[pid].parentPID;
    if(processPidTracker[parent_pid].WaitStatus == true){
        processPidTracker[parent_pid].WaitStatus = false;

        // push parent into ready queue
        PCB parent_process = processPidTracker[parent_pid];
        ReadyQueue.push_back(parent_process);

        //sort by priority number so that process with higher priority is at the end of the list
        //uses operator overloading to achieve this
        std::sort(ReadyQueue.begin(), ReadyQueue.end());

        //set current process to highest priority process
        current_process = ReadyQueue[ReadyQueue.size()-1];
        ReadyQueue.pop_back();
    
        // since not zombie get rid of record
        processPidTracker.erase(pid);
    }
    else{
        // mark as zombie
        processPidTracker[pid].isZombie = true;
    }
};