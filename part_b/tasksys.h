#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <set>
#include <unordered_map>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    std::unordered_map<TaskID, std::set<TaskID>> downDep; // which bulk-tasks depend on this bulk-task
    std::unordered_map<TaskID, std::set<TaskID>> upDep; // this bulk-task depedns on which bulk-tasks
    std::set<TaskID> completedTasks; // completed bulk-tasks, keeps track of what has been done (do not remove)

    std::unordered_map<TaskID, std::pair<IRunnable*, int>> taskInfo; // info of bulk-tasks (task type and total tasks)
    std::set<TaskID> readyTasks; //queue of workable bulk-tasks
    std::unordered_map<TaskID, int> workCounter; // assigned tasks counter from readyTasks
    std::unordered_map<TaskID, int> doneCounter; // assigned tasks counter from readyTasks

    std::thread* threads;
    std::mutex workM;
    std::mutex schedM;
    std::condition_variable workCV;
    std::condition_variable schedCV;

    TaskID currentID; // keep track of current number of bulk-tasks
    bool done_flag;
    int num_T;
    int tcount;

    void waitFunc(int id);
    bool wakeWorker(); 
    bool wakeMain();
    void finishedTask(TaskID tid);    

    public:
	TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

#endif
