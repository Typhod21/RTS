// // A program that can take a task set of any size and then run schedulability tests relating to scheduling algorithms including RM, DM, EDF, LST, PIP, OCPP, and ICPP.
// // Then our program would return the results of the schedulability tests and a visual timeline displaying the tasks in a hyperperiod, like our homework.
// // Also, deadlines can be arbitrary. Meaning that the deadline of a task can be greater than its period.
// // For the resource sharing protocol PIP etc., you may assume they are used together with fixed priority scheduling.
// // Task set up
// // task|WCET|Period|Deadline| for non resource sharing protocols
// // task|WCET|Period|Deadline|Critical Section| for resource sharing protocols
// // example task set
// // 13 60 45
// // 11 50 50
// // 6 20 15
//lst 3 20 7
// // 2 5 4
// // 2 10 8

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <iomanip>

#define CHOICE      0
#define CHOICE_RM   1
#define CHOICE_DM   2
#define CHOICE_EDF  3
#define CHOICE_LST  4
#define CHOICE_PIP  5
#define CHOICE_OCPP 6
#define CHOICE_ICPP 7

struct Task {
    int id;
    int period;
    int WCET;
    int deadline;
    int priority;

};

class Scheduler {
public:
    Scheduler(const std::vector<Task>& tasks, int choice = CHOICE);

    bool runRMDMTest();
    bool runEDFLSTTest();
    bool runPIPTest(); // Optional for now
    bool runOCPPICPPTest(); // Optional for now
    void setPriority();
    void generateTimeline(); // Optional for now

private:
    std::vector<Task> tasks_;
    int choice_;

    double computeUtilization() const;
    int computeHyperperiod() const;
    
};

#endif // SCHEDULER_H
