#include <iostream>
#include <random>
#include "TrafficLight.h"


template <typename T>
T MessageQueue<T>::receive() {
    std::unique_lock<std::mutex> lck(_mutex);
    _cv.wait(lck);
    T received = std::move(_queue.back());
    _queue.pop_back();
    return received;
}

template <typename T>
void MessageQueue<T>::send(T &&msg) {
    std::lock_guard<std::mutex> lck(_mutex);
    // _queue.clear();
    _queue.push_back(msg);
    _cv.notify_one();
}

TrafficLight::TrafficLight()
{
    // start with the RED
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen() {
    while(_queue.receive() == TrafficLightPhase::red) {
        continue;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase() {
    return _currentPhase;
}

void TrafficLight::simulate() {
    TrafficObject::threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
    std::chrono::high_resolution_clock::time_point timeStart;
    std::chrono::high_resolution_clock::time_point timeEnd;


    // Adopted from https://stackoverflow.com/questions/7577452/random-time-delay
    std::mt19937_64 eng{std::random_device{}()};
    std::uniform_int_distribution<int> distribution{4000, 6000};   // setup the uniform distro for 4000-6000ms
    double redGreenCycleLength = distribution(eng);

    std::cout << "TrafficLight::cycleThroughPhases() started.\n";

    timeStart = std::chrono::high_resolution_clock::now(); // get the start of the cycle
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        timeEnd = std::chrono::high_resolution_clock::now(); // get the end of the cycle

        if ((std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd - timeStart).count()) >= redGreenCycleLength) {
            TrafficLightPhase oldPhase = _currentPhase;
            _currentPhase = (_currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green
                            : TrafficLightPhase::red);
            std::cout << "TrafficLight::cycleThroughPhases(): Switching light from " <<
                (( oldPhase == TrafficLightPhase::red) ? "red" : "green")<< " to " <<
                (( _currentPhase == TrafficLightPhase::red) ? "red" : "green") << "\n";
            timeStart = std::chrono::high_resolution_clock::now(); // get the new start of the cycle
        }
        _queue.send(std::move(_currentPhase));
    }
}