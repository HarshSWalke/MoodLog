#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <limits>


using namespace std;

// --------------------- FORWARD / DECLARATIONS ------------------------
void logActivity(const string& username, const string& habitName);
void viewLogs();
void saveHabitsToFile(const vector<class Habit>&, const string&);

// --------------------- HABIT CLASS ----------------------------------
class Habit {
private:
    string name;
    int streak;
    bool completedToday;
    static int totalHabits;

public:
    Habit(const string& n = "Unnamed", int s = 0)
        : name(n), streak(s), completedToday(false) {
        ++totalHabits;
    }

    ~Habit() {
        --totalHabits;
    }

    // Mark the habit complete for today (only once per day)
    void markComplete() {
        if (completedToday) {
            cout << "Habit \"" << name << "\" already marked complete for today. (Streak: " << streak << ")\n";
            return;
        }
        completedToday = true;
        ++streak;
        cout << "Great job! You completed: " << name << " (Streak: " << streak << ")\n";
        // Logging moved to the tracker where username is known
    }

    void resetDay() {
        completedToday = false;
    }

    void display() const {
        cout << left << setw(25) << name
             << " | Streak: " << setw(3) << streak
             << " | Today: " << (completedToday ? "YES" : "NO") << '\n';
    }

    string getName() const { return name; }
    int getStreak() const { return streak; }
    bool isDone() const { return completedToday; }

    static int getTotalHabits() { return totalHabits; }

    // allow saving helper to access private members (keeps API clean)
    friend void saveHabitsToFile(const vector<Habit>& habits, const string& filename);
};

int Habit::totalHabits = 0;

ostream& operator<<(ostream& os, const Habit& h) {
    os << h.getName() << " (" << h.getStreak() << " days)";
    return os;
}

// --------------------- USER CLASS -----------------------------------
class User {
protected:
    string username;

public:
    User(const string& u = "Guest") : username(u) {}
    virtual void displayInfo() {
        cout << "User: " << username << '\n';
    }
};

// --------------------- HABIT TRACKER --------------------------------
class HabitTracker : public User {
private:
    vector<Habit> habits;

public:
    HabitTracker(const string& name = "Guest") : User(name) {}

    // Add habit by name
    void addHabit(const string& name) {
        habits.emplace_back(name, 0);
        cout << "Habit added: " << name << '\n';
    }

    // Interactive add habit (uses getline)
    void addHabitInteractive() {
        cout << "Enter habit name: ";
        string name;
        getline(cin, name);
        if (name.empty()) {
            cout << "Habit name cannot be empty.\n";
            return;
        }
        addHabit(name);
    }

    void deleteHabit() {
        if (habits.empty()) {
            cout << "No habits to delete.\n";
            return;
        }
        cout << "\nSelect habit number to delete:\n";
        for (size_t i = 0; i < habits.size(); ++i)
            cout << i + 1 << ". " << habits[i].getName() << '\n';

        int choice = 0;
        if (!(cin >> choice)) {
            cout << "Invalid input.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice < 1 || choice > static_cast<int>(habits.size())) {
            cout << "Invalid choice.\n";
            return;
        }

        cout << "Habit \"" << habits[choice - 1].getName() << "\" deleted.\n";
        habits.erase(habits.begin() + (choice - 1));
    }

    void markHabit() {
        if (habits.empty()) {
            cout << "No habits added yet.\n";
            return;
        }
        cout << "Select habit number to mark complete:\n";
        for (size_t i = 0; i < habits.size(); ++i)
            cout << i + 1 << ". " << habits[i].getName() << '\n';

        int choice = 0;
        if (!(cin >> choice)) {
            cout << "Invalid input.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice < 1 || choice > static_cast<int>(habits.size())) {
            cout << "Invalid habit number!\n";
            return;
        }

        habits[choice - 1].markComplete();
        // record it with real user
        logActivity(username, habits[choice - 1].getName());
    }

    void showAll() const {
        cout << "\n--- Your Habits ---\n";
        if (habits.empty()) {
            cout << "(none)\n";
        } else {
            for (const auto& h : habits)
                h.display();
        }
        cout << "-------------------\n";
    }

    const vector<Habit>& getHabits() const { return habits; }

    // Save & Load
    void saveData(const string& filename = "habits.txt") {
        saveHabitsToFile(habits, filename);
    }

    void loadData(const string& filename = "habits.txt") {
        ifstream fin(filename);
        if (!fin) {
            // no data yet — not an error
            return;
        }

        habits.clear();
        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            // format: <streak>\t<name>
            stringstream ss(line);
            int streak = 0;
            if (!(ss >> streak)) continue;
            // skip tab/space separator
            if (ss.peek() == '\t' || ss.peek() == ' ') ss.get();
            string name;
            getline(ss, name);
            if (name.empty()) name = "Unnamed";
            habits.emplace_back(name, streak);
            // loaded habits should NOT be marked completedToday nor logged
        }
        fin.close();
    }

    // Polymorphism
    void displayInfo() override {
        cout << "Habit Tracker for user: " << username << '\n';
    }
};

// --------------------- REPORT GENERATOR -----------------------------
class ReportBase {
public:
    virtual void generateReport() = 0;
};

class ReportGenerator : public ReportBase, virtual public User {
private:
    const vector<Habit>& habits;

public:
    ReportGenerator(const string& uname, const vector<Habit>& h)
        : User(uname), habits(h) {}

    void generateReport() override {
        cout << "\n===== HABIT REPORT =====\n";
        int completed = 0;
        for (const auto& h : habits)
            if (h.isDone()) ++completed;

        cout << "User: " << username << '\n';
        cout << "Total Habits: " << habits.size() << '\n';
        cout << "Completed Today: " << completed << '\n';
        int percent = 0;
        if (!habits.empty()) percent = static_cast<int>((completed * 100) / static_cast<int>(habits.size()));
        cout << "Overall Progress: " << percent << "%\n";
        cout << "========================\n";
    }
};

// --------------------- SAVE / LOG FUNCTIONS -------------------------
void saveHabitsToFile(const vector<Habit>& habits, const string& filename) {
    ofstream fout(filename);
    if (!fout) {
        cerr << "Cannot open file to save habits: " << filename << '\n';
        return;
    }
    // Format each line: <streak>\t<name>\n
    for (const auto& h : habits) {
        fout << h.getStreak() << '\t' << h.getName() << '\n';
    }
    fout.close();
    cout << "Data saved to " << filename << '\n';
}

void logActivity(const string& username, const string& habitName) {
    ofstream fout("log.txt", ios::app);
    if (!fout) {
        cerr << "Warning: cannot open log file.\n";
        return;
    }

    time_t now = time(nullptr);
    string t = ctime(&now);
    if (!t.empty() && t.back() == '\n') t.pop_back();

    fout << "[" << t << "] " << username << " completed habit: " << habitName << '\n';
    fout.close();
}

void viewLogs() {
    ifstream fin("log.txt");
    if (!fin) {
        cout << "No logs found.\n";
        return;
    }
    cout << "\n==== PAST ACTIVITY LOG ====\n";
    string line;
    while (getline(fin, line))
        cout << line << '\n';
    cout << "===========================\n";
    fin.close();
}

// --------------------- MAIN -----------------------------------------
int main() {
    try {
        cout << "===== SMART DAILY HABIT TRACKER =====\n";
        cout << "Enter your name: ";
        string name;
        getline(cin, name);
        if (name.empty()) name = "Guest";

        HabitTracker tracker(name);
        tracker.loadData();
        tracker.displayInfo();

        int choice = 0;
        do {
            cout << "\n1. Add Habit\n"
                 << "2. Mark Habit Complete\n"
                 << "3. Show All Habits\n"
                 << "4. Delete Habit\n"
                 << "5. Generate Report\n"
                 << "6. Save & Exit\n"
                 << "7. View Logs\n"
                 << "Enter choice: ";

            if (!(cin >> choice)) {
                cout << "Invalid input. Try again.\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (choice) {
                case 1:
                    tracker.addHabitInteractive();
                    break;
                case 2:
                    tracker.markHabit();
                    break;
                case 3:
                    tracker.showAll();
                    break;
                case 4:
                    tracker.deleteHabit();
                    break;
                case 5: {
                    ReportGenerator report(name, tracker.getHabits());
                    report.generateReport();
                    break;
                }
                case 6:
                    tracker.saveData();
                    cout << "Goodbye!\n";
                    break;
                case 7:
                    viewLogs();
                    break;
                default:
                    cout << "Invalid option.\n";
            }

        } while (choice != 6);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
    return 0;
}
