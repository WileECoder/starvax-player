#include "FullScreenMediaWidget.h"

#include <QMainWindow>
#include <QScreen>
#include "testableAssert.h"

#include "QMDKWindow.h"
#include "StillPictureWidget.h"

#include "qdesktopwidget.h"
#include "qapplication.h"


FullScreenMediaWidget::FullScreenMediaWidget( QMDKWindow * videoWindow,
                                              StillPictureWidget * pictureWidget,
                                              QMainWindow * owner) :
   m_videoWindow( videoWindow),
   m_pictureWidget( pictureWidget),
   m_owner( owner)
{
   /* this is the overlay layer */
   m_videoWindow->setFlags( m_videoWindow->flags() |
                            Qt::SplashScreen );
   m_pictureWidget->setWindowFlags( m_pictureWidget->windowFlags() |
                                    Qt::SplashScreen );

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

void FullScreenMediaWidget::togglePictureVisibility()
{
   m_pictureWidget->isVisible() ? m_pictureWidget->hide() : smartShow( m_pictureWidget);
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

void FullScreenMediaWidget::attachPlayer(mdk::Player &player)
{
   m_videoWindow->attachPlayer( & player);
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

void FullScreenMediaWidget::smartShow(QMDKWindow* window)
{
   int screenId = selectScreen();

   showFullScreen( window, screenId);

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

void FullScreenMediaWidget::showFullScreen(QMDKWindow* window, int screenId)
{
   QScreen * screen = QGuiApplication::screens().at( screenId);
   T_ASSERT( screen != nullptr);

   window->setGeometry( screen->geometry());
   window->show();
}

