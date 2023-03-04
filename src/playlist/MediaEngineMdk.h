#ifndef MEDIAENGINE_MDK_H
#define MEDIAENGINE_MDK_H

#include "IF_MediaEngineInterface.h"
#include "AbstractMediaSource.h"

#include <QTimer>
#include <QMediaPlayer>


class Fader;
class FullScreenMediaWidget_IF;
class ApplicationSettings;
class StatusDisplay;
class QPixmap;


/** this class will aggregate specific objects for plugin used
 *  in implementation. As it is not easily testable, implementation
 *  should be simple and with as few logic as possible.
 */
class MediaEngineMdk : public IF_MediaEngineInterface
{
   Q_OBJECT
public:
   explicit MediaEngineMdk(Fader & fader,
                           FullScreenMediaWidget_IF & displayWidget,
                           StatusDisplay & logger,
                           QObject *parent = nullptr);
   ~MediaEngineMdk() override;


public slots:
   void checkPlatform() override;
   void setCurrentSource( const AbstractMediaSource *source, bool dontStopFlag = false ) override;
   void play() override;
   void pause() override;
   void togglePlayPause() override;
   void stop() override;
   void rewind() override;
   void setStepSizeMs( int stepMs) override;
   void stepForward() override;
   void stepBackward() override;
   void singleFrameForward() override;
   void singleFrameBackward() override;
   void setVolume(int vol) override;
   int volume() override;
   void setTickInterval(qint32 tick_ms ) override;
   void onUserPositionRequested( qint64 positionMs) override;
   void setMuted(bool isMuted) override;
   void setAudioOnly(bool audioOnly) override;
   void showOnTop( bool onTop) override;
   void enableFadeIn( bool enabled) override;
   void setLoopPlayback(bool) override;
   void enableSubtitles() override;
   void disableSubtitles() override;

/* internal signals */
signals:
   void int_videoAvailableChanged( bool available);
   void int_mediaError();

private slots:
   void onTimerTick();
   void onDurationChanged( int64_t duration_ms);
   void onVideoAvailable( bool available);
   void onAudioOnlyRequest();
   void onMediaError();

private:
   void int_stop();
   void evaluateDisplayShow();
   void deletePixmap();

private:
   Fader & m_fader;
   StatusDisplay & m_logger;
   FullScreenMediaWidget_IF & m_displayWidget;
   bool m_videoTrackAvailable;  /* current media has a video track */
   bool m_audioOnlyRequest;  /* user has requested to hide video */
   qint32 m_tickMs;
   int m_stepSizeMs;
   bool m_fadeInFlag;
   bool m_imageFileFlag;
   QPixmap * m_pixmap;
   QString m_currentMediaPath;
   /* requested by GUI or user, not returned by player */
   MediaObject::AvPlayerState m_requestedState;
   QMediaPlayer m_player;
};


#endif // MEDIAENGINE_MDK_H
