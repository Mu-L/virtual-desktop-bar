#include "mdsmodel.h"

#include <KWindowSystem>
#include <KGlobalAccel>

MDSModel::MDSModel(QObject* parent) : QObject(parent),
                                      netRootInfo(QX11Info::connection(), 0) {

    QObject::connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged,
                     this, &MDSModel::currentDesktopChanged);

    QObject::connect(KWindowSystem::self(), &KWindowSystem::numberOfDesktopsChanged,
                     this, &MDSModel::desktopAmountChanged);

    QObject::connect(KWindowSystem::self(), &KWindowSystem::desktopNamesChanged,
                     this, &MDSModel::desktopNamesChanged);

    actionCollection = new KActionCollection(this);

    actionAddNewDesktop = actionCollection->addAction(QStringLiteral("addNewDesktop"));
    actionAddNewDesktop->setText("Add New Virtual Desktop");
    actionAddNewDesktop->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    connect(actionAddNewDesktop, &QAction::triggered, this, [this]() {
        addNewDesktop();
    });
    KGlobalAccel::setGlobalShortcut(actionAddNewDesktop, QKeySequence());

    actionRemoveLastDesktop = actionCollection->addAction(QStringLiteral("removeLastDesktop"));
    actionRemoveLastDesktop->setText("Remove Last Virtual Desktop");
    actionRemoveLastDesktop->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    connect(actionRemoveLastDesktop, &QAction::triggered, this, [this]() {
        removeLastDesktop();
    });
    KGlobalAccel::setGlobalShortcut(actionRemoveLastDesktop, QKeySequence());

    actionRemoveCurrentDesktop = actionCollection->addAction(QStringLiteral("removeCurrentDesktop"));
    actionRemoveCurrentDesktop->setText("Remove Current Virtual Desktop");
    actionRemoveCurrentDesktop->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    connect(actionRemoveCurrentDesktop, &QAction::triggered, this, [this]() {
        removeCurrentDesktop();
    });
    KGlobalAccel::setGlobalShortcut(actionRemoveCurrentDesktop, QKeySequence());

    actionRenameCurrentDesktop = actionCollection->addAction(QStringLiteral("renameCurrentDesktop"));
    actionRenameCurrentDesktop->setText("Rename Current Virtual Desktop");
    actionRenameCurrentDesktop->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
    connect(actionRenameCurrentDesktop, &QAction::triggered, this, [this]() {
        emit currentDesktopNameChangeRequested();
    });
    KGlobalAccel::setGlobalShortcut(actionRenameCurrentDesktop, QKeySequence());
}

QVariantList MDSModel::getDesktopNames() const {
    QVariantList desktopNames;
    int numberOfDesktops = KWindowSystem::numberOfDesktops();
    for (int desktopNumber = 0; desktopNumber < numberOfDesktops; desktopNumber++) {
        const QString& desktopName = KWindowSystem::desktopName(desktopNumber + 1);
        desktopNames << QVariant(desktopName);
    }
    return desktopNames;
}

QVariant MDSModel::getCurrentDesktopName() const {
    const int currentDesktop = KWindowSystem::currentDesktop();
    const QString currentDesktopName = KWindowSystem::desktopName(currentDesktop);
    return QVariant(currentDesktopName);
}

QVariant MDSModel::getCurrentDesktopNumber() const {
    const int currentDesktop = KWindowSystem::currentDesktop();
    return QVariant(currentDesktop);
}

void MDSModel::switchToDesktop(int desktopNumber) {
    KWindowSystem::setCurrentDesktop(desktopNumber);
}

void MDSModel::addNewDesktop(const QString desktopName) {
    int numberOfDesktops = KWindowSystem::numberOfDesktops();
    netRootInfo.setNumberOfDesktops(numberOfDesktops + 1);
    if (!desktopName.isNull()) {
        KWindowSystem::setDesktopName(numberOfDesktops + 1, desktopName);
    }
}

void MDSModel::removeLastDesktop() {
    int numberOfDesktops = KWindowSystem::numberOfDesktops();
    if (numberOfDesktops > 1) {
        netRootInfo.setNumberOfDesktops(numberOfDesktops - 1);
    }
}

void MDSModel::removeDesktop(int desktopNumber) {
    const int numberOfDesktops = KWindowSystem::numberOfDesktops();
    if (numberOfDesktops > 1) {
        if (desktopNumber == numberOfDesktops) {
            removeLastDesktop();
            return;
        }

        QList<WId> windows = KWindowSystem::windows();
        for (WId id : windows) {
            if (KWindowSystem::hasWId(id)) {
                KWindowInfo info = KWindowInfo(id, NET::Property::WMDesktop);
                if (info.valid()) {
                    const int windowDesktopNumber = info.desktop();
                    if (windowDesktopNumber != NET::OnAllDesktops
                        && windowDesktopNumber > desktopNumber) {
                        KWindowSystem::setOnDesktop(id, windowDesktopNumber - 1);
                    }
                }
            }
        }

        QVariantList desktopNames = getDesktopNames();
        for (int i = desktopNumber - 1; i < numberOfDesktops - 1; i++) {
            const QString desktopName = desktopNames.at(i + 1).toString();
            KWindowSystem::setDesktopName(i + 1, desktopName);
        }

        removeLastDesktop();
    }
}

void MDSModel::removeCurrentDesktop() {
    const int currentDesktop = KWindowSystem::currentDesktop();
    removeDesktop(currentDesktop);
}

void MDSModel::renameDesktop(const int desktopNumber, const QString desktopName) {
    KWindowSystem::setDesktopName(desktopNumber, desktopName);
}

void MDSModel::renameCurrentDesktop(const QString desktopName) {
    const int currentDesktopNumber = KWindowSystem::currentDesktop();
    renameDesktop(currentDesktopNumber, desktopName);
}
