/*
  main.cpp
  Operating Systems Project 2
  Thread-Based Process Simulation and Synchronization

  Student note:
  This project reuses the same process input style from Project 1,
  but now each process is executed as its own thread. I also added
  a Dining Philosophers synchronization demo using mutexes.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <string>

using namespace std;

struct Process
{
  int pid;
  int arrival;
  int burst;
  int priority;
};

mutex printMutex;

bool isNumber(const string &s)
{
  if (s.empty())
    return false;

  for (char c : s)
  {
    if (!isdigit(c))
      return false;
  }

  return true;
}

vector<Process> readProcesses(const string &filename)
{
  ifstream fin(filename);
  vector<Process> processes;
  string line;

  if (!fin)
  {
    lock_guard<mutex> lock(printMutex);
    cout << "Could not open " << filename << ". Make sure the file is in the same folder as main.cpp.\n";
    return processes;
  }

  while (getline(fin, line))
  {
    if (line.empty())
      continue;

    istringstream iss(line);
    string a, b, c, d;
    iss >> a;

    if (!isNumber(a))
      continue;

    iss >> b >> c >> d;

    if (!isNumber(b) || !isNumber(c) || !isNumber(d))
      continue;

    Process p;
    p.pid = stoi(a);
    p.arrival = stoi(b);
    p.burst = stoi(c);
    p.priority = stoi(d);

    processes.push_back(p);
  }

  return processes;
}

void printLoadedProcesses(const vector<Process> &processes)
{
  lock_guard<mutex> lock(printMutex);

  cout << "\nLoaded " << processes.size() << " process(es)\n";
  cout << "PID  AT  BT  PR\n";

  for (const auto &p : processes)
  {
    cout << p.pid << "    "
         << p.arrival << "   "
         << p.burst << "   "
         << p.priority << "\n";
  }
}

void runProcessThread(Process p)
{
  {
    lock_guard<mutex> lock(printMutex);
    cout << "[Process " << p.pid << "] Created. Arrival=" << p.arrival
         << ", Burst=" << p.burst
         << ", Priority=" << p.priority << "\n";
  }

  if (p.arrival > 0)
  {
    {
      lock_guard<mutex> lock(printMutex);
      cout << "[Process " << p.pid << "] Waiting for arrival time...\n";
    }

    this_thread::sleep_for(chrono::seconds(p.arrival));
  }

  {
    lock_guard<mutex> lock(printMutex);
    cout << "[Process " << p.pid << "] Started execution.\n";
  }

  this_thread::sleep_for(chrono::seconds(p.burst));

  {
    lock_guard<mutex> lock(printMutex);
    cout << "[Process " << p.pid << "] Finished execution.\n";
  }
}

void simulateProcessThreads(const vector<Process> &processes)
{
  if (processes.empty())
  {
    lock_guard<mutex> lock(printMutex);
    cout << "\nNo processes were loaded, so the thread simulation cannot run.\n";
    return;
  }

  {
    lock_guard<mutex> lock(printMutex);
    cout << "\nStarting thread-based process simulation...\n";
  }

  vector<thread> workers;

  for (const auto &p : processes)
  {
    workers.emplace_back(runProcessThread, p);
  }

  for (auto &t : workers)
  {
    if (t.joinable())
    {
      t.join();
    }
  }

  {
    lock_guard<mutex> lock(printMutex);
    cout << "All process threads finished.\n";
  }
}

void philosopherTask(int id, vector<mutex> &forks, int rounds)
{
  int leftFork = id;
  int rightFork = (id + 1) % 5;

  for (int round = 1; round <= rounds; round++)
  {
    {
      lock_guard<mutex> lock(printMutex);
      cout << "[Philosopher " << id << "] Round " << round << ": Thinking...\n";
    }

    this_thread::sleep_for(chrono::milliseconds(500));

    int firstFork = min(leftFork, rightFork);
    int secondFork = max(leftFork, rightFork);

    {
      lock_guard<mutex> lock(printMutex);
      cout << "[Philosopher " << id << "] Waiting for forks "
           << firstFork << " and " << secondFork << "...\n";
    }

    forks[firstFork].lock();

    {
      lock_guard<mutex> lock(printMutex);
      cout << "[Philosopher " << id << "] Picked up fork " << firstFork << ".\n";
    }

    this_thread::sleep_for(chrono::milliseconds(200));

    forks[secondFork].lock();

    {
      lock_guard<mutex> lock(printMutex);
      cout << "[Philosopher " << id << "] Picked up fork " << secondFork << ".\n";
      cout << "[Philosopher " << id << "] Eating...\n";
    }

    this_thread::sleep_for(chrono::milliseconds(700));

    forks[secondFork].unlock();
    forks[firstFork].unlock();

    {
      lock_guard<mutex> lock(printMutex);
      cout << "[Philosopher " << id << "] Released forks "
           << firstFork << " and " << secondFork << ".\n";
    }

    this_thread::sleep_for(chrono::milliseconds(300));
  }

  {
    lock_guard<mutex> lock(printMutex);
    cout << "[Philosopher " << id << "] Finished all rounds.\n";
  }
}

void simulateDiningPhilosophers()
{
  {
    lock_guard<mutex> lock(printMutex);
    cout << "\nStarting Dining Philosophers simulation...\n";
    cout << "Deadlock avoidance rule: each philosopher picks up the lower-numbered fork first.\n";
  }

  vector<mutex> forks(5);
  vector<thread> philosophers;
  int rounds = 2;

  for (int i = 0; i < 5; i++)
  {
    philosophers.emplace_back(philosopherTask, i, ref(forks), rounds);
  }

  for (auto &t : philosophers)
  {
    if (t.joinable())
    {
      t.join();
    }
  }

  {
    lock_guard<mutex> lock(printMutex);
    cout << "Dining Philosophers simulation finished.\n";
  }
}

int main()
{
  vector<Process> processes = readProcesses("processes.txt");
  printLoadedProcesses(processes);

  while (true)
  {
    cout << "\nChoose an option:\n";
    cout << "1) Run process thread simulation\n";
    cout << "2) Run Dining Philosophers simulation\n";
    cout << "3) Run both\n";
    cout << "4) Quit\n";
    cout << "Selection: ";

    int choice;
    cin >> choice;

    if (choice == 1)
    {
      simulateProcessThreads(processes);
    }
    else if (choice == 2)
    {
      simulateDiningPhilosophers();
    }
    else if (choice == 3)
    {
      simulateProcessThreads(processes);
      simulateDiningPhilosophers();
    }
    else if (choice == 4)
    {
      cout << "Exiting program.\n";
      break;
    }
    else
    {
      cout << "Invalid choice. Try again.\n";
    }
  }

  return 0;
}
