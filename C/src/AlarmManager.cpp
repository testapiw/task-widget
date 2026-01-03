#include "AlarmManager.h"
#include <QDateTime>
#include <algorithm>

AlarmManager::AlarmManager(QObject *parent) : QObject(parent) {
    m_blinkTimer = new QTimer(this);
    connect(m_blinkTimer, &QTimer::timeout, this, [this]() {
        m_blinkState = !m_blinkState;
        emit blinkTick(m_blinkState);
    });
}

/**
 * @brief Determines if an alarm should be triggered for a specific task at the current time.
 * * This function evaluates three conditions:
 * 1. Is the task's schedule active?
 * 2. Does the current day of the week match the task's scheduled days?
 * 3. Does the current time (hour and minute) match the task's start time?
 * * It also includes a safety check to prevent the alarm from triggering multiple times 
 * within the same minute.
 * * @param task The task object containing schedule and state information.
 * @param now The current date and time to check against.
 * @return true if all conditions are met and the alarm should fire; false otherwise.
 */
bool AlarmManager::shouldTrigger(const Task &task, const QDateTime &now) {
    if (!task.schedule.active) return false;

    // 1. Check the day of the week (std::find returns end() if the element is not found)
    const auto &days = task.schedule.daysOfWeek;
    if (std::find(days.begin(), days.end(), now.date().dayOfWeek()) == days.end()) {
        return false;
    }

    // 2. Check time (hours and minutes)
    if (task.schedule.startTime.hour() != now.time().hour() || 
        task.schedule.startTime.minute() != now.time().minute()) {
        return false;
    }

    // 3. Prevent re-triggering within the same minute
    if (task.schedule.lastAlarmTriggered.date() == now.date() &&
        task.schedule.lastAlarmTriggered.time().minute() == now.time().minute()) {
        return false;
    }

    return true;
}

/**
 * @brief Iterates through a list of tasks to see if any alarm should fire.
 * * This method checks each task against the current system time. If a match is found,
 * it updates the task's state, triggers the alarm signals, and stops searching 
 * (to handle one alarm at a time).
 * * @param tasks A reference to the vector of tasks to be checked.
 */
void AlarmManager::checkTasks(std::vector<Task>& tasks) {
    QDateTime now = QDateTime::currentDateTime();

    for (auto &task : tasks) {
        if (shouldTrigger(task, now)) {
            m_isRinging = true;
            m_activeTaskId = task.id;
            
            // Record the trigger time to prevent immediate re-triggering
            task.schedule.lastAlarmTriggered = now; 

            emit alarmTriggered(task);
            emit tasksChanged(); // Signal UI to save changes to persistent storage
            startBlink();
            break; 
        }
    }
}

/**
 * @brief Initializes and starts the background timer that monitors alarm tasks.
 * * This method sets up a periodic check (every 30 seconds) to ensure that tasks
 * are evaluated against the current time. It uses a singleton-style check to
 * ensure the timer is only created once.
 */
void AlarmManager::startMonitoring() {
    if (!m_checkTimer) {
        m_checkTimer = new QTimer(this);
        
        // Request the system to check tasks every 30 seconds
        // Usually, the task list is passed from MainWindow via signal or direct call
        connect(m_checkTimer, &QTimer::timeout, this, [this]() {
            // This can be left empty if checkTasks is called externally (e.g., from UI.cpp)
            // Or emit a signal indicating it's time to check the tasks:
            emit tasksChanged(); 
        });
        
        m_checkTimer->start(30000); 
    }
}

/**
 * @brief Activates the visual blinking feedback.
 * * This method starts the timer responsible for toggling visual states (like colors 
 * or visibility) to alert the user that an alarm is active. It ensures the timer 
 * is not restarted if it is already running.
 */
void AlarmManager::startBlink() {
    if (!m_blinkTimer->isActive()) {
        // Start the timer with a 500ms interval (2 beats per second)
        m_blinkTimer->start(500);
    }
}

/**
 * @brief Deactivates the currently ringing alarm and resets the manager's state.
 * * This method stops all alert mechanisms, including the visual blinking and 
 * the internal ringing state. It also clears the reference to the active task.
 */
void AlarmManager::stopAlarm() {
    m_isRinging = false;

    // Clear the active task ID (using a null UUID to indicate no task is active)
    m_activeTaskId = QUuid();

    // Stop the visual feedback timer
    m_blinkTimer->stop();

    // Reset the internal blink state and notify the UI to return to normal
    m_blinkState = false;
    emit blinkTick(false);
}

