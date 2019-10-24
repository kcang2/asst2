#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "tasksys.h"
#include <vector>
#include <functional>

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
	std::vector<std::vector<int>> works;
	
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
    this->threads = new std::thread[this->num_T];
    this->mutex_ = new std::mutex();

    this->work_counter = 0;
    this->not_done = 0;
    this->total_work = 0;
    this->done_flag = {false};

    for (int i = 0; i < this->num_T; i++) {
        this->threads[i] = std::thread(&TaskSystemParallelThreadPoolSpinning::waitFunc, this);
    }
}


TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    this->done_flag = true;

    for (int i = 0; i < this->num_T; i++) {
        this->threads[i].join();
    }

    delete this->mutex_;
    delete[] this->threads;
}

void TaskSystemParallelThreadPoolSpinning::waitFunc() {
    while(done_flag==false){
        this->mutex_->lock();

        if (this->not_done == 0 && this->total_work != 0) {  // ALL work done
	    this->work_counter = 0;
            this->total_work = 0;
        }

	int total = this->total_work;
        int id = this->work_counter;
        if (id == total) {  // NO work initiated or NO work left
            this->mutex_->unlock();
            continue;
        }

        ++(this->work_counter);  // increment counter
        this->mutex_->unlock();  // Let others access counters to work

        this->runnable->runTask(id, total); // do work

	this->mutex_->lock();
	--(this->not_done); // decrement counter after work done
	this->mutex_->unlock();
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // set-up work
    this->mutex_->lock();
    this->runnable = runnable;
    this->not_done = num_total_tasks;
    this->total_work = num_total_tasks;
    this->mutex_->unlock();

    // Poll to check if ALL work is done  
    while (true){
        this->mutex_->lock();
        if (this->total_work==0) {
            this->mutex_->unlock();
            break;
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

    this->num_T = std::max(1, num_threads - 1);
    this->threads = new std::thread[this->num_T];
    this->mutex_ = new std::mutex();
    this->cond_ = new std::condition_variable();
   
    this->total_work = 0;
    this->not_done = 0;
    this->work_counter = 0;
    this->num_wait = 0;
    this->done_flag = {false};
    //std::cout << "INITIALIZED" << std::endl;

    for (int i = 0; i < this->num_T; i++) {
        this->threads[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::waitFunc, this);
    }
    //std::cout << "WORKERS CREATED" << std::endl;
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    std::unique_lock<std::mutex> lock(*this->mutex_);
    this->cond_->wait(lock,
                    std::bind(&TaskSystemParallelThreadPoolSleeping::wakeMain, this));
    this->done_flag = true;
    //std::cout << "SET DONE FLAG" << std::endl;
    this->mutex_->unlock();

    this->cond_->notify_all();

    for (int i = 0; i < this->num_T; i++) {
        this->threads[i].join();
    }
    //std::cout << "WORKERS JOINED" << std::endl;
    delete this->mutex_;
    delete[] this->threads;
    delete this->cond_;
    //std::cout << "DELETED" << std::endl;
}

bool TaskSystemParallelThreadPoolSleeping::wakeWorker() {
    return (this->done_flag == true ||
		    this->total_work != 0);
}

bool TaskSystemParallelThreadPoolSleeping::wakeMain() {
    return this->total_work == 0;
}

void TaskSystemParallelThreadPoolSleeping::waitFunc() {
std::unique_lock<std::mutex> lock(*this->mutex_);
//std::cout << "WORKER TAKES LOCK" << std::endl;
lock.unlock();

    while(true) {
	this->num_wait++;

	lock.lock();
	//std::cout << "WORKER WAITS" << std::endl;
	this->cond_->wait(lock,
		       	std::bind(&TaskSystemParallelThreadPoolSleeping::wakeWorker, this));
	if (this->done_flag == true) {  // wake worker to terminate it
	    lock.unlock();
	    break;
	}
	this->num_wait--;
	lock.unlock();
        this->cond_->notify_all();  // notify other workers?

	// Spinning
	while (true) {
            this->mutex_->lock();

            if (this->not_done == 0) {  // ALL work done
		if (this->total_work != 0) {  // 1st time seen by workers
		    this->total_work = 0;
		    this->cond_->notify_all();
		}
		this->mutex_->unlock();
		break;
            }
    
            int total = this->total_work;
            int id = this->work_counter;
            if (id == total) {  // NO work left
                this->mutex_->unlock();
                continue;
            }
    
            ++(this->work_counter);  // increment counter
            this->mutex_->unlock();  // Let others access counters to work
    
            this->runnable->runTask(id, total); // do work
    
            --(this->not_done); // decrement counter after work done
	}
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    
    // Check readiness
    //std::cout << "*********************************" << std::endl;
    while (this->num_wait < this->num_T) {  // Check if all ready
    }
    //std::cout << "ALL WORKERS READY" << std::endl;

    // Set-up work then notify workers
    this->mutex_->lock();
    this->runnable = runnable;
    this->work_counter = 0;
    this->not_done = num_total_tasks;
    this->total_work = num_total_tasks;
    //std::cout << "SETUP COMPLETE" << std::endl;
    this->mutex_->unlock();
    this->cond_->notify_all();

    // Wait for workers to complete work
    std::unique_lock<std::mutex> lock(*this->mutex_);
    //std::cout << "MAIN WAITS" << std::endl;
    this->cond_->wait(lock,
		    std::bind(&TaskSystemParallelThreadPoolSleeping::wakeMain, this));
    //std::cout << "MAIN EXITS" << std::endl;
    lock.unlock();
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
