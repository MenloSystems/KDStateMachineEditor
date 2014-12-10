/*
  quickrecursiveinstantiator.cpp

  This file is part of the KDAB State Machine Editor Library.

  Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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

#include "quickrecursiveinstantiator.h"

#include "objecttreemodel.h"

#include <QAbstractItemModel>
#include "debug.h"
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

using namespace KDSME;

QuickRecursiveInstantiator::QuickRecursiveInstantiator(QQuickItem* parent)
    : QQuickItem(parent)
    , m_model(nullptr)
    , m_delegate(nullptr)
{
}

QList< QObject* > QuickRecursiveInstantiator::rootItems() const
{
    return m_rootItems;
}

QAbstractItemModel* QuickRecursiveInstantiator::model() const
{
    return m_model;
}

void QuickRecursiveInstantiator::setModel(QAbstractItemModel* model)
{
    if (m_model == model) {
        return;
    }
    if (m_model) {
        disconnect(m_model, &QAbstractItemModel::modelReset, this, &QuickRecursiveInstantiator::modelReset);
        disconnect(m_model, &QAbstractItemModel::rowsInserted, this, &QuickRecursiveInstantiator::rowsInserted);
        disconnect(m_model, &QAbstractItemModel::rowsRemoved, this, &QuickRecursiveInstantiator::rowsRemoved);
        disconnect(m_model, &QAbstractItemModel::destroyed, this, &QuickRecursiveInstantiator::modelDestroyed);
    }
    m_model = model;
    if (m_model) {
        connect(m_model, &QAbstractItemModel::modelReset, this, &QuickRecursiveInstantiator::modelReset);
        connect(m_model, &QAbstractItemModel::rowsInserted, this, &QuickRecursiveInstantiator::rowsInserted);
        connect(m_model, &QAbstractItemModel::rowsRemoved, this, &QuickRecursiveInstantiator::rowsRemoved);
        connect(m_model, &QAbstractItemModel::destroyed, this, &QuickRecursiveInstantiator::modelDestroyed);
    }
    emit modelChanged(m_model);
}

QQmlComponent* QuickRecursiveInstantiator::delegate() const
{
    return m_delegate;
}

void QuickRecursiveInstantiator::setDelegate(QQmlComponent* delegate)
{
    if (m_delegate == delegate) {
        return;
    }

    m_delegate = delegate;
    emit delegateChanged(m_delegate);
}

void QuickRecursiveInstantiator::modelReset()
{
    Q_ASSERT(m_model);

    qDeleteAll(m_rootItems);
    m_rootItems.clear();

    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto rootIndex = m_model->index(0, 0);
        m_rootItems << createItems(rootIndex, this);
    }
}

void QuickRecursiveInstantiator::modelDestroyed()
{
    qDeleteAll(m_rootItems);
    m_rootItems.clear();

    m_model = nullptr;
}

QObject* QuickRecursiveInstantiator::createItems(const QModelIndex& index, QObject* parent)
{
    Q_ASSERT(m_delegate);
    Q_ASSERT(m_model);
    Q_ASSERT(index.isValid());

    auto object = index.data(ObjectTreeModel::ObjectRole).value<QObject*>();
    auto creationContext = m_delegate->creationContext();
    auto context = new QQmlContext(creationContext ? creationContext : qmlContext(this));
    context->setContextProperty("object", object);

    auto createdObject = m_delegate->create(context);
    createdObject->setParent(parent);
    if (auto quickItem = qobject_cast<QQuickItem*>(createdObject)) {
        if (auto quickParentItem = qobject_cast<QQuickItem*>(parent)) {
            quickItem->setParentItem(quickParentItem);
        } else {
            quickItem->setParentItem(this);
        }
    }

    // create items for child indices recursively
    for (int i = 0; i < m_model->rowCount(index); ++i) {
        auto childIndex = m_model->index(i, 0, index);
        createItems(childIndex, createdObject);
    }

    return createdObject;
}

void QuickRecursiveInstantiator::rowsInserted(const QModelIndex& parent, int row, int column)
{
    Q_UNUSED(parent)
    Q_UNUSED(row)
    Q_UNUSED(column)
    // TODO: Handle
}

void QuickRecursiveInstantiator::rowsRemoved(const QModelIndex& parent, int row, int column)
{
    Q_UNUSED(parent)
    Q_UNUSED(row)
    Q_UNUSED(column)
    // TODO: Handle
}