#include "scheduler.h"
using namespace std;

Scheduler::Scheduler(const vector<Task>& tasks, int choice)
    : tasks_(tasks), choice_(choice) {}

double Scheduler::computeUtilization() const {
    double utilization = 0.0;
    for (const auto& task : tasks_) {
        utilization += static_cast<double>(task.WCET) / task.deadline;
    }
    return utilization;
}

int Scheduler::computeHyperperiod() const {
    int h = 1;
    for (const auto& task : tasks_) {
        h = lcm(h, task.period);
    }
    return h;
}

bool Scheduler::runRMDMTest() {
    cout << "\nRunning RM/DM schedulability tests...\n";
    double utilization = computeUtilization();
    double bound = tasks_.size() * (pow(2, 1.0 / tasks_.size()) - 1);

    if (utilization <= bound) {
        cout << "Schedulable: " << utilization << " <= " << bound << endl;
        return true;
    } else {
        cout << utilization << " >= " << bound << endl;
        cout << "Inconclusive using utilization. Checking response time analysis...\n";
    }

    bool schedulable = true;
    for( const auto &task : tasks_){
        cout << "Task " << task.id << " response time analysis:" << endl;
        int previousTime = 0;
        int responseTime = task.WCET;
        int constantTime = 0;
        while(previousTime <= task.deadline){
            constantTime = responseTime;
            responseTime = task.WCET;
            cout << "Task " << task.id << " response time:";
            for (const auto &otherTask : tasks_){
                if (otherTask.id != task.id && otherTask.deadline <= task.deadline){
                    cout << " ceil(" << constantTime << " / " << otherTask.period << ") * " << otherTask.WCET;
                    responseTime += ceil(static_cast<double>(constantTime) / otherTask.period) * otherTask.WCET;
                }
            }
            cout << " = " << responseTime << endl;
            if(previousTime == responseTime)
                break;
            previousTime = responseTime;
        }
        if (responseTime > task.deadline){
            cout << "Task " << task.id << " is not schedulable. \n\n";
            schedulable = false;
        }
        else{
            cout << "Task " << task.id << " is schedulable with response time: " << responseTime << "\n\n";
        }
    }
    
    return schedulable;
}

bool Scheduler::runEDFLSTTest() {
    cout << "\nRunning EDF/LST schedulability test...\n";
    double utilization = 0.0;
    bool usesDeadline = false;
    int hyper = computeHyperperiod();
    vector<int> L;

    for (const auto& task : tasks_) {
        utilization += static_cast<double>(task.WCET) / task.deadline;
        if (task.deadline < task.period) {
            usesDeadline = true;
        }
    }

    if (utilization <= 1.0) {
        cout << "Schedulable: " << utilization << " <= 1\n";
        return true;
    } else if (!usesDeadline) {
        cout << "Unschedulable: " << utilization <<  " > 1 with D == T\n";
        return false;
    }
    else{
        cout << "Inconclusive using utilization, applying processor demand criterion...\n";
    }

    for (const auto& task : tasks_) {
        for (int i = 0; i <= hyper; i += task.period) {
            int deadlinePoint = i + task.deadline;
            if (deadlinePoint <= hyper && find(L.begin(), L.end(), deadlinePoint) == L.end()) {
                L.push_back(deadlinePoint);
            }
        }
    }

    sort(L.begin(), L.end());
    for (const auto& l : L) {
        int demand = 0;
        for (const auto& task : tasks_) {
            demand += floor((l + task.period - task.deadline) / task.period) * task.WCET;
        }

        if (demand > l) {
            cout << "Unschedulable at time " << l << ": " << demand << " > " << l << "\n";
            return false;
        } else {
            cout << "Schedulable at time " << l << ": " << demand << " <= " << l << "\n";
        }
    }

    return true;
}
bool Scheduler::runPIPTest() {
    // Optional PIP feature
    cout << "PIP test not implemented.\n";
    return true;
}
bool Scheduler::runOCPPICPPTest() {
    // Optional OCPP/ICPP feature
    cout << "OCPP/ICPP test not implemented.\n";
    return true;
}

void Scheduler::generateTimeline() {
    // Optional timeline feature
    cout << "Timeline generation not implemented.\n";
}

int main() {
    int choice;
    cout << "Choose a scheduling algorithm:\n";
    cout << CHOICE_RM   << ". Rate-Monotonic (RM)\n";
    cout << CHOICE_DM   << ". Deadline-Monotonic (DM)\n";
    cout << CHOICE_EDF  << ". Earliest Deadline First (EDF)\n";
    cout << CHOICE_LST  << ". Least Slack Time (LST)\n";
    cout << CHOICE_PIP  << ". Priority Inheritance Protocol (PIP)\n";
    cout << CHOICE_OCPP << ". Original Ceiling Priority Protocol (OCPP)\n";
    cout << CHOICE_ICPP << ". Immediate Ceiling Priority Protocol (ICPP)\n";
    cout << "Enter your choice (1-7): ";
    cin >> choice;

    if (choice < CHOICE_RM || choice > CHOICE_ICPP) {
        cout << "Invalid Input\n";
        return 1;
    }

    int numTasks;
    cout << "Enter the number of tasks: ";
    cin >> numTasks;

    if (numTasks <= 0) {
        cout << "Invalid number of tasks. Exiting.\n";
        return 1;
    }

    vector<Task> tasks;
    for (int i = 0; i < numTasks; ++i) {
        Task task;
        task.id = i + 1;
        cout << "Enter WCET, period, deadline for Task " << task.id << ": ";
        cin >> task.WCET >> task.period >> task.deadline;
        tasks.push_back(task);
    }

    Scheduler scheduler(tasks, choice);

    if (choice == CHOICE_RM || choice == CHOICE_DM) {
        scheduler.runRMDMTest();
    } else if (choice == CHOICE_EDF || choice == CHOICE_LST) {
        scheduler.runEDFLSTTest();
    } else if (choice == CHOICE_PIP) {
        
    } else if (choice == CHOICE_OCPP || choice == CHOICE_ICPP) {
       
    }
    else {
        cout << "Selected protocol with resource sharing not yet implemented.\n";
    }

    return 0;
}