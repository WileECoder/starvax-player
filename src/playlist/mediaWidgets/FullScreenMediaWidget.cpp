#include "FullScreenMediaWidget.h"

#include <QMainWindow>
#include <QScreen>
#include <QApplication>
#include "testableAssert.h"

#include "StillPictureWidget.h"


FullScreenMediaWidget::FullScreenMediaWidget( QWidget* videoWindow,
                                              StillPictureWidget * pictureWidget,
                                              QMainWindow * owner) :
   m_videoWindow( videoWindow),
   m_pictureWidget( pictureWidget),
   m_owner( owner)
{
   m_videoWindow->setWindowFlags( m_pictureWidget->windowFlags() |
                                  Qt::SplashScreen);
   m_pictureWidget->setWindowFlags( m_pictureWidget->windowFlags() |
                                    Qt::SplashScreen);

   hideAll();
}

void FullScreenMediaWidget::setPixmap( const QPixmap & pixmap)
{
   m_pictureWidget->setPixmap( pixmap);
}

void FullScreenMediaWidget::showVideo()
{
   smartShow( m_videoWindow);
   m_pictureWidget->setVisible( false);
}

void FullScreenMediaWidget::showPicture()
{
   m_videoWindow->setVisible( false);
   smartShow( m_pictureWidget);
}

void FullScreenMediaWidget::hidePicture()
{
   m_pictureWidget->setVisible( false);
}

bool FullScreenMediaWidget::togglePictureVisibility()
{
   m_pictureWidget->isVisible() ? m_pictureWidget->hide() : smartShow( m_pictureWidget);

   return m_pictureWidget->isVisible();
}

void FullScreenMediaWidget::hideVideo()
{
   m_videoWindow->setVisible( false);
}

void FullScreenMediaWidget::hideAll()
{
   hideVideo();
   hidePicture();
}


void FullScreenMediaWidget::setOnTop(bool onTop)
{
   bool pictVisible = m_pictureWidget->isVisible();
   bool videoVisible = m_videoWindow->isVisible();

   if (onTop == true)
   {
      m_videoWindow->setWindowFlags( m_videoWindow->windowFlags() | Qt::WindowStaysOnTopHint);
      m_pictureWidget->setWindowFlags( m_pictureWidget->windowFlags() | Qt::WindowStaysOnTopHint);
   }
   else
   {
      m_videoWindow->setWindowFlags( m_videoWindow->windowFlags() & ( ~ Qt::WindowStaysOnTopHint));
      m_pictureWidget->setWindowFlags( m_pictureWidget->windowFlags() & ( ~ Qt::WindowStaysOnTopHint));
   }

   m_videoWindow->setVisible( videoVisible);
   m_pictureWidget->setVisible( pictVisible);
}


int FullScreenMediaWidget::selectScreen()
{
   int screenId = 0;  /* use default screen by default */

   if (QGuiApplication::screens().size() > 1)
   {
      /* second monitor available. Use it */
      screenId = 1;
   }

   return screenId;
}

void FullScreenMediaWidget::smartShow( QWidget * widget)
{
   int screenId = selectScreen();

   showFullScreen( widget, screenId);

   /* keep focus on main window, otherwise widgets
    * get focus and main window loses keyboard inputs */
   m_owner->activateWindow();
}


void FullScreenMediaWidget::showFullScreen( QWidget * widget, int screenId)
{
   QScreen * screen = QGuiApplication::screens().at( screenId);
   T_ASSERT( screen != nullptr);

   widget->setGeometry( screen->geometry());
   widget->show();
}

