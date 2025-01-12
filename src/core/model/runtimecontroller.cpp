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

#include "runtimecontroller.h"

#include "state.h"
#include "ringbuffer.h"
#include "transition.h"

using namespace KDSME;

namespace {

template<typename ContainerType, typename ItemType>
inline qreal relativePosition(const ContainerType& list, const ItemType& t)
{
    const int index = list.indexOf(t);
    return (index+1.0) / list.size();
}

}

struct RuntimeController::Private
{
    Private(RuntimeController* qq)
        : q(qq)
        , m_lastConfigurations(5)
        , m_lastTransitions(5)
        , m_isRunning(false)
    {}

    void updateActiveRegion();

    RuntimeController* q;

    RingBuffer<Configuration> m_lastConfigurations;
    RingBuffer<Transition*> m_lastTransitions;
    bool m_isRunning;
    QRectF m_activeRegion;
};

void RuntimeController::Private::updateActiveRegion()
{
    Configuration configuration = q->activeConfiguration();

    // Calculate the bounding rect of all states in that are currently active
    QRectF activeRegion;
    foreach (State* state, configuration) {
        activeRegion = activeRegion.united(state->boundingRect());
    }
    m_activeRegion = activeRegion;
    emit q->activeRegionChanged(m_activeRegion);
}

RuntimeController::RuntimeController(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    qRegisterMetaType<QSet<State*>>();
}

RuntimeController::~RuntimeController()
{
}

int RuntimeController::historySize() const
{
    return d->m_lastConfigurations.size();
}

void RuntimeController::setHistorySize(int size)
{
    d->m_lastConfigurations.setCapacity(size);
    d->m_lastTransitions.setCapacity(size);
}

QRectF RuntimeController::activeRegion() const
{
    return d->m_activeRegion;
}

void RuntimeController::clear()
{
    d->m_lastConfigurations.clear();
    d->m_lastTransitions.clear();
}

RuntimeController::Configuration RuntimeController::activeConfiguration() const
{
    return (d->m_lastConfigurations.size() > 0 ? d->m_lastConfigurations.last() : Configuration());
}

QList<RuntimeController::Configuration> RuntimeController::lastConfigurations() const
{
    return d->m_lastConfigurations.entries();
}

void RuntimeController::setActiveConfiguration(const RuntimeController::Configuration& configuration)
{
    if (d->m_lastConfigurations.size() > 0 && d->m_lastConfigurations.last() == configuration)
        return;

    d->m_lastConfigurations.enqueue(configuration);
    emit activeConfigurationChanged(configuration);
    d->updateActiveRegion();
}

QList<Transition*> RuntimeController::lastTransitions() const
{
    return d->m_lastTransitions.entries();
}

Transition* RuntimeController::lastTransition() const
{
    return (d->m_lastTransitions.size() > 0 ? d->m_lastTransitions.last() : nullptr);
}

void RuntimeController::setLastTransition(Transition* transition)
{
    if (!transition)
        return;

    d->m_lastTransitions.enqueue(transition);
}

bool RuntimeController::isRunning() const
{
    return d->m_isRunning;
}

void RuntimeController::setIsRunning(bool isRunning)
{
    if (d->m_isRunning == isRunning)
        return;

    d->m_isRunning = isRunning;
    emit isRunningChanged(d->m_isRunning);
}

float RuntimeController::activenessForState(State* state) const
{
    const int count = d->m_lastConfigurations.size();
    for (int i = d->m_lastConfigurations.size()-1; i >= 0; --i) {
        if (d->m_lastConfigurations.at(i).contains(state)) {
            return (i+1.)/count;
        }
    }
    return 0.;
}

float RuntimeController::activenessForTransition(Transition* transition)
{
    return relativePosition<QList<Transition*>>(d->m_lastTransitions.entries(), transition);
}
