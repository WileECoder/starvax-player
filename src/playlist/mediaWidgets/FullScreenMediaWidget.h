#ifndef FULLSCREEN_MEDIA_WIDGET_H
#define FULLSCREEN_MEDIA_WIDGET_H

#include "FullScreenMediaWidget_IF.h"

class QWidget;
class QMDKWindow;
class StillPictureWidget;
class QMainWindow;

namespace mdk {
class Player;
}

/**
 * @brief a widget that hosts a video and still-picture canvas,
 *  with the intelligence to choose the secondary screen, if available.
 *
 * This widget handles its own visibility, according to playing state
 * and video availability of media object.
 */

class FullScreenMediaWidget : public FullScreenMediaWidget_IF
{
public:
   /**
    * @brief FullScreenMediaWidget
    * @param videoWidget
    * @param pictureWidget
    * @param owner - is activated after other widgets are shown to keep keyboard focus.
    * @param onTop - set 'always-on-top' flag to widgets
    */
   explicit FullScreenMediaWidget( QMDKWindow* videoWindow,
                                   StillPictureWidget * pictureWidget,
                                   QMainWindow * owner);
   virtual ~FullScreenMediaWidget() override {}

   // FullScreenMediaWidget_IF interface
public:
   void setPixmap( const QPixmap & pixmap) override;
   void showVideo() override;
   void showPicture() override;

   /** request to hide still image widget */
   void hidePicture() override;

   bool togglePictureVisibility() override;

   /** request to hide video widget */
   void hideVideo() override;

   /** request to hide both video and picture widget */
   void hideAll() override;

   /** set or unset this window as 'Always on top'. This affects both picture and video */
   void setOnTop( bool onTop) override;

   /** attach specific video player */
   void attachPlayer( mdk::Player & player) override;

private:
   QMDKWindow * m_videoWindow;
   StillPictureWidget * m_pictureWidget;
   QMainWindow * m_owner;

private:
   void smartShow( QWidget * widget);
   void smartShow( QMDKWindow * window);
   void showFullScreen( QWidget * widget, int screenId);
   void showFullScreen(QMDKWindow * window, int screenId);
   int selectScreen();
};

#endif // FULLSCREEN_MEDIA_WIDGET_H
