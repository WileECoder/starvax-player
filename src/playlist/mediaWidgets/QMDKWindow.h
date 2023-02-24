/*
 * Copyright (c) 2016-2020 WangBin <wbsecg1 at gmail.com>
 * MDK SDK with QOpenGLWindow example
 */
#ifndef QMDKWINDOW_H
#define QMDKWINDOW_H

#include <QOpenGLWindow>
#include <memory>
#include "mdk/Player.h"

namespace mdk {
class Player;
}
class QMDKWindow : public QOpenGLWindow
{
   Q_OBJECT
public:
   QMDKWindow( QWidget * mainWindow, QWindow *parent = Q_NULLPTR);
   ~QMDKWindow() override;

   /** attach specific video player.
    *  Must be called before other functions in this module */
   void attachPlayer( mdk::Player * player);

   void setDecoders(const QStringList& dec);

protected:
   void resizeGL(int w, int h) override;
   void paintGL() override;

   // QWindow interface
protected:
   void focusInEvent(QFocusEvent*) override;

private:
   QWidget* m_mainWindow;
   mdk::Player * m_player;
};

#endif // QMDKWINDOW_H
