#include "settings/settings_page_about.h"

#include "settings/settings_ui_helpers.h"
#include "ui/i18n.h"
#include "ui/icons.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
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
    // 页面根布局占满整行（不压缩宽度），内容用居中的 HBox 容器承载，
    // 保证 QScrollArea 的 heightForWidth 计算正确：内容超高时可滚动，
    // logo 与文本不会被压缩截断。
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(20, 20, 20, 20);
    rootLayout->setSpacing(0);

    auto *centerRow = new QHBoxLayout;
    centerRow->setContentsMargins(0, 0, 0, 0);
    centerRow->addStretch();

    auto *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(14);
    contentLayout->setSizeConstraint(QLayout::SetFixedSize);

    // 1. Mark Shot 软件图标（不是公司 logo）：位于产品名称上方，
    //    与"维护者"区的公司 logo 明确区分。
    auto *appIcon = new QLabel(this);
    appIcon->setObjectName(QStringLiteral("settingsAboutAppIcon"));
    appIcon->setAlignment(Qt::AlignCenter);
    const QIcon appIconSource = markshot::ui::applicationIcon();
    if (!appIconSource.isNull()) {
        appIcon->setPixmap(appIconSource.pixmap(96, 96));
    }
    contentLayout->addWidget(appIcon);

    auto *title = new QLabel(QStringLiteral("Mark Shot"), this);
    title->setObjectName(QStringLiteral("settingsAboutTitle"));
    title->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(title);

    auto *edition = new QLabel(MS_TR("Open Source Community Edition"), this);
    edition->setObjectName(QStringLiteral("settingsAboutEdition"));
    edition->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(edition);

    const QString version = QStringLiteral(MARK_SHOT_VERSION);
    auto *versionLabel = new QLabel(MS_TR("Version %1").arg(version), this);
    versionLabel->setObjectName(QStringLiteral("settingsAboutText"));
    versionLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(versionLabel);

    contentLayout->addSpacing(10);

    // 2. 维护者信息：公司 logo 放在此处（与软件图标区分），标注公司全称。
    auto *companyLogo = new QLabel(this);
    companyLogo->setObjectName(QStringLiteral("settingsAboutCompanyLogo"));
    companyLogo->setAlignment(Qt::AlignCenter);
    const QPixmap companyPixmap(QStringLiteral(":/icons/company-logo.png"));
    if (!companyPixmap.isNull()) {
        const int targetSize = 48;
        companyLogo->setPixmap(companyPixmap.scaled(targetSize,
                                                    targetSize,
                                                    Qt::KeepAspectRatio,
                                                    Qt::SmoothTransformation));
    }
    contentLayout->addWidget(companyLogo);

    auto *maintained = createLinkLabel(
        MS_TR("Maintained by %1 and contributors.").arg(maintainerName()),
        this);
    contentLayout->addWidget(maintained);

    auto *repository = createLinkLabel(
        MS_TR("This is the open-source community edition of Mark Shot. Source code and issue tracking: %1")
            .arg(QStringLiteral("<a href=\"%1\">%1</a>").arg(QString::fromLatin1(kCommunityRepository))),
        this);
    contentLayout->addWidget(repository);

    contentLayout->addSpacing(10);

    // 3. 对原上游项目及其作者、贡献者的致谢。
    auto *ackTitle = new QLabel(MS_TR("Acknowledgment"), this);
    ackTitle->setObjectName(QStringLiteral("settingsAboutSectionTitle"));
    ackTitle->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(ackTitle);

    auto *ack = createLinkLabel(
        MS_TR("Mark Shot is based on the original upstream project %1. We sincerely thank its author and all contributors for their outstanding work.")
            .arg(QStringLiteral("<a href=\"%1\">%1</a>").arg(QString::fromLatin1(kUpstreamRepository))),
        this);
    contentLayout->addWidget(ack);

    auto *license = new QLabel(MS_TR("Licensed under the MIT License."), this);
    license->setObjectName(QStringLiteral("settingsAboutText"));
    license->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(license);

    centerRow->addLayout(contentLayout);
    centerRow->addStretch();
    rootLayout->addLayout(centerRow);
    rootLayout->addStretch();
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
