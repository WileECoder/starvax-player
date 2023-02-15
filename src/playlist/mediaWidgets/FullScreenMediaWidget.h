#ifndef FULLSCREEN_MEDIA_WIDGET_H
#define FULLSCREEN_MEDIA_WIDGET_H

#include "FullScreenMediaWidget_IF.h"

class QWidget;
class VideoWidget;
class StillPictureWidget;
class QMainWindow;

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
   explicit FullScreenMediaWidget( VideoWidget * videoWidget,
                                   StillPictureWidget * pictureWidget,
                                   QMainWindow * owner, bool onTop=false);
   virtual ~FullScreenMediaWidget() override {}

   // FullScreenMediaWidget_IF interface
public:
   void setPixmap( const QPixmap & pixmap) override;
   void showVideo() override;
   void showPicture() override;

   /** request to hide still image widget */
   void hidePicture() override;

   /** request to hide video widget */
   void hideVideo() override;

   /** request to hide both video and picture widget */
   void hideAll() override;


private:
   VideoWidget * m_videoWidget;
   StillPictureWidget * m_pictureWidget;
   QMainWindow * m_owner;

private:
   void smartShow( QWidget * widget);
   void showFullScreen( QWidget * widget, int screenId);
};

#endif // FULLSCREEN_MEDIA_WIDGET_H
