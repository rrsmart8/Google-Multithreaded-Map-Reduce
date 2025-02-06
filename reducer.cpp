#include "sharedData.h"

void *reducer_thread(void *arg) {
    ReducerArgs *args = (ReducerArgs *)arg;
    int reducer_id = args->id;
    int num_reducers = args->num_reducers;

    // Synchronize: wait for all mappers to finish
    pthread_barrier_wait(&args->program_data->barrier);

    // Calculate the range of letters each reducer will process
    int letters_per_reducer = 26 / num_reducers;
    int remainder_letters = 26 % num_reducers;

    char start_letter = 'a' + reducer_id * letters_per_reducer + min(reducer_id, remainder_letters);
    char end_letter = start_letter + letters_per_reducer - 1 + (reducer_id < remainder_letters ? 1 : 0);
    if (end_letter > 'z') end_letter = 'z';

    vector<pair<string, set<int>>> local_results;

    // Lock to read from global reducer_data safely
    pthread_mutex_lock(&args->program_data->reducer_mutex);
    for (const auto &entry : args->program_data->reducer_data) {
        if (entry.first[0] >= start_letter && entry.first[0] <= end_letter) {
            local_results.push_back(entry);
        }
    }
    pthread_mutex_unlock(&args->program_data->reducer_mutex);

    // Sort the results: primarily by frequency (desc), then by word (asc)
    sort(local_results.begin(), local_results.end(),
         [](const pair<string, set<int>> &a, const pair<string, set<int>> &b) {
             if (a.second.size() != b.second.size()) {
                 return a.second.size() > b.second.size(); // Sort by frequency (descending)
             }
             return a.first < b.first; // Sort alphabetically (ascending)
         });

    // Write the results to separate files based on the letters processed by this reducer
    for (char letter = start_letter; letter <= end_letter; ++letter) {
        string file_name = string(1, letter) + ".txt";
        ofstream file(file_name, ofstream::out);

        for (const auto &entry : local_results) {
            if (entry.first[0] == letter) {
                file << entry.first << ":[";
                for (auto it = entry.second.begin(); it != entry.second.end(); ++it) {
                    if (it != entry.second.begin()) file << " ";
                    file << *it;
                }
                file << "]\n";
            }
        }
        file.close();
    }

    pthread_exit(nullptr);
}