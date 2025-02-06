#include <pthread.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

// Main structure containing all shared data
struct ProgramData {
    vector<string> files;             // List of input files
    map<string, set<int>> reducer_data; // Global data for the Reducer
    pthread_mutex_t reducer_mutex;     // Mutex for protecting reducer data
    pthread_mutex_t mapper_mutex;      // Mutex for protecting mapper data
    pthread_barrier_t barrier;         // Barrier to synchronize threads
};

// Structure for thread arguments
struct ThreadArgs {
    int id;                          // Thread ID
    vector<int> assigned_files;      // Files assigned to this thread
    ProgramData* program_data;       // Pointer to the shared ProgramData
};

// Structure for Reducer thread arguments
struct ReducerArgs {
    int id;                          // Reducer ID
    int num_reducers;                // Total number of reducers
    ProgramData* program_data;       // Pointer to the shared ProgramData
};
