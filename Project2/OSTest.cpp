#include <iostream>
#include <vector>
#include <queue>
#include <string>

using namespace std;

// ���̿��ƿ� (PCB) �ṹ��
struct PCB {
    int pid; // ����ID
    int parentPid; // ������ID
    string status; // ����״̬�����������С�������
    string fileName; // �ļ���
    vector<int> children; //�ӽ���ID�б�
    bool isZombie;//�Ƿ�Ϊ��ʬ����
    bool isWaiting;//�Ƿ����ڵȴ��ӽ�����ֹ
};

// Ϊ PCB �ṹ�嶨�� operator== �����
bool operator==(const PCB& a, const PCB& b) {
    return a.pid == b.pid;
}

// Ӳ��IO����ṹ��
struct IORequest {
    int pid; // ����ID
    int track; // �ŵ���
};

// ȫ�ֱ���
int memoryCapacity; // �ڴ�����
int pageSize; // ҳ��/��ܴ�С
int diskCount; // Ӳ������
int maxDiskCapacity; // ÿ��Ӳ�̵��������

vector<PCB> processes; // �����б�
queue<PCB> readyQueue; // ��������

vector<vector<int>> memory; // �ڴ�������¼ÿ��ҳ�������Ľ��̺�ҳ��
vector<int> LRUList;//LRU�б���¼ҳ�����˳��

vector<queue<IORequest>> ioQueues; // Ӳ��IO����

// ��ʼ��������ѯ���û�������Ϣ��������ʼ����
void initialize() {
    cout << "======================================" << endl;
    cout << "         ��ӭʹ���������ϵͳ��        " << endl;
    cout << "======================================" << endl;

    cout << "�����������ڴ������Ƕ��٣����ֽڣ�";
    cin >> memoryCapacity;

    cout << "����������ҳ��/��ܴ�С�Ƕ��٣�";
    cin >> pageSize;

    cout << "���������ж��ٸ�Ӳ�̣�";
    cin >> diskCount;

    cout << "ÿ��Ӳ�̵���������Ƕ���MB��";
    cin >> maxDiskCapacity;

    PCB initialProcess;
    initialProcess.pid = 1;
    initialProcess.parentPid = 0;
    initialProcess.status = "����";
    initialProcess.fileName = "";

    processes.push_back(initialProcess);

    ioQueues.resize(diskCount);

}

void createProcess() {
    PCB newProcess;
    newProcess.pid = processes.size() + 1;
    newProcess.parentPid = 1;
    newProcess.status = "����";
    newProcess.fileName = "";
    newProcess.isZombie = false;
    newProcess.isWaiting = false;

    processes.push_back(newProcess);

    processes[0].children.push_back(newProcess.pid);

    // ���ϵͳ�����������У�����״̬����Ϊ��������������������
    for (PCB& process : processes) {
        if (process.pid == 1 && process.status == "����") {
            process.status = "����";
            readyQueue.push(process);
            break;
        }
    }


    cout << "�������̳ɹ���" << endl;


}

void showProcesses() {
    cout << "======================================" << endl;
    cout << "��ǰ���еĽ��̣�" << endl;

    bool hasRunningProcess = false;
    bool hasNonSystemProcess = false;

    for (PCB& process : processes) {
        if (process.pid != 1) {
            hasNonSystemProcess = true;
        }
        if (process.status == "����") {
            cout << "����ID��" << process.pid << "��������ID��" << process.parentPid << "���ӽ���ID��";
            for (int childPid : process.children) {
                cout << childPid << " ";
            }
            cout << endl;
            hasRunningProcess = true;
        }
    }

    // ���û�з�ϵͳ���̣�����ʾϵͳ����
    if (!hasNonSystemProcess && !hasRunningProcess) {
        for (PCB& process : processes) {
            if (process.pid == 1 && process.status != "�ȴ�") {
                cout << "����ID��" << process.pid << "��������ID��" << process.parentPid << "���ӽ���ID��";
                for (int childPid : process.children) {
                    cout << childPid << " ";
                }
                cout << endl;
                break;
            }
        }
    }

    cout << "���������еĽ��̣�" << endl;
    queue<PCB> tempQueue;
    while (!readyQueue.empty()) {
        PCB process = readyQueue.front();
        readyQueue.pop();
        tempQueue.push(process);

        if (process.status != "�ȴ�" && process.pid != 1) {
            cout << "����ID��" << process.pid << "��������ID��" << process.parentPid << "���ӽ���ID��";
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
            if (process.status == "����") {
                process.status = "����";
                readyQueue.push(process);
                break;
            }
        }

        PCB runningProcess = readyQueue.front();
        readyQueue.pop();
        cout << "�������еĽ��̣�" << runningProcess.pid << endl;

        for (PCB& process : processes) {
            if (process.pid == runningProcess.pid) {
                process.status = "����";
                break;
            }
        }

        cout << "======================================" << endl;
        cout << "���������";
    }
}

void forkProcess() {
    PCB* runningProcess = nullptr;
    bool isRunning = false;

    for (PCB& process : processes) {
        if (process.status == "����") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    if (isRunning && runningProcess != nullptr) {

        PCB childProcess;

        childProcess.pid = processes.back().pid + 1;
        childProcess.status = "����";
        childProcess.fileName = "";
        childProcess.isZombie = false;
        childProcess.isWaiting = false;

        // �ڴ����ӽ���ʱ��ȷ���ӽ��̵ĸ������ǵ�ǰ�������еĽ���
        for (PCB& process : processes) {
            if (process.status == "����") {
                childProcess.parentPid = process.pid;
                process.children.push_back(childProcess.pid);
                break;
            }
        }

        processes.push_back(childProcess);

        readyQueue.push(childProcess);

        cout << "�����ӽ��̳ɹ���" << endl;
    }
    else {
        cout << "û���������еĽ���,�޷������ӽ��̡�" << endl;
    }
}

void exit() {
    PCB* runningProcess = nullptr;
    bool isRunning = false;

    for (PCB& process : processes) {
        if (process.status == "����") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    if (isRunning && runningProcess != nullptr) {

        // ����˳��Ľ�����ϵͳ���̣�������ִ��exit����
        if (runningProcess->pid == 1) {
            cout << "ϵͳ���̲���ִ��exit���" << endl;
            return;
        }

        runningProcess->status = "��ʬ";
        runningProcess->isZombie = true;

        // �����������е���һ����������Ϊ����״̬
        if (!readyQueue.empty()) {
            PCB nextRunningProcess = readyQueue.front();
            readyQueue.pop();

            for (PCB& process : processes) {
                if (process.pid == nextRunningProcess.pid) {
                    process.status = "����";
                    break;
                }
            }
        }

        // �������̴ӵȴ�״̬תΪ����״̬�������뵽�������е�ĩβ
        for (PCB& process : processes) {
            if (process.pid == runningProcess->parentPid && process.isWaiting) {
                process.status = "����";
                process.isWaiting = false;
                readyQueue.push(process);
                break;
            }
        }

        // ����Ƿ���������ϵͳ���������л��ھ���״̬
        bool hasNonSystemProcess = false;
        for (PCB& process : processes) {
            if (process.pid != 1 && (process.status == "����" || process.status == "����")) {
                hasNonSystemProcess = true;
                break;
            }
        }

        // ���û��������ϵͳ���������л��ھ���״̬����ϵͳ����1����Ϊ����״̬
        if (!hasNonSystemProcess) {
            for (PCB& process : processes) {
                if (process.pid == 1) {
                    process.status = "����";
                    break;
                }
            }
        }

    }
    else {
        cout << "û���������еĽ��̣��޷�ִ��exit���" << endl;
    }
}

void wait() {
    PCB* runningProcess = nullptr;
    bool isRunning = false;

    for (PCB& process : processes) {
        if (process.status == "����") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    if (isRunning && runningProcess != nullptr) {

        // ����ȴ��Ľ�����ϵͳ���̣�������ִ��wait����
        if (runningProcess->pid == 1) {
            cout << "ϵͳ���̲���ִ��wait���" << endl;
            return;
        }

        runningProcess->status = "�ȴ�";
        runningProcess->isWaiting = true;

        // �����������е���һ����ϵͳ��������Ϊ����״̬
        while (!readyQueue.empty()) {
            PCB nextRunningProcess = readyQueue.front();
            readyQueue.pop();

            if (nextRunningProcess.pid != 1) {
                for (PCB& process : processes) {
                    if (process.pid == nextRunningProcess.pid) {
                        process.status = "����";
                        break;
                    }
                }
                break;
            }
        }

    }
    else {
        cout << "û���������еĽ��̣��޷�ִ��wait���" << endl;
    }
}

// �������IO����
void handleIORequest(int diskNumber, int track) {
    if (diskNumber >= 0 && diskNumber < diskCount) {
        PCB* runningProcess = nullptr;

        // ���ҵ�ǰ�������еĽ���
        for (PCB& process : processes) {
            if (process.status == "����") {
                runningProcess = &process;
                break;
            }
        }

        if (runningProcess != nullptr) {
            IORequest ioRequest;
            ioRequest.pid = runningProcess->pid;
            ioRequest.track = track;

            // ��IO������ӵ�Ӳ�̶����У���������״̬����Ϊ���ȴ���
            ioQueues[diskNumber].push(ioRequest);
            runningProcess->status = "�ȴ�";

            cout << "����" << runningProcess->pid << "�������" << diskNumber << "��ȡ��д��ŵ�" << track << endl;

            // ��鵱ǰ�Ƿ����������еĽ��̣����û�У���Ӿ��������е���һ������
            bool hasRunningProcess = false;
            for (const PCB& process : processes) {
                if (process.status == "����") {
                    hasRunningProcess = true;
                    break;
                }
            }

            if (!hasRunningProcess && !readyQueue.empty()) {
                while (!readyQueue.empty()) {
                    PCB nextProcess = readyQueue.front();
                    readyQueue.pop();

                    // ����Ӿ��������е��ȵĽ����ǽ���1��ϵͳ���̣����������������Ե�����һ������
                    if (nextProcess.pid == 1) {
                        continue;
                    }

                    for (PCB& process : processes) {
                        if (process.pid == nextProcess.pid) {
                            process.status = "����";
                            cout << "����" << process.pid << "�ѱ�����Ϊ����״̬��" << endl;
                            break;
                        }
                    }

                    break;  // ���ȳɹ����˳�ѭ��
                }
            }

        }
        else {
            cout << "û���������еĽ��̣��޷��������IO����" << endl;
        }
    }
    else {
        cout << "���̺ų�����Χ��" << endl;
    }
}

// �������IO���
void handleIODone(int diskNumber) {
    if (diskNumber >= 0 && diskNumber < diskCount) {
        if (!ioQueues[diskNumber].empty()) {
            IORequest ioRequest = ioQueues[diskNumber].front();
            ioQueues[diskNumber].pop();

            PCB* requestingProcess = nullptr;

            for (PCB& process : processes) {
                if (process.pid == ioRequest.pid) {
                    requestingProcess = &process;
                    break;
                }
            }

            if (requestingProcess != nullptr) {
                requestingProcess->status = "����";
                readyQueue.push(*requestingProcess);
            }

            cout << "����" << diskNumber << "��ɽ���" << ioRequest.pid << "��IO���󣬽����Ѽ���������С�" << endl;
        }
        else {
            cout << "��ǰ����û�еȴ���IO����" << endl;
        }
    }
    else {
        cout << "���̺ų�����Χ��" << endl;
    }

    // ��鵱ǰ�Ƿ����������еĽ��̣����û�У���Ӿ��������е���һ������
    bool hasRunningProcess = false;
    for (const PCB& process : processes) {
        if (process.status == "����") {
            hasRunningProcess = true;
            break;
        }
    }

    if (!hasRunningProcess && !readyQueue.empty()) {
        PCB nextProcess = readyQueue.front();
        readyQueue.pop();

        for (PCB& process : processes) {
            if (process.pid == nextProcess.pid) {
                process.status = "����";
                cout << "����" << process.pid << "�ѱ�����Ϊ����״̬��" << endl;
                break;
            }
        }
    }
}

// �����ڴ����
void requestMemory(int pid) {
    int freeFrame = -1;

    for (int i = 0; i < memory.size(); ++i) {
        if (memory[i].empty()) {
            freeFrame = i;
            break;
        }
    }

    if (freeFrame == -1) {
        if (static_cast<int>(memory.size()) * pageSize < memoryCapacity) {
            vector<int> newPage = { pid, static_cast<int>(memory.size()) };
            memory.push_back(newPage);
            LRUList.push_back(static_cast<int>(memory.size()) - 1);
            cout << "Ϊ����" << pid << "����ҳ��" << static_cast<int>(memory.size()) - 1 << endl;
        }
        else {
            // ʹ��LRU�㷨�滻ҳ��
            int leastRecentlyUsedFrame = LRUList.front();
            LRUList.erase(LRUList.begin());
            vector<int> replacedPage = { pid, leastRecentlyUsedFrame };
            memory[leastRecentlyUsedFrame] = replacedPage;
            LRUList.push_back(leastRecentlyUsedFrame);
            cout << "Ϊ����" << pid << "�滻ҳ��" << leastRecentlyUsedFrame << endl;
        }
    }
    else {
        vector<int> newPage = { pid, freeFrame };
        memory[freeFrame] = newPage;
        LRUList.push_back(freeFrame);
        cout << "Ϊ����" << pid << "����ҳ��" << freeFrame << endl;
    }
}

// �ͷ��ڴ�
void releaseMemory(int pid) {
    for (int i = 0; i < memory.size(); ++i) {
        if (!memory[i].empty() && memory[i][0] == pid) {
            memory[i].clear();
            LRUList.erase(remove(LRUList.begin(), LRUList.end(), i), LRUList.end());
            cout << "�ͷŽ���" << pid << "��ҳ��" << i << endl;
        }
    }
}

// �����߼���ַ
void handleAddress(int address) {
    PCB* runningProcess = nullptr;

    for (PCB& process : processes) {
        if (process.status == "����") {
            runningProcess = &process;
            break;
        }
    }

    if (runningProcess != nullptr) {
        int pageNumber = address / pageSize;
        int offset = address % pageSize;

        for (int i = 0; i < memory.size(); ++i) {
            if (!memory[i].empty() && memory[i][0] == runningProcess->pid && memory[i][1] == pageNumber) {
                LRUList.erase(remove(LRUList.begin(), LRUList.end(), i), LRUList.end());
                LRUList.push_back(i);
                cout << "����" << runningProcess->pid << "����ҳ��" << pageNumber << "�������ַΪ��" << i * pageSize + offset << endl;
                return;
            }
        }

        cout << "����" << runningProcess->pid << "���ʵ�ҳ��" << pageNumber << "�����ڴ��У���Ҫ�����ڴ���䡣" << endl;
        requestMemory(runningProcess->pid);
    }
    else {
        cout << "û���������еĽ��̣��޷������߼���ַ��" << endl;
    }
}


// ��ʾӲ��IO״̬
void showIOStatus() {
    for (int i = 0; i < diskCount; i++) {
        cout << "======================================" << endl;
        cout << "Ӳ��" << i << "�ϵĽ��̣�" << endl;

        queue<IORequest> tempQueue = ioQueues[i];  // ����һ����ʱ�����Ա����޸�ԭʼ����

        while (!tempQueue.empty()) {
            IORequest ioRequest = tempQueue.front();
            tempQueue.pop();

            cout << "����ID��" << ioRequest.pid << "���ŵ���" << ioRequest.track << endl;
        }
    }

    cout << "======================================" << endl;
}

// ��ʾ�ڴ�״̬
void showMemoryStatus() {
    cout << "======================================" << endl;
    for (int i = 0; i < memory.size(); i++) {
        vector<int> page = memory[i];

        if (!page.empty()) {
            int processId = page[0];
            int pageNumber = page[1];

            cout << "ҳ��" << i << "�е�ҳ�棺" << endl;
            cout << "����ID��" << processId << "��ҳ�ţ�" << pageNumber << endl;
        }
    }

    cout << "======================================" << endl;
}

int main() {
    initialize();

    string command;
    cout << "���������";
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

        cout << "���������";
    }

    return 0;
}
