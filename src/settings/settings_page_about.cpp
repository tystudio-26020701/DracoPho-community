#include "settings/settings_page_about.h"

#include "settings/settings_ui_helpers.h"
#include "ui/i18n.h"

#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

namespace markshot::settings {
namespace {

/// @brief 开源社区版仓库地址。
constexpr const char *kCommunityRepository = "https://github.com/tystudio-26020701/mark-shot";

/// @brief 原上游项目仓库地址。
constexpr const char *kUpstreamRepository = "https://github.com/jswysnemc/mark-shot";

/// @brief 维护公司名称。
const QString &maintainerName()
{
    static const QString name = MS_TR("Beijing Taiyin Zhaowu Technology Co., Ltd.");
    return name;
}

/// @brief 创建一段居中、可换行的富文本链接标签。
/// @param html 富文本内容。
/// @param parent 父控件。
/// @return 链接标签。
QLabel *createLinkLabel(const QString &html, QWidget *parent)
{
    auto *label = new QLabel(html, parent);
    label->setObjectName(QStringLiteral("settingsAboutText"));
    label->setWordWrap(true);
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignCenter);
    label->setOpenExternalLinks(true);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    return label;
}

}  // namespace

SettingsPageAbout::SettingsPageAbout(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = createSettingsPageLayout(this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    auto *logo = new QLabel(this);
    logo->setObjectName(QStringLiteral("settingsAboutLogo"));
    logo->setAlignment(Qt::AlignCenter);
    const QPixmap logoPixmap(QStringLiteral(":/icons/company-logo.png"));
    if (!logoPixmap.isNull()) {
        const int targetSize = 132;
        logo->setPixmap(logoPixmap.scaled(targetSize,
                                          targetSize,
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
    }
    layout->addWidget(logo);

    auto *title = new QLabel(QStringLiteral("Mark Shot"), this);
    title->setObjectName(QStringLiteral("settingsAboutTitle"));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto *edition = new QLabel(MS_TR("Open Source Community Edition"), this);
    edition->setObjectName(QStringLiteral("settingsAboutEdition"));
    edition->setAlignment(Qt::AlignCenter);
    layout->addWidget(edition);

    const QString version = QStringLiteral(MARK_SHOT_VERSION);
    auto *versionLabel = new QLabel(MS_TR("Version %1").arg(version), this);
    versionLabel->setObjectName(QStringLiteral("settingsAboutText"));
    versionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(versionLabel);

    layout->addSpacing(10);

    auto *maintained = createLinkLabel(
        MS_TR("Maintained by %1 and contributors.").arg(maintainerName()),
        this);
    layout->addWidget(maintained);

    auto *repository = createLinkLabel(
        QStringLiteral("%1<br><a href=\"%2\">%3</a>")
            .arg(MS_TR("This is the open-source community edition of Mark Shot. Source code and issue tracking: %1"),
                 QString::fromLatin1(kCommunityRepository),
                 QString::fromLatin1(kCommunityRepository)),
        this);
    layout->addWidget(repository);

    layout->addSpacing(10);

    auto *ackTitle = new QLabel(MS_TR("Acknowledgment"), this);
    ackTitle->setObjectName(QStringLiteral("settingsAboutSectionTitle"));
    ackTitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(ackTitle);

    auto *ack = createLinkLabel(
        QStringLiteral("%1<br><a href=\"%2\">%3</a>")
            .arg(MS_TR("Mark Shot is based on the original upstream project %1. We sincerely thank its author and all contributors for their outstanding work."),
                 QString::fromLatin1(kUpstreamRepository),
                 QString::fromLatin1(kUpstreamRepository)),
        this);
    layout->addWidget(ack);

    auto *license = new QLabel(MS_TR("Licensed under the MIT License."), this);
    license->setObjectName(QStringLiteral("settingsAboutText"));
    license->setAlignment(Qt::AlignCenter);
    layout->addWidget(license);

    layout->addStretch();
}

void SettingsPageAbout::setConfig(const SettingsConfig &config)
{
    Q_UNUSED(config);
}

void SettingsPageAbout::updateConfig(SettingsConfig *config) const
{
    Q_UNUSED(config);
}

bool SettingsPageAbout::isModified() const
{
    return false;
}

}  // namespace markshot::settings
