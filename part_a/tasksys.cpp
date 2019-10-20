#include <thread>
#include <atomic>
#include <mutex>
#include <iostream>
#include "tasksys.h"
#include <vector>

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
	this->num_T = num_threads;
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::workFunc(IRunnable* runnable, std::vector<int> work, int num_total_tasks) {
	for (int i = 0; i< work.size(); i++) {
		 runnable->runTask(work[i], num_total_tasks);
	}
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

        std::thread* threads = new std::thread[this->num_T - 1];
	std::vector<std::vector> works;
	
	// Work allocation
	for (int i = 0; i < this->num_T; i++) {
		std::vector<int> temp;
		for (int j = i ; j < num_total_tasks; j += this->num_T) {
			temp.push_back(j);
		}
		works.push_back(temp);
	}
	
	// Kick-off workers
	for (int i = 0; i< this->num_T - 1; i++) {
		threads[i] = std::thread(&TaskSystemParallelSpawn::workFunc, 
			    this, runnable, works[i], num_total_tasks);
	}	
	
	// Main works
	for (int i = 0; i< works[this->num_T-1].size(); i++) {
		 runnable->runTask(works[this->num_T-1][i], num_total_tasks);
	}
	
	// Workers join main
	for (int i = 0; i< this->num_T - 1; i++) {
		threads[i].join();
	}
	
	delete[] threads;
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

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
    this->num_T = std::max(1, num_threads - 1);
    this->threads = new std::thread[num_threads - 1];
    this->mutex_ = new std::mutex();
    this->work_counter = 0;
    this->total_work = 0;
    this->done_flag = {false};

    for (int i = 0; i < this->num_T; i++) {
        this->threads[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::waitFunc, this);
    }
}


TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    done_flag = true;

    for (int i = 0; i < this->num_T; i++) {
        this->threads[i].join();
    }

    delete this->mutex_;
    delete[] this->threads;
}

void TaskSystemParallelThreadPoolSpinning::waitFunc() {
    while(done_flag==false){
        this->mutex_->lock();

        if (this->total_work==0) {  // NO work
            this->mutex_->unlock();
            continue;
        }

        int id = this->work_counter;
        if (id >= this->total_work) {  // ALL work done
            this->total_work = 0;
            this->mutex_->unlock();
            continue;
        }
	int total = this->total_work;
	//std::cout << id << " " << total << std::endl;
        ++(this->work_counter);  // increment counter
        this->runnable->runTask(id, total); // do work
        this->mutex_->unlock();
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    this->mutex_->lock();
    this->runnable = runnable;
    this->work_counter = 0;
    this->total_work = num_total_tasks;
    this->mutex_->unlock();

    bool done = false;
    while (done==false){
        this->mutex_->lock();
        if (this->work_counter > 0 && this->total_work == 0) {
            done = true;
        }
        this->mutex_->unlock();
    }

}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

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
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
