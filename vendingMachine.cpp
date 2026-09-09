// ----- Product class (merged) -----
class Product {
private:
    string productId;
    string name;
    int price;
    int quantity;
    bool available;

public:
    Product(std::string productId, std::string name, double price, int quantity = 0)
        : productId(std::move(productId)), name(std::move(name)), price(price), quantity(quantity), available(true) {}

    // Accessors
    std::string getProductId() const { return productId; }
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
    string transactionId;
    string productId;
    int quantity;
    int amount;

public:
    Transaction(std::string transactionId, std::string productId, int quantity, double amount)
    }

    // Accessors
    std::string getTransactionId() const { return transactionId; }
    std::string getProductId() const { return productId; }
};

// ----- VendingMachine class (merged) -----
class VendingMachine {
private:
    string machineId;
    vector<Product*> products;
    vector<Transaction*> transactions;
    double cashBalance;
    int productIdCounter;
    int transactionIdCounter;

public:
    VendingMachine(std::string machineId)
        : machineId(std::move(machineId)), cashBalance(0.0), operational(true),
          productIdCounter(1), transactionIdCounter(1) {}
   
    // Basic getters

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
    Transaction* purchaseProduct(const std::string& productId, int qty, double payment) {
        Product* p = findProduct(productId);
        if (!p || !p->isAvailable() || p->getQuantity() < qty) return nullptr;
        double total = p->getPrice() * qty;
        if (payment < total) return nullptr;
        std::string tid = generateTransactionId();
        Transaction* tr = new Transaction(tid, productId, qty, total);
        if (p->removeQuantity(qty)) {
            cashBalance += total;
            transactions.push_back(tr);
            return tr;
        }
        return nullptr;
    }
    void addCash(double amount) { cashBalance += amount; }
    bool withdrawCash(double amount) {
        if (amount <= cashBalance) { cashBalance -= amount; return true; }
        return false;
    }
