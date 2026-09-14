#include <bits/stdc++.h>
using namespace std;

enum class Direction {
    UP,
    DOWN,
    IDLE
};

class Door {
    bool open = false;

public:
    void openDoor() {
        open = true;
    }

    void closeDoor() {
        open = false;
    }
};

class Elevator {
    int id;
    int currentFloor = 1;
    Direction direction = Direction::IDLE;
    Door door;
    set<int> requests;

public:
    Elevator(int id) : id(id) {}

    void addRequest(int floor) {
        requests.insert(floor);
    }

    void move() {
        if (requests.empty()) {
            direction = Direction::IDLE;
            return;
        }

        int target = *requests.begin();

        if (target > currentFloor)
            direction = Direction::UP;
        else if (target < currentFloor)
            direction = Direction::DOWN;
        else {
            door.openDoor();
            requests.erase(target);
            direction = Direction::IDLE;
            return;
        }

        currentFloor += (direction == Direction::UP ? 1 : -1);
    }

    int getFloor() {
        return currentFloor;
    }

    Direction getDirection() {
        return direction;
    }
};

class ElevatorSystem {
    vector<Elevator> elevators;

public:
    ElevatorSystem(int n) {
        for (int i = 1; i <= n; i++)
            elevators.emplace_back(i);
    }

    void selectFloor(int elevatorId, int floor) {
        elevators[elevatorId].addRequest(floor);
    }

};
