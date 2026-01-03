#include "TaskDialog.h"
#include "StyleHelper.h"
#include <QFileDialog>

/**
 * @brief Constructor for the TaskDialog.
 * * Initializes the dialog, applies the centralized styling, and populates
 * the UI components with existing task data.
 */
TaskDialog::TaskDialog(const Task &task, bool isEdit, QWidget *parent) 
    : QDialog(parent), m_currentTask(task) 
{
    m_level = task.level;
    setStyleSheet(StyleHelper::getDialogStyle());
    setupUi(isEdit);
    loadTaskData(task);
}

/**
 * @brief Sets up the user interface components for the dialog.
 * * This method dynamically builds the layout based on the task's hierarchy level.
 * Top-level tasks (level 1) receive full scheduling and path options, while
 * subtasks maintain a simplified interface.
 * * @param isEdit Boolean flag to toggle between "Create" and "Update" modes.
 */
void TaskDialog::setupUi(bool isEdit) {
    setWindowTitle(isEdit ? "Edit Task" : "New Task");
    setFixedWidth(360);
    auto *layout = new QVBoxLayout(this);

    // Basic task information
    layout->addWidget(new QLabel("Title:"));
    m_editTitle = new QLineEdit();
    layout->addWidget(m_editTitle);

    // Conditional UI elements for Root Tasks (Level 1)
    if (m_level == 1) {
        layout->addSpacing(10);
        layout->addWidget(new QLabel("Project Directory:"));
        auto *pathLayout = new QHBoxLayout();
        m_editPath = new QLineEdit();
        auto *btnBrowse = new QPushButton("📁");
        btnBrowse->setFixedWidth(40);
        connect(btnBrowse, &QPushButton::clicked, this, &TaskDialog::onBrowsePath);
        pathLayout->addWidget(m_editPath);
        pathLayout->addWidget(btnBrowse);
        layout->addLayout(pathLayout);

        layout->addSpacing(10);
        m_checkActive = new QCheckBox("Enable Alarm Notification");
        m_timeStart = new QTimeEdit(QTime(9, 0));
        
        // Setup recurring days selection (Monday to Sunday)
        auto *daysLayout = new QHBoxLayout();
        QStringList dayNames = {"M", "T", "W", "T", "F", "S", "S"};
        for (int i = 1; i <= 7; ++i) {
            auto *btn = new QPushButton(dayNames[i-1]);
            btn->setObjectName("dayBtn");
            btn->setCheckable(true);
            btn->setFixedSize(30, 30);
            m_dayButtons.append(btn);
            daysLayout->addWidget(btn);
        }

        // Bottom action button
        layout->addWidget(m_checkActive);
        layout->addWidget(new QLabel("Start Time:"));
        layout->addWidget(m_timeStart);
        layout->addLayout(daysLayout);
    }

    layout->addStretch();
    auto *btnSave = new QPushButton(isEdit ? "Update" : "Create");
    btnSave->setObjectName("btnSave");
    connect(btnSave, &QPushButton::clicked, this, &TaskDialog::onSave);
    layout->addWidget(btnSave);
}

/**
 * @brief Populates the dialog's widgets with data from a Task object.
 * * This method performs the "Data Mapping" from the model to the view. 
 * It ensures that when a user edits an existing task, all fields—including 
 * the recurring day buttons—reflect the current state of the task.
 * * @param task The source Task structure containing the data to display.
 */
void TaskDialog::loadTaskData(const Task &task) {
    m_editTitle->setText(task.title);

    // Only populate advanced fields if this is a top-level task (Level 1)
    if (m_level == 1) {
        m_editPath->setText(task.projectPath);
        m_checkActive->setChecked(task.schedule.active);
        m_timeStart->setTime(task.schedule.startTime);

        // Map the daysOfWeek vector to the toggle states of the day buttons
        for (int i = 0; i < m_dayButtons.size(); ++i) {
            // Days are stored as 1-7 (Monday-Sunday)
            int d = i + 1;

            // Check if the current day number exists in the task's schedule
            auto it = std::find(task.schedule.daysOfWeek.begin(), task.schedule.daysOfWeek.end(), d);
            
            // If found, set the corresponding button to the 'checked' (pressed) state
            m_dayButtons[i]->setChecked(it != task.schedule.daysOfWeek.end());
        }
    }
}

void TaskDialog::onBrowsePath() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Project", m_editPath->text());
    if (!dir.isEmpty()) m_editPath->setText(dir);
}

/**
 * @brief Handles the save action by extracting data from the UI back into the task model.
 * * This method performs "Reverse Mapping." It validates the input, updates the 
 * internal m_currentTask object with the values from the widgets, and closes 
 * the dialog with an 'Accepted' result.
 */
void TaskDialog::onSave() {
    // 1. Validation: Prevent saving tasks without a title
    if (m_editTitle->text().isEmpty()) return;

    // 2. Map basic data
    m_currentTask.title = m_editTitle->text();

    // 3. Map advanced data only for Root Tasks (Level 1)
    if (m_level == 1) {
        m_currentTask.projectPath = m_editPath->text();
        m_currentTask.schedule.active = m_checkActive->isChecked();
        m_currentTask.schedule.startTime = m_timeStart->time();
        m_currentTask.schedule.daysOfWeek.clear();
        for(int i=0; i<m_dayButtons.size(); ++i) 
            if(m_dayButtons[i]->isChecked()) m_currentTask.schedule.daysOfWeek.push_back(i+1);
    }

    // 4. Close the dialog and return QDialog::Accepted
    accept();
}

Task TaskDialog::getTask() const { return m_currentTask; }