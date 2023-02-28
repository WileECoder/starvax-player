/*
 * Copyright (c) 2016-2022 WangBin <wbsecg1 at gmail.com>
 * MDK SDK with QOpenGLWindow example
 */
#include "QMDKWindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QKeyEvent>
#include <QStringList>
#include <QScreen>
#include <QWidget>

#include "testableAssert.h"

using namespace MDK_NS;
QMDKWindow::QMDKWindow(QWidget* mainWindow, QWindow *parent) :
   QOpenGLWindow(NoPartialUpdate, parent),
   m_mainWindow(mainWindow),
   m_player(nullptr)
{
   hide();
}

QMDKWindow::~QMDKWindow()
{
   makeCurrent();
   m_player->setVideoSurfaceSize(-1, -1); // cleanup gl renderer resources
}

void QMDKWindow::attachPlayer(mdk::Player *player)
{
   m_player = player;

   m_player->setRenderCallback([this](void*){
      QCoreApplication::instance()->postEvent(this, new QEvent(QEvent::UpdateRequest), INT_MAX);
   });
}

void QMDKWindow::setDecoders(const QStringList &dec)
{
   std::vector<std::string> v;

   foreach (QString d, dec) {
      v.push_back(d.toStdString());
   }

   T_ASSERT( m_player != nullptr);
   m_player->setDecoders(MediaType::Video, v);
}


void QMDKWindow::resizeGL(int w, int h)
{
   m_player->setVideoSurfaceSize( (int)((qreal)w*devicePixelRatio()),
                                  (int)((qreal)h*devicePixelRatio()) );
}


void QMDKWindow::paintGL()
{
   m_player->renderVideo();
}


void QMDKWindow::focusInEvent(QFocusEvent*)
{
   // return focus to owner, so we don't need to handle keystrokes here.
   m_mainWindow->topLevelWidget()->activateWindow();
}
