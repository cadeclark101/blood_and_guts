// blood_and_guts.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <ctime>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <sqlite3.h> 
#include <typeinfo>
#include <tuple>
#include <variant>
#include <string>
#include <cstdlib>
#include <iterator>

using namespace std;


void Clear()
{
#if defined _WIN32
    system("cls");
    //clrscr(); // including header file : conio.h
#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
    system("clear");
    //std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences 
#elif defined (__APPLE__)
    system("clear");
#endif

    cout << "  __    __                __                   __                __          \n" ;
    cout << " |  |--|  .-----.-----.--|  |  .---.-.-----.--|  |  .-----.--.--|  |_.-----. \n" ;
    cout << " |  _  |  |  _  |  _  |  _  |  |  _  |     |  _  |  |  _  |  |  |   _|__ --| \n" ;
    cout << " |_____|__|_____|_____|_____|  |___._|__|__|_____|  |___  |_____|____|_____| \n" ;
    cout << "                                                    |_____| \n" ;
    cout << "        TRACKING, PROGRESSION AND EVALUATION OF YOUR WORKOUTS                \n\n " ;
    cout.flush();
}

void MainMenu() {
    cout << "1. Add new workout.\n" << "2. View all workouts.\n" << "3. View workout statistics and progression.\n" << "4. Edit previous workouts.\n\n";
    int choice;
    cin >> choice;
    switch (choice) {
    case 1:
        Clear();
        cout << "Blah" << endl;
        break;
    }
}

struct Workout {
public:
    int id;
    string date;
    string workout_type;
    float difficulty;
    string comments;
};


struct Exercise {
public:
    int id;
    string name;
    float effectiveness;
    int workout_id;
};

struct Set {
public:
    int id;
    int weight;
    int reps;
    int RPE;
    int exercise_id;

};

template <typename T>
void PopulateVector(T& object_type, int& rc, sqlite3* db, vector<variant<int, float, string>>& constructor_vector) {
    sqlite3_stmt* stmt;
    float float_holder;
    string string_holder;
    string blob_holder;

    string object_type_str = typeid(object_type).name();
    constructor_vector.push_back(object_type_str.substr(7));
    string sql = "SELECT * from '" + object_type_str.substr(7) + "';";

    cout << sql << endl;

    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    int ncols = sqlite3_column_count(stmt);

    if (rc != SQLITE_OK) {
        cerr << "SELECT failed: " << sqlite3_errmsg(db) << endl;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        for (int i = 0; i < ncols; i++) {

            int col_type = sqlite3_column_type(stmt, i);


            switch (col_type) {
            case 1:
                constructor_vector.push_back(sqlite3_column_int(stmt, i));
                break;
            case 2:
                float_holder = sqlite3_column_double(stmt, i);
                constructor_vector.push_back(float_holder);
                break;
            case 3:
                string_holder = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
                constructor_vector.push_back(string_holder);
                break;
            case 4:
                cerr << "SQL_NULL" << endl;
                break;
            case 5:
                cerr << "SQL_BLOB" << endl;
                break;
            default:
                cerr << "COULD NOT IDENTIFY COLUMN TYPE" << endl;
                break;
            }
        }
    }
    sqlite3_finalize(stmt);

}

void printVectorVariantValues(vector<variant<int, float, string>> arg) {
    try {
        for (auto val : arg) {
            if (const auto intPtr(get_if<int>(&val)); intPtr)
                cout << *intPtr << " ";
            else if (const auto floatPtr(get_if<float>(&val)); floatPtr)
                cout << *floatPtr << " ";
            else if (const auto stringPtr(get_if<string>(&val)); stringPtr)
                cout << *stringPtr << " ";
        }
    }
    catch (bad_variant_access err) {
        cout << "something" << " ";
    }
}

void createStructs(vector<variant<int, float, string>>& constructor_vector, vector<Workout>& workouts, vector<Set>& sets, vector<Exercise>& exercises) {
    const auto first_val_ptr(get_if<string>(&constructor_vector.front()));

    if (*first_val_ptr == "Workout") {
        Workout a;

        auto idPtr(get_if<int>(&constructor_vector[1]));
        auto datePtr(get_if<string>(&constructor_vector[2]));
        auto workoutPtr(get_if<string>(&constructor_vector[3]));
        auto difficultyPtr(get_if<float>(&constructor_vector[4]));
        auto commentsPtr(get_if<string>(&constructor_vector[5]));

        a.id = *idPtr;
        a.date = *datePtr;
        a.workout_type = *workoutPtr;
        a.difficulty = *difficultyPtr;
        a.comments = *commentsPtr;

    }

    else if (*first_val_ptr == "Exercises") {
        Exercise a;

        auto idPtr(get_if<int>(&constructor_vector[1]));
        auto namePtr(get_if<string>(&constructor_vector[2]));
        auto effectivenessPtr(get_if<float>(&constructor_vector[3]));
        auto workout_id_Ptr(get_if<int>(&constructor_vector[4]));

        a.id = *idPtr;
        a.name = *namePtr;
        a.effectiveness = *effectivenessPtr;
        a.workout_id = *workout_id_Ptr;
    }

    else if (*first_val_ptr == "Set") {
        Set a;

        auto idPtr(get_if<int>(&constructor_vector[1]));
        auto weightPtr(get_if<int>(&constructor_vector[2]));
        auto repsPtr(get_if<int>(&constructor_vector[3]));
        auto rpePtr(get_if<int>(&constructor_vector[4]));
        auto exercise_id_Ptr(get_if<int>(&constructor_vector[5]));

        a.id = *idPtr;
        a.weight = *weightPtr;
        a.reps = *repsPtr;
        a.RPE = *rpePtr;
        a.exercise_id = *exercise_id_Ptr;
    }
}



int main()
{   
    sqlite3* db;
    string sql;

    vector<Workout> workouts;
    vector<Set> sets;
    vector<Exercise> exercises;
    vector<variant<int, float, string> > constructor_vector;

    Workout workout_type;
    Exercise exercise_type;
    Set set_type;

    Clear();

    int rc = sqlite3_open("blood_and_guts.db", &db);

    PopulateVector(workout_type, rc, db, constructor_vector);
 
    sqlite3_close(db);

    printVectorVariantValues(constructor_vector);
    createStructs(constructor_vector, workouts, sets, exercises);

    return 0;
};



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
