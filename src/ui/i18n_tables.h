#pragma once

#include <QHash>
#include <QString>

// 各语言翻译表实现文件（src/ui/i18n_*.cpp）。返回的哈希表以英文源串为键，
// 目标语言文本为值；键必须与 i18n.cpp::translate() 收到的运行时字符串完全
// 一致（含 %1 占位符与转义换行符）。
namespace markshot::i18n {

const QHash<QString, QString> &tableZhCN();
const QHash<QString, QString> &tableZhTW();
const QHash<QString, QString> &tableJa();
const QHash<QString, QString> &tableKo();
const QHash<QString, QString> &tableRu();
const QHash<QString, QString> &tableIt();
const QHash<QString, QString> &tableAr();
const QHash<QString, QString> &tableFr();
const QHash<QString, QString> &tableDe();
const QHash<QString, QString> &tableEs();
const QHash<QString, QString> &tablePt();

}  // namespace markshot::i18n
