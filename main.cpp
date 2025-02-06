#include "sharedData.h"

// Function to normalize words (remove non-alphabetical characters and convert to lowercase)
string normalize(string word) {
    string result;
    for (char c : word) {
        if (isalpha(c)) {
            result += tolower(c);  // Only add alphabetic characters in lowercase
        }
    }
    return result;
}

int main(int argc, char **argv) {
    
    // Check the command-line arguments
    if (argc != 4) {
        cerr << "[Error] Usage: <num_mappers> <num_reducers> <input_file>" << endl;
        return -1;
    }

    int num_mappers = stoi(argv[1]);  // Number of mapper threads
    int num_reducers = stoi(argv[2]); // Number of reducer threads
    string input_file = argv[3];      // Input file name

    // Open the input file containing the list of files
    ifstream file(input_file);
    if (!file.is_open()) {
        cerr << "[Error] Cannot open the input file." << endl;
        return -1;
    }

    int num_files;
    file >> num_files;  // Number of files to process
    vector<string> files(num_files + 1);

    // Read file names from the input file
    for (int i = 1; i <= num_files; i++) {
        file >> files[i];
    }
    file.close();

    // Initialize shared data structure
    ProgramData program_data;
    program_data.files = files;

    // Initialize barrier for synchronization and mutexes for protecting shared data
    pthread_barrier_init(&program_data.barrier, nullptr, num_mappers + num_reducers);
    pthread_mutex_init(&program_data.reducer_mutex, nullptr);
    pthread_mutex_init(&program_data.mapper_mutex, nullptr);

    // Distribute files across mappers
    vector<vector<int>> file_assignments(num_mappers);
    int files_per_mapper = num_files / num_mappers;
    int extra_files = num_files % num_mappers;
    int current_file = 1;

    // Assign files to mappers
    for (int i = 0; i < num_mappers; i++) {
        int files_to_process = files_per_mapper + (i < extra_files ? 1 : 0);
        for (int j = 0; j < files_to_process; j++) {
            file_assignments[i].push_back(current_file++);
        }
    }

    // Create and start mapper threads
    pthread_t mapper_threads[num_mappers];
    ThreadArgs mapper_args[num_mappers];

    for (int i = 0; i < num_mappers; i++) {
        mapper_args[i].id = i;
        mapper_args[i].assigned_files = file_assignments[i];
        mapper_args[i].program_data = &program_data;
        pthread_create(&mapper_threads[i], nullptr, mapper_thread, &mapper_args[i]);
    }

    // Create and start reducer threads
    pthread_t reducer_threads[num_reducers];
    ReducerArgs reducer_args[num_reducers];
    for (int i = 0; i < num_reducers; i++) {
        reducer_args[i].id = i;
        reducer_args[i].num_reducers = num_reducers;
        reducer_args[i].program_data = &program_data;
        pthread_create(&reducer_threads[i], nullptr, reducer_thread, &reducer_args[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_mappers; i++) {
        pthread_join(mapper_threads[i], nullptr);
    }

    for (int i = 0; i < num_reducers; i++) {
        pthread_join(reducer_threads[i], nullptr);
    }

    // Clean up resources
    pthread_barrier_destroy(&program_data.barrier);
    pthread_mutex_destroy(&program_data.reducer_mutex);
    pthread_mutex_destroy(&program_data.mapper_mutex);

    return 0;
}
