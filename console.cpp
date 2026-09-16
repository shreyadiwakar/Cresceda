#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib> // For system()
#include <limits>
#include <algorithm>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
inline void makeFolder(const char* path) {
    _mkdir(path);
}
#else
#include <sys/stat.h>
inline void makeFolder(const char* path) {
    mkdir(path, 0777);
}
#endif

using namespace std;

// --- ANSI Color Definitions ---
const string CYAN = "\033[96m";
const string GREEN = "\033[92m";
const string RED = "\033[91m";
const string YELLOW = "\033[93m";
const string WHITE = "\033[0m"; // Reset code

// Platform and console helper functions
void enableVirtualTerminal() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

void clearScreen() {
    cout << "\033[2J\033[H" << flush;
}

void openURL(const string& url) {
    if (url.empty()) return;
#ifdef _WIN32
    string cmd = "cmd.exe /c start \"\" \"" + url + "\"";
    system(cmd.c_str());
#elif __APPLE__
    string cmd = "open \"" + url + "\"";
    system(cmd.c_str());
#else
    string cmd = "xdg-open \"" + url + "\"";
    system(cmd.c_str());
#endif
}

void pauseConsole() {
    cout << "\n" << YELLOW << "Press Enter to continue..." << WHITE << flush;
    string dummy;
    getline(cin, dummy);
}

void ensureDataDirectories() {
    makeFolder("data");
    makeFolder("data/students");
    makeFolder("data/courses");
    makeFolder("data/submissions");
}

string getSafeFilename(const string& rollNo, const string& suffix) {
    string safeRoll = rollNo;
    for (char &ch : safeRoll) {
        if (ch == '/' || ch == '\\' || ch == ':' || ch == '*' || ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch == '|') {
            ch = '_';
        }
    }
    return safeRoll + suffix;
}

string getSafePath(const string& dir, const string& rollNo, const string& suffix) {
    ensureDataDirectories();
    string safeRoll = rollNo;
    for (char &ch : safeRoll) {
        if (ch == '/' || ch == '\\' || ch == ':' || ch == '*' || ch == '?' || ch == '"' || ch == '<' || ch == '>' || ch == '|') {
            ch = '_';
        }
    }
    return dir + "/" + safeRoll + suffix;
}

string readLine(const string& prompt) {
    if (!prompt.empty()) {
        cout << YELLOW << prompt << WHITE << flush;
    }
    string val;
    if (getline(cin, val)) {
        if (!val.empty() && val.back() == '\r') {
            val.pop_back();
        }
        size_t first = val.find_first_not_of(" \t\r\n");
        if (first != string::npos) {
            size_t last = val.find_last_not_of(" \t\r\n");
            return val.substr(first, (last - first + 1));
        }
        return "";
    }
    if (cin.eof()) {
        exit(0);
    }
    return "";
}

int readInt(const string& prompt) {
    while (true) {
        string str = readLine(prompt);
        if (str.empty()) {
            if (cin.eof()) exit(0);
            cout << RED << "Input cannot be empty. Please enter a valid number.\n" << WHITE << flush;
            continue;
        }
        try {
            return stoi(str);
        } catch (...) {
            cout << RED << "Invalid input. Please enter a valid number.\n" << WHITE << flush;
        }
    }
}

// Forward declarations
class Course;
class Test;
class Question;
class Student;

// --- Global Course List ---
vector<Course> g_all_courses; 

//=================================================================
// CLASS DEFINITIONS
//=================================================================

// --- 1. Question Class ---
class Question {
    string text;
    vector<string> options;
    int correctOption;

public:
    Question(const string& t, const vector<string>& opts, int correct)
        : text(t), options(opts), correctOption(correct) {}
    
    void display() const {
        cout << text << "\n";
        for (size_t i = 0; i < options.size(); i++) {
            cout << i + 1 << ". " << options[i] << "\n";
        }
    }

    int getCorrectOption() const {
        return correctOption;
    }

    string getText() const {
        return text;
    }

    const vector<string>& getOptions() const {
        return options;
    }
};

// --- 2. Test Class ---
class Test {
    string title;
    vector<Question> questions;
public:
    Test(const string& title) : title(title) {}
    
    void addQuestion(const Question& q) {
        questions.push_back(q);
    }
    
    string getTitle() const {
        return title;
    }
    
    void displayQuestions() const {
        cout << YELLOW << "Test: " << title << "\n" << WHITE;
        for (size_t i = 0; i < questions.size(); i++) {
            cout << "\nQuestion " << i + 1 << ": \n";
            questions[i].display();
        }
    }

    vector<Question>& getQuestions() { return questions; }
    const vector<Question>& getQuestions() const { return questions; }
};

// --- 3. Course Class ---
class Course {
    string code;
    string name;
    int semester;
    string instructor_name;
    string videoLink;
    vector<string> assignments;
    vector<Test> tests;

public:
    Course(const string& c, const string& n, int sem, const string& instructor, const string& link)
        : code(c), name(n), semester(sem), instructor_name(instructor), videoLink(link) {}

    void display() const {
        cout << WHITE << "Course Code: " << YELLOW << code << "\n";
        cout << WHITE << "Course Name: " << YELLOW << name << "\n" << WHITE;
    }
    
    void displayAssignments() const {
        if (assignments.empty()) {
            cout << RED << "No assignments available for this course.\n" << WHITE;
            return;
        }
        
        cout << CYAN << "\n--- Assignments ---\n" << WHITE;
        for (size_t i = 0; i < assignments.size(); i++) {
            cout << (i + 1) << ". " << assignments[i] << endl << endl;
        }
    }
    
    void displayTests() const {
        if (tests.empty()) {
            cout << RED << "No tests available for " << name << ".\n" << WHITE;
            return;
        }

        cout << CYAN << "\n--- Available Tests for " << name << " ---\n" << WHITE;
        for (size_t i = 0; i < tests.size(); i++) {
            const Test& t = tests[i];
            cout << i + 1 << ". " << t.getTitle() << "\n";
        }
    }
    
    void addAssignment(const string& a) {
        assignments.push_back(a);
    }

    void addTest(const Test& t) {
        tests.push_back(t);
    }
    
    string getCode() const {
        return code;
    }
    
    string getName() const {
        return name;
    }

    int getSemester() const {
        return semester;
    }

    string getInstructor() const {
        return instructor_name;
    }
    
    string getVideoLink() const {
        return videoLink;
    }
    
    vector<Test>& getTests() {
        return tests;
    }

    const vector<Test>& getTests() const {
        return tests;
    }
    
    vector<string> getAssignments() const {
        return assignments;
    }
};

int selectCourseIndex(const vector<Course>& courses, const string& input) {
    if (input.empty()) return -1;

    bool isNumeric = true;
    for (char c : input) {
        if (!isdigit(c)) {
            isNumeric = false;
            break;
        }
    }

    if (isNumeric) {
        try {
            int idx = stoi(input);
            if (idx >= 1 && idx <= (int)courses.size()) {
                return idx - 1;
            }
        } catch (...) {}
    }

    string upperInput = input;
    transform(upperInput.begin(), upperInput.end(), upperInput.begin(), ::toupper);

    for (size_t i = 0; i < courses.size(); i++) {
        string upperCode = courses[i].getCode();
        transform(upperCode.begin(), upperCode.end(), upperCode.begin(), ::toupper);
        if (upperCode == upperInput) {
            return (int)i;
        }
    }

    return -1;
}

// --- 4. Student Class ---
class Student {
    string name;
    string rollNo;
    string username;
    string password;
    int semester;
    vector<Course> registered;
    vector<Course> non_registered;
    
public:
    Student(string n, string r, string u, string p, int sem)
        : name(n), rollNo(r), username(u), password(p), semester(sem) {}

    static void createAccountInFile(string n, string r, string u, string p, int sem) {
        ensureDataDirectories();
        string filename = "data/students/students.txt";
        ofstream file(filename, ios::app);
        if (!file) {
            cerr << RED << "Error opening file " << filename << "!" << WHITE << endl;
            return;
        }

        file << n << ":" << r << ":" << u << ":" << p << ":" << sem << ":\n";
        file.close();
        cout << GREEN << "Student account saved in " << filename << WHITE << endl;
        cout << GREEN << "\nAccount created successfully! Now you will have to login with your username and password.\n" << WHITE << endl;
    }

    void displayRegisteredCourses() const {
        cout << CYAN << "\n--- Registered Courses ---\n" << WHITE;
        if (registered.empty()) {
            cout << "You haven't registered for any courses yet.\n";
        } else {
            for (size_t i = 0; i < registered.size(); i++) {
                cout << i + 1 << ". ";
                registered[i].display();
            }
        }
    }
    
    void displayNonregisteredCourses() const {
        cout << CYAN << "\n--- Available (Non-registered) Courses ---\n" << WHITE;
        if (non_registered.empty()) {
            cout << "No more available courses to register.\n";
        } else {
            for (size_t i = 0; i < non_registered.size(); i++) {
                cout << i + 1 << ". ";
                non_registered[i].display();
            }
        }
    }
    
    const vector<Course>& getRegisteredCourses() const {
        return registered;
    }

    const vector<Course>& getNonregisteredCourses() const {
        return non_registered;
    }
    
    void initializeCourseLists(const vector<Course>& all_courses) {
        non_registered = all_courses;
        registered.clear();

        ensureDataDirectories();
        string filename = getSafePath("data/courses", rollNo, "_courses.txt");
        ifstream file(filename);
        if (!file) {
            filename = getSafeFilename(rollNo, "_courses.txt");
            file.open(filename);
        }
        if (!file) {
            return;
        }

        string courseCode;
        while (getline(file, courseCode)) {
            if (!courseCode.empty() && courseCode.back() == '\r') {
                courseCode.pop_back();
            }
            for (auto it = non_registered.begin(); it != non_registered.end(); ++it) {
                if (it->getCode() == courseCode) {
                    registered.push_back(*it);
                    non_registered.erase(it);
                    break;
                }
            }
        }
        file.close();
    }
    
    void registerCourse(const Course& c1) {
        registered.push_back(c1);
        for (auto it = non_registered.begin(); it != non_registered.end(); ++it) {
            if (it->getCode() == c1.getCode()) {
                non_registered.erase(it);
                break;
            }
        }

        ensureDataDirectories();
        string filename = getSafePath("data/courses", rollNo, "_courses.txt");
        ofstream file(filename, ios::app);
        if (!file) {
            cerr << RED << "Error saving course registration!" << WHITE << endl;
            return;
        }
        file << c1.getCode() << "\n";
        file.close();

        cout << GREEN << "Course registered successfully in " << filename << WHITE << endl;
    }

    Course* getCourseByCode(const string& code) {
        for (auto& c : registered) {
            if (c.getCode() == code)
                return &c;
        }
        return nullptr;
    }

    string getRollNo() const {
        return rollNo;
    }
};

//=================================================================
// FUNCTION DEFINITIONS
//=================================================================

Student* login() {
    clearScreen();
    cout << CYAN << "====== Student Login ======\n\n" << WHITE << flush;
    string username = readLine("Enter Username: ");
    string password = readLine("Enter Password: ");

    ensureDataDirectories();
    string filename = "data/students/students.txt";
    ifstream file(filename);
    if (!file) {
        filename = "students.txt";
        file.open(filename);
    }
    if (!file) {
        cout << RED << "\nNo accounts found! Please create an account first.\n" << WHITE << flush;
        pauseConsole();
        return nullptr;
    }
    
    string line;
    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        size_t pos1 = line.find(":");
        if (pos1 == string::npos) continue;
        size_t pos2 = line.find(":", pos1 + 1);
        if (pos2 == string::npos) continue;
        size_t pos3 = line.find(":", pos2 + 1);
        if (pos3 == string::npos) continue;
        size_t pos4 = line.find(":", pos3 + 1);
        if (pos4 == string::npos) continue;
        size_t pos5 = line.find(":", pos4 + 1);
        if (pos5 == string::npos) continue;
        
        string name = line.substr(0, pos1);
        string roll = line.substr(pos1 + 1, pos2 - pos1 - 1);
        string fileUsername = line.substr(pos2 + 1, pos3 - pos2 - 1);
        string filePassword = line.substr(pos3 + 1, pos4 - pos3 - 1);
        int semester = 1;
        try {
            semester = stoi(line.substr(pos4 + 1, pos5 - pos4 - 1));
        } catch (...) {}
        
        if (fileUsername == username && filePassword == password) {
            cout << GREEN << "\nLogin successful! Welcome " << name << "\n" << WHITE << flush;
            pauseConsole();
            file.close();
            
            Student* s = new Student(name, roll, fileUsername, filePassword, semester);
            s->initializeCourseLists(g_all_courses);
            return s;
        }
    }
    
    cout << RED << "\nInvalid username or password.\n" << WHITE << flush;
    pauseConsole();
    file.close();
    return nullptr;
}

void createAccount() {
    clearScreen();
    cout << CYAN << "====== Create New Account ======\n\n" << WHITE << flush;
    string name = readLine("Enter Name: ");
    string roll = readLine("Enter Roll Number: ");
    string username = readLine("Enter Username: ");
    string password = readLine("Enter Password: ");
    int sem = readInt("Enter Semester: ");

    Student::createAccountInFile(name, roll, username, password, sem);
    pauseConsole();
}

void mainMenu();
void menu2(Student& s);
void menu3(Student& s, Course& c);

void exportJSON() {
    cout << "[\n";
    for (size_t i = 0; i < g_all_courses.size(); i++) {
        const auto& c = g_all_courses[i];
        cout << "  {\n";
        cout << "    \"code\": \"" << c.getCode() << "\",\n";
        cout << "    \"name\": \"" << c.getName() << "\",\n";
        cout << "    \"semester\": " << c.getSemester() << ",\n";
        cout << "    \"instructor\": \"" << c.getInstructor() << "\",\n";
        cout << "    \"videoLink\": \"" << c.getVideoLink() << "\",\n";
        cout << "    \"assignments\": [";
        auto assign = c.getAssignments();
        for (size_t a = 0; a < assign.size(); a++) {
            cout << "\"" << assign[a] << "\"" << (a + 1 < assign.size() ? ", " : "");
        }
        cout << "],\n";
        cout << "    \"tests\": [\n";
        auto tests = c.getTests();
        for (size_t t = 0; t < tests.size(); t++) {
            cout << "      {\n";
            cout << "        \"title\": \"" << tests[t].getTitle() << "\",\n";
            cout << "        \"questions\": [\n";
            auto qList = tests[t].getQuestions();
            for (size_t q = 0; q < qList.size(); q++) {
                cout << "          {\n";
                cout << "            \"text\": \"" << qList[q].getText() << "\",\n";
                cout << "            \"options\": [";
                auto opts = qList[q].getOptions();
                for (size_t o = 0; o < opts.size(); o++) {
                    cout << "\"" << opts[o] << "\"" << (o + 1 < opts.size() ? ", " : "");
                }
                cout << "],\n";
                cout << "            \"correctOption\": " << qList[q].getCorrectOption() << "\n";
                cout << "          }" << (q + 1 < qList.size() ? "," : "") << "\n";
            }
            cout << "        ]\n";
            cout << "      }" << (t + 1 < tests.size() ? "," : "") << "\n";
        }
        cout << "    ]\n";
        cout << "  }" << (i + 1 < g_all_courses.size() ? "," : "") << "\n";
    }
    cout << "]\n";
}

void initializeGlobalCourses() {
    Course course1("CS101", "Programming Fundamentals", 1, "Mr. Khan", "https://www.youtube.com/watch?v=8mAITcNt710");
    Course course2("CS102", "Data Structures", 2, "Ms. Sharma", "https://www.youtube.com/watch?v=bum_19loj9A");
    Course course3("CS201", "Algorithms", 3, "Dr. Verma", "https://www.youtube.com/watch?v=rL8X2mlNHPM");
    Course course4("CS301", "Operating Systems", 5, "Prof. Reddy", "https://www.youtube.com/watch?v=26QPDBe-NB8");

    course1.addAssignment("Hello World Program: Write a program to print 'Hello World'");
    course2.addAssignment("Linked List Implementation: Implement a singly linked list with basic operations");
    course3.addAssignment("QuickSort Analysis: Implement QuickSort and analyze time complexity");
    course4.addAssignment("CPU Scheduling: Simulate a simple CPU scheduling algorithm");

    Test test1("Basics Quiz");
    test1.addQuestion(Question(
        "What is the output of cout << 2+3;", 
        {"23", "5", "Error", "Nothing"},
        2
    ));
    test1.addQuestion(Question(
        "Which keyword is used for inheritance in C++?",
        {"private", "protected", "public", "virtual"},
        3
    ));
    course1.addTest(test1);

    Test test2("Linked List Test");
    test2.addQuestion(Question(
        "Which operation adds a node at the end?", 
        {"insertAtStart", "deleteNode", "append", "search"},
        3
    ));
    test2.addQuestion(Question(
        "Time complexity of searching in a linked list?",
        {"O(1)", "O(n)", "O(log n)", "O(n^2)"},
        2
    ));
    course2.addTest(test2);

    g_all_courses.push_back(course1);
    g_all_courses.push_back(course2);
    g_all_courses.push_back(course3);
    g_all_courses.push_back(course4);
}

int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    cout << unitbuf;

    enableVirtualTerminal();
    ensureDataDirectories();
    initializeGlobalCourses();

    if (argc > 1 && string(argv[1]) == "--export-json") {
        exportJSON();
        return 0;
    }

    mainMenu();
    return 0;
}

void mainMenu() {
    int choice = 0;
    do {
        clearScreen();
        cout << CYAN << "====== Student E-Learning Platform ======\n\n" << WHITE << flush;
        cout << "1. Create New Account\n";
        cout << "2. Login\n";
        cout << "3. Exit\n\n";
        
        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:
                createAccount();
                break;
            case 2: {
                Student* s = login();
                if (s != nullptr) {
                    menu2(*s);
                    delete s;
                }
                break;
            }
            case 3:
                cout << GREEN << "Exiting the program. Goodbye!\n" << WHITE << flush;
                break;
            default:
                cout << RED << "Invalid choice. Please try again.\n\n" << WHITE << flush;
                pauseConsole();
                break;
        }
    } while (choice != 3);
}

void menu2(Student& s) {
    int choice = 0;
    do {
        clearScreen();
        cout << CYAN << "\n===== Student Dashboard =====\n\n" << WHITE << flush;
        cout << "1. Available (Non-registered) Courses\n";
        cout << "2. Register a Course\n";
        cout << "3. View Registered Courses\n";
        cout << "4. Logout\n\n";

        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1:
                s.displayNonregisteredCourses();
                pauseConsole();
                break;
                
            case 2: {
                s.displayNonregisteredCourses();
                if (s.getNonregisteredCourses().empty()) {
                    pauseConsole();
                    break;
                }
                string input = readLine("Enter course number or code to register (e.g. 4 or CS301): ");
                int idx = selectCourseIndex(s.getNonregisteredCourses(), input);
                if (idx != -1) {
                    s.registerCourse(s.getNonregisteredCourses()[idx]);
                } else {
                    cout << RED << "Invalid course selection! Please enter a valid number or course code.\n" << WHITE << flush;
                    pauseConsole();
                }
                break;
            }
            
            case 3: {
                s.displayRegisteredCourses();
                if (s.getRegisteredCourses().empty()) {
                    pauseConsole();
                    break;
                }
                string input = readLine("Enter course number or code to open: ");
                int idx = selectCourseIndex(s.getRegisteredCourses(), input);
                if (idx != -1) {
                    string code = s.getRegisteredCourses()[idx].getCode();
                    Course* c = s.getCourseByCode(code);

                    if (c != nullptr) {
                        menu3(s, *c);
                    } else {
                        cout << RED << "Error: Could not find course by code.\n" << WHITE << flush;
                        pauseConsole();
                    }
                } else {
                    cout << RED << "Invalid course selection! Please enter a valid number or course code.\n" << WHITE << flush;
                    pauseConsole();
                }
                break;
            }
                
            case 4:
                cout << GREEN << "Logging out...\n" << WHITE << flush;
                pauseConsole();
                break;
                
            default:
                cout << RED << "Invalid choice! (Must be 1-4)\n" << WHITE << flush;
                pauseConsole();
                break;
        }
    } while (choice != 4);
}

void menu3(Student& s, Course& c) {
    int choice = 0;
    do {
        clearScreen();
        cout << CYAN << "\n===== Course: " << c.getName() << " ======\n\n" << WHITE << flush;
        cout << "1. Watch Video\n";
        cout << "2. View Assignments\n";
        cout << "3. Submit Assignment\n";
        cout << "4. Take Test\n";
        cout << "5. Back to Dashboard\n\n";

        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1: {
                if (c.getVideoLink().empty()) {
                    cout << RED << "No video uploaded for this course.\n" << WHITE << flush;
                } else {
                    cout << GREEN << "Opening video: " << c.getVideoLink() << "\n" << WHITE << flush;
                    openURL(c.getVideoLink());
                }
                pauseConsole();
                break;
            }
            
            case 2:
                c.displayAssignments();
                pauseConsole();
                break;
                
            case 3: {
                c.displayAssignments();
                if (c.getAssignments().empty()) {
                    pauseConsole();
                    break;
                }
                int idx = readInt("Enter assignment number to submit: ");
                if (idx < 1 || idx > (int)c.getAssignments().size()) {
                    cout << RED << "Invalid assignment number!\n" << WHITE << flush;
                    pauseConsole();
                    break;
                }

                string submissionText = readLine("Enter your submission text:\n");

                ensureDataDirectories();
                string filename = getSafePath("data/submissions", s.getRollNo(), "_assignments.txt");
                ofstream file(filename, ios::app);
                if (!file) {
                    cerr << RED << "Error opening file " << filename << "\n" << WHITE << flush;
                    pauseConsole();
                    break;
                }

                file << "Course: " << c.getName() << "\n";
                file << "Assignment: " << c.getAssignments()[idx - 1] << "\n\n";
                file << "Answer: " << submissionText << "\n";
                file << "----------------------------------\n";
                file.close();

                cout << GREEN << "Assignment submitted and saved in " << filename << "\n" << WHITE << flush;
                pauseConsole();
                break;
            }
            
            case 4: {
                c.displayTests();
                if (c.getTests().empty()) {
                    pauseConsole();
                    break;
                }
                
                int idx = readInt("Enter test number to take: ");
                if (idx < 1 || idx > (int)c.getTests().size()) {
                    cout << RED << "Invalid test number!\n" << WHITE << flush;
                    pauseConsole();
                    break;
                }

                Test& t = c.getTests()[idx - 1];
                clearScreen();
                cout << CYAN << "Starting test: " << t.getTitle() << "\n\n" << WHITE << flush;

                double score = 0;
                for (size_t i = 0; i < t.getQuestions().size(); i++) {
                    t.getQuestions()[i].display();
                    
                    int studentAnswer = readInt("Your answer (1-4): ");

                    if (studentAnswer == t.getQuestions()[i].getCorrectOption()) {
                        cout << GREEN << "Correct!\n" << WHITE << flush;
                        score++;
                    } else {
                        cout << RED << "Wrong! Correct answer is: " << t.getQuestions()[i].getCorrectOption() << "\n" << WHITE << flush;
                    }
                }

                cout << GREEN << "\nTest submitted!\n" << WHITE << flush;
                
                if (!t.getQuestions().empty()) {
                    cout << "Your score is: " << (score / t.getQuestions().size()) * 100 << "%\n";
                } else {
                    cout << "This test has no questions. Score: 0%\n";
                }
                
                pauseConsole();
                break;
            }
            
            case 5:
                cout << "Returning to student dashboard...\n" << flush;
                break;
                
            default:
                cout << RED << "Invalid choice!\n" << WHITE << flush;
                pauseConsole();
                break;
        }
    } while (choice != 5);
}