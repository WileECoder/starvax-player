#include "VideoWidget.h"
#include "testableAssert.h"

#include <QKeyEvent>
#include <QGridLayout>
#include <QAction>

#include "IF_MediaEngineInterface.h"

VideoWidget::VideoWidget(QWidget *parent) :
   QWidget( parent)
{
   setLayout( new QGridLayout());
   layout()->setContentsMargins(0,0,0,0);

   Qt::WindowFlags flags;

   setStyleSheet("background:black;");

   flags = topLevelWidget()->windowFlags();
   topLevelWidget()->setWindowFlags( flags | Qt::SplashScreen | Qt::FramelessWindowHint );

   hide();

   // let main window keep focus, so it will handle keyboard
   topLevelWidget()->setFocusPolicy( Qt::NoFocus);
   setFocusPolicy( Qt::NoFocus);
   setAttribute( Qt::WA_ShowWithoutActivating);

   // for transparent images to show background
   setAttribute(Qt::WA_TranslucentBackground,true);
   setAutoFillBackground(false);
   setStyleSheet("background:transparent;");

   setContextMenuPolicy( Qt::ActionsContextMenu);

   QAction * a = new QAction("Do something", this);
   this->addAction(a);
}

void VideoWidget::keyPressEvent(QKeyEvent * event)
{
   switch (event->key())
   {
   case Qt::Key_Escape:
      emit stopRequest();
      break;

   case Qt::Key_H:
      emit toggleAudioOnlyRequest();
      break;

   case Qt::Key_Space:
      emit togglePlayPauseRequest();
      break;

   case Qt::Key_PageUp:
      emit stepForewardRequest();
      break;

   case Qt::Key_PageDown:
      emit stepBackwardRequest();
      break;

   default:
      break;
   }
}



