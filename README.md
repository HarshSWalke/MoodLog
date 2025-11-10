# 🧠 Smart Daily Habit Tracker

## 📋 Overview

**Smart Daily Habit Tracker** is a C++ console-based application designed to help users build and analyze their daily habits using structured data tracking.
It combines **tree structures**, **graph relationships**, and **JSON export** to visualize habit progress, motivation, and inter-habit influences.

This project is ideal for anyone who wants to:

* Track good and bad habits daily
* Monitor motivation and consistency
* View analytics on habit formation
* Save progress and export data for dashboard visualization

---

## ⚙️ Features

### 🧍 User Management

* Each session starts with a user entering their name.
* Personalized habit tracking per user.

### 🌱 Habit Management

* Add **Good** or **Bad** habits interactively.
* Delete unwanted habits.
* View all current habits with streaks and daily status.

### ✅ Daily Tracking

* Mark habits as **Complete** or **Missed**.
* Record **motivation level (1–10)** for each entry.
* Automatically updates progress trees and influence graphs.
* Logs all activity with timestamps in `log.txt`.

### 🌳 Habit Progress Tree (21-day tracking)

Each habit has its own **binary tree** (`HabitTree`) structure that records:

* Daily success/failure.
* Motivation level.
* Calculates:

  * Success/failure counts.
  * Average motivation.
  * Habit formation progress (% of 21-day goal).

Data can be exported to JSON (`habit_tree.json`).

### 🔗 Habit Influence Graph

A **graph model** represents how habits influence each other:

* Positive or negative weights between related habits.
* Influence updates dynamically based on performance and motivation.
* Graph exported as JSON (`habit_graph.json`) for visualization dashboards.

### 📈 Reports and Analytics

* Generates a detailed **Habit Analysis Report**:

  * Success & failure statistics
  * Average motivation level
  * Formation progress (%)
  * Displays influence network summary
  * Identifies most influential habit

### 🧾 Logging System

* Every completed habit is logged with timestamp in `log.txt`.
* View historical logs from the main menu.

### 💾 Persistent Data

* All habits are saved to `habits.txt`:

  ```
  <streak>   <type>   <habit name>
  ```
* Data automatically reloaded at startup.

### 📤 Dashboard Data Export

Automatically generates:

* `dashboard_data/<habit_name>_tree.json`
* `dashboard_data/habit_graph.json`

This enables visualization using Python, D3.js, or other external tools.

---

## 🏗️ Architecture Overview

### Core Classes

| Class               | Responsibility                                                          |
| ------------------- | ----------------------------------------------------------------------- |
| **HabitNode**       | Represents a single day record in the habit’s binary tree               |
| **HabitTree**       | Manages 21-day progress, stores daily performance & motivation          |
| **HabitGraph**      | Tracks influence weights between habits                                 |
| **Habit**           | Represents an individual habit with type, streak, and progress tracking |
| **HabitTracker**    | Manages user’s habit collection and all interactive menu actions        |
| **User**            | Base class providing username and display methods                       |
| **ReportGenerator** | Inherits from `User` and `ReportBase` to produce detailed reports       |

---

## 📂 File Structure

```
SmartHabitTracker/
│
├── main.cpp                  # Main source code (your file)
├── habits.txt                # Saved habits data
├── log.txt                   # Activity log file
├── dashboard_data/           # Auto-generated folder for JSON exports
│   ├── <habit>_tree.json
│   └── habit_graph.json
├── nlohmann/json.hpp         # JSON library (required header)
└── README.md                 # Project documentation (this file)
```

---

## 🧰 Dependencies

### Required:

* **C++17** or newer
* **nlohmann/json** library (included in `json.hpp`)

### Optional (for visualization):

* Python / D3.js / Power BI for dashboard imports (uses JSON files)

---

## 🚀 How to Build and Run

### 🖥️ Compilation

#### On Linux / macOS:

```bash
g++ -std=c++17 main.cpp -o habit_tracker
./habit_tracker
```

#### On Windows:

```bash
g++ -std=c++17 main.cpp -o habit_tracker.exe
habit_tracker.exe
```

---

## 🕹️ Menu Navigation

| Option                            | Description                               |
| :-------------------------------- | :---------------------------------------- |
| **1. Add Habit**                  | Add a new good or bad habit               |
| **2. Mark Habit Complete/Missed** | Update daily status and motivation        |
| **3. Show All Habits**            | Display habit list and current progress   |
| **4. Delete Habit**               | Remove unwanted habit                     |
| **5. Generate Report**            | Analyze all habits and display statistics |
| **6. View Logs**                  | Show activity history                     |
| **7. Save & Exit**                | Save all data and exit safely             |

---

## 🧮 Example Output

```
==============================================
         SMART DAILY HABIT TRACKER
==============================================
User: John
--------------------------------------------
1. Add Habit
2. Mark Habit Complete / Missed
3. Show All Habits
4. Delete Habit
5. Generate Report
6. View Logs
7. Save & Exit
Enter your choice: 5

==============================================
             HABIT ANALYSIS REPORT
==============================================
User: John
----------------------------------------------
Habit: Exercise
Type: Good
Streak: 7 days
Successes: 5 | Failures: 2
Average Motivation: 8.4
Formation Progress: 23.8%
----------------------------------------------
--- Habit Influence Network ---
Exercise → (Read Books, +3)
Read Books → (Meditate, +1)
----------------------------------------------
Most Influential Habit: Exercise
==============================================
```

---

## 📊 Data Export Example

### `dashboard_data/Exercise_tree.json`

```json
{
  "day": 1,
  "success": true,
  "motivation": 8,
  "left": {
    "day": 2,
    "success": true,
    "motivation": 7
  },
  "right": null
}
```

### `dashboard_data/habit_graph.json`

```json
{
  "nodes": ["Exercise", "Read Books"],
  "edges": [
    {"source": "Exercise", "target": "Read Books", "weight": 3}
  ]
}
```

---

## 🧩 Future Enhancements

* [ ] Add JSON import for reloading graph/tree data
* [ ] Visualize graphs directly in terminal using ASCII art
* [ ] Add habit reminders and notifications
* [ ] Support multi-user login
* [ ] Add dashboard web UI with charts

---

## 👨‍💻 Author

**Developed by:** [Your Name]
**Role:** Software Developer & Data Visualization Enthusiast
**Language:** C++17
**License:** MIT

---

## 💬 Summary

This project demonstrates **data structure design**, **OOP principles**, and **data-driven habit analysis**.
By using **trees** for habit progress and **graphs** for inter-habit influence, it combines behavioral tracking with analytical insights — making it not just a habit tracker, but a **smart self-improvement tool**.
