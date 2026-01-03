#pragma once
#include <QString>
#include <QUuid>
#include <QDateTime>
#include <QTime>
#include <QDate>
#include <vector>

/**
 * @enum TaskStatus
 * @brief Represents the current lifecycle state of a task.
 */
enum class TaskStatus { Todo, InProgress, Done, Paused };

/**
 * @struct TaskSchedule
 * @brief Defines when and how often a task should trigger an alarm.
 */
struct TaskSchedule {
    bool active = false;          // Whether the alarm is currently enabled
    QTime startTime;              // Scheduled trigger time
    QTime endTime;                // Scheduled end time (if applicable)
    QDate exactDate;              // Specific date for one-time tasks
    std::vector<int> daysOfWeek;  // Recurring days: 1-7 (Mon-Sun)
    
    // Prevents multiple triggers during the same minute interval
    QDateTime lastAlarmTriggered; 
};

/**
 * @struct Task
 * @brief The primary data model representing a unit of work with scheduling capabilities.
 */
struct Task {
    QUuid id;                     // Unique identifier for the task
    QString title;                // Display name of the task
    int level;                    // Hierarchy level (for nested subtasks)
    QUuid parent;                 // ID of the parent task (Null if top-level)
    QString projectPath;          // Categorization or folder-like path
    TaskStatus status = TaskStatus::Todo;
    QDateTime lastActivated;      // Timestamp of the last time the task was started
    TaskSchedule schedule;        // Alarm and recurrence settings
    bool isExpanded = false;      // UI state: whether subtasks are visible

    /**
     * @brief Converts TaskStatus enum to a string for serialization (JSON/XML).
     */
    static QString statusToString(TaskStatus s) {
        switch(s) {
            case TaskStatus::InProgress: return "InProgress";
            case TaskStatus::Done:       return "Done";
            case TaskStatus::Paused:     return "Paused";
            default:                     return "Todo";
        }
    }

    /**
     * @brief Converts a string back into a TaskStatus enum.
     */
    static TaskStatus stringToStatus(const QString& s) {
        if (s == "InProgress") return TaskStatus::InProgress;
        if (s == "Done")       return TaskStatus::Done;
        if (s == "Paused")     return TaskStatus::Paused;
        return TaskStatus::Todo;
    }
};