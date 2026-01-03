#pragma once
#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QUuid>
#include <vector>
#include "Task.h"

/**
 * @class AlarmManager
 * @brief The core engine for monitoring and triggering scheduled tasks.
 * * This class manages the lifecycle of an alarm. It monitors a list of tasks,
 * determines when an alarm should fire based on time and day of the week,
 * and handles the visual feedback state (blinking).
 */
class AlarmManager : public QObject {
    Q_OBJECT
public:
    explicit AlarmManager(QObject *parent = nullptr);

    /**
     * @brief Initializes the background monitoring timer.
     */
    void startMonitoring();
    
    /**
     * @brief Evaluates all tasks to see if any meet the criteria to trigger.
     * @param tasks Reference to the task list (allows updating 'lastAlarmTriggered').
     */
    void checkTasks(std::vector<Task>& tasks);

    /**
     * @brief Stops the current alarm and resets all notification flags.
     */
    void stopAlarm();

    // Getters for internal state
    bool isRinging() const { return m_isRinging; }
    QUuid activeTaskId() const { return m_activeTaskId; }

signals:
    /** @brief Emitted when a task meets its schedule requirements. */
    void alarmTriggered(const Task &task);

    /** @brief Emitted periodically to create a flashing UI effect. */
    void blinkTick(bool active);

    /** @brief Emitted when task data changes or needs to be saved. */
    void tasksChanged();

private:
    /** @brief Starts the timer for visual blinking feedback. */
    void startBlink(); 
    
    /** @brief Encapsulates the logic for checking if a specific task should fire. */
    bool shouldTrigger(const Task &task, const QDateTime &now);

    QTimer *m_checkTimer; // Periodic timer for task evaluation
    QTimer *m_blinkTimer; // Timer for visual alert animation
    bool m_isRinging = false;
    bool m_blinkState = false;
    QUuid m_activeTaskId; // Stores the ID of the currently ringing task
};