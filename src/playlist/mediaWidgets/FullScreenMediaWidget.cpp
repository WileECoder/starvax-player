#include "FullScreenMediaWidget.h"

#include <QMainWindow>
#include <QScreen>
#include "testableAssert.h"

#include "VideoWidget.h"
#include "StillPictureWidget.h"

#include "qdesktopwidget.h"
#include "qapplication.h"



FullScreenMediaWidget::FullScreenMediaWidget( VideoWidget * videoWidget,
                                              StillPictureWidget * pictureWidget,
                                              QMainWindow * owner, bool onTop) :
   m_videoWidget( videoWidget),
   m_pictureWidget( pictureWidget),
   m_owner( owner)
{
   if (onTop)
   {
      /* this is the overlay layer */
      m_videoWidget->setWindowFlags( m_videoWidget->windowFlags() |
                                     Qt::WindowStaysOnTopHint );
      m_pictureWidget->setWindowFlags( m_pictureWidget->windowFlags() |
                                     Qt::WindowStaysOnTopHint );
   }

   hideAll();
}

void FullScreenMediaWidget::setPixmap( const QPixmap & pixmap)
{
   m_pictureWidget->setPixmap( pixmap);
}

void FullScreenMediaWidget::showVideo()
{
   smartShow( m_videoWidget);
   m_pictureWidget->setVisible( false);
}

void FullScreenMediaWidget::showPicture()
{
   m_videoWidget->setVisible( false);
   smartShow( m_pictureWidget);
}

void FullScreenMediaWidget::hidePicture()
{
   m_pictureWidget->setVisible( false);
}

void FullScreenMediaWidget::hideVideo()
{
   m_videoWidget->setVisible( false);
}

void FullScreenMediaWidget::hideAll()
{
   hideVideo();
   hidePicture();
}


void FullScreenMediaWidget::smartShow( QWidget * widget)
{
   int screenId = 0;  /* use default screen by default */

   if (QGuiApplication::screens().size() > 1)
   {
      /* second monitor available. Use it */
      screenId = 1;
   }

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

