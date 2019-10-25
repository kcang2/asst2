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

void TaskSystemParallelSpawn::workFunc(IRunnable* runnable, std::vector<int> work, int total) {
	for (int i=0; i<work.size(); i++) {
		runnable->runTask(work[i], total);
	}
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

        std::thread* threads = new std::thread[this->num_T - 1];
	
	// Work allocation
	for (int i = 0; i < this->num_T; i++) {
	   std::vector<int> temp;
	   for (int j = i ; j < num_total_tasks; j += this->num_T) {
	 	temp.push_back(j);
	   }

           if (i < this->num_T-1) {
		threads[i] = std::thread(&TaskSystemParallelSpawn::workFunc, 
			    this, runnable, temp, num_total_tasks);
	   }

	    else {
	    // Main works
	    	for (int k = 0; k < temp.size(); k++) {
		   runnable->runTask(temp[k], num_total_tasks);
	        }
	    }
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
    this->num_T = std::max(1, num_threads - 1);
    this->threads = new std::thread[this->num_T];
    this->mutex_ = new std::mutex();
    this->cond_ = new std::condition_variable();
   
    this->total_work = 0;
    this->not_done = 0;
    this->work_counter = 0;
    this->done_flag = {false};

    for (int i = 0; i < this->num_T; i++) {
        this->threads[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::waitFunc, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    this->done_flag = true;
    this->cond_->notify_all();

    for (int i = 0; i < this->num_T; i++) {
        this->threads[i].join();
    }

    delete this->mutex_;
    delete[] this->threads;
    delete this->cond_;
}

bool TaskSystemParallelThreadPoolSleeping::wakeWorker() {
    return (this->done_flag == true ||
	   this->work_counter < this->total_work );
}

bool TaskSystemParallelThreadPoolSleeping::wakeMain() {
    return this->total_work == 0;
}

void TaskSystemParallelThreadPoolSleeping::waitFunc() {
    std::unique_lock<std::mutex> lock(*this->mutex_);

    while(true) {
	this->cond_->wait(lock,
	       	std::bind(&TaskSystemParallelThreadPoolSleeping::wakeWorker, this));
	if (this->done_flag == true) {  // wake worker to terminate it
	    lock.unlock();
	    break;
	}
	lock.unlock();

	while (true) {    // Spinning 
	    lock.lock();
            int total = this->total_work;
            int id = this->work_counter;
	    //std::cout<<id<<" "<<total<<std::endl;
            if (id >= total) {  // NO work left
		if (total != 0 && this->not_done == 0) {
		    this->total_work = 0;
		    lock.unlock();
		    this->cond_->notify_all();
		    break;
		}
                lock.unlock();
                break;
            }
            ++(this->work_counter);  // increment counter
            lock.unlock();  // Let others access counters to work
    
            this->runnable->runTask(id, total); // do work
    
            --(this->not_done); // decrement counter after work done
	}
	lock.lock();
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    // Set-up work then notify workers
    std::unique_lock<std::mutex> lock(*this->mutex_);
    this->runnable = runnable;
    this->work_counter = 0;
    this->not_done = num_total_tasks;
    this->total_work = num_total_tasks;
    lock.unlock();
    this->cond_->notify_all();

    // Wait for workers to complete work
    lock.lock();
    this->cond_->wait(lock,
		    std::bind(&TaskSystemParallelThreadPoolSleeping::wakeMain, this));
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
