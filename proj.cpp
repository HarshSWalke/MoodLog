#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <limits>
#include <map>
#include <algorithm>
#include <climits>
#include "nlohmann/json.hpp"
#include <cstdlib>
using json = nlohmann::json;



using namespace std;

// Cross-platform clear screen function
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// -------------------- TREE & GRAPH SUPPORT STRUCTURES --------------------
struct HabitNode {
    int day;
    bool success;
    int motivation;
    HabitNode* left;
    HabitNode* right;

    HabitNode(int d, bool s, int m)
        : day(d), success(s), motivation(m), left(nullptr), right(nullptr) {}
};

// For tracking per-habit 21-day progress
class HabitTree {
private:
    HabitNode* root;
    int currentDay;

public:
    HabitTree() : root(nullptr), currentDay(0) {}
// in HabitTree (public)
        json nodeToJson(HabitNode* node) const {
            if (!node) return nullptr;
            json j;
            j["day"] = node->day;
            j["success"] = node->success;
            j["motivation"] = node->motivation;
            if (node->left)  j["left"]  = nodeToJson(node->left);
            else j["left"] = nullptr;
            if (node->right) j["right"] = nodeToJson(node->right);
            else j["right"] = nullptr;
            return j;
        }

        // export whole tree to file (public)
        void exportToJsonFile(const std::string& filename) const {
            json out = nodeToJson(root);
            std::ofstream fout(filename);
            if (fout) fout << out.dump(4);
        }

    void insert(bool success, int motivation) {
        ++currentDay;
        if (!root) {
            root = new HabitNode(currentDay, success, motivation);
            return;
        }
        HabitNode* curr = root;
        while (true) {
            if (success) {
                if (!curr->left) {
                    curr->left = new HabitNode(currentDay, success, motivation);
                    break;
                }
                curr = curr->left;
            } else {
                if (!curr->right) {
                    curr->right = new HabitNode(currentDay, success, motivation);
                    break;
                }
                curr = curr->right;
            }
        }
    }

    void inorder(HabitNode* node, int& successCount, int& failCount, int& totalMotivation, int& nodeCount) const {
        if (!node) return;
        inorder(node->left, successCount, failCount, totalMotivation, nodeCount);
        if (node->success) successCount++;
        else failCount++;
        totalMotivation += node->motivation;
        nodeCount++;
        inorder(node->right, successCount, failCount, totalMotivation, nodeCount);
    }

    void getStats(double& formation, double& avgMotivation, int& successCount, int& failCount) const {
        successCount = failCount = 0;
        int totalMotivation = 0, nodeCount = 0;
        inorder(root, successCount, failCount, totalMotivation, nodeCount);
        // formation as percentage of 21-day habit formation target (if desired)
        formation = (successCount / 21.0) * 100.0;
        avgMotivation = (nodeCount > 0) ? (totalMotivation / static_cast<double>(nodeCount)) : 0.0;
    }
};

// -------------------- HABIT INFLUENCE GRAPH --------------------
class HabitGraph {
private:
    map<string, vector<pair<string, int>>> adj;  // habit -> [(relatedHabit, weight)]

public:
    void addHabit(const string& name) {
        if (adj.find(name) == adj.end()) adj[name] = {};
    }

    void addInfluence(const string& from, const string& to, int weight = 2) {
        adj[from].push_back({to, weight});
    }

    void updateInfluence(const string& habit, bool success, bool goodHabit, int motivation) {
        int delta = (motivation > 7) ? 2 : (motivation >= 4 ? 1 : 0);
        if (!goodHabit && success) delta *= -1;  // bad habit relapsed (adjust logic as intended)

        // ensure habit exists in adjacency map
        if (adj.find(habit) == adj.end()) adj[habit] = {};

        for (auto& edge : adj[habit]) {
            int& w = edge.second;
            w += (success ? delta : -delta);
            w = max(-5, min(5, w)); // clamp weights between -5 and +5
        }
    }
// public:
        void exportToJsonFile(const std::string& filename) const {
            json j;
            j["edges"] = json::array();
            j["nodes"] = json::array();

            // nodes: collect unique nodes
            for (const auto& kv : adj) {
                j["nodes"].push_back(kv.first);
            }
            // edges
            for (const auto& kv : adj) {
                const string& src = kv.first;
                for (const auto& p : kv.second) {
                    j["edges"].push_back({
                        {"source", src},
                        {"target", p.first},
                        {"weight", p.second}
                    });
                }
            }
            std::ofstream fout(filename);
            if (fout) fout << j.dump(4);
        }

    void showInfluences() const {
        cout << "\n--- Habit Influence Network ---\n";
        for (const auto& kv : adj) {
            const string& src = kv.first;
            const auto& edges = kv.second;
            cout << src << " → ";
            for (const auto& p : edges) {
                const string& dst = p.first;
                int w = p.second;
                cout << "(" << dst << ", " << (w >= 0 ? "+" : "") << w << ") ";
            }
            cout << "\n";
        }

        cout << "--------------------------------\n";
    }

    string getMostInfluential() const {
        string best = "None";
        int maxWeight = INT_MIN;
        for (const auto& kv : adj) {
            const string& src = kv.first;
            const auto& edges = kv.second;
            int total = 0;
            for (const auto& e : edges) total += e.second;
            if (total > maxWeight) {
                maxWeight = total;
                best = src;
            }
        }
        return best;
    }
};

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
    bool isGood; // new: distinguishes Good vs Bad habit
    static int totalHabits;
    HabitTree progressTree;  // new: track 21-day formation
public:
    Habit(const string& n = "Unnamed", int s = 0, bool good = true)
        : name(n), streak(s), completedToday(false), isGood(good) {
        ++totalHabits;
    }

    ~Habit() { --totalHabits; }

    void markComplete(int motivation) {
        if (completedToday) {
            cout << "Habit \"" << name << "\" already marked complete for today.\n";
            return;
        }
        completedToday = true;
        ++streak;
        progressTree.insert(true, motivation);
        cout << "✅ Great job! You completed: " << name << " (Streak: " << streak << ")\n";
    }
    // in Habit public methods:
        void exportProgressJson(const std::string& folder = ".") const {
            // sanitize file name if you want; simple version:
            std::string fname = folder + "/" + getName() + "_tree.json";
            // replace spaces with underscores
            std::replace(fname.begin(), fname.end(), ' ', '_');
            // call HabitTree exporter — need a const-export method in HabitTree (we added it)
            const_cast<HabitTree&>(progressTree).exportToJsonFile(fname); // const_cast because method not const? If method is const above, no need.
        }


    void markMissed(int motivation) {
        if (completedToday) {
            cout << "Habit \"" << name << "\" already marked complete for today.\n";
            return;
        }
        completedToday = true;
        progressTree.insert(false, motivation);
        cout << "⚠️ You missed: " << name << " today.\n";
    }

    void resetDay() { completedToday = false; }

    void display() const {
        cout << left << setw(25) << name
             << " | Streak: " << setw(3) << streak
             << " | Type: " << (isGood ? "Good" : "Bad")
             << " | Today: " << (completedToday ? "YES" : "NO") << '\n';
    }

    string getName() const { return name; }
    int getStreak() const { return streak; }
    bool isDone() const { return completedToday; }
    bool isGoodHabit() const { return isGood; }

    void getProgress(double& formation, double& avgMotivation, int& success, int& fail) const {
        progressTree.getStats(formation, avgMotivation, success, fail);
    }

    static int getTotalHabits() { return totalHabits; }

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

            cout << "Is this a Good habit or a Bad habit? (G/B): ";
            char type;
            cin >> type;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            bool isGood = (type == 'G' || type == 'g');

            habits.emplace_back(name, 0, isGood);
            cout << (isGood ? "✅ Good habit added: " : "⚠️ Bad habit added: ") << name << '\n';
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

    void markHabit(HabitGraph& graph) {
        if (habits.empty()) {
            cout << "No habits added yet.\n";
            return;
        }
        cout << "Select habit number:\n";
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

        Habit& h = habits[choice - 1];
        cout << "Mark status for \"" << h.getName() << "\":\n1. Done\n2. Missed\nChoice: ";
        int status;
        if (!(cin >> status)) {
            cout << "Invalid input.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Enter your motivation level (1–10): ";
        int motivation;
        if (!(cin >> motivation)) {
            cout << "Invalid motivation input. Using default 5.\n";
            motivation = 5;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (status == 1) h.markComplete(motivation);
        else h.markMissed(motivation);

        graph.updateInfluence(h.getName(), (status == 1), h.isGoodHabit(), motivation);
                logActivity(username, h.getName());
                #ifdef _WIN32
            std::system("if not exist dashboard_data mkdir dashboard_data");
        #else
            std::system("mkdir -p dashboard_data");
        #endif

         h.exportProgressJson("dashboard_data");
         graph.exportToJsonFile("dashboard_data/habit_graph.json");
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

   void loadData() {
    ifstream fin("habits.txt");
    if (!fin.is_open()) return;

    string line;
    while (getline(fin, line)) {
        stringstream ss(line);

        int streak = 0;
        char typeChar;

        // Read streak and type (G or B)
        if (!(ss >> streak >> typeChar)) continue;

        // Skip tab or space before the name
        if (ss.peek() == '\t' || ss.peek() == ' ') ss.get();

        // Read full habit name (can include spaces)
        string name;
        getline(ss, name);

        // Determine if it's a good or bad habit
        bool isGood = (typeChar == 'G' || typeChar == 'g');

        // Add to the list
        habits.emplace_back(name, streak, isGood);
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
    virtual void generateReport(HabitGraph& graph) = 0;
};

class ReportGenerator : public ReportBase, public User {
private:
    const vector<Habit>& habits;

public:
    ReportGenerator(const string& uname, const vector<Habit>& h)
        : User(uname), habits(h) {}

    void generateReport(HabitGraph& graph) override {
    cout << "\n==============================================\n";
    cout << "             HABIT ANALYSIS REPORT\n";
    cout << "==============================================\n";
    cout << "User: " << username << "\n";
    cout << "----------------------------------------------\n";

    if (habits.empty()) {
        cout << "No habits to analyze yet.\n";
        return;
    }

    for (const auto& h : habits) {
        double formation = 0.0, avgMotivation = 0.0;
        int success = 0, fail = 0;
        h.getProgress(formation, avgMotivation, success, fail);

        cout << "Habit: " << h.getName() << "\n";
        cout << "Type: " << (h.isGoodHabit() ? "Good" : "Bad") << "\n";
        cout << "Streak: " << h.getStreak() << " days\n";
        cout << "Successes: " << success << " | Failures: " << fail << "\n";
        cout << "Average Motivation: " << fixed << setprecision(1) << avgMotivation << "\n";
        cout << "Formation Progress: " << formation << "%\n";
        cout << "----------------------------------------------\n";
    }

    graph.showInfluences();
    cout << "Most Influential Habit: " << graph.getMostInfluential() << "\n";
    cout << "==============================================\n";
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
    fout << h.getStreak() << '\t'
         << (h.isGoodHabit() ? "G" : "B") << '\t'
         << h.getName() << '\n';
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
void showHeader(const string& username) {
    cout << "============================================\n";
    cout << "         SMART DAILY HABIT TRACKER\n";
    cout << "============================================\n";
    cout << "User: " << username << "\n";
    cout << "--------------------------------------------\n";
}

// --------------------- MAIN -----------------------------------------
int main() {
    try {
        clearScreen();
        cout << "===== SMART DAILY HABIT TRACKER =====\n";
        cout << "Enter your name: ";
        string name;
        getline(cin, name);
        if (name.empty()) name = "Guest";

        HabitTracker tracker(name);
        HabitGraph graph;

        tracker.loadData();

        int choice = 0;
        do {
            clearScreen();
            showHeader(name);
            cout << "1. Add Habit\n"
                 << "2. Mark Habit Complete / Missed\n"
                 << "3. Show All Habits\n"
                 << "4. Delete Habit\n"
                 << "5. Generate Report\n"
                 << "6. View Logs\n"
                 << "7. Save & Exit\n"
                 << "Enter your choice: ";

            if (!(cin >> choice)) {
                cout << "Invalid input. Try again.\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (choice) {
                case 1: {
                    clearScreen();
                    showHeader(name);
                    tracker.addHabitInteractive();
                    cout << "\nPress Enter to return to Main Menu...";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                case 2: {
                    clearScreen();
                    showHeader(name);
                    tracker.markHabit(graph);
                    cout << "\nPress Enter to return to Main Menu...";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                case 3: {
                    clearScreen();
                    showHeader(name);
                    tracker.showAll();
                    cout << "\nPress Enter to return to Main Menu...";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                case 4: {
                    clearScreen();
                    showHeader(name);
                    tracker.deleteHabit();
                    cout << "\nPress Enter to return to Main Menu...";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                case 5: {
                    clearScreen();
                    showHeader(name);
                    ReportGenerator report(name, tracker.getHabits());
                    report.generateReport(graph);
                    cout << "\nPress Enter to return to Main Menu...";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                case 6: {
                    clearScreen();
                    showHeader(name);
                    viewLogs();
                    cout << "\nPress Enter to return to Main Menu...";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    break;
                }
                case 7: {
                    clearScreen();
                    showHeader(name);
                    tracker.saveData();
                    cout << "\nAll data saved successfully.\nGoodbye, " << name << "!\n";
                    break;
                }
                default:
                    cout << "\nInvalid option. Try again.\n";
                    break;
            }

        } while (choice != 7);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}


