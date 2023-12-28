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

/*
    createProcess()��������ҪĿ���Ǵ���һ���µĽ��̣���������ӵ�ϵͳ�С�
    �����ȴ���һ���µĽ��̿��ƿ飨PCB�����󣬲����������ԣ�Ȼ���´����Ľ�����ӵ������б��С�
    ���ϵͳ�����������У����Ὣ��״̬����Ϊ�����������������������С�
*/
void createProcess() {
    //����һ���µĽ��̿��ƿ飨PCB������
    PCB newProcess;
    //�����½��̵�ID
    newProcess.pid = processes.size() + 1;
    //�����½��̵ĸ�����ID
    newProcess.parentPid = 1;
    //�����½��̵�״̬Ϊ�����С�
    newProcess.status = "����";
    //��ʼ���½��̹������ļ���Ϊ��
    newProcess.fileName = "";
    //��ʼ���½���Ϊ�ǽ�ʬ����
    newProcess.isZombie = false;
    //��ʼ���½���Ϊ�ǵȴ�״̬
    newProcess.isWaiting = false;

    //���´����Ľ�����ӵ������б���
    processes.push_back(newProcess);

    //���´����Ľ���ID��ӵ�ϵͳ���̣�id=1�����ӽ����б���
    processes[0].children.push_back(newProcess.pid);

    //�������н���
    for (PCB& process : processes) {
        //����ҵ��˽���idΪ1��ϵͳ���̣�����״̬Ϊ����ʱ
        if (process.pid == 1 && process.status == "����") {
            //����ϵͳ���̵�״̬����Ϊ��������
            process.status = "����";
            //�������������
            readyQueue.push(process);
            break;
        }
    }
    cout << "�������̳ɹ���" << endl;
}

/*
    showProcesses()��������ҪĿ������ʾ��ǰ���еĽ����Լ����������еĽ��̡�
    �����ȱ������н��̣��ҵ�������������еĽ��̡�
    Ȼ�����������������еĽ��̲�������ǵ���Ϣ��
    ���û�з�ϵͳ���̣�����ʾϵͳ���̡�
    �ڱ��������У���ʹ��һ����ʱ�������洢�Ѿ��������Ľ��̣������Щ���̷Żص����������С�
*/
void showProcesses() {
    cout << "======================================" << endl;
    cout << "��ǰ���еĽ��̣�" << endl;

    //��ʼ��һ��������������ʾ�Ƿ����������еĽ���
    bool hasRunningProcess = false;
    //��ʼ��һ��������������ʾ�Ƿ��з�ϵͳ����
    bool hasNonSystemProcess = false;

    //�������н���
    for (PCB& process : processes) {
        //����ҵ���ϵͳ���̣���hasRunningProcess������Ϊtrue����ʾ��ǰ�д��ڷ�ϵͳ����
        if (process.pid != 1) {
            hasNonSystemProcess = true;
        }
        //����ҵ��������еĽ���
        if (process.status == "����") {
            //�������
            cout << "����ID��" << process.pid << "��������ID��" << process.parentPid << "���ӽ���ID��";
            //��������ý��̵��ӽ���id
            for (int childPid : process.children) {
                cout << childPid << " ";
            }
            cout << endl;
            //��ʱ�����������еĽ���
            hasRunningProcess = true;
        }
    }

    // ���û�з�ϵͳ���̣���û���������еĽ��̣�����ʾϵͳ����
    if (!hasNonSystemProcess && !hasRunningProcess) {
        //��������
        for (PCB& process : processes) {
            //�õ�ϵͳ����
            if (process.pid == 1 && process.status != "�ȴ�") {
                //�������
                cout << "����ID��" << process.pid << "��������ID��" << process.parentPid << "���ӽ���ID��";
                //��������ӽ����б�
                for (int childPid : process.children) {
                    cout << childPid << " ";
                }
                cout << endl;
                break;
            }
        }
    }

    cout << "���������еĽ��̣�" << endl;
    //����һ����ʱ���У����ڴ洢���������еĽ���
    queue<PCB> tempQueue;
    //�������������еĽ���
    while (!readyQueue.empty()) {
        //��ȡ�����еĵ�һ������
        PCB process = readyQueue.front();
        //��������̴Ӿ����������Ƴ�
        readyQueue.pop();
        //��������̷�����ʱ������
        tempQueue.push(process);

        //���������̲��ǵȴ�״̬�Ҳ���ϵͳ���̣����������Ϣ
        if (process.status != "�ȴ�" && process.pid != 1) {
            //�����Ϣ
            cout << "����ID��" << process.pid << "��������ID��" << process.parentPid << "���ӽ���ID��";
            //��������ӽ�����Ϣ
            for (int childPid : process.children) {
                cout << childPid << " ";
            }
            cout << endl;
        }
    }

    //����ʱ�����еĽ��̷Żص�����������
    while (!tempQueue.empty()) {
        //��ȡ��ʱ�����еĵ�һ������
        PCB process = tempQueue.front();
        //��������̴���ʱ�������Ƴ�
        tempQueue.pop();
        //��������̷Żص�����������
        readyQueue.push(process);
    }

    cout << "======================================" << endl;
}

/*
    schedule()��������ҪĿ���Ƕ�ϵͳ�еĽ��̽��е��ȡ�
    �����ȼ����������Ƿ�Ϊ�գ�
    �����Ϊ�գ��򽫵�ǰ�������еĽ��̸���Ϊ����״̬�����������������С�
    Ȼ�󣬴Ӿ��������л�ȡ��һ�����̣�����������Ϊ�������С�
    ����ϵͳ���̵��ȣ�ʱ��Ƭ��ת�㷨��RR��
*/
void schedule() {
    //����������в�Ϊ��
    if (!readyQueue.empty()) 
        //�������н��̣����������еĽ��̸���Ϊ����״̬������������������
        for (PCB& process : processes) {
            if (process.status == "����") {
                process.status = "����";
                readyQueue.push(process);
                break;
            }
        }
        //��ȡ���������еĵ�һ������
        PCB runningProcess = readyQueue.front();
        //��������̴Ӿ����������Ƴ�
        readyQueue.pop();
        //�������
        cout << "�������еĽ��̣�" << runningProcess.pid << endl;
        //�������н��̣����ոմӾ����������Ƴ��Ľ�������Ϊ��������
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

/*
    forkProcess()��������ҪĿ���Ǵ���һ���µ��ӽ��̡�
    �������ҵ���ǰ�������еĽ��̣�Ȼ�󴴽�һ���µ��ӽ��̣������������ԡ�
    ���ţ����´������ӽ�����ӵ�ϵͳ�У����������������С�
    ���û���������еĽ��̣������������Ϣ��
*/
void forkProcess() {
    //��ʼ��һ��ָ��ǰ�������н��̵�ָ��
    PCB* runningProcess = nullptr;
    //��ʼ��һ��������������ʾ�Ƿ����������еĽ���
    bool isRunning = false;

    //�������н��̣��ҵ��������еĽ���
    for (PCB& process : processes) {
        if (process.status == "����") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    //������������еĽ���
    if (isRunning && runningProcess != nullptr) {

        //����һ���µ��ӽ���
        PCB childProcess;

        //��ʼ���ӽ��̹��̣�����ID��״̬���ļ������Ƿ�Ϊ��ʬ���̡��Ƿ�Ϊ�ȴ�״̬
        childProcess.pid = processes.back().pid + 1;
        childProcess.status = "����";
        childProcess.fileName = "";
        childProcess.isZombie = false;
        childProcess.isWaiting = false;

        // �ڴ����ӽ���ʱ��ȷ���ӽ��̵ĸ������ǵ�ǰ�������еĽ���
        for (PCB& process : processes) {
            if (process.status == "����") {
                //�����ӽ��̵ĸ�����ID
                childProcess.parentPid = process.pid;
                //���ӽ��̵�ID��ӵ������̵��ӽ����б�
                process.children.push_back(childProcess.pid);
                break;
            }
        }

        //���´������ӽ�����ӵ������б���
        processes.push_back(childProcess);

        //���´������ӽ��̷������������
        readyQueue.push(childProcess);

        cout << "�����ӽ��̳ɹ���" << endl;
    }
    else {
        cout << "û���������еĽ���,�޷������ӽ��̡�" << endl;
    }
}

/*
    exit()��������ҪĿ����ʹ��ǰ�������еĽ����˳���
    �����ȼ���Ƿ����������еĽ��̣�Ȼ����״̬����Ϊ����ʬ�������Ϊ��ʬ���̡�
    ���ţ������������е���һ����������Ϊ����״̬�����������̴ӵȴ�״̬תΪ����״̬��
    ������û��������ϵͳ���������л��ھ���״̬����ϵͳ����1����Ϊ����״̬��
*/
void exit() {
    // ��ʼ��һ��ָ��ǰ�������еĽ��̵�ָ��
    PCB* runningProcess = nullptr;
    // ��ʼ��һ��������������ʾ�Ƿ����������еĽ���
    bool isRunning = false;

    // �������н��̣��ҵ��������еĽ���
    for (PCB& process : processes) {
        if (process.status == "����") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    // ������������еĽ���
    if (isRunning && runningProcess != nullptr) {

        // ����˳��Ľ�����ϵͳ���̣�������ִ��exit����
        if (runningProcess->pid == 1) {
            cout << "ϵͳ���̲���ִ��exit���" << endl;
            return;
        }

        // ����ǰ�������еĽ���״̬����Ϊ����ʬ��
        runningProcess->status = "��ʬ";
        // ����ǰ�������еĽ��̱��Ϊ��ʬ����
        runningProcess->isZombie = true;

        // �����������е���һ����������Ϊ����״̬
        if (!readyQueue.empty()) {
            // ��ȡ���������еĵ�һ������
            PCB nextRunningProcess = readyQueue.front();
            // ��������̴Ӿ����������Ƴ�
            readyQueue.pop();

            // �������н��̣����ոմӾ����������Ƴ��Ľ�������Ϊ��������
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
                // ���ĸ����̵�״̬Ϊ��������
                process.status = "����";
                // �������̵ĵȴ�״̬����Ϊfalse
                process.isWaiting = false;
                // �������̷������������
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
    //���������Ϣ
    else {
        cout << "û���������еĽ��̣��޷�ִ��exit���" << endl;
    }
}

/*
    wait()��������ҪĿ����ʹ��ǰ�������еĽ��̵ȴ����ӽ��̡�
    �����ȼ���Ƿ����������еĽ��̣�Ȼ����״̬����Ϊ���ȴ��������Ϊ�ȴ�״̬��
    ���ţ������������е���һ����ϵͳ��������Ϊ����״̬��
*/
void wait() {
    // ��ʼ��һ��ָ��ǰ�������еĽ��̵�ָ��
    PCB* runningProcess = nullptr;
    // ��ʼ��һ��������������ʾ�Ƿ����������еĽ���
    bool isRunning = false;

    // �������н��̣��ҵ��������еĽ���
    for (PCB& process : processes) {
        if (process.status == "����") {
            runningProcess = &process;
            isRunning = true;
            break;
        }
    }

    // ������������еĽ���
    if (isRunning && runningProcess != nullptr) {

        // ����ȴ��Ľ�����ϵͳ���̣�������ִ��wait����
        if (runningProcess->pid == 1) {
            cout << "ϵͳ���̲���ִ��wait���" << endl;
            return;
        }

        // ����ǰ�������еĽ���״̬����Ϊ���ȴ���
        runningProcess->status = "�ȴ�";
        // ����ǰ�������еĽ��̱��Ϊ�ȴ�״̬
        runningProcess->isWaiting = true;

        // �����������е���һ����ϵͳ��������Ϊ����״̬
        while (!readyQueue.empty()) {
            // ��ȡ���������еĵ�һ������
            PCB nextRunningProcess = readyQueue.front();
            // ��������̴Ӿ����������Ƴ�
            readyQueue.pop();

            // ���������̲���ϵͳ���̣���������Ϊ��������
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

/*
    handleIORequest()��������ҪĿ���Ǵ������IO����
    �����ȼ����̺��Ƿ��ںϷ���Χ�ڣ�Ȼ����ҵ�ǰ�������еĽ��̡�
    ���ţ���IO������ӵ�Ӳ�̶����У���������״̬����Ϊ���ȴ�����
    ��󣬼�鵱ǰ�Ƿ����������еĽ��̣����û�У���Ӿ��������е���һ�����̡�
*/
void handleIORequest(int diskNumber, int track) {
    // �����̺��Ƿ��ںϷ���Χ��
    if (diskNumber >= 0 && diskNumber < diskCount) {
        PCB* runningProcess = nullptr;

        // ���ҵ�ǰ�������еĽ���
        for (PCB& process : processes) {
            if (process.status == "����") {
                runningProcess = &process;
                break;
            }
        }

        // ����ҵ����������еĽ���
        if (runningProcess != nullptr) {
            IORequest ioRequest;
            // ����IO����Ľ���ID
            ioRequest.pid = runningProcess->pid;
            // ����IO����Ĵŵ���
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

            // ���û���������еĽ����Ҿ������зǿ�
            if (!hasRunningProcess && !readyQueue.empty()) {
                while (!readyQueue.empty()) {
                    // ��ȡ���������еĵ�һ������
                    PCB nextProcess = readyQueue.front();
                    // ��������̴Ӿ����������Ƴ�
                    readyQueue.pop();

                    // ����Ӿ��������е��ȵĽ����ǽ���1��ϵͳ���̣����������������Ե�����һ������
                    if (nextProcess.pid == 1) {
                        continue;
                    }

                    // �������н��̣��ҵ���Ҫ���ȵĽ���
                    for (PCB& process : processes) {
                        if (process.pid == nextProcess.pid) {
                            // ����״̬����Ϊ�����С�
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

/*
    handleIODone()��������ҪĿ���Ǵ������IO��ɡ�
    �����ȼ����̺��Ƿ��ںϷ���Χ�ڣ�Ȼ��ӵ�ǰ���̵�IO�����л�ȡ��һ��IO����
    ���ţ�������IO����Ľ���״̬����Ϊ�������������������������С�
    ��󣬼�鵱ǰ�Ƿ����������еĽ��̣����û�У���Ӿ��������е���һ�����̡�
*/
void handleIODone(int diskNumber) {
    // �����̺��Ƿ��ںϷ���Χ��
    if (diskNumber >= 0 && diskNumber < diskCount) {
        // �����ǰ���̵�IO���в�Ϊ��
        if (!ioQueues[diskNumber].empty()) {
            // ��ȡ�����еĵ�һ��IO����
            IORequest ioRequest = ioQueues[diskNumber].front();
            // �����IO����Ӷ������Ƴ�
            ioQueues[diskNumber].pop();

            PCB* requestingProcess = nullptr;

            // ���ҷ���IO����Ľ���
            for (PCB& process : processes) {
                if (process.pid == ioRequest.pid) {
                    requestingProcess = &process;
                    break;
                }
            }

            // ����ҵ��˷���IO����Ľ���
            if (requestingProcess != nullptr) {
                // ����״̬����Ϊ��������
                requestingProcess->status = "����";
                // ����������������
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

    // ���û���������еĽ����Ҿ������зǿ�
    if (!hasRunningProcess && !readyQueue.empty()) {
        // ��ȡ���������еĵ�һ������
        PCB nextProcess = readyQueue.front();
        // ��������̴Ӿ����������Ƴ�
        readyQueue.pop();

        // �������н��̣��ҵ���Ҫ���ȵĽ���
        for (PCB& process : processes) {
            if (process.pid == nextProcess.pid) {
                // ����״̬����Ϊ�����С�
                process.status = "����";
                cout << "����" << process.pid << "�ѱ�����Ϊ����״̬��" << endl;
                break;
            }
        }
    }
}

// �����ڴ����
/*
    ��δ������ҪĿ����Ϊ�ض��Ľ��̷����ڴ档
    ����ڴ����п��е�֡�����Ὣ�µ�ҳ����䵽�������֡��
    ���û�п���֡���ڴ滹��ʣ��ռ䣬���ᴴ��һ���µ�ҳ�沢��ӵ��ڴ��С�
    ����ڴ�����������ʹ���������ʹ�ã�LRU���㷨���ҵ�һ�����ʹ�õ�֡���������֡�滻Ϊ�µ�ҳ�档
*/
void requestMemory(int pid) {
    //��ʼ��һ���������洢�ҵ��Ŀ���֡������
    int freeFrame = -1;

    //�����ڴ��е�����֡
    for (int i = 0; i < memory.size(); ++i) {
        //����ҵ�һ������֡
        if (memory[i].empty()) {
            //��¼�������֡������
            freeFrame = i;
            break;
        }
    }

    int timestamp = static_cast<int>(time(nullptr));  // ��ȡ��ǰʱ���

    //���û���ҵ�����֡
    if (freeFrame == -1) {
        //����ڴ滹��ʣ��ռ�
        if (static_cast<int>(memory.size()) * pageSize < memoryCapacity) {
            //����һ����ҳ�棬��������ID��ҳ��������ʱ���
            vector<int> newPage = { pid, static_cast<int>(memory.size()), timestamp };
            //����ҳ����ӵ��ڴ���
            memory.push_back(newPage);
            //����ҳ���������ӵ�LRU�б��ĩβ
            LRUList.push_back(static_cast<int>(memory.size()) - 1);
            cout << "Ϊ����" << pid << "����ҳ��" << static_cast<int>(memory.size()) - 1 << endl;
        }
        else {
            //ʹ��LRU�㷨�滻ҳ��
            //��ȡLRU�б������ʹ�õ�֡������
            int leastRecentlyUsedFrame = LRUList.front();
            //�����֡��LRU�б����Ƴ�
            LRUList.erase(LRUList.begin());
            //����һ����ҳ�����滻���ʹ�õ�֡����������ID�����滻��֡��������ʱ���
            vector<int> replacedPage = { pid, leastRecentlyUsedFrame, timestamp };
            //���ڴ����滻���֡
            memory[leastRecentlyUsedFrame] = replacedPage;
            //�����滻��֡��������ӵ�LRU�б��ĩβ
            LRUList.push_back(leastRecentlyUsedFrame);
            cout << "Ϊ����" << pid << "�滻ҳ��" << leastRecentlyUsedFrame << endl;
        }
    }
    else {
        //����һ����ҳ�棬��������ID������֡��������ʱ���
        vector<int> newPage = { pid, freeFrame, timestamp };
        //���ڴ��еĿ���֡�������ҳ��
        memory[freeFrame] = newPage;
        //������֡��������ӵ�LRU�б��ĩβ
        LRUList.push_back(freeFrame);
        cout << "Ϊ����" << pid << "����ҳ��" << freeFrame << endl;
    }
}

/*
    releaseMemory()��������ҪĿ�����ͷ�ָ������ռ�õ��ڴ档
    �����ȱ����ڴ��е�����ҳ�棬Ȼ���ҵ���Ҫ�ͷŵĽ�����ռ�õ�ҳ�沢��ո�ҳ�档
    ���ţ�����ҳ���LRU�б����Ƴ���
*/
void releaseMemory(int pid) {
    // �����ڴ��е�����ҳ��
    for (int i = 0; i < memory.size(); ++i) {
        // ����ҵ�����Ҫ�ͷŵĽ�����ռ�õ�ҳ��
        if (!memory[i].empty() && memory[i][0] == pid) {\
            // ��ո�ҳ��
            memory[i].clear();
            // ��LRU�б����Ƴ���ҳ��
            LRUList.erase(remove(LRUList.begin(), LRUList.end(), i), LRUList.end());
            cout << "�ͷŽ���" << pid << "��ҳ��" << i << endl;
        }
    }
}

/*
    handleAddress()��������ҪĿ���Ǵ����߼���ַ��
    �����Ȳ��ҵ�ǰ�������еĽ��̣�Ȼ������߼���ַ��Ӧ��ҳ�ź�ƫ������
    ���ţ������ڴ��е�����ҳ�棬���ҷ��ʵ�ҳ���Ƿ����ڴ��С�
    ������ʵ�ҳ�治���ڴ��У��������ڴ���䡣
    ��󣬽���ǰ���еĽ����ƻص���������ĩβ�������������п�ͷ�Ľ�������Ϊ����״̬��
*/
void handleAddress(int address) {
    PCB* runningProcess = nullptr;

    // �������н��̣��ҵ��������еĽ���
    for (PCB& process : processes) {
        if (process.status == "����") {
            runningProcess = &process;
            break;
        }
    }

    // ����ҵ����������еĽ���
    if (runningProcess != nullptr) {
        // �����߼���ַ��Ӧ��ҳ��
        int pageNumber = address / pageSize;
        // �����߼���ַ�е�ƫ����
        int offset = address % pageSize;

        bool pageInMemory = false;

        // �����ڴ��е�����ҳ�棬���ҷ��ʵ�ҳ���Ƿ����ڴ���
        for (int i = 0; i < memory.size(); ++i) {
            if (!memory[i].empty() && memory[i][0] == runningProcess->pid && memory[i][1] == pageNumber) {
                // ����ҳ���LRU�б����Ƴ�
                LRUList.erase(remove(LRUList.begin(), LRUList.end(), i), LRUList.end());
                // ����ҳ��������ӵ�LRU�б��ĩβ
                LRUList.push_back(i);
                // ��������ַ
                cout << "����" << runningProcess->pid << "����ҳ��" << pageNumber << "�������ַΪ��" << i * pageSize + offset << endl;
                pageInMemory = true;
                break;
            }
        }

        // ������ʵ�ҳ�治���ڴ��У��������ڴ����
        if (!pageInMemory) {
            cout << "����" << runningProcess->pid << "���ʵ�ҳ��" << pageNumber << "�����ڴ��У���Ҫ�����ڴ���䡣" << endl;
            // ���ݵ�ǰ���еĽ���ID
            requestMemory(runningProcess->pid);  
        }

        // ����ǰ���еĽ����ƻص���������ĩβ
        runningProcess->status = "����";
        readyQueue.push(*runningProcess);

        // ���������п�ͷ�Ľ�������Ϊ����״̬
        PCB nextProcess = readyQueue.front();
        readyQueue.pop();

        // �������н��̣��ҵ���Ҫ���ȵĽ���
        for (PCB& process : processes) {
            if (process.pid == nextProcess.pid) {
                process.status = "����";
                cout << "����" << process.pid << "�ѱ�����Ϊ����״̬��" << endl;
                break;
            }
        }
    }
    else {
        cout << "û���������еĽ��̣��޷������߼���ַ��" << endl;
    }
}


/*
    showIOStatus()��������ҪĿ������ʾӲ��IO״̬��
    �����ȱ�������Ӳ�̣�Ȼ�󴴽�һ����ʱ�����Ա����޸�ԭʼ���С�
    ���ţ�����ʱ���в�Ϊ��ʱ����ȡ�����еĵ�һ��IO���󲢽���Ӷ������Ƴ���
    ����������ID�ʹŵ��š�
*/
void showIOStatus() {
    // ��������Ӳ��
    for (int i = 0; i < diskCount; i++) {
        cout << "======================================" << endl;
        cout << "Ӳ��" << i << "�ϵĽ��̣�" << endl;

        // ����һ����ʱ�����Ա����޸�ԭʼ����
        queue<IORequest> tempQueue = ioQueues[i];  

        // ����ʱ���в�Ϊ��ʱ
        while (!tempQueue.empty()) {
            // ��ȡ�����еĵ�һ��IO����
            IORequest ioRequest = tempQueue.front();
            // �����IO����Ӷ������Ƴ�
            tempQueue.pop();

            // �������ID�ʹŵ���
            cout << "����ID��" << ioRequest.pid << "���ŵ���" << ioRequest.track << endl;
        }
    }

    cout << "======================================" << endl;
}

/*
    showMemoryStatus()��������ҪĿ������ʾ�ڴ�״̬��
    �����������ͷ��Ȼ������ڴ��е�����ҳ�档
    ���ţ������ǰҳ�治Ϊ�գ���ĳ������ռ�ã��������ҳ��š�ҳ��š�����ID��ʱ�����
    �����ǰҳ��Ϊ�գ�����������С���
*/
void showMemoryStatus() {
    // �����ͷ
    cout << "ҳ��\tҳ��\tPID\tʱ���" << endl;
    // �����ڴ��е�����ҳ��
    for (int i = 0; i < memory.size(); ++i) {
        const auto& frame = memory[i];
        // �����ǰҳ�治Ϊ�գ���ĳ������ռ�ã�
        if (!frame.empty()) {
            // ���ҳ��š�ҳ��š�����ID��ʱ���
            cout << i << "\t" << frame[1] << "\t" << frame[0] << "\t" << frame[2] << endl;
        }
        else {
            // �����ǰҳ��Ϊ�գ�����������С�
            cout << i << "\t����" << endl;
        }
    }
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
