// ----- Enums -----
enum class BookingStatus { PENDING, CONFIRMED, CANCELLED };

enum class MovieGenre { ACTION, COMEDY, DRAMA, HORROR, SCIFI, THRILLER };

enum class ShowStatus { SCHEDULED, RUNNING, COMPLETED, CANCELLED };

// ----- Movie class -----
class Movie {
private:
    string movieId;
    string title;
    string description;
    MovieGenre genre;
    int durationMinutes;
    bool active;
public:
    Movie(string movieId, string title, string description,
          MovieGenre genre, int durationMinutes, string language)
        : movieId(move(movieId)), title(move(title)), description(move(description)),
          genre(genre), durationMinutes(durationMinutes), language(move(language)), active(true) {}
    string getMovieId()  { return movieId; }
    string getTitle()  { return title; }
    string getDescription()  { return description; }
    MovieGenre getGenre()  { return genre; }
    int getDurationMinutes()  { return durationMinutes; }
    string getLanguage()  { return language; }
    bool isActive()  { return active; }
     vector<string>& getCast()  { return cast; }
    void setActive(bool status) { active = status; }
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
    bool isSeatAvailable(int seatNumber)  {
        return !seats[seatNumber];
    }

    bool bookSeat(int seatNumber) {
        if (!isSeatAvailable(seatNumber)) return false;
        seats[seatNumber] = true;
        return true;
    }
    void cancelSeatBooking(int seatNumber) {
        seats[seatNumber] = false;
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
        
    }

    string getBookingId() const { return bookingId; }
    Show* getShow() const { return show; }
   
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
