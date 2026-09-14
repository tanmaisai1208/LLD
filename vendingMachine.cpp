// ----- Product class (merged) -----
class Product {
private:
    string productId;
    string name;
    int price;
    int quantity;
    bool available;

public:
    Product(string productId,string name, double price, int quantity = 0)
        : productId(move(productId)), move(name), price(price), quantity(quantity), available(true) {}

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

// ----- VendingMachine class (merged) -----
class VendingMachine {
private:
    string machineId;
    vector<Product*> products;
    double cashBalance;
    int productIdCounter;

public:
    VendingMachine(string machineId)
        : machineId(move(machineId)), cashBalance(0.0), operational(true),
          productIdCounter(1) {}
   
    // Basic getters

    // Core operations
    Product* addProduct(string& name, double price, int quantity = 0) {
        string pid = generateProductId();
        Product* p = new Product(pid, name, price, quantity);
        products.push_back(p);
        return p;
    }

    void removeProduct(const std::string& productId) {
        auto it = find_if(products.begin(), products.end(),
            [&](Product* p){ return p->getProductId() == productId; });
        if (it != products.end()) { delete *it; products.erase(it); }
    }

    bool restockProduct(string& productId, int qty) {
        Product* p = findProduct(productId);
        if (!p) return false;
        p->addQuantity(qty);
        return true;
    }
    
    bool purchaseProduct(const std::string& productId, int qty, double payment) {
        Product* p = findProduct(productId);
        if (!p || !p->isAvailable() || p->getQuantity() < qty) return false;
        double total = p->getPrice() * qty;
        if (payment < total) return false;
        if (p->removeQuantity(qty)) {
            cashBalance += total;
        }
        return true;
    }
    
