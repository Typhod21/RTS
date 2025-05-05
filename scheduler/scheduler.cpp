#include "scheduler.h"
#include <climits>
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

void Scheduler::setPriority() {
    //set priority based on the deadline of the task
    // RM: Higher priority for shorter periods
    // DM: Higher priority for shorter deadlines
    int shortest, prevShortest = 0;
    int priority, shortestIndex = tasks_.size();
    for (size_t i = 0; i < tasks_.size(); i++) {
        shortest = computeHyperperiod();
        for(size_t j = 0; j < tasks_.size(); j++){
            if(tasks_[j].priority != 0)
                continue;
            if(tasks_[j].period < shortest && choice_ == CHOICE_RM){
                shortest = tasks_[j].period;
                shortestIndex = j;
            }
            else if(tasks_[j].deadline < shortest && choice_ == CHOICE_DM){
                shortest = tasks_[j].deadline;
                shortestIndex = j;
            }
        }
        if(shortest != prevShortest){
            tasks_[shortestIndex].priority = priority;
            prevShortest = shortest;
            priority--;
        }
        else{
            cout << "Error: Task " << tasks_[shortestIndex].id << " has same priority as task " << tasks_[i].id << endl;
        }
    }
    for(auto &task : tasks_){
        cout << "Task " << task.id << " has priority " << task.priority << endl;
    }
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
    int hyperperiod = computeHyperperiod();
    vector<int> remaining(tasks_.size(), 0);
    vector<int> nextRelease(tasks_.size(), 0);
    vector<int> nextDeadline(tasks_.size(), 0);

    std::cout << "\nTimeline (0 to " << hyperperiod << "):\n";

    for (int t = 0; t < hyperperiod; ++t) {
        // Release tasks
        for (size_t i = 0; i < tasks_.size(); ++i) {
            if (t == nextRelease[i]) {
                remaining[i] += tasks_[i].WCET;
                nextDeadline[i] = tasks_[i].deadline + nextRelease[i];
                nextRelease[i] += tasks_[i].period;
            }
        }
        int runningTask = -1;
        static int previousTask = -1;
        int priority = 0;
        int minDeadline = INT_MAX;
        int minSlack = INT_MAX;
        for(size_t i = 0; i < tasks_.size(); ++i) {
            if(choice_ == CHOICE_RM || choice_ == CHOICE_DM){
                if(remaining[i] > 0 && tasks_[i].priority > priority){
                    priority = tasks_[i].priority;
                    runningTask = i;
                }
            }
            else if(choice_ == CHOICE_EDF){
                if(remaining[i] > 0 && nextDeadline[i] < minDeadline){
                    if(nextDeadline[i] < minDeadline){
                        minDeadline = nextDeadline[i];
                        runningTask = i;
                    }
                    else if(nextDeadline[i] == minDeadline){
                        runningTask = previousTask;
                    }  
                }
            }
            else if(choice_ == CHOICE_LST){
                int slack = (nextDeadline[i] - t) - remaining[i];
                if(remaining[i] > 0){
                    if(slack < minSlack){
                        minSlack = slack;
                        runningTask = i;
                    }
                    else if(slack == minSlack){
                        runningTask = previousTask;
                    }
                }
            }
        }

        // Print which task runs
        if (runningTask != -1) {
            std::cout << "|T" << tasks_[runningTask].id;
            remaining[runningTask]--;
            previousTask = runningTask;
        } else {
            std::cout << "|ID";
        }
    }
    std::cout << "|\n";


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
        if(choice == CHOICE_PIP || choice == CHOICE_OCPP || choice == CHOICE_ICPP) {
            cout << "Enter WCET, period, deadline, priority for Task " << task.id << ": ";
            cin >> task.WCET >> task.period >> task.deadline >> task.priority;
        } else {
            cout << "Enter WCET, period, deadline for Task " << task.id << ": ";
            cin >> task.WCET >> task.period >> task.deadline;
            task.priority = 0; // Default priority for non-resource sharing protocols
        }
      
        tasks.push_back(task);
    }

    Scheduler scheduler(tasks, choice);

    if (choice == CHOICE_RM || choice == CHOICE_DM) {
        if(scheduler.runRMDMTest()== true){
            scheduler.setPriority();
            scheduler.generateTimeline();
        }
    } else if (choice == CHOICE_EDF || choice == CHOICE_LST) {
        if(scheduler.runEDFLSTTest()){
            scheduler.generateTimeline();
        }
    } else if (choice == CHOICE_PIP) {
        
    } else if (choice == CHOICE_OCPP || choice == CHOICE_ICPP) {
       
    }
    else {
        cout << "Selected protocol with resource sharing not yet implemented.\n";
    }

    return 0;
}