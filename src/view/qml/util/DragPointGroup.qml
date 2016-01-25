/*
  DragPointGroup.qml

  This file is part of the KDAB State Machine Editor Library.

  Copyright (C) 2014-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
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

import QtQuick 2.0

Item {
    id: root

    /// Whether this accepts mouse events
    property bool enabled: true

    /// The model of this group
    property list<DragPoint> points

    /// @see QtQuick.Drag.keys
    property var dragKeys: []

    /// Data that will be passe
    property var dragData

    /**
     * Returns the point at index @p index in the coordinate system
     * of the root QML view
     *
     * @return type:Qt.point
     */
    function pointAt(index) {
        var element = repeater.itemAt(index);
        var x = element.x+element.width/2;
        var y = element.y+element.height/2;
        return Qt.point(x, y);
    }

    /**
     * Emitted whenever one of the drag point delegates is moved
     *
     * Call pointAt(index) in order to find out the current position
     * of the changed drag point
     */
    signal changed(int index)

    /**
     * Emitted whenever one of the drag point delegates
     * is dropped onto a DropArea
     */
    signal dropped(int index, var target)

    Repeater {
        id: repeater

        model: points
        delegate: DragPointDelegate {
            enabled: root.enabled
            dragData: root.dragData
            dragIndex: index

            Drag.keys: root.dragKeys

            x: modelData.x-width/2
            y: modelData.y-height/2

            onXChanged: { if (Drag.active) root.changed(index) }
            onYChanged: { if (Drag.active) root.changed(index) }
            onDropped: { root.dropped(index, target) }
        }
    }

}