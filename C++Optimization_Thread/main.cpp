#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>


using namespace std;


class ThreadPool
{
public:
    ThreadPool(size_t numThreads)
    {
        running = true;
        for (size_t i = 0; i < numThreads; ++i)
        {
            workers.emplace_back(
                [this]()
                {
                    while (true)
                    {
                        function<void()> task;


                        // Prendre une tâche dans la queue
                        {
                            unique_lock<mutex> lock = unique_lock<mutex>(queueMutex);
                            condition.wait(lock,
                                [this] { return !taskQueue.empty() || !running; });


                            if (!running && taskQueue.empty())
                                return;


                            task = move(taskQueue.front());
                            taskQueue.pop();
                        } // cette fin de bloc termine le unique_lock


                        // Exécuter la tâche
                        task();
                    }
                });
        }
    }


    ~ThreadPool()
    {
        // Arrêter tous les threads proprement
        {
            unique_lock<mutex> lock = unique_lock<mutex>(queueMutex);
            running = false;
        }


        condition.notify_all();
        for (thread& worker : workers)
            worker.join();
    }


    void enqueueTask(function<void()> task)
    {
        {
            unique_lock<mutex> lock = unique_lock<mutex>(queueMutex);
            taskQueue.push(move(task));
        } //la fin de bloc termine le unique_lock
        condition.notify_one();
    }


private:
    vector<thread> workers;
    queue<function<void()>> taskQueue;
    mutex queueMutex;
    condition_variable condition;
    atomic<bool> running;
};


int main()
{
    ThreadPool pool(4); // 4 threads


    for (int i = 0; i < 10; ++i)
    {
        pool.enqueueTask(
            [i]()
            {
                cout << "Task " << i << " exécutée par " << this_thread::get_id() << endl;
                this_thread::sleep_for(chrono::milliseconds(100)); // Simule du travail
            });
    }


    this_thread::sleep_for(chrono::seconds(2)); // Laisse le temps aux tâches de finir


    int a;
    cin >> a;
    return 0;
}
