#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QKeySequence>
#include <QString>
#include <QStringList>

#include <optional>

namespace markshot::config {

QJsonObject objectValue(const QJsonObject &object, const QString &key);
QJsonObject firstObjectValue(const QJsonObject &object, const QString &key);
QJsonObject firstObjectValue(const QJsonObject &object, const QStringList &keys);
QJsonObject firstNonEmptyObjectValue(const QJsonObject &object, const QStringList &keys);
QJsonValue valueForKeys(const QJsonObject &object, const QStringList &keys);

QString normalizedKey(QString key);

std::optional<bool> boolValue(const QJsonValue &value);
std::optional<bool> boolValue(const QJsonObject &object, const QStringList &keys);
std::optional<int> intValue(const QJsonValue &value);
std::optional<int> clampedIntValue(const QJsonValue &value, int minimum, int maximum);
std::optional<QString> environmentStringValue(const QJsonValue &value);
std::optional<QKeySequence> keySequenceValue(const QJsonValue &value);

}  // namespace markshot::config
