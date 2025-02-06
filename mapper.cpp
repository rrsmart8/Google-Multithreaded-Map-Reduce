#include "sharedData.h"

void *mapper_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;

    map<string, set<int>> local_data;  // Local data for this Mapper

    // Process each assigned file
    for (int file_index : args->assigned_files) {
        ifstream file_content(args->program_data->files[file_index]);
        if (!file_content.is_open()) {
            cerr << "[Error] Cannot open file: " << args->program_data->files[file_index] << endl;
            continue;
        }

        string word;
        // Read words from the file, normalize them, and store them in local_data
        while (file_content >> word) {
            word = normalize(word);
            if (!word.empty()) {
                local_data[word].insert(file_index);
            }
        }
        file_content.close();
    }

    // Lock to update the global reducer_data safely
    pthread_mutex_lock(&args->program_data->mapper_mutex);
    for (const auto &entry : local_data) {
        args->program_data->reducer_data[entry.first].insert(entry.second.begin(), entry.second.end());
    }
    pthread_mutex_unlock(&args->program_data->mapper_mutex);

    // Wait for all mappers to finish processing before proceeding
    pthread_barrier_wait(&args->program_data->barrier);  
    pthread_exit(nullptr);
}