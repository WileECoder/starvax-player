#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>



class VideoWidget : public QWidget
{
   Q_OBJECT
public:
   explicit VideoWidget( QWidget * parent = nullptr);

signals:
   void stopRequest();
   void toggleAudioOnlyRequest();
   void togglePlayPauseRequest();
   void stepForewardRequest();
   void stepBackwardRequest();

   // QWidget interface
protected:
   void keyPressEvent(QKeyEvent * event) override;

};

#endif // VIDEOWIDGET_H
