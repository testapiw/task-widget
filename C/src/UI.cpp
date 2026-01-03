#include "UI.h"
#include "Storage.h"
#include "TaskDialog.h"
#include "StyleHelper.h"
#include "PluginSettingsManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProcess>
#include <QSettings>
#include <QApplication>
#include <QHeaderView>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QCheckBox>
#include <QFormLayout>
#include <QPropertyAnimation>


/**
 * @brief Constructs the MainWindow, initializes managers, and sets up signal-slot connections.
 * * This constructor handles the initial state restoration including tasks, window flags, 
 * and position, while also starting the background monitoring service.
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_tasks = Storage::loadTasks();
    
    m_alarmManager = new AlarmManager(this);
    m_pluginManager = new PluginManager(this);
    m_pluginManager->loadPlugins(this);

    QSettings settings("TaskWidget", "TaskWidget");
    bool onTop = settings.value("alwaysOnTop", true).toBool();
    if (onTop) {
        setWindowFlag(Qt::WindowStaysOnTopHint, true);
    }

    setupUi();
       
    m_trayIcon = new QSystemTrayIcon(QIcon(":/src/icon/vscode.png"), this);
    m_trayIcon->show();

    // Save expanded state of tree items when a node is expanded
    connect(m_treeWidget, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem* item) {
        QUuid id = item->data(0, Qt::UserRole).toUuid();
        for (auto &t : m_tasks) if (t.id == id) t.isExpanded = true;
        Storage::saveTasks(m_tasks);
    });

    // Save expanded state of tree items when a node is collapsed
    connect(m_treeWidget, &QTreeWidget::itemCollapsed, this, [this](QTreeWidgetItem* item) {
        QUuid id = item->data(0, Qt::UserRole).toUuid();
        for (auto &t : m_tasks) if (t.id == id) t.isExpanded = false;
        Storage::saveTasks(m_tasks);
    });

    // Alarm Manager connections
    connect(m_alarmManager, &AlarmManager::alarmTriggered, 
        this, &MainWindow::handleAlarmTriggered, 
        Qt::QueuedConnection);
    connect(m_alarmManager, &AlarmManager::blinkTick, this, &MainWindow::updateVisualState);
    connect(m_alarmManager, &AlarmManager::tasksChanged, this, [this]() { 
        Storage::saveTasks(m_tasks); 
    });

    // Background task checking (every 30 seconds)
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() { m_alarmManager->checkTasks(m_tasks); });
    timer->start(30000);
    m_alarmManager->startMonitoring();

    // Restore window position if it was previously saved
    if (settings.contains("pos")) move(settings.value("pos").toPoint());

    refreshList();
}

MainWindow::~MainWindow() = default;

/**
 * @brief Initializes the main user interface with a frameless, modern design.
 * * Sets up window flags for transparency and "Always on Top" behavior, 
 * creates the custom header with management controls, and initializes 
 * the hierarchical task tree.
 */
void MainWindow::setupUi() {
    // Enable transparency and remove standard OS window borders
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFixedSize(380, 650);

    auto *central = new QWidget();
    central->setObjectName("centralWidget");
    auto *layout = new QVBoxLayout(central);

    // Header Layout: Contains the drag handle and action buttons
    auto *header = new QHBoxLayout();

    // Drag handle icon (indicates the window can be moved)
    auto *dragHandle = new QLabel("☰");
    dragHandle->setObjectName("dragHandle");

    // Mini/Maximize button: Toggles window height for a "compact mode"
    auto *btnResize = new QPushButton("↕");
    btnResize->setFixedSize(24, 24);
    btnResize->setStyleSheet("QPushButton { border: none; background: transparent; font-size: 16px; color: #888; }");
    connect(btnResize, &QPushButton::clicked, this, [this]() {
        // Toggle between compact (200px) and full (650px) height
        if (height() > 300) setFixedHeight(200);
        else setFixedHeight(650);
    });

    // Settings button: Opens the plugin and global configuration
    auto *btnSettings = new QPushButton("⚙");
    btnSettings->setFixedSize(24, 24);
    btnSettings->setStyleSheet("QPushButton { border: none; background: transparent; font-size: 16px; color: #888; }");
    connect(btnSettings, &QPushButton::clicked, this, &MainWindow::showSettingsDialog);

    // Add Task button: Creates a new level 1 (Root) project
    auto *btnNew = new QPushButton("➕ New Project");
    btnNew->setObjectName("btnNew");
    connect(btnNew, &QPushButton::clicked, this, [this]() { showTaskDialog(QUuid(), 1); });

    header->addWidget(dragHandle);
    // header->addWidget(btnPin);
    header->addWidget(btnResize);
    header->addWidget(btnSettings);
    header->addWidget(btnNew, 1); // stretch factor of 1 allows button to fill space
    layout->addLayout(header);

    // Main Task List (Tree structure)
    m_treeWidget = new QTreeWidget();
    m_treeWidget->viewport()->installEventFilter(this); // Catch clicks to stop alarms
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setIndentation(15);
    layout->addWidget(m_treeWidget);
    
    setCentralWidget(central);

    // Apply initial QSS style (non-alarm state)
    updateVisualState(false);
}

/**
 * @brief Orchestrates the creation or modification of a task.
 * * This method prepares the data for the TaskDialog, executes the modal 
 * interface, and handles the persistence and UI refresh once the user 
 * confirms changes.
 * * @param parentId The UUID of the parent task (used for new subtasks).
 * @param level The hierarchy depth (1 for projects, >1 for subtasks).
 * @param isEdit Flag to indicate if we are updating an existing task.
 * @param editId The UUID of the specific task to be modified.
 */
void MainWindow::showTaskDialog(QUuid parentId, int level, bool isEdit, QUuid editId) {
    Task targetTask;
    if (isEdit) {
        // Find the existing task in the collection
        auto it = std::find_if(m_tasks.begin(), m_tasks.end(), [&](const Task& t) { return t.id == editId; });
        if (it != m_tasks.end()) targetTask = *it;
    } else {
        // Initialize a new task with default parameters
        targetTask.id = QUuid::createUuid();
        targetTask.parent = parentId;
        targetTask.level = level;
        targetTask.status = TaskStatus::Todo;
    }

    // Launch the Modal Dialog
    TaskDialog dialog(targetTask, isEdit, this);
    if (dialog.exec() == QDialog::Accepted) {
        Task result = dialog.getTask();
        if (isEdit) {
            // Update the existing task in our vector
            for (auto &t : m_tasks) if (t.id == result.id) t = result;
        } else {
            // Append the new task to the collection
            m_tasks.push_back(result);
        }

        // PERSISTENCE: Save the entire list to disk immediately
        Storage::saveTasks(m_tasks);

        // UI REFRESH: Rebuild the tree widget to reflect changes
        refreshList();
    }
}

/**
 * @brief Creates a custom widget to be displayed inside a tree row.
 * * This method generates a complex layout containing status toggles, 
 * task titles, alarm indicators, and management buttons (Edit/Add/Delete).
 * * @param task The task data used to populate and configure the widget.
 * @return A pointer to the QWidget container for the tree item.
 */
QWidget* MainWindow::createTaskWidget(const Task &task) {
    auto *container = new QWidget();
    // Unique object name allows finding this widget during alarm animations (blinking)
    container->setObjectName("taskCont_" + task.id.toString());
    
    auto *layout = new QHBoxLayout(container);
    layout->setContentsMargins(6, 0, 8, 0);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignVCenter); 

    // Determine status color and icon based on task state
    QString color;
    QString icon;
    if (task.status == TaskStatus::Todo) { color = "#888"; icon = "•"; }
    else if (task.status == TaskStatus::InProgress) { color = "#3498db"; icon = "▶"; }
    else if (task.status == TaskStatus::Done) { color = "#2ecc71"; icon = "✔"; }
    else { color = "#e67e22"; icon = "⏸"; }

    // Status toggle button
    auto *btnStatus = new QPushButton(icon);
    btnStatus->setFixedSize(24, 24);
    btnStatus->setStyleSheet(StyleHelper::getStatusStyle(color));
    connect(btnStatus, &QPushButton::clicked, this, [this, task]() {
        // Cycle through statuses: Todo -> InProgress -> Done -> Paused -> Todo
        TaskStatus next = TaskStatus::Todo;
        if (task.status == TaskStatus::Todo) next = TaskStatus::InProgress;
        else if (task.status == TaskStatus::InProgress) next = TaskStatus::Done;
        else if (task.status == TaskStatus::Done) next = TaskStatus::Paused;
        for (auto &t : m_tasks) if (t.id == task.id) t.status = next;
        Storage::saveTasks(m_tasks);
        refreshList();
    });
    layout->addWidget(btnStatus);

    // Task Title label
    auto *label = new QLabel(task.title);
    // Apply strikethrough effect for completed tasks
    label->setStyleSheet(task.status == TaskStatus::Done ? "text-decoration: line-through; color: #555;" : "color: #eee;");
    layout->addWidget(label, 1);

    // ALARM INDICATOR
    // Displays a bell icon if the task has an active schedule
    if (task.schedule.active) {
        auto *lblAlarm = new QLabel("🔔");
        lblAlarm->setStyleSheet("font-size: 11px; color: #f1c40f;");
        lblAlarm->setToolTip("Alarm: " + task.schedule.startTime.toString("HH:mm"));
        layout->addWidget(lblAlarm);
    }

    // VS CODE INTEGRATION
    // Launches VS Code in the project directory if the path is set (Root tasks only)
    if (!task.projectPath.isEmpty() && task.level == 1) {
        auto *btnVs = new QPushButton();
        btnVs->setFixedSize(22, 22);
        btnVs->setIcon(QIcon(":/src/icon/vscode.png")); // Ensure resource path is correct
        btnVs->setFlat(true);
        connect(btnVs, &QPushButton::clicked, this, [this, task]() {
            QProcess::startDetached("code", {task.projectPath});
        });
        layout->addWidget(btnVs);
    }

    // MANAGEMENT BUTTONS (Edit, Add Subtask, Delete)
    auto *btnEdit = new QPushButton("✎");
    auto *btnAdd = new QPushButton("+");
    auto *btnDel = new QPushButton("✕");
    for(auto* b : {btnEdit, btnAdd, btnDel}) {
        b->setFixedSize(20, 20);
        b->setStyleSheet("QPushButton { color: #666; border: none; } QPushButton:hover { color: #fff; }");
    }

    // Edit current task
    connect(btnEdit, &QPushButton::clicked, this, [this, task]() { showTaskDialog(task.parent, task.level, true, task.id); });
    
    // Add child task (increments hierarchy level)
    connect(btnAdd, &QPushButton::clicked, this, [this, task]() { showTaskDialog(task.id, task.level + 1); });
    
    // Delete task and its immediate children
    connect(btnDel, &QPushButton::clicked, this, [this, id = task.id]() {
        m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(), [&](const Task& t) { 
            return t.id == id || t.parent == id; 
        }), m_tasks.end());
        Storage::saveTasks(m_tasks);
        refreshList();
    });

    layout->addWidget(btnEdit);
    layout->addWidget(btnAdd);
    layout->addWidget(btnDel);
    
    return container;
}

/**
 * @brief Synchronizes the QTreeWidget with the internal task collection.
 * * This method clears the current view and rebuilds the hierarchical tree 
 * by identifying root-level tasks and initiating the recursive addition process.
 */
void MainWindow::refreshList() {
    // 1. Temporarily block signals to prevent redundant UI updates during the rebuild
    m_treeWidget->blockSignals(true);

    // 2. Remove all existing items and custom widgets from the tree
    m_treeWidget->clear();

    // 3. Iterate through the task collection and find "Root" tasks (parent is Null)
    for (const auto& task : m_tasks) {
        if (task.parent.isNull()) {
            // Initiate recursive tree construction for this root and its children
            addTaskToTree(task);
        }
    }

    // 4. Restore signal processing
    m_treeWidget->blockSignals(false);
}

/**
 * @brief Recursively builds the tree structure for a specific task and its children.
 * * This method handles the creation of tree items, attaches custom widgets,
 * and maintains the hierarchical relationship by scanning the task collection 
 * for children.
 * * @param task The task to be added to the tree.
 * @param parentItem The parent QTreeWidgetItem. If null, the task is added to the root.
 */
void MainWindow::addTaskToTree(const Task &task, QTreeWidgetItem *parentItem) {
    // Create the item, attaching it either to a parent or the invisible root
    auto *item = new QTreeWidgetItem(parentItem ? parentItem : m_treeWidget->invisibleRootItem());
    
    // Store the UUID in the UserRole to easily find this task during UI events
    item->setData(0, Qt::UserRole, task.id);

    // Increase row height for better touch/click targets and visual clarity
    item->setSizeHint(0, QSize(0, 36));
    
    // Restore the expansion state (whether subtasks are shown) from the data model
    item->setExpanded(task.isExpanded);

    // Attach the interactive custom widget (buttons, status, etc.) to this row
    m_treeWidget->setItemWidget(item, 0, createTaskWidget(task));
    
    // RECURSION: Search for any tasks that have this task's ID as their parent
    for (const auto& t : m_tasks) {
        if (t.parent == task.id) addTaskToTree(t, item);
    }
}

/**
 * @brief Responds to an alarm trigger by alerting the user and updating task state.
 * * This method ensures the application grabs the user's attention, sends a system 
 * notification, and records the trigger time to prevent duplicate alarms within 
 * the same minute.
 */
void MainWindow::handleAlarmTriggered(const Task &task) {
    // 1. State Management: Record the trigger time to prevent "alarm spam"
    for (auto &t : m_tasks) {
        if (t.id == task.id) {
            // Update the last triggered timestamp in the persistent model
            t.schedule.lastAlarmTriggered = QDateTime::currentDateTime();
            break;
        }
    }

    // Save state immediately so the alarm doesn't re-fire if the app restarts
    Storage::saveTasks(m_tasks);
    
    // 2. Visual Feedback: Bring the window to the foreground
    // sendNotification uses the system tray or OS-level toast notifications
    sendNotification("Alarm", task.title);

    // Ensure the window is visible and focused regardless of its current state
    this->show();           // Makes window visible if it was hidden
    this->activateWindow(); // Attempts to take keyboard/mouse focus
    
    // Reset minimized state and force the window to become the active foreground window
    this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    this->raise(); // Moves the window to the top of the Z-order (above other apps)
}

/**
 * @brief Updates the visual theme of the window and individual task rows.
 * * This method synchronizes the UI with the alarm state. It triggers the global 
 * window flashing and applies a specific "ringing" style to the task row currently 
 * causing the alarm.
 * * @param isAlarmActive The current phase of the blink cycle (true = highlighted, false = normal).
 */
void MainWindow::updateVisualState(bool isAlarmActive) {
    setStyleSheet(StyleHelper::getMainStyle(isAlarmActive));
   
    // Iterate through tasks to apply row-specific alarm effects
    for (const auto& task : m_tasks) {
        // Locate the specific custom widget for this task using its unique object name
        QWidget* widget = m_treeWidget->findChild<QWidget*>("taskCont_" + task.id.toString());
        if (!widget) continue;

        // Check if this specific task is currently the source of an active alarm
        if (m_alarmManager->isRinging() && m_alarmManager->activeTaskId() == task.id) {
            // Visual feedback: Toggle a red left border and subtle background tint
            QString color = isAlarmActive ? "#ff4757" : "transparent";
            widget->setStyleSheet(QString(".QWidget { border-left: 4px solid %1; background: rgba(255, 71, 87, 20); }").arg(color));
        } else {
            // Reset to default style for non-ringing tasks
            widget->setStyleSheet(".QWidget { border-left: 4px solid transparent; background: transparent; }");
        }
    }
}

/**
 * @brief Halts all active alert signals and resets the UI to its normal state.
 * * This method acts as the "Off Switch" for the alarm. It coordinates between 
 * the time-tracking logic and the external notification plugins (sound, visuals, etc.) 
 * to ensure the system returns to a quiet background state.
 */
void MainWindow::stopAlarmAnimation() {
    // Check if the alarm system is currently in a 'Ringing' state
    if (m_alarmManager->isRinging()) {
        // 1. Logic Reset: Stop the internal timer and clear the 'activeTaskId'
        m_alarmManager->stopAlarm();

        // 2. Multimedia Reset: Stop audio playback or external plugin notifications
        m_pluginManager->stopAll();

        // 3. UI Reset: Force one final visual update to remove red borders and flashing
        updateVisualState(false);
    }
}

/**
 * @brief Dispatches alerts through system-level notifications and internal plugins.
 * * This method ensures the user is notified even if the application is minimized 
 * or hidden by using the OS native notification daemon and the custom plugin system.
 * * @param title The brief header for the notification (e.g., "Task Alarm").
 * @param message The detailed description (e.g., the name of the task).
 */
void MainWindow::sendNotification(const QString &title, const QString &message) {
    // 1. OS-Level Notification (Linux/Ubuntu standard)
    // Uses 'notify-send' to show a system toast. '-u critical' ensures it stays visible.
    QProcess::startDetached("notify-send", {"-u", "critical", title, message});

    // 2. Plugin Notification
    // Notifies all loaded plugins (e.g., Sound, Telegram Bot, or LED controllers)
    m_pluginManager->triggerAll(message, this);
}

/**
 * @brief Handles mouse press events to support window dragging and alarm silencing.
 * * Since the window is frameless (no title bar), this method implements manual 
 * dragging logic. It also serves as a global "Snooze" trigger—clicking anywhere 
 * on the window background will stop an active alarm.
 */
void MainWindow::mousePressEvent(QMouseEvent *event) {
    stopAlarmAnimation();

    // Dragging logic: Initiate manual window movement for frameless UI
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;

        // Calculate the offset between the mouse cursor and the top-left corner 
        // of the window to ensure smooth movement in mouseMoveEvent.
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        
        event->accept();
    }
}

/**
 * @brief Handles the window movement while the mouse is being dragged.
 * * This method works in tandem with mousePressEvent. It updates the window's
 * position on the screen based on the current mouse coordinates and the 
 * previously calculated offset.
 */
void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    // Only move the window if the left mouse button was pressed on the background
    if (m_dragging) {
        // Calculate the new top-left corner by subtracting the initial offset
        // from the current global mouse position.
        move(event->globalPos() - m_dragPosition);

        // Mark the event as handled to prevent further propagation
        event->accept();
    }
}

/**
 * @brief Resets the dragging state when the mouse button is released.
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *) { m_dragging = false; }

/**
 * @brief Handles the window move event to save the window's position.
 * * This ensures that if the user moves the widget, it will reappear in the 
 * same location the next time the application is launched.
 */
void MainWindow::moveEvent(QMoveEvent *) {
    // Only save position if the window is visible to avoid saving 
    // off-screen or minimized coordinates.
    if (this->isVisible()) {
        QSettings settings("TaskWidget", "TaskWidget");
        settings.setValue("pos", pos());
    }
}

/**
 * @brief Filters events for watched objects (like the TreeView viewport).
 * * This allows the MainWindow to intercept mouse clicks on internal components 
 * to stop active alarms without needing to connect every individual child widget.
 * * @param obj The object being watched.
 * @param event The intercepted event.
 */
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    // If the user clicks anywhere inside the task list (viewport), silence the alarm
    if (event->type() == QEvent::MouseButtonPress) stopAlarmAnimation();
    
    // Pass the event back to the base class so standard behavior (selection, etc.) continues
    return QMainWindow::eventFilter(obj, event);
}

/**
 * @brief Handles keyboard shortcuts for quick interaction.
 * * Supports silencing alarms with 'Escape' and closing the application 
 * with a combined 'Ctrl + Escape' shortcut.
 */
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) stopAlarmAnimation();
    else if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Escape)) close();
}


// --- settings dialog ---
/**
 * @brief Constructs and displays a dynamic settings dialog.
 * * This method manages global application settings (like "Always on Top") and 
 * dynamically generates configuration UI for loaded plugins. It features 
 * animated dropdown panels and secure data persistence for plugin credentials.
 */
void MainWindow::showSettingsDialog() {
    // 1. Initialize the Dialog and Styles
    QDialog dialog(this);
    dialog.setWindowTitle("Settings");
    dialog.setFixedWidth(360);
    dialog.setStyleSheet(
        "QDialog { background-color: #1e1e1e; color: white; } "
        "QLineEdit { background: #2d2d2d; border: 1px solid #444; color: white; padding: 4px; border-radius: 2px; } "
        "QLabel { color: #bbb; } "
        "QCheckBox { color: white; } "
        "QCheckBox::indicator { width: 16px; height: 16px; }"
    );
    
    auto *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);


    // --- GLOBAL SETTINGS: Always on Top ---
    QSettings globalSettings("TaskWidget", "TaskWidget");
    auto *onTopCheck = new QCheckBox("Always on Top");
    onTopCheck->setChecked(globalSettings.value("alwaysOnTop", true).toBool());
    onTopCheck->setStyleSheet("QCheckBox { font-weight: bold; color: #3498db; margin-bottom: 5px; }");

    connect(onTopCheck, &QCheckBox::toggled, this, [this](bool checked) {
        QSettings settings("TaskWidget", "TaskWidget");
        settings.setValue("alwaysOnTop", checked);

        // Update window flags dynamically
        Qt::WindowFlags flags = this->windowFlags();
        if (checked) {
            flags |= Qt::WindowStaysOnTopHint;
        } else {
            flags &= ~Qt::WindowStaysOnTopHint;
        }
        
        this->setWindowFlags(flags);
        this->show(); // Re-show required on some OS after flag modification
    });
    
    mainLayout->addWidget(onTopCheck);

    // Visual Separator
    auto *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("background-color: #333;");
    mainLayout->addWidget(line);
    


    // --- DYNAMIC PLUGIN CONFIGURATION ---
    if (!m_pluginManager) return;
    auto plugins = m_pluginManager->getPlugins();

    for (auto *plugin : plugins) {
        if (!plugin) continue;

        auto *pluginRow = new QHBoxLayout();
        
        // Plugin activation toggle
        auto *check = new QCheckBox(plugin->pluginName());
        QSettings globalSettings("TaskWidget", "Plugins");
        check->setChecked(globalSettings.value(plugin->pluginName(), true).toBool());
        
        connect(check, &QCheckBox::toggled, [plugin](bool checked) {
            QSettings gs("TaskWidget", "Plugins");
            gs.setValue(plugin->pluginName(), checked);
        });

        pluginRow->addWidget(check);
        pluginRow->addStretch();

        // Check if plugin provides configurable settings
        QStringList keys = plugin->defaultSettings();
        if (!keys.isEmpty()) {
            QString configPath = plugin->getConfigPath();
            bool isSec = plugin->isSecurity();

            // Ensure the config file exists and initialize shortcuts
            PluginSettingsManager::initShortcut(configPath, keys, isSec);

            // Settings Toggle Button (Gear Icon)
            auto *btnConfig = new QPushButton("⚙");
            btnConfig->setFixedSize(26, 26);
            btnConfig->setCursor(Qt::PointingHandCursor);
            btnConfig->setFlat(true);
            btnConfig->setStyleSheet(
                "QPushButton { border: none; font-size: 18px; color: #888; } "
                "QPushButton:hover { color: white; }"
            );
            pluginRow->addWidget(btnConfig);

            // Container for collapsible settings
            auto *settingsContainer = new QWidget();
            auto *settingsLayout = new QFormLayout(settingsContainer);
            
            settingsContainer->setMaximumHeight(0); // Collapsed by default
            settingsContainer->setVisible(false);
            settingsContainer->setContentsMargins(0, 5, 0, 5);
            settingsContainer->setStyleSheet("background: #252525; border-radius: 4px;");

            // Load saved plugin data
            auto data = PluginSettingsManager::loadSettings(configPath, isSec);

            for (const QString &key : keys) {
                auto *edit = new QLineEdit(data.value(key));
                if (isSec) edit->setEchoMode(QLineEdit::Password);
                
                settingsLayout->addRow(key + ":", edit);

                // Auto-save on every character change
                connect(edit, &QLineEdit::textChanged, [configPath, key, edit, isSec]() {
                    PluginSettingsManager::saveSetting(configPath, key, edit->text(), isSec);
                });
            }

            // Expand/Collapse Animation Logic
            connect(btnConfig, &QPushButton::clicked, [settingsContainer, &dialog]() {
                if (!settingsContainer->layout()) return;

                // Calculate the natural height required by the fields inside
                int targetHeight = settingsContainer->layout()->sizeHint().height();
                int currentMaxHeight = settingsContainer->maximumHeight();
                
                // Determine direction: if current max height is 0, we are opening the panel
                bool opening = (currentMaxHeight == 0);
                int startHeight = settingsContainer->height();
                int endHeight = opening ? targetHeight : 0;

                // If opening, make the widget visible immediately so the animation is seen
                if (opening) settingsContainer->setVisible(true);

                auto *ani = new QPropertyAnimation(settingsContainer, "maximumHeight");
                ani->setDuration(300);
                ani->setStartValue(startHeight);
                ani->setEndValue(endHeight);
                ani->setEasingCurve(QEasingCurve::InOutQuad); // Smooth start and finish

                // Synchronize the main dialog size with the container's expansion
                QObject::connect(ani, &QPropertyAnimation::valueChanged, [&dialog]() {
                    dialog.adjustSize();
                });

                // After closing completely, set visibility to false to remove it from layout calculations
                QObject::connect(ani, &QPropertyAnimation::finished, [settingsContainer, endHeight]() {
                    if (endHeight == 0) settingsContainer->setVisible(false);
                });

                ani->start(QAbstractAnimation::DeleteWhenStopped);
            });

            mainLayout->addLayout(pluginRow);
            mainLayout->addWidget(settingsContainer);
        } else {
            // If there are no settings, just add the row (Checkbox + Stretch)
            // This keeps the UI clean for "plug-and-play" plugins.
            mainLayout->addLayout(pluginRow);
        }
    } // End of plugin loop

    // Done Button
    auto *btnClose = new QPushButton("Done");
    btnClose->setFixedHeight(32);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet(
        "QPushButton { background: #333; border: 1px solid #444; border-radius: 4px; color: white; margin-top: 10px; } "
        "QPushButton:hover { background: #444; }"
    );
    connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::accept);
    mainLayout->addWidget(btnClose);

    dialog.exec();
}