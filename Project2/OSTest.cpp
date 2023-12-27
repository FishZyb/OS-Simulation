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
        //如果此时processes进程列表中没有被标记为运行的进程
        //则直接从就绪队列中拿出一个进程，标记为运行状态，加入processes列表，并从队列中移除
        //此逻辑待实现
        
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
    PCB runningProcess;
    bool isRunning = false;

    // 查找正在运行的进程
    for (PCB& process : processes) {
        if (process.status == "运行") {
            runningProcess = process;
            isRunning = true;
            break;
        }
    }

    if (isRunning) {
        PCB childProcess;
        //为新进程选择PID时，从2(1属于第一个系统进程)开始向上。不要重复使用已终止进程的PID。
        childProcess.pid = processes.back().pid + 1;
        childProcess.parentPid = runningProcess.pid; // 父 进 程 是 当 前 正 在 运 行 的 进 程 
        childProcess.status = "就绪";
        childProcess.fileName = "";
        childProcess.isZombie = false;
        childProcess.isWaiting = false;

        //将子进程ID添加到父进程children列表中
        for (PCB& process : processes) {
            if (process.pid == runningProcess.pid) {
                process.children.push_back(childProcess.pid);
                break;
            }
        }

        processes.push_back(childProcess);

        // 将新创建的子 进 程 添加 到 就 绪 队 列 的末尾
        readyQueue.push(childProcess);

        cout << "创建子 进 程 成 功 ！" << endl;
    }
    else {
        cout << "没有正在运行的 进 程 ，无法创建子 进 程 。" << endl;
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
        }
        else if (command == "wait") {
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
