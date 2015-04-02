/***************************************************************************
 *   Copyright (C) 2012 by Aurélien Gâteau <agateau@kde.org>               *
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "recentcontactsmodel.h"
#include "actionlist.h"

#include <QDebug>
#include <QAction>

#include <KLocalizedString>

#include <KActivitiesExperimentalStats/ResultModel>
#include <KActivitiesExperimentalStats/Terms>

#include <kpeople/widgets/actions.h> //FIXME TODO: Pretty include in KPeople broken.
#include <kpeople/widgets/persondetailsdialog.h>
#include <KPeople/PersonData>

namespace KAStats = KActivities::Experimental::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

RecentContactsModel::RecentContactsModel(QObject *parent) : ForwardingModel(parent)
{
    refresh();
}

RecentContactsModel::~RecentContactsModel()
{
}

QVariant RecentContactsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QString id = sourceModel()->data(index, ResultModel::ResourceRole).toString();
    qDebug() << id;

    KPeople::PersonData *data = 0;

    if (m_idToData.contains(id)) {
        data = m_idToData[id];
    }

    if (!data) {
        const_cast<RecentContactsModel *>(this)->insertPersonData(id, index.row());
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return data->name();
    } else if (role == Qt::DecorationRole) {
        return data->presenceIconName();
    } else if (role == Kicker::HasActionListRole) {
        return true;
    } else if (role == Kicker::ActionListRole) {
        QVariantList actionList ;

        actionList << Kicker::createActionItem(i18n("Show Contact Information..."), "showContactInfo");

        /* FIXME TODO: No support in KActivities.
        actionList.prepend(Kicker::createSeparatorActionItem());

        const QVariantMap &forgetAllAction = Kicker::createActionItem(i18n("Forget All Contacts"), "forgetAll");
        actionList.prepend(forgetAllAction);

        const QVariantMap &forgetAction = Kicker::createActionItem(i18n("Forget Contact"), "forget");
        actionList.prepend(forgetAction);
        */

        return actionList;
    }

    return QVariant();
}

bool RecentContactsModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    Q_UNUSED(argument)

    if (row < 0 || row >= rowCount()) {
        return false;
    }

    if (actionId.isEmpty()) {
        QString id = sourceModel()->data(sourceModel()->index(row, 0), ResultModel::ResourceRole).toString();

        const QList<QAction *> actionList = KPeople::actionsForPerson(id, this);

        if (!actionList.isEmpty()) {
            QAction *chat = 0;

            foreach (QAction *action, actionList) {
                const QVariant &actionType = action->property("actionType");

                if (!actionType.isNull() && actionType.toInt() == KPeople::ActionType::TextChatAction) {
                    chat = action;
                }
            }

            if (chat) {
                chat->trigger();

                return true;
            }
        }

        return false;
    } else if (actionId == "showContactInfo") {
        QString id = sourceModel()->data(sourceModel()->index(row, 0), ResultModel::ResourceRole).toString();

        KPeople::PersonDetailsDialog *view = new KPeople::PersonDetailsDialog(0);
        KPeople::PersonData *data = new KPeople::PersonData(id, view);
        view->setPerson(data);
        view->setAttribute(Qt::WA_DeleteOnClose);
        view->show();
    } else if (actionId == "forget") {
        forget(row);

        return false;
    } else if (actionId == "forgetAll") {
        forgetAll();

        return true;
    }

    return false;
}

void RecentContactsModel::refresh()
{
    QObject *oldModel = sourceModel();

    auto query = UsedResources
                    | RecentlyUsedFirst
                    | Agent("KTp")
                    | Type::any()
                    | Activity::current()
                    | Url::startsWith("ktp");

    ResultModel *model = new ResultModel(query);
    model->setItemCountLimit(15);
    model->fetchMore(QModelIndex());

    // FIXME TODO: Don't wipe entire cache on transactions.
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(buildCache()), Qt::UniqueConnection);
    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(buildCache()), Qt::UniqueConnection);
    connect(model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(buildCache()), Qt::UniqueConnection);

    setSourceModel(model);

    buildCache();

    delete oldModel;
}

void RecentContactsModel::buildCache()
{
    qDeleteAll(m_idToData.values());
    m_idToData.clear();
    m_dataToRow.clear();

    QString id;

    for(int i = 0; i < sourceModel()->rowCount(); ++i) {
        id = sourceModel()->data(sourceModel()->index(i, 0), ResultModel::ResourceRole).toString();

        if (!m_idToData.contains(id)) {
            insertPersonData(id, i);
        }
    }
}

void RecentContactsModel::insertPersonData(const QString& id, int row)
{
    KPeople::PersonData *data = new KPeople::PersonData(id);

    m_idToData[id] = data;
    m_dataToRow[data] = row;

    connect(data, SIGNAL(dataChanged()), this, SLOT(personDataChanged()));
}

void RecentContactsModel::personDataChanged()
{
    KPeople::PersonData *data = static_cast<KPeople::PersonData *>(sender());

    if (m_dataToRow.contains(data)) {
        int row = m_dataToRow[data];

        QModelIndex idx = sourceModel()->index(row, 0);

        emit dataChanged(idx, idx);
    }
}

void RecentContactsModel::forget(int row)
{
    Q_UNUSED(row)

    // FIXME TODO: No support in KActivities.
}

void RecentContactsModel::forgetAll()
{
    // FIXME TODO: FIXME TODO: No support in KActivities.
}