#include "ui/i18n_tables.h"

#include <QHash>
#include <QString>
#include <QtTest/QtTest>

#include <set>

namespace {

/// @brief 返回所有语言翻译表。
/// @return 语言名到翻译表的映射。
QHash<QString, const QHash<QString, QString> *> allTables()
{
    return {
        {QStringLiteral("zh-CN"), &markshot::i18n::tableZhCN()},
        {QStringLiteral("zh-TW"), &markshot::i18n::tableZhTW()},
        {QStringLiteral("ja"), &markshot::i18n::tableJa()},
        {QStringLiteral("ko"), &markshot::i18n::tableKo()},
        {QStringLiteral("ru"), &markshot::i18n::tableRu()},
        {QStringLiteral("it"), &markshot::i18n::tableIt()},
        {QStringLiteral("ar"), &markshot::i18n::tableAr()},
        {QStringLiteral("fr"), &markshot::i18n::tableFr()},
        {QStringLiteral("de"), &markshot::i18n::tableDe()},
        {QStringLiteral("es"), &markshot::i18n::tableEs()},
        {QStringLiteral("pt"), &markshot::i18n::tablePt()},
    };
}

/// @brief 提取字符串中的 %N 占位符集合。
/// @param text 待检查字符串。
/// @return 占位符集合。
std::set<QString> placeholders(const QString &text)
{
    std::set<QString> result;
    int index = 0;
    while (index < text.size()) {
        const int percent = text.indexOf(QLatin1Char('%'), index);
        if (percent < 0 || percent + 1 >= text.size()) {
            break;
        }
        const QChar next = text.at(percent + 1);
        if (next.isDigit()) {
            result.insert(QStringLiteral("%%1").arg(next));
        }
        index = percent + 1;
    }
    return result;
}

}  // namespace

class I18nTablesTest : public QObject {
    Q_OBJECT

private slots:
    /**
     * 验证所有语言表以简体中文表为基准，键集合完全一致（无缺失/多余）。
     */
    void tablesShareCompleteKeySets()
    {
        const QHash<QString, QString> &zhCN = markshot::i18n::tableZhCN();
        QVERIFY(!zhCN.isEmpty());

        const auto tables = allTables();
        for (auto it = tables.cbegin(); it != tables.cend(); ++it) {
            const QHash<QString, QString> &table = *it.value();
            QCOMPARE(table.size(), zhCN.size());
            for (auto key = zhCN.cbegin(); key != zhCN.cend(); ++key) {
                QVERIFY2(table.contains(key.key()),
                         qPrintable(QStringLiteral("%1 缺失键：%2").arg(it.key(), key.key())));
            }
        }
    }

    /**
     * 验证每个翻译值都保留其键中的 %N 占位符（顺序与集合一致）。
     */
    void placeholdersArePreserved()
    {
        const QHash<QString, QString> &zhCN = markshot::i18n::tableZhCN();
        const auto tables = allTables();
        for (auto key = zhCN.cbegin(); key != zhCN.cend(); ++key) {
            const auto expected = placeholders(key.key());
            if (expected.empty()) {
                continue;
            }
            for (auto it = tables.cbegin(); it != tables.cend(); ++it) {
                const QString value = it.value()->value(key.key());
                const auto actual = placeholders(value);
                QVERIFY2(actual == expected,
                         qPrintable(QStringLiteral("%1 键「%2」占位符不一致").arg(it.key(), key.key())));
            }
        }
    }

    /**
     * 验证每个语言表都没有空翻译值。
     */
    void noEmptyTranslations()
    {
        const auto tables = allTables();
        for (auto it = tables.cbegin(); it != tables.cend(); ++it) {
            const QHash<QString, QString> &table = *it.value();
            for (auto entry = table.cbegin(); entry != table.cend(); ++entry) {
                QVERIFY2(!entry.value().trimmed().isEmpty(),
                         qPrintable(QStringLiteral("%1 键「%2」的翻译为空").arg(it.key(), entry.key())));
            }
        }
    }
};

QTEST_GUILESS_MAIN(I18nTablesTest)
#include "i18n_tables_test.moc"
