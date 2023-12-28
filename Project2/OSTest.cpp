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
vector<int> LRUList;//LRU列表，记录页面访问顺序

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

/*
    createProcess()函数的主要目标是创建一个新的进程，并将其添加到系统中。
    它首先创建一个新的进程控制块（PCB）对象，并设置其属性，然后将新创建的进程添加到进程列表中。
    如果系统进程仍在运行，它会将其状态更改为就绪，并将其放入就绪队列中。
*/
void createProcess() {
    //创建一个新的进程控制块（PCB）对象
    PCB newProcess;
    //设置新进程的ID
    newProcess.pid = processes.size() + 1;
    //设置新进程的父进程ID
    newProcess.parentPid = 1;
    //设置新进程的状态为“运行”
    newProcess.status = "运行";
    //初始化新进程关联的文件名为空
    newProcess.fileName = "";
    //初始化新进程为非僵尸进程
    newProcess.isZombie = false;
    //初始化新进程为非等待状态
    newProcess.isWaiting = false;

    //将新创建的进程添加到进程列表中
    processes.push_back(newProcess);

    //将新创建的进程ID添加到系统进程（id=1）的子进程列表中
    processes[0].children.push_back(newProcess.pid);

    //遍历所有进程
    for (PCB& process : processes) {
        //如果找到了进程id为1（系统进程），且状态为运行时
        if (process.pid == 1 && process.status == "运行") {
            //将该系统进程的状态设置为“就绪”
            process.status = "就绪";
            //放入就绪队列中
            readyQueue.push(process);
            break;
        }
    }
    cout << "创建进程成功！" << endl;
}

/*
    showProcesses()函数的主要目标是显示当前运行的进程以及就绪队列中的进程。
    它首先遍历所有进程，找到并输出正在运行的进程。
    然后，它遍历就绪队列中的进程并输出它们的信息。
    如果没有非系统进程，则显示系统进程。
    在遍历过程中，它使用一个临时队列来存储已经遍历过的进程，最后将这些进程放回到就绪队列中。
*/
void showProcesses() {
    cout << "======================================" << endl;
    cout << "当前运行的进程：" << endl;

    //初始化一个布尔变量，表示是否有正在运行的进程
    bool hasRunningProcess = false;
    //初始化一个布尔变量，表示是否有非系统进程
    bool hasNonSystemProcess = false;

    //遍历所有进程
    for (PCB& process : processes) {
        //如果找到非系统进程，则将hasRunningProcess比设置为true，表示当前有存在非系统进程
        if (process.pid != 1) {
            hasNonSystemProcess = true;
        }
        //如果找到正在运行的进程
        if (process.status == "运行") {
            //输出内容
            cout << "进程ID：" << process.pid << "，父进程ID：" << process.parentPid << "，子进程ID：";
            //遍历输出该进程的子进程id
            for (int childPid : process.children) {
                cout << childPid << " ";
            }
            cout << endl;
            //此时存在正在运行的进程
            hasRunningProcess = true;
        }
    }

    // 如果没有非系统进程，且没有正在运行的进程，则显示系统进程
    if (!hasNonSystemProcess && !hasRunningProcess) {
        //遍历进程
        for (PCB& process : processes) {
            //拿到系统进程
            if (process.pid == 1 && process.status != "等待") {
                //输出内容
                cout << "进程ID：" << process.pid << "，父进程ID：" << process.parentPid << "，子进程ID：";
                //遍历输出子进程列表
                for (int childPid : process.children) {
                    cout << childPid << " ";
                }
                cout << endl;
                break;
            }
        }
    }

    cout << "就绪队列中的进程：" << endl;
    //创建一个临时队列，用于存储就绪队列中的进程
    queue<PCB> tempQueue;
    //遍历就绪队列中的进程
    while (!readyQueue.empty()) {
        //获取队列中的第一个进程
        PCB process = readyQueue.front();
        //将这个进程从就绪队列中移除
        readyQueue.pop();
        //将这个进程放入临时队列中
        tempQueue.push(process);

        //如果这个进程不是等待状态且不是系统进程，则输出其信息
        if (process.status != "等待" && process.pid != 1) {
            //输出信息
            cout << "进程ID：" << process.pid << "，父进程ID：" << process.parentPid << "，子进程ID：";
            //遍历输出子进程信息
            for (int childPid : process.children) {
                cout << childPid << " ";
            }
            cout << endl;
        }
    }

    //将临时队列中的进程放回到就绪队列中
    while (!tempQueue.empty()) {
        //获取临时队列中的第一个进程
        PCB process = tempQueue.front();
        //将这个进程从临时队列中移除
        tempQueue.pop();
        //将这个进程放回到就绪队列中
        readyQueue.push(process);
    }

    cout << "======================================" << endl;
}

/*
    schedule()函数的主要目标是对系统中的进程进行调度。
    它首先检查就绪队列是否为空，
    如果不为空，则将当前正在运行的进程更改为就绪状态，并将其放入就绪队列。
    然后，从就绪队列中获取第一个进程，并将其设置为正在运行。
    操作系统进程调度，时间片轮转算法（RR）
*/
void schedule() {
    //如果就绪队列不为空
    if (!readyQueue.empty()) 
        //遍历所有进程，将正在运行的进程更改为就绪状态，并将其放入就绪队列
        for (PCB& process : processes) {
            if (process.status == "运行") {
                process.status = "就绪";
                readyQueue.push(process);
                break;
            }
        }
        //获取就绪队列中的第一个进程
        PCB runningProcess = readyQueue.front();
        //将这个进程从就绪队列中移除
        readyQueue.pop();
        //输出内容
        cout << "正在运行的进程：" << runningProcess.pid << endl;
        //遍历所有进程，将刚刚从就绪队列中移除的进程设置为正在运行
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

/*
    forkProcess()函数的主要目标是创建一个新的子进程。
    它首先找到当前正在运行的进程，然后创建一个新的子进程，并设置其属性。
    接着，将新创建的子进程添加到系统中，并将其放入就绪队列。
    如果没有正在运行的进程，则输出错误信息。
*/
void forkProcess() {
    //初始化一个指向当前正在运行进程的指针
    PCB* runningProcess = nullptr;
    //初始化一个布尔变量，表示是否有正在运行的进程
    bool isRunning = false;

    //遍历所有进程，找到正在运行的进程
    for (PCB& process : processes) {
        if (process.status == "运行") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    //如果有正在运行的进程
    if (isRunning && runningProcess != nullptr) {

        //创建一个新的子进程
        PCB childProcess;

        //初始化子进程过程，设置ID、状态、文件名、是否为僵尸进程、是否为等待状态
        childProcess.pid = processes.back().pid + 1;
        childProcess.status = "就绪";
        childProcess.fileName = "";
        childProcess.isZombie = false;
        childProcess.isWaiting = false;

        // 在创建子进程时，确保子进程的父进程是当前正在运行的进程
        for (PCB& process : processes) {
            if (process.status == "运行") {
                //设置子进程的父进程ID
                childProcess.parentPid = process.pid;
                //将子进程的ID添加到父进程的子进程列表
                process.children.push_back(childProcess.pid);
                break;
            }
        }

        //将新创建的子进程添加到进程列表中
        processes.push_back(childProcess);

        //将新创建的子进程放入就绪队列中
        readyQueue.push(childProcess);

        cout << "创建子进程成功！" << endl;
    }
    else {
        cout << "没有正在运行的进程,无法创建子进程。" << endl;
    }
}

/*
    exit()函数的主要目标是使当前正在运行的进程退出。
    它首先检查是否有正在运行的进程，然后将其状态更改为“僵尸”并标记为僵尸进程。
    接着，将就绪队列中的下一个进程设置为运行状态，并将父进程从等待状态转为就绪状态。
    最后，如果没有其他非系统进程在运行或处于就绪状态，则将系统进程1设置为运行状态。
*/
void exit() {
    // 初始化一个指向当前正在运行的进程的指针
    PCB* runningProcess = nullptr;
    // 初始化一个布尔变量，表示是否有正在运行的进程
    bool isRunning = false;

    // 遍历所有进程，找到正在运行的进程
    for (PCB& process : processes) {
        if (process.status == "运行") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    // 如果有正在运行的进程
    if (isRunning && runningProcess != nullptr) {

        // 如果退出的进程是系统进程，不允许执行exit命令
        if (runningProcess->pid == 1) {
            cout << "系统进程不能执行exit命令。" << endl;
            return;
        }

        // 将当前正在运行的进程状态设置为“僵尸”
        runningProcess->status = "僵尸";
        // 将当前正在运行的进程标记为僵尸进程
        runningProcess->isZombie = true;

        // 将就绪队列中的下一个进程设置为运行状态
        if (!readyQueue.empty()) {
            // 获取就绪队列中的第一个进程
            PCB nextRunningProcess = readyQueue.front();
            // 将这个进程从就绪队列中移除
            readyQueue.pop();

            // 遍历所有进程，将刚刚从就绪队列中移除的进程设置为正在运行
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
                // 更改父进程的状态为“就绪”
                process.status = "就绪";
                // 将父进程的等待状态设置为false
                process.isWaiting = false;
                // 将父进程放入就绪队列中
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
    //输出错误信息
    else {
        cout << "没有正在运行的进程，无法执行exit命令。" << endl;
    }
}

/*
    wait()函数的主要目标是使当前正在运行的进程等待其子进程。
    它首先检查是否有正在运行的进程，然后将其状态更改为“等待”并标记为等待状态。
    接着，将就绪队列中的下一个非系统进程设置为运行状态。
*/
void wait() {
    // 初始化一个指向当前正在运行的进程的指针
    PCB* runningProcess = nullptr;
    // 初始化一个布尔变量，表示是否有正在运行的进程
    bool isRunning = false;

    // 遍历所有进程，找到正在运行的进程
    for (PCB& process : processes) {
        if (process.status == "运行") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    // 如果有正在运行的进程
    if (isRunning && runningProcess != nullptr) {

        // 如果等待的进程是系统进程，不允许执行wait命令
        if (runningProcess->pid == 1) {
            cout << "系统进程不能执行wait命令。" << endl;
            return;
        }

        // 将当前正在运行的进程状态设置为“等待”
        runningProcess->status = "等待";
        // 将当前正在运行的进程标记为等待状态
        runningProcess->isWaiting = true;

        // 将就绪队列中的下一个非系统进程设置为运行状态
        while (!readyQueue.empty()) {
            // 获取就绪队列中的第一个进程
            PCB nextRunningProcess = readyQueue.front();
            // 将这个进程从就绪队列中移除
            readyQueue.pop();

            // 如果这个进程不是系统进程，则将其设置为正在运行
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

/*
    handleIORequest()函数的主要目标是处理磁盘IO请求。
    它首先检查磁盘号是否在合法范围内，然后查找当前正在运行的进程。
    接着，将IO请求添加到硬盘队列中，并将进程状态更改为“等待”。
    最后，检查当前是否有正在运行的进程，如果没有，则从就绪队列中调度一个进程。
*/
void handleIORequest(int diskNumber, int track) {
    // 检查磁盘号是否在合法范围内
    if (diskNumber >= 0 && diskNumber < diskCount) {
        PCB* runningProcess = nullptr;

        // 查找当前正在运行的进程
        for (PCB& process : processes) {
            if (process.status == "运行") {
                runningProcess = &process;
                break;
            }
        }

        // 如果找到了正在运行的进程
        if (runningProcess != nullptr) {
            IORequest ioRequest;
            // 设置IO请求的进程ID
            ioRequest.pid = runningProcess->pid;
            // 设置IO请求的磁道号
            ioRequest.track = track;

            // 将IO请求添加到硬盘队列中，并将进程状态更改为“等待”
            ioQueues[diskNumber].push(ioRequest);
            runningProcess->status = "等待";

            cout << "进程" << runningProcess->pid << "请求磁盘" << diskNumber << "读取或写入磁道" << track << endl;

            // 检查当前是否有正在运行的进程，如果没有，则从就绪队列中调度一个进程
            bool hasRunningProcess = false;
            for (const PCB& process : processes) {
                if (process.status == "运行") {
                    hasRunningProcess = true;
                    break;
                }
            }

            // 如果没有正在运行的进程且就绪队列非空
            if (!hasRunningProcess && !readyQueue.empty()) {
                while (!readyQueue.empty()) {
                    // 获取就绪队列中的第一个进程
                    PCB nextProcess = readyQueue.front();
                    // 将这个进程从就绪队列中移除
                    readyQueue.pop();

                    // 如果从就绪队列中调度的进程是进程1（系统进程），则跳过它并尝试调度下一个进程
                    if (nextProcess.pid == 1) {
                        continue;
                    }

                    // 遍历所有进程，找到需要调度的进程
                    for (PCB& process : processes) {
                        if (process.pid == nextProcess.pid) {
                            // 将其状态设置为“运行”
                            process.status = "运行";
                            cout << "进程" << process.pid << "已被调度为运行状态。" << endl;
                            break;
                        }
                    }

                    break;  // 调度成功后退出循环
                }
            }

        }
        else {
            cout << "没有正在运行的进程，无法处理磁盘IO请求。" << endl;
        }
    }
    else {
        cout << "磁盘号超出范围。" << endl;
    }
}

/*
    handleIODone()函数的主要目标是处理磁盘IO完成。
    它首先检查磁盘号是否在合法范围内，然后从当前磁盘的IO队列中获取第一个IO请求。
    接着，将发起IO请求的进程状态设置为“就绪”并将其放入就绪队列中。
    最后，检查当前是否有正在运行的进程，如果没有，则从就绪队列中调度一个进程。
*/
void handleIODone(int diskNumber) {
    // 检查磁盘号是否在合法范围内
    if (diskNumber >= 0 && diskNumber < diskCount) {
        // 如果当前磁盘的IO队列不为空
        if (!ioQueues[diskNumber].empty()) {
            // 获取队列中的第一个IO请求
            IORequest ioRequest = ioQueues[diskNumber].front();
            // 将这个IO请求从队列中移除
            ioQueues[diskNumber].pop();

            PCB* requestingProcess = nullptr;

            // 查找发起IO请求的进程
            for (PCB& process : processes) {
                if (process.pid == ioRequest.pid) {
                    requestingProcess = &process;
                    break;
                }
            }

            // 如果找到了发起IO请求的进程
            if (requestingProcess != nullptr) {
                // 将其状态设置为“就绪”
                requestingProcess->status = "就绪";
                // 将其放入就绪队列中
                readyQueue.push(*requestingProcess);
            }

            cout << "磁盘" << diskNumber << "完成进程" << ioRequest.pid << "的IO请求，进程已加入就绪队列。" << endl;
        }
        else {
            cout << "当前磁盘没有等待的IO请求。" << endl;
        }
    }
    else {
        cout << "磁盘号超出范围。" << endl;
    }

    // 检查当前是否有正在运行的进程，如果没有，则从就绪队列中调度一个进程
    bool hasRunningProcess = false;
    for (const PCB& process : processes) {
        if (process.status == "运行") {
            hasRunningProcess = true;
            break;
        }
    }

    // 如果没有正在运行的进程且就绪队列非空
    if (!hasRunningProcess && !readyQueue.empty()) {
        // 获取就绪队列中的第一个进程
        PCB nextProcess = readyQueue.front();
        // 将这个进程从就绪队列中移除
        readyQueue.pop();

        // 遍历所有进程，找到需要调度的进程
        for (PCB& process : processes) {
            if (process.pid == nextProcess.pid) {
                // 将其状态设置为“运行”
                process.status = "运行";
                cout << "进程" << process.pid << "已被调度为运行状态。" << endl;
                break;
            }
        }
    }
}

// 请求内存分配
/*
    这段代码的主要目标是为特定的进程分配内存。
    如果内存中有空闲的帧，它会将新的页面分配到这个空闲帧。
    如果没有空闲帧但内存还有剩余空间，它会创建一个新的页面并添加到内存中。
    如果内存已满，它会使用最近最少使用（LRU）算法来找到一个最不常使用的帧，并将这个帧替换为新的页面。
*/
void requestMemory(int pid) {
    //初始化一个变量来存储找到的空闲帧的索引
    int freeFrame = -1;

    //遍历内存中的所有帧
    for (int i = 0; i < memory.size(); ++i) {
        //如果找到一个空闲帧
        if (memory[i].empty()) {
            //记录这个空闲帧的索引
            freeFrame = i;
            break;
        }
    }

    int timestamp = static_cast<int>(time(nullptr));  // 获取当前时间戳

    //如果没有找到空闲帧
    if (freeFrame == -1) {
        //如果内存还有剩余空间
        if (static_cast<int>(memory.size()) * pageSize < memoryCapacity) {
            //创建一个新页面，包含进程ID、页面索引和时间戳
            vector<int> newPage = { pid, static_cast<int>(memory.size()), timestamp };
            //将新页面添加到内存中
            memory.push_back(newPage);
            //将新页面的索引添加到LRU列表的末尾
            LRUList.push_back(static_cast<int>(memory.size()) - 1);
            cout << "为进程" << pid << "分配页面" << static_cast<int>(memory.size()) - 1 << endl;
        }
        else {
            //使用LRU算法替换页面
            //获取LRU列表中最不常使用的帧的索引
            int leastRecentlyUsedFrame = LRUList.front();
            //将这个帧从LRU列表中移除
            LRUList.erase(LRUList.begin());
            //创建一个新页面来替换最不常使用的帧，包含进程ID、被替换的帧的索引和时间戳
            vector<int> replacedPage = { pid, leastRecentlyUsedFrame, timestamp };
            //在内存中替换这个帧
            memory[leastRecentlyUsedFrame] = replacedPage;
            //将被替换的帧的索引添加到LRU列表的末尾
            LRUList.push_back(leastRecentlyUsedFrame);
            cout << "为进程" << pid << "替换页面" << leastRecentlyUsedFrame << endl;
        }
    }
    else {
        //创建一个新页面，包含进程ID、空闲帧的索引和时间戳
        vector<int> newPage = { pid, freeFrame, timestamp };
        //在内存中的空闲帧处添加新页面
        memory[freeFrame] = newPage;
        //将空闲帧的索引添加到LRU列表的末尾
        LRUList.push_back(freeFrame);
        cout << "为进程" << pid << "分配页面" << freeFrame << endl;
    }
}

/*
    releaseMemory()函数的主要目标是释放指定进程占用的内存。
    它首先遍历内存中的所有页面，然后找到需要释放的进程所占用的页面并清空该页面。
    接着，将该页面从LRU列表中移除。
*/
void releaseMemory(int pid) {
    // 遍历内存中的所有页面
    for (int i = 0; i < memory.size(); ++i) {
        // 如果找到了需要释放的进程所占用的页面
        if (!memory[i].empty() && memory[i][0] == pid) {\
            // 清空该页面
            memory[i].clear();
            // 从LRU列表中移除该页面
            LRUList.erase(remove(LRUList.begin(), LRUList.end(), i), LRUList.end());
            cout << "释放进程" << pid << "的页面" << i << endl;
        }
    }
}

/*
    handleAddress()函数的主要目标是处理逻辑地址。
    它首先查找当前正在运行的进程，然后计算逻辑地址对应的页号和偏移量。
    接着，遍历内存中的所有页面，查找访问的页面是否在内存中。
    如果访问的页面不在内存中，则请求内存分配。
    最后，将当前运行的进程移回到就绪队列末尾，并将就绪队列开头的进程设置为运行状态。
*/
void handleAddress(int address) {
    PCB* runningProcess = nullptr;

    // 遍历所有进程，找到正在运行的进程
    for (PCB& process : processes) {
        if (process.status == "运行") {
            runningProcess = &process;
            break;
        }
    }

    // 如果找到了正在运行的进程
    if (runningProcess != nullptr) {
        // 计算逻辑地址对应的页号
        int pageNumber = address / pageSize;
        // 计算逻辑地址中的偏移量
        int offset = address % pageSize;

        bool pageInMemory = false;

        // 遍历内存中的所有页面，查找访问的页面是否在内存中
        for (int i = 0; i < memory.size(); ++i) {
            if (!memory[i].empty() && memory[i][0] == runningProcess->pid && memory[i][1] == pageNumber) {
                // 将该页面从LRU列表中移除
                LRUList.erase(remove(LRUList.begin(), LRUList.end(), i), LRUList.end());
                // 将该页面重新添加到LRU列表的末尾
                LRUList.push_back(i);
                // 输出物理地址
                cout << "进程" << runningProcess->pid << "访问页面" << pageNumber << "，物理地址为：" << i * pageSize + offset << endl;
                pageInMemory = true;
                break;
            }
        }

        // 如果访问的页面不在内存中，则请求内存分配
        if (!pageInMemory) {
            cout << "进程" << runningProcess->pid << "访问的页面" << pageNumber << "不在内存中，需要请求内存分配。" << endl;
            // 传递当前运行的进程ID
            requestMemory(runningProcess->pid);  
        }

        // 将当前运行的进程移回到就绪队列末尾
        runningProcess->status = "就绪";
        readyQueue.push(*runningProcess);

        // 将就绪队列开头的进程设置为运行状态
        PCB nextProcess = readyQueue.front();
        readyQueue.pop();

        // 遍历所有进程，找到需要调度的进程
        for (PCB& process : processes) {
            if (process.pid == nextProcess.pid) {
                process.status = "运行";
                cout << "进程" << process.pid << "已被调度为运行状态。" << endl;
                break;
            }
        }
    }
    else {
        cout << "没有正在运行的进程，无法处理逻辑地址。" << endl;
    }
}


/*
    showIOStatus()函数的主要目标是显示硬盘IO状态。
    它首先遍历所有硬盘，然后创建一个临时队列以避免修改原始队列。
    接着，当临时队列不为空时，获取队列中的第一个IO请求并将其从队列中移除。
    最后，输出进程ID和磁道号。
*/
void showIOStatus() {
    // 遍历所有硬盘
    for (int i = 0; i < diskCount; i++) {
        cout << "======================================" << endl;
        cout << "硬盘" << i << "上的进程：" << endl;

        // 创建一个临时队列以避免修改原始队列
        queue<IORequest> tempQueue = ioQueues[i];  

        // 当临时队列不为空时
        while (!tempQueue.empty()) {
            // 获取队列中的第一个IO请求
            IORequest ioRequest = tempQueue.front();
            // 将这个IO请求从队列中移除
            tempQueue.pop();

            // 输出进程ID和磁道号
            cout << "进程ID：" << ioRequest.pid << "，磁道：" << ioRequest.track << endl;
        }
    }

    cout << "======================================" << endl;
}

/*
    showMemoryStatus()函数的主要目标是显示内存状态。
    它首先输出表头，然后遍历内存中的所有页面。
    接着，如果当前页面不为空（被某个进程占用），则输出页框号、页面号、进程ID和时间戳；
    如果当前页面为空，则输出“空闲”。
*/
void showMemoryStatus() {
    // 输出表头
    cout << "页框\t页面\tPID\t时间戳" << endl;
    // 遍历内存中的所有页面
    for (int i = 0; i < memory.size(); ++i) {
        const auto& frame = memory[i];
        // 如果当前页面不为空（被某个进程占用）
        if (!frame.empty()) {
            // 输出页框号、页面号、进程ID和时间戳
            cout << i << "\t" << frame[1] << "\t" << frame[0] << "\t" << frame[2] << endl;
        }
        else {
            // 如果当前页面为空，则输出“空闲”
            cout << i << "\t空闲" << endl;
        }
    }
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
            int address = stoi(command.substr(2));

            handleAddress(address);
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
