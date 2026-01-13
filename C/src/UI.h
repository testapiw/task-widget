#pragma once

#include <QMainWindow>
#include <QTreeWidget>
#include <QSystemTrayIcon>
#include <QUuid>
#include <QMouseEvent>
#include <QTimer>
#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include "Task.h"
#include "AlarmManager.h"
#include "PluginManager.h"

/**
 * @class MainWindow
 * @brief The primary controller and UI container for the Task Alarm application.
 * * This class orchestrates the interaction between the data (Tasks), the alarm 
 * monitoring logic, and the external plugins. It also handles custom window 
 * movement (frameless UI support) and system tray integration.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // Custom window dragging logic (for frameless windows)
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // System and input event handling
    void moveEvent(QMoveEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    /** @brief Triggered when the AlarmManager identifies a task that should fire. */
    void handleAlarmTriggered(const Task &task);
    
    /** @brief Updates the window style (e.g., blinking borders) based on alarm state. */
    void updateVisualState(bool isAlarmActive);
    
    /** @brief Opens the TaskDialog for creating or modifying tasks. */
    void showTaskDialog(QUuid parentId, int level, bool isEdit = false, QUuid editId = QUuid());
    
    /** @brief Opens the configuration dialog for plugins and global settings. */
    void showSettingsDialog();

private:
/** @brief Initializes the UI layout and core components. */
    void setupUi();
    
    /** @brief Rebuilds the QTreeWidget based on the current m_tasks vector. */
    void refreshList();
    
    /** @brief Recursively adds tasks to the tree, respecting hierarchy. */
    void addTaskToTree(const Task &task, QTreeWidgetItem *parentItem = nullptr);
    
    /** @brief Creates the custom widget (buttons, labels) for a tree row. */
    QWidget* createTaskWidget(const Task &task); 
    
    /** @brief Resets UI animations and stops the current alarm feedback. */
    void stopAlarmAnimation();
    
    /** @brief Displays a system-level notification via the tray icon. */
    void sendNotification(const QString &title, const QString &message);

    void toggleServer();
    void checkServerActualStatus();


    // Core Logic Modules
    std::vector<Task> m_tasks;      // In-memory collection of all tasks
    AlarmManager *m_alarmManager;   // Handles time-based triggers
    PluginManager *m_pluginManager; // Handles external notifications (sound, etc.)

    // UI Components
    QTreeWidget *m_treeWidget;      // Hierarchical display of tasks
    QSystemTrayIcon *m_trayIcon;    // Background presence and notifications
    
    // Window dragging state variables
    bool m_dragging = false;
    QPoint m_dragPosition;

    QWidget *m_serverControlWidget;
    QPushButton *m_btnServer;
    QLabel *m_serverStatusLed;
    bool m_isServerRunning = false;
};