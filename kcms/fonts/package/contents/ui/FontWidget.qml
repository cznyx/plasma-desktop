/*
   Copyright (c) 2015 Antonis Tsiapaliokas <antonis.tsiapaliokas@kde.org>
   Copyright (c) 2017 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as QtControls
import QtQuick.Dialogs 1.2 as QtDialogs
import org.kde.kirigami 2.3 as Kirigami
import org.kde.kcm 1.0


FocusScope {
    id: root
    property string label
    property string category
    property font font
    Kirigami.FormData.label: root.label
    activeFocusOnTab: true

    Layout.minimumWidth: layout.Layout.minimumWidth
    Layout.preferredWidth: layout.Layout.preferredWidth
    Layout.minimumHeight: layout.Layout.minimumHeight
    Layout.preferredHeight: layout.Layout.preferredHeight
    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    RowLayout {
        id: layout

        QtControls.TextField {
            enabled: false
            text: root.font.family + " " + root.font.pointSize
            font: root.font
            Layout.fillHeight: true
        }

        QtControls.Button {
            text: i18n("Choose...")
            Layout.fillHeight: true
            Kirigami.MnemonicData.enabled: false
            focus: true
            onClicked: {
                fontDialog.adjustAllFonts = false;
                fontDialog.currentCategory = root.category
                fontDialog.font = root.font;
                fontDialog.currentFont = root.font;
                fontDialog.open()
            }
        }
    }
}

