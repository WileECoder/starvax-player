/*  This file is part of the KDE project
    Copyright (C) 2006-2008 Ricardo Villalba <rvm@escomposlinux.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), Nokia Corporation
    (or its successors, if any) and the KDE Free Qt Foundation, which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SWIFTSLIDER_H
#define SWIFTSLIDER_H

#include <QSlider>

/** \class SwiftSlider swiftslider.h
 * \short Modified QSlider that allows sudden/quick moves instead of stepped moves ("Click'n'Go" QSlider)
 *
 * Ricardo Villalba, the original author of MySlider.cpp (from the SMPlayer project)
 * gave his permission for the inclusion of this code inside Phonon by
 * switching MySlider.cpp to the LGPLv2.1+ license. I hope it's no problem if I use it here, too.
 *
 * The original discussion about a "Click'n'Go QSlider":
 * http://lists.trolltech.com/qt-interest/2006-11/msg00363.html
 */
class SwiftSlider : public QSlider
{
   Q_OBJECT
public:
   SwiftSlider(Qt::Orientation orientation, QWidget * parent);
   ~SwiftSlider() override;

private:
   inline int pick(const QPoint &pt) const;
   int pixelPosToRangeValue(int pos) const;
   int mouseEventToRangeValue(const QMouseEvent *ev);

protected:
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *ev) override;
   void focusInEvent(QFocusEvent * event) override;
   void keyReleaseEvent(QKeyEvent * event) override;

   virtual QString formatTip( qint64 value);
   QString formatHandleTip( qint64 value);

private:
   /* value that user is editing by keyboard */
   QString keyEditedValue;
};


#endif //SWIFTSLIDER_H
