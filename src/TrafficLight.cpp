#include <iostream>
#include <random>
#include "TrafficLight.h"


template <class T>
T MessageQueue<T>::receive() {
    std::unique_lock<std::mutex> lck(_mutex);
    // if the condition (there is something in the queue) is satisfied, wait() returns and the execution continues
    // otherwise, wait() unlocks lck and puts the thread in the waiting state.
    // the thread is woken up when notify_one() from the send() method is sent
    _cv.wait(lck, [this] () { return !_queue.empty(); } );
    T message = std::move(_queue.back()); // get the last message from the queue
    _queue.pop_back();  // remove it from the queue
    return message;
}

template <class T>
void MessageQueue<T>::send(T &&msg) {
    std::lock_guard<std::mutex> lck(_mutex); // add the message to the queue under the lock
    // added at the suggestion of the reviewer. The purpose is to
    _queue.clear();
    _queue.emplace_back(std::move(msg));
    _cv.notify_one(); // notify client
}

TrafficLight::TrafficLight()
{
    // start with the RED
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen() {
    // basically loop until the light has turned green
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
            _queue.send(std::move(_currentPhase));
        }
    }
}