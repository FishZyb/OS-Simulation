#include <iostream>
#include <vector>
#include <queue>
#include <string>

using namespace std;

// 进程控制块 (PCB) 结构体
struct PCB {
    int pid; // 进程ID
    int parentPid; // 父进程ID
    string status; // 进程状态（就绪、运行、阻塞）
    string fileName; // 文件名
    vector<int> children; //子进程ID列表
    bool isZombie;//是否为僵尸进程
    bool isWaiting;//是否正在等待子进程终止
};

// 硬盘IO请求结构体
struct IORequest {
    int pid; // 进程ID
    int track; // 磁道号
};

// 全局变量
int memoryCapacity; // 内存容量
int pageSize; // 页面/框架大小
int diskCount; // 硬盘数量
int maxDiskCapacity; // 每个硬盘的最大容量

vector<PCB> processes; // 进程列表
queue<PCB> readyQueue; // 就绪队列

vector<vector<int>> memory; // 内存分配表，记录每个页面所属的进程和页号

vector<queue<IORequest>> ioQueues; // 硬盘IO队列

// 初始化函数，询问用户配置信息并创建初始进程
void initialize() {
    cout << "======================================" << endl;
    cout << "         欢迎使用虚拟操作系统！        " << endl;
    cout << "======================================" << endl;

    cout << "虚拟计算机的内存容量是多少？（字节）";
    cin >> memoryCapacity;

    cout << "虚拟计算机的页面/框架大小是多少？";
    cin >> pageSize;

    cout << "虚拟计算机有多少个硬盘？";
    cin >> diskCount;

    cout << "每个硬盘的最大容量是多少MB？";
    cin >> maxDiskCapacity;

    PCB initialProcess;
    initialProcess.pid = 1;
    initialProcess.parentPid = 0;
    initialProcess.status = "运行";
    initialProcess.fileName = "";

    processes.push_back(initialProcess);
}

// 创建新进程
void createProcess() {
    PCB newProcess;
    newProcess.pid = processes.size() + 1;
    newProcess.parentPid = 1; // 父进程为初始进程
    newProcess.status = "就绪";
    newProcess.fileName = "";
    newProcess.isZombie = false;
    newProcess.isWaiting = false;

    processes.push_back(newProcess);

    //将新创建的进程添加到就绪队列前面
    readyQueue.push(newProcess);

    //将子进程的ID添加到父进程的children列表中
    processes[0].children.push_back(newProcess.pid);

    cout << "创建进程成功！" << endl;
}

// 进程调度，使用RR调度算法（时间片轮转调度算法）
void schedule() {
    if (!readyQueue.empty()) {
        //更新当前运行的进程状态,并将当前进程放到就绪队列末尾
        for (PCB& process : processes) {
            if (process.status == "运行") {
                process.status = "就绪";
                readyQueue.push(process);
                break;
            }
        }
        
        PCB runningProcess = readyQueue.front();
        readyQueue.pop();
        cout << "正在运行的进程：" << runningProcess.pid << endl;
        //执行一个时间片
        //这里不设置任何时间片，用"-"指令表示一个节拍向前推进进程

        //更新process列表中对应进程的状态
        for (PCB& process : processes) {
            if (process.pid == runningProcess.pid) {
                process.status = "运行";
                break;
            }
        }

        cout << "======================================" << endl;
        cout << "请输入命令：";
    }
}

//创建子进程
void forkProcess() {
    //找到当前正在运行的进程
    PCB* runningProcess = nullptr;
    for (PCB& process : processes) {
        if (process.status == "运行") {
            runningProcess = &process;
            break;
        }
    }
    if (runningProcess == nullptr) {
        cout << "没有正在运行的进程，无法创建子进程！" << endl;
        return;
    }
    //创建一个新的子进程，它是当前正在运行的进程的副本
    PCB newProcess = *runningProcess;
    //为新创建的子进程分配一个唯一且递增的PID
    newProcess.pid = processes.back().pid + 1;
    //设置新创建的子进程状态为就绪
    newProcess.status = "就绪";
    //将子进程ID添加到父进程children列表中
    runningProcess->children.push_back(newProcess.pid);
    processes.push_back(newProcess);

    //将新创建的子进程添加到就绪队列末尾
    readyQueue.push(newProcess);
       
    cout << "创建子进程成功！" << endl;
    
}

// 处理磁盘IO请求
void handleIORequest(int diskNumber, int track) {
    if (diskNumber >= 0 && diskNumber < diskCount) {
        if (!readyQueue.empty()) {
            PCB requestingProcess = readyQueue.front();
            readyQueue.pop();

            IORequest ioRequest;
            ioRequest.pid = requestingProcess.pid;
            ioRequest.track = track;

            ioQueues[diskNumber].push(ioRequest);

            cout << "进程" << requestingProcess.pid << "请求磁盘" << diskNumber << "读取或写入磁道" << track << endl;
        }
    }
}

// 处理磁盘IO完成
void handleIODone(int diskNumber) {
    if (diskNumber >= 0 && diskNumber < diskCount) {
        if (!ioQueues[diskNumber].empty()) {
            IORequest ioRequest = ioQueues[diskNumber].front();
            ioQueues[diskNumber].pop();

            cout << "磁盘" << diskNumber << "完成进程" << ioRequest.pid << "的IO请求" << endl;
        }
    }
}

// 请求内存分配
void requestMemory(int pid) {
    // TODO: 实现请求内存分配的逻辑
    int numPages = memoryCapacity / pageSize; // 计算内存中页面的数量
    // 检查是否有足够的空闲页面
    int count = 0;
    for (int i = 0; i < numPages; i++) {
        if (memory[i].empty()) {
            count++;
            if (count == 4) { // 如果有连续的4个空闲页面，则分配给进程
                for (int j = i - 3; j <= i; j++) {
                    memory[j] = { pid, j - (i - 3) };
                }
                cout << "进程" << pid << "获得了" << count << "个连续的页面" << endl;
                return;
            }
        }
        else {
            count = 0;
        }
    }

    // 如果没有连续的4个空闲页面，则无法分配给进程
    cout << "内存中没有足够的空闲页面，进程" << pid << "无法分配内存" << endl;
}

// 释放内存
void releaseMemory(int pid) {
    // TODO: 实现释放内存的逻辑
    int numPages = memoryCapacity / pageSize; // 计算内存中页面的数量
    for (int i = 0; i < numPages; i++) {
        if (!memory[i].empty() && memory[i][0] == pid) { // 找到属于该进程的页面
            memory[i].clear(); // 清空该页面
        }
    }

    cout << "进程" << pid << "释放了内存" << endl;

}

// 显示当前进程和就绪队列中的进程
void showProcesses() {
    cout << "======================================" << endl;
    cout << "当前运行的进程：" << endl;
    for (PCB process : processes) {
        if (process.status == "运行") {
            cout << "进程ID：" << process.pid << "，父进程ID：" << process.parentPid << "，子进程ID：";
            for (int childPid : process.children) {
                cout << childPid << " ";
            }
            cout << endl;
        }
    }

    cout << "就绪队列中的进程：" << endl;
    queue<PCB> tempQueue;
    while (!readyQueue.empty()) {
        PCB process = readyQueue.front();
        readyQueue.pop();
        tempQueue.push(process);

        cout << "进程ID：" << process.pid << "，父进程ID：" << process.parentPid << "，子进程ID：";
        for (int childPid : process.children) {
            cout << childPid << " ";
        }
        cout << endl;
    }

    while (!tempQueue.empty()) {
        PCB process = tempQueue.front();
        tempQueue.pop();
        readyQueue.push(process);
    }

    cout << "======================================" << endl;
}

// 显示硬盘IO状态
void showIOStatus() {
    cout << "======================================" << endl;
    for (int i = 0; i < diskCount; i++) {
        cout << "硬盘" << i << "上的进程：" << endl;

        queue<IORequest> ioQueue = ioQueues[i];
        while (!ioQueue.empty()) {
            IORequest ioRequest = ioQueue.front();
            ioQueue.pop();

            cout << "进程" << ioRequest.pid << "正在读取或写入磁道" << ioRequest.track << endl;

            ioQueue.push(ioRequest);
        }
    }

    cout << "======================================" << endl;
}

// 显示内存状态
void showMemoryStatus() {
    cout << "======================================" << endl;
    for (int i = 0; i < memory.size(); i++) {
        vector<int> page = memory[i];

        if (!page.empty()) {
            int processId = page[0];
            int pageNumber = page[1];

            cout << "页框" << i << "中的页面：" << endl;
            cout << "进程ID：" << processId << "，页号：" << pageNumber << endl;
        }
    }

    cout << "======================================" << endl;
}

int main() {
    initialize();

    string command;
    cout << "请输入命令：";
    while (getline(cin,command)) {
        if (command == "new") {
            createProcess();
        }
        else if (command == "-") {
            schedule();
        }
        else if (command == "fork") {
            forkProcess();
            // TODO: 实现fork命令的逻辑
        //    if (!readyQueue.empty()) {
        //        PCB parentProcess = readyQueue.front();
        //        readyQueue.pop();

        //        PCB childProcess;
        //        childProcess.pid = processes.size() + 1;
        //        childProcess.parentPid = parentProcess.pid;
        //        childProcess.status = "就绪";
        //        childProcess.fileName = "";

        //        processes.push_back(childProcess);
        //        readyQueue.push(childProcess);

        //        // 将子进程的ID添加到父进程的children列表中
        //        parentProcess.children.push_back(childProcess.pid);

        //        cout << "进程" << childProcess.pid << "创建成功！" << endl;

        //        // 将父进程放回就绪队列末尾
        //        readyQueue.push(parentProcess);
        //    }

        }
        else if (command == "exit") {
            // TODO: 实现exit命令的逻辑
            if (!readyQueue.empty()) {
                PCB currentProcess = readyQueue.front();
                readyQueue.pop();

                // 终止当前进程
                cout << "进程" << currentProcess.pid << "终止！" << endl;
                currentProcess.status = "终止";

                // 释放当前进程使用的内存

                // 处理级联终止
                for (int childPid : currentProcess.children) {
                    for (PCB& process : processes) {
                        if (process.pid == childPid) {
                            process.status = "终止";
                            process.isZombie = true;
                            break;
                        }
                    }
                }

                // 处理僵尸进程
                if (currentProcess.parentPid != -1) {
                    for (PCB& process : processes) {
                        if (process.pid == currentProcess.parentPid) {
                            if (process.isWaiting) {
                                process.status = "就绪";
                                readyQueue.push(process);
                                break;
                            }
                        }
                    }
                }
            }
        }
        else if (command == "wait") {
            // TODO: 实现wait命令的逻辑
            if (!readyQueue.empty()) {
                PCB currentProcess = readyQueue.front();
                readyQueue.pop();

                // 检查是否存在僵尸子进程
                bool hasZombieChild = false;
                for (int childPid : currentProcess.children) {
                    for (PCB& process : processes) {
                        if (process.pid == childPid && process.isZombie) {
                            hasZombieChild = true;
                            // 删除僵尸子进程的PCB
                            processes.erase(remove_if(processes.begin(), processes.end(),
                                [&](const PCB& p) { return p.pid == childPid; }), processes.end());
                            break;
                        }
                    }
                }

                // 如果存在僵尸子进程，立即继续使用CPU
                if (hasZombieChild) {
                    cout << "存在僵尸子进程，进程" << currentProcess.pid << "继续执行！" << endl;
                    readyQueue.push(currentProcess);
                }
                else {
                    // 进程转到就绪队列的末尾
                    currentProcess.status = "等待";
                    currentProcess.isWaiting = true;
                    readyQueue.push(currentProcess);
                }
            }
        }
        else if (command.substr(0, 2) == "d ") {
            int diskNumber = stoi(command.substr(2, 1));
            int track = stoi(command.substr(4));

            handleIORequest(diskNumber, track);
        }
        else if (command.substr(0, 2) == "D ") {
            int diskNumber = stoi(command.substr(2));

            handleIODone(diskNumber);
        }
        else if (command.substr(0, 2) == "m ") {
            int pid = stoi(command.substr(2));

            requestMemory(pid);
        }
        else if (command == "show p") {
            //cout << "调试路过" << endl;
            showProcesses();
        }
        else if (command == "show i") {
            showIOStatus();
        }
        else if (command == "show m") {
            showMemoryStatus();
        }

        cout << "请输入命令：";
    }

    return 0;
}
