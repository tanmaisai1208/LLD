// ----- Board class -----
class Board {
private:
    int size;
    map<int, int> snakes;   // start -> end
    map<int, int> ladders;  // start -> end
public:
    Board(int size = 100) : size(size) {}
    int getSize() const { return size; }
    void addSnake(int start, int end) {
        if (start > end && start <= size) {
            snakes[start] = end;
        }
    }
    void addLadder(int start, int end) {
        if (end > start && end <= size) {
            ladders[start] = end;
        }
    }
    int getNextPosition(int currentPos) const {
        auto snakeIt = snakes.find(currentPos);
        if (snakeIt != snakes.end()) {
            cout << "Oops! Snake at position " << currentPos << ". Moving down to " << snakeIt->second << endl;
            return snakeIt->second;
        }
        auto ladderIt = ladders.find(currentPos);
        if (ladderIt != ladders.end()) {
            cout << "Yay! Ladder at position " << currentPos << ". Moving up to " << ladderIt->second << endl;
            return ladderIt->second;
        }
        return currentPos;
    }
    void displayInfo() const {
        
    }
};

// ----- Dice class -----
class Dice {
private:
    int sides;
public:
    Dice(int sides = 6) : sides(sides) {
        srand(static_cast<unsigned int>(time(nullptr)));
    }
    int roll() const {
        return (rand() % sides) + 1;
    }
};

// ----- Player class -----
class Player {
private:
    string name;
    int position;
    bool winner;
public:
    Player(string name) : name(move(name)), position(0), winner(false) {}
    string getName() const { return name; }
    int getPosition() const { return position; }
    bool isWinner() const { return winner; }
    void setPosition(int pos) { position = pos; }
    void setWinner(bool w) { winner = w; }
    void displayInfo() const {
        cout << "Player: " << name << endl;
        cout << "Position: " << position << endl;
        cout << "Status: " << (winner ? "Winner!" : "Playing") << endl;
    }
};

// ----- Game class -----
class Game {
private:
    Board board;
    vector<Player*> players;
    Dice dice;
    int currentPlayerIndex;
    bool gameOver;
public:
    Game(int boardSize = 100) : board(boardSize), dice(6), currentPlayerIndex(0), gameOver(false) {}
    
    void addPlayer(const string &name) {
        players.push_back(new Player(name));
    }
    void setupBoard() {
        // input about snakes and ladder positions would be given and we store them
    }
    Player* getCurrentPlayer() const { return players[currentPlayerIndex]; }
    void movePlayer(Player *player, int steps) {
        int newPos = player->getPosition() + steps;
        if (newPos > board.getSize()) {
            cout << "Cannot move, need exact number to finish" << endl;
            return;
        }
        newPos = board.getNextPosition(newPos);
        player->setPosition(newPos);
        cout << player->getName() << " moved to position " << newPos << endl;
    }
    bool checkWin(Player *player) const { return player->getPosition() == board.getSize(); }
    bool makeMove() {
        if (gameOver || players.empty()) return false;
        Player *cur = getCurrentPlayer();
        cout << "\n" << cur->getName() << "'s turn" << endl;
        int roll = dice.roll();
        cout << "Rolled: " << roll << endl;
        movePlayer(cur, roll);
        if (checkWin(cur)) {
            cur->setWinner(true);
            gameOver = true;
            cout << "\nCongratulations! " << cur->getName() << " wins!" << endl;
            return false;
        }
        currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
        return true;
    }
    void displayStatus() const {
        
    }
    void play() {
        if (players.empty()) {
            cout << "No players in the game!" << endl;
            return;
        }
        cout << "\nStarting the game..." << endl;
        while (makeMove()) {
            displayStatus();
            this_thread::sleep_for(chrono::seconds(1));
        }
        displayStatus();
    }
};


