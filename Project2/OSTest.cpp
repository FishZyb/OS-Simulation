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
}

// �����½���
void createProcess() {
    PCB newProcess;
    newProcess.pid = processes.size() + 1;
    newProcess.parentPid = 1; // ������Ϊ��ʼ����
    newProcess.status = "����";
    newProcess.fileName = "";
    newProcess.isZombie = false;
    newProcess.isWaiting = false;

    processes.push_back(newProcess);

    //���´����Ľ�����ӵ���������ǰ��
    readyQueue.push(newProcess);

    //���ӽ��̵�ID��ӵ������̵�children�б���
    processes[0].children.push_back(newProcess.pid);

    cout << "�������̳ɹ���" << endl;
}

// ���̵��ȣ�ʹ��RR�����㷨��ʱ��Ƭ��ת�����㷨��
void schedule() {
    if (!readyQueue.empty()) {
        //���µ�ǰ���еĽ���״̬,������ǰ���̷ŵ���������ĩβ
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
        //ִ��һ��ʱ��Ƭ
        //���ﲻ�����κ�ʱ��Ƭ����"-"ָ���ʾһ��������ǰ�ƽ�����

        //����process�б��ж�Ӧ���̵�״̬
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

//�����ӽ���
void forkProcess() {
    //�ҵ���ǰ�������еĽ���
    PCB* runningProcess = nullptr;
    for (PCB& process : processes) {
        if (process.status == "����") {
            runningProcess = &process;
            break;
        }
    }
    if (runningProcess == nullptr) {
        cout << "û���������еĽ��̣��޷������ӽ��̣�" << endl;
        return;
    }
    //����һ���µ��ӽ��̣����ǵ�ǰ�������еĽ��̵ĸ���
    PCB newProcess = *runningProcess;
    //Ϊ�´������ӽ��̷���һ��Ψһ�ҵ�����PID
    newProcess.pid = processes.back().pid + 1;
    //�����´������ӽ���״̬Ϊ����
    newProcess.status = "����";
    //���ӽ���ID��ӵ�������children�б���
    runningProcess->children.push_back(newProcess.pid);
    processes.push_back(newProcess);

    //���´������ӽ�����ӵ���������ĩβ
    readyQueue.push(newProcess);
       
    cout << "�����ӽ��̳ɹ���" << endl;
    
}

// �������IO����
void handleIORequest(int diskNumber, int track) {
    if (diskNumber >= 0 && diskNumber < diskCount) {
        if (!readyQueue.empty()) {
            PCB requestingProcess = readyQueue.front();
            readyQueue.pop();

            IORequest ioRequest;
            ioRequest.pid = requestingProcess.pid;
            ioRequest.track = track;

            ioQueues[diskNumber].push(ioRequest);

            cout << "����" << requestingProcess.pid << "�������" << diskNumber << "��ȡ��д��ŵ�" << track << endl;
        }
    }
}

// �������IO���
void handleIODone(int diskNumber) {
    if (diskNumber >= 0 && diskNumber < diskCount) {
        if (!ioQueues[diskNumber].empty()) {
            IORequest ioRequest = ioQueues[diskNumber].front();
            ioQueues[diskNumber].pop();

            cout << "����" << diskNumber << "��ɽ���" << ioRequest.pid << "��IO����" << endl;
        }
    }
}

// �����ڴ����
void requestMemory(int pid) {
    // TODO: ʵ�������ڴ������߼�
    int numPages = memoryCapacity / pageSize; // �����ڴ���ҳ�������
    // ����Ƿ����㹻�Ŀ���ҳ��
    int count = 0;
    for (int i = 0; i < numPages; i++) {
        if (memory[i].empty()) {
            count++;
            if (count == 4) { // �����������4������ҳ�棬����������
                for (int j = i - 3; j <= i; j++) {
                    memory[j] = { pid, j - (i - 3) };
                }
                cout << "����" << pid << "�����" << count << "��������ҳ��" << endl;
                return;
            }
        }
        else {
            count = 0;
        }
    }

    // ���û��������4������ҳ�棬���޷����������
    cout << "�ڴ���û���㹻�Ŀ���ҳ�棬����" << pid << "�޷������ڴ�" << endl;
}

// �ͷ��ڴ�
void releaseMemory(int pid) {
    // TODO: ʵ���ͷ��ڴ���߼�
    int numPages = memoryCapacity / pageSize; // �����ڴ���ҳ�������
    for (int i = 0; i < numPages; i++) {
        if (!memory[i].empty() && memory[i][0] == pid) { // �ҵ����ڸý��̵�ҳ��
            memory[i].clear(); // ��ո�ҳ��
        }
    }

    cout << "����" << pid << "�ͷ����ڴ�" << endl;

}

// ��ʾ��ǰ���̺;��������еĽ���
void showProcesses() {
    cout << "======================================" << endl;
    cout << "��ǰ���еĽ��̣�" << endl;
    for (PCB process : processes) {
        if (process.status == "����") {
            cout << "����ID��" << process.pid << "��������ID��" << process.parentPid << "���ӽ���ID��";
            for (int childPid : process.children) {
                cout << childPid << " ";
            }
            cout << endl;
        }
    }

    cout << "���������еĽ��̣�" << endl;
    queue<PCB> tempQueue;
    while (!readyQueue.empty()) {
        PCB process = readyQueue.front();
        readyQueue.pop();
        tempQueue.push(process);

        cout << "����ID��" << process.pid << "��������ID��" << process.parentPid << "���ӽ���ID��";
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

// ��ʾӲ��IO״̬
void showIOStatus() {
    cout << "======================================" << endl;
    for (int i = 0; i < diskCount; i++) {
        cout << "Ӳ��" << i << "�ϵĽ��̣�" << endl;

        queue<IORequest> ioQueue = ioQueues[i];
        while (!ioQueue.empty()) {
            IORequest ioRequest = ioQueue.front();
            ioQueue.pop();

            cout << "����" << ioRequest.pid << "���ڶ�ȡ��д��ŵ�" << ioRequest.track << endl;

            ioQueue.push(ioRequest);
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
            // TODO: ʵ��fork������߼�
        //    if (!readyQueue.empty()) {
        //        PCB parentProcess = readyQueue.front();
        //        readyQueue.pop();

        //        PCB childProcess;
        //        childProcess.pid = processes.size() + 1;
        //        childProcess.parentPid = parentProcess.pid;
        //        childProcess.status = "����";
        //        childProcess.fileName = "";

        //        processes.push_back(childProcess);
        //        readyQueue.push(childProcess);

        //        // ���ӽ��̵�ID��ӵ������̵�children�б���
        //        parentProcess.children.push_back(childProcess.pid);

        //        cout << "����" << childProcess.pid << "�����ɹ���" << endl;

        //        // �������̷Żؾ�������ĩβ
        //        readyQueue.push(parentProcess);
        //    }

        }
        else if (command == "exit") {
            // TODO: ʵ��exit������߼�
            if (!readyQueue.empty()) {
                PCB currentProcess = readyQueue.front();
                readyQueue.pop();

                // ��ֹ��ǰ����
                cout << "����" << currentProcess.pid << "��ֹ��" << endl;
                currentProcess.status = "��ֹ";

                // �ͷŵ�ǰ����ʹ�õ��ڴ�

                // ��������ֹ
                for (int childPid : currentProcess.children) {
                    for (PCB& process : processes) {
                        if (process.pid == childPid) {
                            process.status = "��ֹ";
                            process.isZombie = true;
                            break;
                        }
                    }
                }

                // ����ʬ����
                if (currentProcess.parentPid != -1) {
                    for (PCB& process : processes) {
                        if (process.pid == currentProcess.parentPid) {
                            if (process.isWaiting) {
                                process.status = "����";
                                readyQueue.push(process);
                                break;
                            }
                        }
                    }
                }
            }
        }
        else if (command == "wait") {
            // TODO: ʵ��wait������߼�
            if (!readyQueue.empty()) {
                PCB currentProcess = readyQueue.front();
                readyQueue.pop();

                // ����Ƿ���ڽ�ʬ�ӽ���
                bool hasZombieChild = false;
                for (int childPid : currentProcess.children) {
                    for (PCB& process : processes) {
                        if (process.pid == childPid && process.isZombie) {
                            hasZombieChild = true;
                            // ɾ����ʬ�ӽ��̵�PCB
                            processes.erase(remove_if(processes.begin(), processes.end(),
                                [&](const PCB& p) { return p.pid == childPid; }), processes.end());
                            break;
                        }
                    }
                }

                // ������ڽ�ʬ�ӽ��̣���������ʹ��CPU
                if (hasZombieChild) {
                    cout << "���ڽ�ʬ�ӽ��̣�����" << currentProcess.pid << "����ִ�У�" << endl;
                    readyQueue.push(currentProcess);
                }
                else {
                    // ����ת���������е�ĩβ
                    currentProcess.status = "�ȴ�";
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
            //cout << "����·��" << endl;
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
