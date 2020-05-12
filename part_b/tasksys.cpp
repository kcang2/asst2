#include "tasksys.h"
#include <functional>
#include <iostream>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    currentID = 0;
    done_flag = false;
    tcount = 0;

    num_T = std::max(1, num_threads-1);
    threads = new std::thread[this->num_T];

    for (int i = 0; i < num_T; i++) {
        threads[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::waitFunc, this, i);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    std::unique_lock<std::mutex> workLock(workM);
    done_flag = true;
    workLock.unlock();
    workCV.notify_all();

    for (int i = 0; i < num_T; i++) {
        threads[i].join();
    }

    delete[] threads;
}

bool TaskSystemParallelThreadPoolSleeping::wakeWorker() {
    return (done_flag == true || !(readyTasks.empty()));
}

bool TaskSystemParallelThreadPoolSleeping::wakeMain() {
    return (currentID == completedTasks.size()); // sync condition
}

void TaskSystemParallelThreadPoolSleeping::finishedTask(TaskID tid) {
      for (auto& d : downDep[tid]) { // erase each child's dependency on this completed task
	 if (upDep.find(d) != upDep.end()) {
	        upDep[d].erase(tid);
         	if (upDep[d].empty()) { // no more dependencies in child
      			std::unique_lock<std::mutex> workLock(workM);
            		readyTasks.insert(d); // child is ready to be worked
    	    		workCounter[d] = 0;
	    		doneCounter[d] = 0;
    	    		upDep.erase(d); // remove child's deps
    	    		workLock.unlock();
			workCV.notify_all();
         	}
	 }
      }
      std::unique_lock<std::mutex> workLock(workM);
      completedTasks.insert(tid); // completed current task
      taskInfo.erase(tid); // erase info on current task
      workCounter.erase(tid); // erase counter
      doneCounter.erase(tid);
      workLock.unlock(); 
      upDep.erase(tid);// erase dependency
      downDep.erase(tid);
}

void TaskSystemParallelThreadPoolSleeping::waitFunc(int id) {
    int tid = id;
    while(true) {
    	std::unique_lock<std::mutex> workLock(workM);
	workCV.wait(workLock, std::bind(&TaskSystemParallelThreadPoolSleeping::wakeWorker, this));
	if (done_flag == true) {  // wake worker to terminate it
	    workLock.unlock();
	    break;
	}
	workLock.unlock();
	
	while (true) {
	workLock.lock();
	if (readyTasks.empty()) {
	    workLock.unlock();
	    break;	
	}
	TaskID currentTask = *readyTasks.begin(); // always oldest?
	//std::cout << "Thread "<< tid << ": Task " << currentTask << std::endl;
	int id = workCounter[currentTask];
	auto infoIt = taskInfo[currentTask];
	IRunnable* runner = infoIt.first;
        int total = infoIt.second;
	//std::cout << "\tWork " << id << ": Done " << doneCounter[currentTask] << std::endl;

	workCounter[currentTask] += 1;  // increment counter
	if (workCounter[currentTask] >= total) {
		//std::cout << "Thread" << tid << ": Remove " << currentTask << std::endl;
	        readyTasks.erase(currentTask);
	}
	workLock.unlock();

        runner->runTask(id, total); // do work

	workLock.lock();
	doneCounter[currentTask] += 1;
	if (doneCounter[currentTask] >= total) { // all work done
		//std::cout << "Thread" << tid << ": Finished " << currentTask << std::endl;
		workLock.unlock();
		std::unique_lock<std::mutex> schedLock(schedM);
		finishedTask(currentTask);
		schedLock.unlock();
		schedCV.notify_all(); // can this finish sync?	
	} else {
		workLock.unlock();
	}
	}
   }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
	std::vector<TaskID> deps;
	runAsyncWithDeps(runnable, num_total_tasks, deps);
	sync();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {
    	bool ready = true;
	// TODO Serial when asynchronous
	std::unique_lock<std::mutex> schedLock(schedM);
	for (auto& d: deps) { // should skip for-loop if empty
	    if (completedTasks.find(d) != completedTasks.end()) // done parent
	  	continue;

	    ready = false;
	    if (downDep.find(d) == downDep.end()) { //empty? create new set
		    downDep[d] = std::set<TaskID>();
	    }
	    downDep[d].insert(currentID); // update parent
	    if (upDep.find(currentID) == upDep.end()) { //empty? create new set
		    upDep[currentID] = std::set<TaskID>();
	    }
	    upDep[currentID].insert(d); //update child
	}
	schedLock.unlock();

	std::unique_lock<std::mutex> workLock(workM);
	//std::cout << "New Task " << currentID << std::endl;
  	taskInfo[currentID] = {runnable, num_total_tasks};
	if (ready) {
	   readyTasks.insert(currentID);
	   workCounter[currentID] = 0;
	   doneCounter[currentID] = 0;
	}	
  	workLock.unlock();

	if (ready) {
	   workCV.notify_all();
	}

    	return currentID++;
}

void TaskSystemParallelThreadPoolSleeping::sync() {
    std::unique_lock<std::mutex> schedLock(schedM);
    schedCV.wait(schedLock, std::bind(&TaskSystemParallelThreadPoolSleeping::wakeMain, this)); 
    schedLock.unlock();
    return;
}

