// resource-consumer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <vector>

int main()
{
    auto cur = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto dur = std::chrono::seconds{ 5 };
    auto end = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + dur);

    std::vector<std::thread> threads;
    // 12 logical processors, full speed running.
    for (int index = 0; index < 12; ++index) {
        threads.emplace_back(std::thread([&]() {
            while (std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) < end)
            {
                // do-nothing
            }
        }));
    }

    for (auto& th : threads) {
        th.join();
    }

    std::cout << "Hello World!\n";
}

