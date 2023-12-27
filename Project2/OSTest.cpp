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

// 为 PCB 结构体定义 operator== 运算符
bool operator==(const PCB& a, const PCB& b) {
    return a.pid == b.pid;
}

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

    ioQueues.resize(diskCount);

}

void createProcess() {
    PCB newProcess;
    newProcess.pid = processes.size() + 1;
    newProcess.parentPid = 1;
    newProcess.status = "运行";
    newProcess.fileName = "";
    newProcess.isZombie = false;
    newProcess.isWaiting = false;

    processes.push_back(newProcess);

    processes[0].children.push_back(newProcess.pid);

    // 如果系统进程仍在运行，则将其状态更改为就绪并将其放入就绪队列
    for (PCB& process : processes) {
        if (process.pid == 1 && process.status == "运行") {
            process.status = "就绪";
            readyQueue.push(process);
            break;
        }
    }


    cout << "创建进程成功！" << endl;


}

void showProcesses() {
    cout << "======================================" << endl;
    cout << "当前运行的进程：" << endl;

    bool hasRunningProcess = false;
    bool hasNonSystemProcess = false;

    for (PCB& process : processes) {
        if (process.pid != 1) {
            hasNonSystemProcess = true;
        }
        if (process.status == "运行") {
            cout << "进程ID：" << process.pid << "，父进程ID：" << process.parentPid << "，子进程ID：";
            for (int childPid : process.children) {
                cout << childPid << " ";
            }
            cout << endl;
            hasRunningProcess = true;
        }
    }

    // 如果没有非系统进程，则显示系统进程
    if (!hasNonSystemProcess && !hasRunningProcess) {
        for (PCB& process : processes) {
            if (process.pid == 1 && process.status != "等待") {
                cout << "进程ID：" << process.pid << "，父进程ID：" << process.parentPid << "，子进程ID：";
                for (int childPid : process.children) {
                    cout << childPid << " ";
                }
                cout << endl;
                break;
            }
        }
    }

    cout << "就绪队列中的进程：" << endl;
    queue<PCB> tempQueue;
    while (!readyQueue.empty()) {
        PCB process = readyQueue.front();
        readyQueue.pop();
        tempQueue.push(process);

        if (process.status != "等待" && process.pid != 1) {
            cout << "进程ID：" << process.pid << "，父进程ID：" << process.parentPid << "，子进程ID：";
            for (int childPid : process.children) {
                cout << childPid << " ";
            }
            cout << endl;
        }
    }

    while (!tempQueue.empty()) {
        PCB process = tempQueue.front();
        tempQueue.pop();
        readyQueue.push(process);
    }

    cout << "======================================" << endl;
}

void schedule() {
    if (!readyQueue.empty()) {

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

void forkProcess() {
    PCB* runningProcess = nullptr;
    bool isRunning = false;

    for (PCB& process : processes) {
        if (process.status == "运行") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    if (isRunning && runningProcess != nullptr) {

        PCB childProcess;

        childProcess.pid = processes.back().pid + 1;
        childProcess.status = "就绪";
        childProcess.fileName = "";
        childProcess.isZombie = false;
        childProcess.isWaiting = false;

        // 在创建子进程时，确保子进程的父进程是当前正在运行的进程
        for (PCB& process : processes) {
            if (process.status == "运行") {
                childProcess.parentPid = process.pid;
                process.children.push_back(childProcess.pid);
                break;
            }
        }

        processes.push_back(childProcess);

        readyQueue.push(childProcess);

        cout << "创建子进程成功！" << endl;
    }
    else {
        cout << "没有正在运行的进程,无法创建子进程。" << endl;
    }
}

void exit() {
    PCB* runningProcess = nullptr;
    bool isRunning = false;

    for (PCB& process : processes) {
        if (process.status == "运行") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    if (isRunning && runningProcess != nullptr) {

        // 如果退出的进程是系统进程，不允许执行exit命令
        if (runningProcess->pid == 1) {
            cout << "系统进程不能执行exit命令。" << endl;
            return;
        }

        runningProcess->status = "僵尸";
        runningProcess->isZombie = true;

        // 将就绪队列中的下一个进程设置为运行状态
        if (!readyQueue.empty()) {
            PCB nextRunningProcess = readyQueue.front();
            readyQueue.pop();

            for (PCB& process : processes) {
                if (process.pid == nextRunningProcess.pid) {
                    process.status = "运行";
                    break;
                }
            }
        }

        // 将父进程从等待状态转为就绪状态，并加入到就绪队列的末尾
        for (PCB& process : processes) {
            if (process.pid == runningProcess->parentPid && process.isWaiting) {
                process.status = "就绪";
                process.isWaiting = false;
                readyQueue.push(process);
                break;
            }
        }

        // 检查是否还有其他非系统进程在运行或处于就绪状态
        bool hasNonSystemProcess = false;
        for (PCB& process : processes) {
            if (process.pid != 1 && (process.status == "运行" || process.status == "就绪")) {
                hasNonSystemProcess = true;
                break;
            }
        }

        // 如果没有其他非系统进程在运行或处于就绪状态，则将系统进程1设置为运行状态
        if (!hasNonSystemProcess) {
            for (PCB& process : processes) {
                if (process.pid == 1) {
                    process.status = "运行";
                    break;
                }
            }
        }

    }
    else {
        cout << "没有正在运行的进程，无法执行exit命令。" << endl;
    }
}

void wait() {
    PCB* runningProcess = nullptr;
    bool isRunning = false;

    for (PCB& process : processes) {
        if (process.status == "运行") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    if (isRunning && runningProcess != nullptr) {

        // 如果等待的进程是系统进程，不允许执行wait命令
        if (runningProcess->pid == 1) {
            cout << "系统进程不能执行wait命令。" << endl;
            return;
        }

        runningProcess->status = "等待";
        runningProcess->isWaiting = true;

        // 将就绪队列中的下一个非系统进程设置为运行状态
        while (!readyQueue.empty()) {
            PCB nextRunningProcess = readyQueue.front();
            readyQueue.pop();

            if (nextRunningProcess.pid != 1) {
                for (PCB& process : processes) {
                    if (process.pid == nextRunningProcess.pid) {
                        process.status = "运行";
                        break;
                    }
                }
                break;
            }
        }

    }
    else {
        cout << "没有正在运行的进程，无法执行wait命令。" << endl;
    }
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

}

// 释放内存
void releaseMemory(int pid) {


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
        }
        else if (command == "exit") {
            exit();
        }
        else if (command == "wait") {
            wait();
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
