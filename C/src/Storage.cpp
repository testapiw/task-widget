#include "Storage.h"
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

QString Storage::getFilePath() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return QDir(path).filePath(".task_widget.json");
}

std::vector<Task> Storage::loadTasks() {
    std::vector<Task> tasks;
    std::ifstream file(getFilePath().toStdString());
    if (!file.is_open()) return tasks;

    try {
        json j;
        file >> j;
        for (auto& item : j) {
            Task t;
            t.id = QUuid::fromString(QString::fromStdString(item["id"]));
            t.title = QString::fromStdString(item["title"]);
            t.level = item["level"];
            if (item.contains("parent") && !item["parent"].is_null())
                t.parent = QUuid::fromString(QString::fromStdString(item["parent"]));
            t.projectPath = QString::fromStdString(item.value("project_path", ""));
            t.status = Task::stringToStatus(QString::fromStdString(item.value("status", "Todo")));
            t.isExpanded = item.value("is_expanded", false);

            if (item.contains("schedule")) {
                auto sch = item["schedule"];
                t.schedule.active = sch.value("active", false);
                t.schedule.startTime = QTime::fromString(QString::fromStdString(sch.value("start", "00:00")), "HH:mm");
                t.schedule.endTime = QTime::fromString(QString::fromStdString(sch.value("end", "00:00")), "HH:mm");
                t.schedule.exactDate = QDate::fromString(QString::fromStdString(sch.value("date", "")), Qt::ISODate);
                t.schedule.daysOfWeek = sch.value("days", std::vector<int>{});
                t.schedule.lastAlarmTriggered = QDateTime::fromString(QString::fromStdString(sch.value("last_triggered", "")), Qt::ISODateWithMs);
            }
            tasks.push_back(t);
        }
    } catch (...) {}
    return tasks;
}

void Storage::saveTasks(const std::vector<Task>& tasks) {
    json j = json::array();
    for (const auto& t : tasks) {
        json sch;
        sch["active"] = t.schedule.active;
        sch["start"] = t.schedule.startTime.toString("HH:mm").toStdString();
        sch["end"] = t.schedule.endTime.toString("HH:mm").toStdString();
        sch["date"] = t.schedule.exactDate.toString(Qt::ISODate).toStdString();
        
        // The vector is automatically serialized into a JSON array [1, 2, 3]
        sch["days"] = t.schedule.daysOfWeek;
        sch["last_triggered"] = t.schedule.lastAlarmTriggered.toString(Qt::ISODateWithMs).toStdString();


        json item;
        item["id"] = t.id.toString().toStdString();
        item["title"] = t.title.toStdString();
        item["level"] = t.level;

        // Store an empty string instead of a null value for consistent parsing
        item["parent"] = t.parent.isNull() ? "" : t.parent.toString().toStdString();
        
        item["project_path"] = t.projectPath.toStdString();
        item["status"] = Task::statusToString(t.status).toStdString();
        item["schedule"] = sch;
        item["is_expanded"] = t.isExpanded;
        j.push_back(item);
    }
    std::ofstream file(getFilePath().toStdString());
    if (file.is_open()) {
        file << j.dump(4);
    }
}