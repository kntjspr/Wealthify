#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <locale>
#include <map>
#include <functional> // Added for std::function
#include <cstring> // For strcmp

// Transaction structure
struct Transaction {
    std::string description;
    double amount;
    time_t timestamp;
    std::vector<std::string> tags;

    // Constructor with parameters
    Transaction(const std::string& desc = "", double amt = 0.0, time_t ts = std::time(nullptr), const std::vector<std::string>& t = {})
        : description(desc), amount(amt), timestamp(ts), tags(t) {}

    // Function to add tags
    void addTag(const std::string& tag) {
        tags.push_back(tag);
    }

    // Function to check if a tag exists
    bool hasTag(const std::string& tag) const {
        return std::find(tags.begin(), tags.end(), tag) != tags.end();
    }
};

// Finance tracker class
class FinanceTracker {
private:
    double balance;
    std::vector<Transaction> transactions;
    std::map<std::string, std::vector<Transaction>> categorizedTransactions;

public:
    // Constructor to initialize balance
    FinanceTracker() : balance(0.0) {}

    // Function to add a transaction
    void addTransaction(const std::string& description, double amount, const std::vector<std::string>& tags = {}) {
        Transaction t(description, amount, std::time(nullptr), tags);
        transactions.push_back(t);
        balance += amount;

        // Categorize transaction based on tags
        for (const auto& tag : tags) {
            categorizedTransactions[tag].push_back(t);
        }
    }

    // Function to display all transactions
    void displayTransactions() const {
        std::cout << "=============================================================================================" << std::endl;
        std::cout << "| Date                   | Description                      | Amount       | Tags           |" << std::endl;
        std::cout << "=============================================================================================" << std::endl;

        for (const auto& transaction : transactions) {
            std::cout << "| " << std::left << std::setw(24) << formatDateTime(transaction.timestamp)
                      << "| " << std::left << std::setw(31) << transaction.description
                      << "| " << std::right << std::setw(13) << std::fixed << std::setprecision(2) << transaction.amount
                      << " | " << std::left << std::setw(15) << join(transaction.tags, ", ")
                      << " |" << std::endl;
        }
        std::cout << "=============================================================================================\n\n" << std::endl;


        std::cout << "================================================================================" << std::endl;
        std::cout << "| " << std::setw(63) << "Current Balance: " << std::right << std::setw(13) << balance << " |" << std::endl;
        std::cout << "================================================================================" << std::endl;
    }

    // Function to save transactions to a file
    void saveTransactionsToFile(const std::string& filename) const {
        std::ofstream file(filename);

        if (file.is_open()) {
            file << "Date , " << "Description , " << "Amount , " << "Tags" << std::endl;
            for (const auto& transaction : transactions) {
                file << formatDateTime(transaction.timestamp) << "," << transaction.description << ","
                     << transaction.amount << "," << join(transaction.tags, ";") << std::endl;
            }
            file.close();
            std::cout << "Transactions saved to " << filename << " successfully." << std::endl;
        } else {
            std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
        }
    }

// Function to parse a string in the format "MM/DD/YYYY - hh:mmAM/PM" into a timestamp
time_t parseDateTime(const std::string& datetimeStr) {
    std::tm tm = {};
    std::stringstream ss(datetimeStr);

    // Parse the date part "MM/DD/YYYY"
    ss >> std::get_time(&tm, "%m/%d/%Y");

    // Extract the delimiter " - "
    char delimiter;
    ss >> delimiter;

    // Parse the time part "hh:mmAM/PM"
    int hour, minute;
    char meridian[3];
    ss >> std::setw(2) >> hour;   // Read hours
    ss.ignore();                  // Skip ':'
    ss >> std::setw(2) >> minute; // Read minutes
    ss >> meridian;               // Read AM/PM

    // Convert 12-hour format to 24-hour format
    if (std::strcmp(meridian, "PM") == 0 && hour != 12) {
        hour += 12;
    } else if (std::strcmp(meridian, "AM") == 0 && hour == 12) {
        hour = 0;
    }

    // Set tm structure
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = 0;

    // Convert tm struct to time_t
    time_t timestamp = std::mktime(&tm);
    if (timestamp == -1) {
        throw std::runtime_error("Failed to convert time");
    }

    return timestamp;
}


    // Function to load transactions from a file
 void loadTransactionsFromFile(const std::string& filename) {
    std::ifstream file(filename);
    transactions.clear();
    categorizedTransactions.clear();
    balance = 0.0;

    if (file.is_open()) {
        std::string line;
        bool isFirstLine = true;  // Flag to skip the first line

        while (std::getline(file, line)) {
            if (isFirstLine) {
                isFirstLine = false;
                continue;  // Skip the first line (header)
            }

            std::istringstream iss(line);
            std::string timestampStr, description, amountStr, tagsStr;

            if (std::getline(iss, timestampStr, ',') &&
                std::getline(iss, description, ',') &&
                std::getline(iss, amountStr, ',') &&
                std::getline(iss, tagsStr)) {

                try {
                    // Convert timestampStr to a time_t timestamp
                    time_t timestamp = parseDateTime(timestampStr);
                    std::cout << timestamp << std::endl;
                    double amount = std::stod(amountStr);
                    std::vector<std::string> tags = split(tagsStr, ';');

                    Transaction t(description, amount, timestamp, tags);
                    transactions.push_back(t);
                    balance += amount;

                    for (const auto& tag : tags) {
                        categorizedTransactions[tag].push_back(t);
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing line: " << line << std::endl;
                }
            }
        }
        file.close();
        std::cout << "Transactions loaded from " << filename << " successfully." << std::endl;
    } else {
        std::cerr << "Warning: File " << filename << " not found or unable to open." << std::endl;
    }
}
    // Function to calculate total income
    double getTotalIncome() const {
        return getTotalAmount([](const Transaction& t) { return t.amount > 0.0; });
    }

    // Function to calculate total expenses
    double getTotalExpenses() const {
        return getTotalAmount([](const Transaction& t) { return t.amount < 0.0; });
    }

    // Function to calculate total amount based on a predicate
    double getTotalAmount(bool (*predicate)(const Transaction&)) const {
        double total = 0.0;
        for (const auto& transaction : transactions) {
            if (predicate(transaction)) {
                total += std::abs(transaction.amount); // Use absolute value for total
            }
        }
        return total;
    }

    // Function to add a tag to a transaction
    void addTagToTransaction(int index, const std::string& tag) {
        if (index >= 0 && index < transactions.size()) {
            transactions[index].addTag(tag);
            categorizedTransactions[tag].push_back(transactions[index]);
        } else {
            throw std::out_of_range("Invalid transaction index");
        }
    }

    // Function to format date and time as (MM/DD/YYYY - hh:mmAM/PM)
    std::string formatDateTime(time_t timestamp) const {
        std::stringstream ss;
        tm* tmPtr = std::localtime(&timestamp);
        ss << std::setfill('0') << std::setw(2) << tmPtr->tm_mon + 1 << "/"
           << std::setw(2) << tmPtr->tm_mday << "/" << tmPtr->tm_year + 1900 << " - "
           << std::setw(2) << (tmPtr->tm_hour % 12 == 0 ? 12 : tmPtr->tm_hour % 12) << ":"
           << std::setw(2) << tmPtr->tm_min << (tmPtr->tm_hour < 12 ? "AM" : "PM");
        return ss.str();
    }

    // Function to join elements of a vector into a string with delimiter
    std::string join(const std::vector<std::string>& vec, const std::string& delimiter) const {
        std::string result;
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) result += delimiter;
            result += vec[i];
        }
        return result;
    }

    // Function to split a string into a vector of strings based on delimiter
    std::vector<std::string> split(const std::string& s, char delimiter) const {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
};

int main() {
    FinanceTracker tracker;

    int choice;
    double amount;
    std::string description, tagsStr, filename;

    do {

          std::cout << "                        _  _    _      _   __        \n"
                 "                       | || |  | |    (_) / _|       \n"
                 " __      __ ___   __ _ | || |_ | |__   _ | |_  _   _ \n"
                 " \\ \\ /\\ / // _ \\ / _` || || __|| '_ \\ | ||  _|| | | |\n"
                 "  \\ V  V /|  __/| (_| || || |_ | | | || || |  | |_| |\n"
                 "   \\_/\\_/  \\___| \\__,_||_| \\__||_| |_|\\_||_|   \\__, |\n"
                 "                                                __/ |\n"
                 "                                               |___/ \n";
        std::cout << "\n===== Finance Tracker Menu =====" << std::endl;
        std::cout << "1. Add Transaction" << std::endl;
        std::cout << "2. Display Transactions" << std::endl;
        std::cout << "3. Save Transactions to File" << std::endl;
        std::cout << "4. Load Transactions from File" << std::endl;
        std::cout << "5. Display Income and Expenses" << std::endl;
        std::cout << "6. Add Tag to Transaction" << std::endl;
        std::cout << "7. Exit" << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "Enter description: ";
                std::cin.ignore();  // Clear input buffer
                std::getline(std::cin, description);

                std::cout << "Enter amount (+ for income, - for expense): ";
                std::cin >> amount;

                std::cout << "Enter tags (separated by ';', optional): ";
                std::cin.ignore();  // Clear input buffer
                std::getline(std::cin, tagsStr);

                tracker.addTransaction(description, amount, tracker.split(tagsStr, ';'));
                std::cout << "Transaction added successfully." << std::endl;
                break;

            case 2:
                tracker.displayTransactions();
                break;

            case 3:
                std::cout << "Enter filename to save: ";
                std::cin.ignore();  // Clear input buffer
                std::getline(std::cin, filename);
                filename += ".csv";
                tracker.saveTransactionsToFile(filename);
                break;

            case 4:
                std::cout << "Enter filename to load: ";
                std::cin.ignore();  // Clear input buffer
                std::getline(std::cin, filename);

                tracker.loadTransactionsFromFile(filename);
                break;

            case 5:
                std::cout << "Total Income: $" << tracker.getTotalIncome() << std::endl;
                std::cout << "Total Expenses: $" << tracker.getTotalExpenses() << std::endl;
                std::cout << "Current Balance: $" << tracker.getTotalIncome() - tracker.getTotalExpenses() << std::endl;
                break;

            case 6:
                int index;
                std::cout << "Enter index of transaction to add tag: ";
                std::cin >> index;
                std::cout << "Enter tag to add: ";
                std::cin.ignore();  // Clear input buffer
                std::getline(std::cin, tagsStr);

                try {
                    tracker.addTagToTransaction(index, tagsStr);
                    std::cout << "Tag added successfully to transaction." << std::endl;
                } catch (const std::out_of_range& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                }
                break;

            case 7:
                std::cout << "Exiting program." << std::endl;
                break;

            default:
                std::cout << "Invalid choice. Please enter a valid option." << std::endl;
                break;
        }

    } while (choice != 7);

    return 0;
}
