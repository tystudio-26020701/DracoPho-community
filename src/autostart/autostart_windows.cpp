#include "autostart/autostart_manager.h"

#include "ui/i18n.h"

#include <QSettings>
#include <QString>

namespace markshot::autostart::platform {
namespace {

constexpr const char *kRunRegistryPath = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr const char *kRunValueName = "DracoPho";
// 旧版产品名遗留的 Run 值：更名 DracoPho 前以 "Mark Shot" 写入过自启动，
// 升级后该键不再被读取，残留会导致无法检测/无法彻底关闭自启动。
constexpr const char *kLegacyRunValueName = "Mark Shot";

/// @brief 返回 Windows Run 注册表项。
/// @return 当前用户 Run 注册表配置对象。
QSettings runRegistry()
{
    return QSettings(QString::fromLatin1(kRunRegistryPath), QSettings::NativeFormat);
}

/// @brief 清理旧版产品名遗留的 Run 自启动键。
void removeLegacyRunValue(QSettings *settings)
{
    if (settings && settings->contains(QString::fromLatin1(kLegacyRunValueName))) {
        settings->remove(QString::fromLatin1(kLegacyRunValueName));
    }
}

}  // namespace

bool windowsAutostartSupported()
{
#if defined(Q_OS_WIN)
    return true;
#else
    return false;
#endif
}

bool windowsAutostartEnabled()
{
#if defined(Q_OS_WIN)
    QSettings settings = runRegistry();
    // 新键与旧版遗留键任一存在且指向本程序即视为已启用。
    if (settings.value(QString::fromLatin1(kRunValueName)).toString() == startupCommand()) {
        return true;
    }
    return settings.value(QString::fromLatin1(kLegacyRunValueName)).toString() == startupCommand();
#else
    return false;
#endif
}

bool setWindowsAutostartEnabled(bool enabled, QString *error)
{
    if (error) {
        error->clear();
    }

#if defined(Q_OS_WIN)
    QSettings settings = runRegistry();
    if (enabled) {
        // 启用时先清理旧版遗留键，避免新旧两键并存导致开机启动两次。
        removeLegacyRunValue(&settings);
        settings.setValue(QString::fromLatin1(kRunValueName), startupCommand());
    } else {
        settings.remove(QString::fromLatin1(kRunValueName));
        removeLegacyRunValue(&settings);
    }
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        if (error) {
            *error = MS_TR("Cannot update Windows autostart registry.");
        }
        return false;
    }
    return true;
#else
    Q_UNUSED(enabled);
    if (error) {
        *error = MS_TR("Autostart is not supported on this platform.");
    }
    return false;
#endif
}

}  // namespace markshot::autostart::platform
