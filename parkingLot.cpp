// ---------- Enums ----------
enum class VehicleType {
    CAR,
    MOTORCYCLE,
    TRUCK,
    BUS
};

enum class SpotType {
    COMPACT,
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
    string getLicensePlate() const { return licensePlate; }
    VehicleType getType() const { return type; }
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
    int getSpotNumber() const { return spotNumber; }
    SpotType getType() const { return type; }
    Vehicle* getVehicle() const { return vehicle; }
    bool isAvailable() const { return available; }
    bool canFitVehicle(const Vehicle* v) const {
        if (!v) return false;
        switch (v->getType()) {
            case VehicleType::MOTORCYCLE:
                return true; // any spot
            case VehicleType::CAR:
                return type != SpotType::COMPACT; // cannot go into compact
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
    Vehicle* removeVehicle() {
        if (!vehicle) return nullptr;
        Vehicle* removed = vehicle;
        vehicle = nullptr;
        available = true;
        return removed;
    }
};

// ---------- ParkingLot class ----------
class ParkingLot {
private:
    vector<ParkingSpot*> spots;
    map<string, ParkingSpot*> occupiedSpots; // maps licensePlate -> spot
    int capacity;
    int availableSpots;
public:
    ParkingLot(int numCompact, int numRegular, int numLarge)
        : capacity(numCompact + numRegular + numLarge), availableSpots(capacity) {
        int spotNumber = 1;
        for (int i = 0; i < numCompact; ++i) {
            spots.push_back(new ParkingSpot(spotNumber++, SpotType::COMPACT));
        }
        for (int i = 0; i < numRegular; ++i) {
            spots.push_back(new ParkingSpot(spotNumber++, SpotType::REGULAR));
        }
        for (int i = 0; i < numLarge; ++i) {
            spots.push_back(new ParkingSpot(spotNumber++, SpotType::LARGE));
        }
    }
    ~ParkingLot() {
        for (auto s : spots) delete s;
    }
    int getCapacity() const { return capacity; }
    int getAvailableSpots() const { return availableSpots; }
    bool parkVehicle(Vehicle* v) {
        if (!v) return false;
        if (occupiedSpots.find(v->getLicensePlate()) != occupiedSpots.end()) return false;
        ParkingSpot* spot = findAvailableSpot(v);
        if (!spot) return false;
        if (spot->parkVehicle(v)) {
            occupiedSpots[v->getLicensePlate()] = spot;
            --availableSpots;
            return true;
        }
        return false;
    }
    Vehicle* removeVehicle(const string& licensePlate) {
        auto it = occupiedSpots.find(licensePlate);
        if (it == occupiedSpots.end()) return nullptr;
        ParkingSpot* spot = it->second;
        Vehicle* v = spot->removeVehicle();
        if (v) {
            occupiedSpots.erase(it);
            ++availableSpots;
        }
        return v;
    }
    ParkingSpot* findVehicle(const string& licensePlate) const {
        auto it = occupiedSpots.find(licensePlate);
        return it != occupiedSpots.end() ? it->second : nullptr;
    }
   
private:
    ParkingSpot* findAvailableSpot(const Vehicle* v) const {
        for (auto s : spots) {
            if (s->isAvailable() && s->canFitVehicle(v)) return s;
        }
        return nullptr;
    }
};

