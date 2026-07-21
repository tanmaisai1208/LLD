// ----- Product class (merged) -----
class Product {
private:
    std::string productId;
    std::string name;
    double price;
    int quantity;
    bool available;

public:
    Product(std::string productId, std::string name, double price, int quantity = 0)
        : productId(std::move(productId)), name(std::move(name)), price(price), quantity(quantity), available(true) {}

    // Accessors
    std::string getProductId() const { return productId; }
    std::string getName() const { return name; }
    double getPrice() const { return price; }
    int getQuantity() const { return quantity; }
    bool isAvailable() const { return available && quantity > 0; }

    // Mutators
    void setPrice(double p) { price = p; }
    void setQuantity(int q) { quantity = q; }
    void setAvailable(bool status) { available = status; }
    void addQuantity(int amount) { quantity += amount; }
    bool removeQuantity(int amount) {
        if (amount <= quantity) {
            quantity -= amount;
            return true;
        }
        return false;
    }
};


// ----- Transaction class (merged) -----
class Transaction {
private:
    std::string transactionId;
    std::string productId;
    int quantity;
    double amount;
    std::time_t timestamp;
    bool successful;

public:
    Transaction(std::string transactionId, std::string productId, int quantity, double amount)
        : transactionId(std::move(transactionId)),
          productId(std::move(productId)),
          quantity(quantity),
          amount(amount),
          successful(false) {
        timestamp = std::time(nullptr);
    }

    // Accessors
    std::string getTransactionId() const { return transactionId; }
    std::string getProductId() const { return productId; }
    int getQuantity() const { return quantity; }
    double getAmount() const { return amount; }
    std::time_t getTimestamp() const { return timestamp; }
    bool isSuccessful() const { return successful; }

    // Mutators
    void setSuccessful(bool status) { successful = status; }
};



// ----- VendingMachine class (merged) -----
class VendingMachine {
private:
    std::string machineId;
    std::vector<Product*> products;
    std::vector<Transaction*> transactions;
    double cashBalance;
    bool operational;
    int productIdCounter;
    int transactionIdCounter;

public:
    VendingMachine(std::string machineId)
        : machineId(std::move(machineId)), cashBalance(0.0), operational(true),
          productIdCounter(1), transactionIdCounter(1) {}
    ~VendingMachine() {
        for (auto p : products) delete p;
        for (auto t : transactions) delete t;
    }

    // Basic getters
    std::string getMachineId() const { return machineId; }
    double getCashBalance() const { return cashBalance; }
    bool isOperational() const { return operational; }

    // Core operations
    Product* addProduct(const std::string& name, double price, int quantity = 0) {
        std::string pid = generateProductId();
        Product* p = new Product(pid, name, price, quantity);
        products.push_back(p);
        return p;
    }
    void removeProduct(const std::string& productId) {
        auto it = std::find_if(products.begin(), products.end(),
            [&](Product* p){ return p->getProductId() == productId; });
        if (it != products.end()) { delete *it; products.erase(it); }
    }
    bool restockProduct(const std::string& productId, int qty) {
        Product* p = findProduct(productId);
        if (!p) return false;
        p->addQuantity(qty);
        return true;
    }
    bool updatePrice(const std::string& productId, double price) {
        Product* p = findProduct(productId);
        if (!p) return false;
        p->setPrice(price);
        return true;
    }
    Transaction* purchaseProduct(const std::string& productId, int qty, double payment) {
        if (!operational) return nullptr;
        Product* p = findProduct(productId);
        if (!p || !p->isAvailable() || p->getQuantity() < qty) return nullptr;
        double total = p->getPrice() * qty;
        if (payment < total) return nullptr;
        std::string tid = generateTransactionId();
        Transaction* tr = new Transaction(tid, productId, qty, total);
        if (p->removeQuantity(qty)) {
            cashBalance += total;
            tr->setSuccessful(true);
            transactions.push_back(tr);
            return tr;
        }
        delete tr;
        return nullptr;
    }
    void addCash(double amount) { cashBalance += amount; }
    bool withdrawCash(double amount) {
        if (amount <= cashBalance) { cashBalance -= amount; return true; }
        return false;
    }
    void setOperational(bool status) { operational = status; }

private:
    Product* findProduct(const std::string& productId) const {
        auto it = std::find_if(products.begin(), products.end(),
            [&](Product* p){ return p->getProductId() == productId; });
        return it != products.end() ? *it : nullptr;
    }
    std::string generateProductId() { return "P" + std::to_string(productIdCounter++); }
    std::string generateTransactionId() { return "T" + std::to_string(transactionIdCounter++); }
};
