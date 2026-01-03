#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QTimeEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QUuid>
#include "Task.h"

/**
 * @class TaskDialog
 * @brief A modal dialog for creating or editing task details and schedules.
 * * This class provides a user interface to modify task properties such as title,
 * project path, and alarm settings (time and recurring days). It encapsulates
 * the validation and data-mapping logic between the UI widgets and the Task model.
 */
class TaskDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructs a TaskDialog.
     * @param task The task data to populate the fields with.
     * @param isEdit True if modifying an existing task, false if creating a new one.
     * @param parent The parent widget for memory management and centering.
     */
    explicit TaskDialog(const Task &task, bool isEdit, QWidget *parent = nullptr);
    
    /**
     * @brief Retrieves the modified task data after the user accepts the dialog.
     * @return A Task object containing the values entered in the UI.
     */
    Task getTask() const;

private slots:
    /** @brief Opens a file/folder browser to set the project path. */
    void onBrowsePath();

    /** @brief Validates input and accepts the dialog. */
    void onSave();

private:
    void setupUi(bool isEdit);
    void loadTaskData(const Task &task);

    // Input Fields
    QLineEdit *m_editTitle;      // Field for the task name
    QLineEdit *m_editPath;       // Field for the associated project/folder path
    QCheckBox *m_checkActive;    // Toggle for the alarm schedule
    QTimeEdit *m_timeStart;      // Time picker for the alarm
    QList<QPushButton*> m_dayButtons; // List of toggle buttons for Mon-Sun selection
    
    // Internal State
    Task m_currentTask;          // Buffer for the task being edited
    int m_level;                 // Hierarchy depth level
};