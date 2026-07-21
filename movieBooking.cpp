// ----- Enums -----
enum class BookingStatus { PENDING, CONFIRMED, CANCELLED };

enum class MovieGenre { ACTION, COMEDY, DRAMA, HORROR, SCIFI, THRILLER };

enum class ShowStatus { SCHEDULED, RUNNING, COMPLETED, CANCELLED };

// ----- Forward declarations -----
class Show;
class Movie;
class Theater;

// ----- Booking class -----
class Booking {
private:
    string bookingId;
    Show* show;
    string customerName;
    string customerPhone;
    vector<int> seatNumbers;
    double totalAmount;
    BookingStatus status;
    string timestamp;
public:
    Booking(string bookingId, Show* show, string customerName,
            string customerPhone, const vector<int>& seatNumbers)
        : bookingId(move(bookingId)), show(show), customerName(move(customerName)),
          customerPhone(move(customerPhone)), seatNumbers(seatNumbers),
          status(BookingStatus::PENDING) {
        // timestamp
        auto now = time(nullptr);
        auto tm = *localtime(&now);
        ostringstream oss;
        oss << put_time(&tm, "%Y-%m-%d %H:%M:%S");
        timestamp = oss.str();
        calculate();
    }
    string getBookingId() const { return bookingId; }
    Show* getShow() const { return show; }
    string getCustomerName() const { return customerName; }
    string getCustomerPhone() const { return customerPhone; }
    const vector<int>& getSeatNumbers() const { return seatNumbers; }
    double getTotalAmount() const { return totalAmount; }
    BookingStatus getStatus() const { return status; }
    string getTimestamp() const { return timestamp; }
    void calculateTotalAmount() { totalAmount = show->getTicketPrice() * seatNumbers.size(); }
    void setStatus(BookingStatus s) { status = s; }
    void displayInfo() const {
        cout << "\nBooking Details:" << endl;
        cout << "Booking ID: " << bookingId << endl;
        cout << "Customer Name: " << customerName << endl;
        cout << "Customer Phone: " << customerPhone << endl;
        show->displayInfo();
        cout << "Seats: ";
        for (int seat : seatNumbers) cout << seat << " ";
        cout << endl;
        cout << "Total Amount: $" << fixed << setprecision(2) << totalAmount << endl;
        cout << "Status: ";
        switch (status) {
            case BookingStatus::PENDING: cout << "Pending"; break;
            case BookingStatus::CONFIRMED: cout << "Confirmed"; break;
            case BookingStatus::CANCELLED: cout << "Cancelled"; break;
        }
        cout << endl;
        cout << "Booking Time: " << timestamp << endl;
    }
};

// ----- Movie class -----
class Movie {
private:
    string movieId;
    string title;
    string description;
    MovieGenre genre;
    int durationMinutes;
    string language;
    vector<string> cast;
    bool active;
public:
    Movie(string movieId, string title, string description,
          MovieGenre genre, int durationMinutes, string language)
        : movieId(move(movieId)), title(move(title)), description(move(description)),
          genre(genre), durationMinutes(durationMinutes), language(move(language)), active(true) {}
    string getMovieId() const { return movieId; }
    string getTitle() const { return title; }
    string getDescription() const { return description; }
    MovieGenre getGenre() const { return genre; }
    int getDurationMinutes() const { return durationMinutes; }
    string getLanguage() const { return language; }
    bool isActive() const { return active; }
    const vector<string>& getCast() const { return cast; }
    void setActive(bool status) { active = status; }
    void displayInfo() const {

    }
};

// ----- Show class -----
class Show {
private:
    string showId;
    Movie* movie;
    string date;
    string startTime;
    double ticketPrice;
    vector<bool> seats; // true = booked, false = available
    ShowStatus status;
public:
    Show(string showId, Movie* movie, string date, string startTime,
         double ticketPrice, int totalSeats)
        : showId(move(showId)), movie(movie), date(move(date)), startTime(move(startTime)),
          ticketPrice(ticketPrice), seats(totalSeats, false), status(ShowStatus::SCHEDULED) {}
    string getShowId() const { return showId; }
    Movie* getMovie() const { return movie; }
    string getDate() const { return date; }
    string getStartTime() const { return startTime; }
    double getTicketPrice() const { return ticketPrice; }
    ShowStatus getStatus() const { return status; }
    bool isSeatAvailable(int seatNumber) const {
        if (seatNumber < 1 || seatNumber > (int)seats.size()) return false;
        return !seats[seatNumber - 1];
    }

    bool bookSeat(int seatNumber) {
        if (!isSeatAvailable(seatNumber)) return false;
        seats[seatNumber - 1] = true;
        return true;
    }
    void cancelSeatBooking(int seatNumber) {
        if (seatNumber >= 1 && seatNumber <= (int)seats.size()) seats[seatNumber - 1] = false;
    }
    void setStatus(ShowStatus s) { status = s; }
    int getAvailableSeats() const {
        int cnt = 0;
        for (bool b : seats) if (!b) ++cnt;
        return cnt;
    }
    void displayInfo() const {

    }
};

// ----- Theater class -----
class Theater {
private:
    string theaterId;
    string name;
    string location;
    int totalSeats;
    vector<Show*> shows;
    bool active;
public:
    Theater(string theaterId, string name, string location, int totalSeats)
        : theaterId(move(theaterId)), name(move(name)), location(move(location)),
          totalSeats(totalSeats), active(true) {}
    string getTheaterId() const { return theaterId; }
    string getName() const { return name; }
    string getLocation() const { return location; }
    int getTotalSeats() const { return totalSeats; }
    bool isActive() const { return active; }
    const vector<Show*>& getShows() const { return shows; }
    void addShow(Show* s) { shows.push_back(s); }
    void removeShow(Show* s) {
        auto it = find(shows.begin(), shows.end(), s);
        if (it != shows.end()) shows.erase(it);
    }
    void setActive(bool status) { active = status; }
    void displayInfo() const {
    }
};

// ----- BookingSystem class -----
class BookingSystem {
private:
    vector<Movie*> movies;
    vector<Theater*> theaters;
    vector<Booking*> bookings;
    int bookingIdCounter;
public:
    BookingSystem() : bookingIdCounter(1) {}
    
    void addMovie(Movie* m) { movies.push_back(m); }
    void addTheater(Theater* t) { theaters.push_back(t); }
    Booking* createBooking(string showId, string customerName,
                           string customerPhone, const vector<int>& seats) {
        Show* show = findShow(showId);
        if (!show || show->getStatus() != ShowStatus::SCHEDULED) return nullptr;
        // check availability
        for (int seat : seats) if (!show->isSeatAvailable(seat)) return nullptr;
        // book seats
        for (int seat : seats) show->bookSeat(seat);
        Booking* b = new Booking(generateBookingId(), show, customerName, customerPhone, seats);
        b->setStatus(BookingStatus::CONFIRMED);
        bookings.push_back(b);
        return b;
    }
    bool cancelBooking(string bookingId) {
        Booking* b = findBooking(bookingId);
        if (!b || b->getStatus() == BookingStatus::CANCELLED) return false;
        Show* show = b->getShow();
        for (int seat : b->getSeatNumbers()) show->cancelSeatBooking(seat);
        b->setStatus(BookingStatus::CANCELLED);
        return true;
    }
private:
    Show* findShow(const string& showId) const {
        for (const auto& th : theaters) {
            for (const auto& sh : th->getShows()) {
                if (sh->getShowId() == showId) return sh;
            }
        }
        return nullptr;
    }
    Booking* findBooking(const string& bookingId) const {
        for (auto b : bookings) if (b->getBookingId() == bookingId) return b;
        return nullptr;
    }
    string generateBookingId() { return "B" + to_string(bookingIdCounter++); }
};
