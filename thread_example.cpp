/*
 * An example C++ thread program.
 * This is just to help you understand how to create, run, and join threads:
 * it isn't part of the project 2 code you'll need to submit.
 */

#include <iostream>
#include <thread>
#include <vector>
#include <string>

using std::cout;
using std::string;
using std::vector;
using std::thread;

/**
 * Function that the thread will run.
 *
 * @param id A unique ID for the thread.
 * @param name A human friendly name for the thread.
 * @param m A mutex to avoid multiple threads interleaving their output.
 */
void thread_function(int id, string name, std::mutex& m) {
	m.lock();
	cout << "Hi, I'm thread number " << id << ", but I prefer to go by " << name << "\n";
	m.unlock();
}

int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " num_threads\n";
        exit(1);
    }

	string names[5] = {"Alice", "Bob", "Charlie", "Dave", "Eve"};

    int num_threads = std::stoi(argv[1]);
	vector<thread> threads;

	std::mutex print_mutex;

    for (int i = 0; i < num_threads; i++) {
		threads.push_back(thread(thread_function, i, names[i], std::ref(print_mutex)));
    }

	// Wait for each thread to finish using the join function.
    for (size_t i = 0; i < threads.size(); i++) {
		threads[i].join();
    }
}
