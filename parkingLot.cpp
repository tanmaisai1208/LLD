// ---------- Enums ----------
enum class VehicleType {
    CAR,
    MOTORCYCLE,
    TRUCK,
    BUS
};

enum class SpotType {
    SMALL,
    REGULAR,
    LARGE
};

// ---------- Vehicle class ----------
class Vehicle {
private:
    string licensePlate;   // to uniquely define a vehicle
    VehicleType type;
public:
    Vehicle(string licensePlate, VehicleType type)
        : licensePlate(move(licensePlate)), type(type) {}
    string getLicensePlate()  { return licensePlate; }
    VehicleType getType() { return type; }
};

// ---------- ParkingSpot class ----------
class ParkingSpot {
private:
    int spotNumber;
    SpotType type;
    Vehicle* vehicle;           // pointer to vehicle parked in that spot
    bool available;
public:
    ParkingSpot(int spotNumber, SpotType type)
        : spotNumber(spotNumber), type(type), vehicle(nullptr), available(true) {}
    int getSpotNumber()  { return spotNumber; }
    SpotType getType()  { return type; }
    Vehicle* getVehicle()  { return vehicle; }
    bool isAvailable()  { return available; }
    bool canFitVehicle( Vehicle* v)  {
        if (!v) return false;
        switch (v->getType()) {
            case VehicleType::MOTORCYCLE:
                return true; // any spot
            case VehicleType::CAR:
                return type != SpotType::SMALL; // cannot go into compact
            case VehicleType::TRUCK:
            case VehicleType::BUS:
                return type == SpotType::LARGE;
        }
        return false;
    }
    bool parkVehicle(Vehicle* v) {
        if (!available || !canFitVehicle(v)) return false;
        vehicle = v;
        available = false;
        return true;
    }
    
    void removeVehicle() {
        vehicle = nullptr;
        available = true;
    }
};

// ---------- ParkingLot class ----------
class ParkingLot {
private:
    vector<ParkingSpot*> spots;
    map<string, ParkingSpot*> occupiedSpots; // maps licensePlate -> spot
    int capacity;
    int availableSpots;
    ParkingSpot* findAvailableSpot( Vehicle* v)  {
        for (auto s : spots) {
            if (s->isAvailable() && s->canFitVehicle(v)) return s;
        }
        return nullptr;
    }

public:
    ParkingLot(int numCompact, int numRegular, int numLarge)
        : capacity(numCompact + numRegular + numLarge), availableSpots(capacity) {
        
    }
    ~ParkingLot() {
        for (auto s : spots) delete s;
    }
    int getCapacity() { return capacity; }
    int getAvailableSpots()  { return availableSpots; }
    bool parkVehicle(Vehicle* v) {
        if (!v) return false;
        ParkingSpot* spot = findAvailableSpot(v);
        if (!spot) return false;
        if (spot->parkVehicle(v)) {
            occupiedSpots[v->getLicensePlate()] = spot;
            --availableSpots;
            return true;
        }
        return false;
    }
    void removeVehicle( string& licensePlate) {
        auto it = occupiedSpots.find(licensePlate);
        spot = it->second;
        spot->removeVehicle();
        occupiedSpots.erase(it);
        ++availableSpots;
};

