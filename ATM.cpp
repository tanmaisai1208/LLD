// ----- Account class -----
class Account {
private:
    string accountNumber;
    string pin;
    double balance;
public:
    Account(string accountNumber, string pin, double initialBalance = 0.0)
        : accountNumber(move(accountNumber)), pin(move(pin)), balance(initialBalance) {}
    string getAccountNumber() const { return accountNumber; }
    bool validatePin(const string& inputPin) const { return pin == inputPin; }
    double getBalance() const { return balance; }
    bool deposit(double amount) {
        if (amount <= 0) return false;
        balance += amount;
        return true;
    }
    bool withdraw(double amount) {
        if (amount <= 0 || amount > balance) return false;
        balance -= amount;
        return true;
    }
    void displayBalance() const {
        cout << "Current balance: $" << fixed << setprecision(2) << balance << endl;
    }
};

// ----- ATM class -----
class ATM {
private:
    vector<Account*> accounts;
    Account* currentAccount;
    bool isAuthenticated;
public:
    ATM() : currentAccount(nullptr), isAuthenticated(false) {}
    
    void addAccount(Account* account) { accounts.push_back(account); }
    bool authenticate(const string& accountNumber, const string& pin) {
        for (auto account : accounts) {
            if (account->getAccountNumber() == accountNumber && account->validatePin(pin)) {
                currentAccount = account;
                isAuthenticated = true;
                return true;
            }
        }
        return false;
    }
    void logout() { currentAccount = nullptr; isAuthenticated = false; }
    bool deposit(double amount) {
        if (!isAuthenticated || !currentAccount) return false;
        return currentAccount->deposit(amount);
    }
    bool withdraw(double amount) {
        if (!isAuthenticated || !currentAccount) return false;
        return currentAccount->withdraw(amount);
    }
    void checkBalance() const {
        if (!isAuthenticated || !currentAccount) return;
        currentAccount->displayBalance();
    }
    void displayMenu() const {
        cout << "\nATM Menu:" << endl;
        cout << "1. Check Balance" << endl;
        cout << "2. Deposit" << endl;
        cout << "3. Withdraw" << endl;
        cout << "4. Logout" << endl;
        cout << "5. Exit" << endl;
        cout << "Enter your choice: ";
    }
    void start() {
        string accountNumber, pin;
        int choice;
        double amount;
        while (true) {
            if (!isAuthenticated) {
                cout << "\nWelcome to ATM" << endl;
                cout << "Enter account number: ";
                cin >> accountNumber;
                cout << "Enter PIN: ";
                cin >> pin;
                if (!authenticate(accountNumber, pin)) {
                    cout << "Invalid account number or PIN" << endl;
                    continue;
                }
                cout << "Authentication successful!" << endl;
            }
            displayMenu();
            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            switch (choice) {
                case 1:
                    checkBalance();
                    break;
                case 2:
                    cout << "Enter amount to deposit: $";
                    cin >> amount;
                    if (deposit(amount)) {
                        cout << "Deposit successful" << endl;
                        checkBalance();
                    } else {
                        cout << "Invalid amount" << endl;
                    }
                    break;
                case 3:
                    cout << "Enter amount to withdraw: $";
                    cin >> amount;
                    if (withdraw(amount)) {
                        cout << "Withdrawal successful" << endl;
                        checkBalance();
                    } else {
                        cout << "Invalid amount or insufficient funds" << endl;
                    }
                    break;
                case 4:
                    logout();
                    cout << "Logged out successfully" << endl;
                    break;
                case 5:
                    cout << "Thank you for using ATM. Goodbye!" << endl;
                    return;
                default:
                    cout << "Invalid choice" << endl;
            }
        }
    }
};
