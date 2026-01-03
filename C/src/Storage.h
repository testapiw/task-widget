#pragma once
#include <vector>
#include "Task.h"

/**
 * @class Storage
 * @brief A utility class for handling persistent storage of task data.
 * * This class provides static methods to serialize and deserialize the task list,
 * ensuring that user data is preserved between application sessions. It handles
 * file path resolution and data integrity.
 */
class Storage {
public:

    /**
     * @brief Loads the list of tasks from the local storage file.
     * @return A vector of Task objects. Returns an empty vector if the file 
     * does not exist or is corrupted.
     */
    static std::vector<Task> loadTasks();

    /**
     * @brief Saves the current list of tasks to the local storage file.
     * @param tasks The vector of tasks to be serialized and saved.
     */
    static void saveTasks(const std::vector<Task>& tasks);
private:

    /**
     * @brief Determines the platform-specific absolute path to the data file.
     * @return A QString containing the full file path.
     */
    static QString getFilePath();
};