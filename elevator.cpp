1) DIRECTION: 
enum class Direction {
    UP,
    DOWN,
    IDLE
};

--------------------------------------------------------------------------------------------------

2) DOORACTION:
public enum DoorAction {
    OPEN,CLOSE;
}

--------------------------------------------------------------------------------------------------

3) FLOORNUMBER:
public enum FloorNumber {
    FLOOR_NUMBER1,FLOOR_NUMBER2,FLOOR_NUMBER3,FLOOR_NUMBER4,FLOOR_NUMBER5,FLOOR_NUMBER6,FLOOR_NUMBER7,FLOOR_NUMBER8,FLOOR_NUMBER9,FLOOR_NUMBER10,FLOOR_NUMBER11,FLOOR_NUMBER12,FLOOR_NUMBER13,FLOOR_NUMBER14,FLOOR_NUMBER15;
}

--------------------------------------------------------------------------------------------------

4) BUTTON:
public interface Button {
    boolean isPressed();
    boolean press();
}

--------------------------------------------------------------------------------------------------


5) PANNEL:
class Pannel {
public:
    virtual void someMethod() = 0;
};

--------------------------------------------------------------------------------------------------


6) DISPLAY:
public class Display {
    private FloorNumber floorNumber;
    private Direction direction;
    private Integer weight;

    public Display(FloorNumber floorNumber, Direction direction, Integer weight) {
        this.floorNumber = floorNumber;

        this.direction = direction;
        this.weight = weight;
    }

    public Display() {
    }

    public FloorNumber getFloorNumber() {
        return floorNumber;
    }

    public void setFloorNumber(FloorNumber floorNumber) {
        this.floorNumber = floorNumber;
    }

    public Direction getDirection() {
        return direction;
    }

    public void setDirection(Direction direction) {
        this.direction = direction;
    }

    public Integer getWeight() {
        return weight;
    }

    public void setWeight(Integer weight) {
        this.weight = weight;
    }
}

--------------------------------------------------------------------------------------------------

7) DOOR:
import enums.DoorAction;

public class Door {
    private DoorAction doorAction;

    public Door(DoorAction doorAction) {
        this.doorAction = doorAction;
    }

    public Door() {
    }

    public void openDoor(){
        doorAction = DoorAction.OPEN;
    }
    public void closeDoor(){
        doorAction = DoorAction.CLOSE;
    }
}

--------------------------------------------------------------------------------------------------


8) DOORBUTTON:
import enums.DoorAction;
import interfaces.Button;

public class DoorButton implements Button {
    private boolean status;

    private DoorAction doorAction;

    public DoorButton(boolean status, DoorAction doorAction) {
        this.status = status;
        this.doorAction = doorAction;
    }
    public DoorButton() {
    }

    public DoorAction getDoorAction() {
        return doorAction;
    }

    public void setDoorAction(DoorAction doorAction) {
        this.doorAction = doorAction;
    }



    public void setStatus(boolean status) {
        this.status = status;
    }

    @Override
    public boolean isPressed() {
        return status;
    }

    @Override
    public boolean press() {
        status=!status;
        return status;
    }    
}

--------------------------------------------------------------------------------------------------


9) ELEVATOR BUTTON:
#include <iostream>
#include "FloorNumber.h"

class Button {
public:
    virtual bool isPressed() = 0;
    virtual bool press() = 0;
};

class ElevatorButton : public Button {
private:
    bool status;
    FloorNumber floorNumber;

public:
    ElevatorButton(bool status, FloorNumber floorNumber) : status(status), floorNumber(floorNumber) {}
    ElevatorButton() {}

    FloorNumber getFloorNumber() {
        return floorNumber;
    }

    void setFloorNumber(FloorNumber floorNumber) {
        this->floorNumber = floorNumber;
    }

    void setStatus(bool status) {
        this->status = status;
    }

    bool isPressed() override {
        return status;
    }

    bool press() override {
        status = !status;
        return status;
    }
};


--------------------------------------------------------------------------------------------------


10) HALLBUTTON:
import enums.Direction;
import interfaces.Button;

public class HallButton implements Button {
    private boolean status;

    private Direction direction;

    public HallButton(boolean status, Direction direction) {
        this.status = status;
        this.direction = direction;
    }
    public HallButton() {
    }

    public Direction getDirection() {
        return direction;
    }

    public void setDirection(Direction direction) {
        this.direction = direction;
    }



    public void setStatus(boolean status) {
        this.status = status;
    }

    @Override
    public boolean isPressed() {
        return status;
    }

    @Override
    public boolean press() {
        status=!status;
        return status;
    }
}


--------------------------------------------------------------------------------------------------


11) ELEVATOR:
import enums.Direction;
import enums.ElevatorNumber;
import enums.FloorNumber;

public class Elevator {
    private ElevatorNumber elevatorNumber;
    private Door door;
    private InsidePannel insidePannel;
    private Display display;
    private FloorNumber currentFloorNumber;
    private Direction currentDirection;

    public Elevator(ElevatorNumber elevatorNumber, Door door, InsidePannel insidePannel, Display display, FloorNumber currentFloorNumber, Direction currentDirection) {
        this.elevatorNumber = elevatorNumber;
        this.door = door;
        this.insidePannel = insidePannel;
        this.display = display;
        this.currentFloorNumber = currentFloorNumber;
        this.currentDirection = currentDirection;
    }

    public Elevator() {
    }

    public ElevatorNumber getElevatorNumber() {
        return elevatorNumber;
    }

    public void setElevatorNumber(ElevatorNumber elevatorNumber) {
        this.elevatorNumber = elevatorNumber;
    }

    public Door getDoor() {
        return door;
    }

    public void setDoor(Door door) {
        this.door = door;
    }

    public InsidePannel getInsidePannel() {
        return insidePannel;
    }

    public void setInsidePannel(InsidePannel insidePannel) {
        this.insidePannel = insidePannel;
    }

    public Display getDisplay() {
        return display;
    }

    public void setDisplay(Display display) {
        this.display = display;
    }

    public FloorNumber getCurrentFloorNumber() {
        return currentFloorNumber;
    }

    public void setCurrentFloorNumber(FloorNumber currentFloorNumber) {
        this.currentFloorNumber = currentFloorNumber;
    }

    public Direction getCurrentDirection() {
        return currentDirection;
    }

    public void setCurrentDirection(Direction currentDirection) {
        this.currentDirection = currentDirection;
    }
}


--------------------------------------------------------------------------------------------------


12) FLOOR:
import enums.FloorNumber;

public class Floor {
    private FloorNumber floorNumber;
    private OutsidePannel outsidePannel;

    public FloorNumber getFloorNumber() {
        return floorNumber;
    }

    public void setFloorNumber(FloorNumber floorNumber) {
        this.floorNumber = floorNumber;
    }

    public OutsidePannel getOutsidePannel() {
        return outsidePannel;
    }

    public void setOutsidePannel(OutsidePannel outsidePannel) {
        this.outsidePannel = outsidePannel;
    }

    public Floor(FloorNumber floorNumber, OutsidePannel outsidePannel) {
        this.floorNumber = floorNumber;
        this.outsidePannel = outsidePannel;
    }
}


--------------------------------------------------------------------------------------------------


13) INSIDE PANNEL:

public class InsidePannel implements Pannel{
    private List<ElevatorButton> elevatorButtonList;
    private List<DoorButton> doorButtons;

    public InsidePannel() {
    elevatorButtonList=new ArrayList<>();
    doorButtons=new ArrayList<>();
        for (int i = 0; i <15 ; i++) {
            elevatorButtonList.add(new ElevatorButton(false, FloorNumber.values()[i]));
        }

        for (int i = 0; i <3 ; i++) {
            doorButtons.add(new DoorButton(false, DoorAction.values()[i]));
        }
    }

    public boolean pressFloorButton(int floorNumber) {
        return elevatorButtonList.get(floorNumber).press();
    }
    public boolean pressDoorButton(int doorNumber) {
        return doorButtons.get(doorNumber).press();
    }
}


--------------------------------------------------------------------------------------------------


14) OUTSIDE PANNEL:
import enums.Direction;
import interfaces.Pannel;

import java.util.ArrayList;
import java.util.List;

public class OutsidePannel implements Pannel {
    private List<HallButton> hallButtons;

    public OutsidePannel() {
        hallButtons = new ArrayList<>();
        hallButtons.add(new HallButton(false,Direction.UP));
        hallButtons.add(new HallButton(false,Direction.DOWN));
        hallButtons.add(new HallButton(false,Direction.IDLE));
    }

    public void moveUp(){

    }
    public void moveDown(){

    }
}
