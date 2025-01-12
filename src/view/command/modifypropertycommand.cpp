/*
  This file is part of the KDAB State Machine Editor Library.

  Copyright (C) 2014-2019 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
  All rights reserved.
  Author: Kevin Funk <kevin.funk@kdab.com>

  Licensees holding valid commercial KDAB State Machine Editor Library
  licenses may use this file in accordance with the KDAB State Machine Editor
  Library License Agreement provided with the Software.

  This file may be distributed and/or modified under the terms of the
  GNU Lesser General Public License version 2.1 as published by the
  Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.

  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

  Contact info@kdab.com if any conditions of this licensing are not
  clear to you.
*/

#include "modifypropertycommand_p.h"

#include "debug.h"
#include <QJsonObject>
#include <QVariant>

using namespace KDSME;

ModifyPropertyCommand::ModifyPropertyCommand(QObject* object, const char* property,
                                             const QVariant& value, const QString& text, QUndoCommand* parent)
    : Command(text, parent)
    , m_object(object)
{
    m_propertyMap.insert(property, value);
    init();
}

ModifyPropertyCommand::ModifyPropertyCommand(QObject* object, const QJsonObject& propertyMap,
                                             const QString& text, QUndoCommand* parent)
    : Command(text, parent)
    , m_object(object)
{
    auto it = propertyMap.constBegin();
    while (it != propertyMap.constEnd()) {
        m_propertyMap.insert(it.key().toLatin1(), it.value().toVariant());
        ++it;
    }
    init();
}

void ModifyPropertyCommand::init()
{
    if (text().isEmpty() && !m_propertyMap.isEmpty()) {
        if (m_propertyMap.size() == 1) {
            setText(tr("Modify property '%1'").arg(m_propertyMap.begin().value().toString()));
        } else {
            setText(tr("Modify multiple properties"));
        }
    } else {
        setText(tr("N/A"));
    }
}

void ModifyPropertyCommand::redo()
{
    if (!m_object) {
        qCDebug(KDSME_VIEW) << "Invalid object";
        return;
    }

    auto it = m_propertyMap.constBegin();
    while (it != m_propertyMap.constEnd()) {
        m_oldPropertyMap.insert(it.key(), m_object->property(it.key()));
        if (!m_object->setProperty(it.key(), it.value())) {
            qCDebug(KDSME_VIEW) << "Failed to set property" << it.key();
        }
        ++it;
    }
}

void ModifyPropertyCommand::undo()
{
    if (!m_object) {
        qCDebug(KDSME_VIEW) << "Invalid object";
        return;
    }

    auto it = m_oldPropertyMap.constBegin();
    while (it != m_oldPropertyMap.constEnd()) {
        if (!m_object->setProperty(it.key(), it.value())) {
            qCDebug(KDSME_VIEW) << "Failed to set property" << it.key();
        }
        ++it;
    }
    m_oldPropertyMap.clear();
}
