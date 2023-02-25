#ifndef MEDIAENGINE_MDK_H
#define MEDIAENGINE_MDK_H

#include "IF_MediaEngineInterface.h"
#include "AbstractMediaSource.h"

#include <QTimer>
#include <mdk/Player.h>

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
   void setLoopPlayback( bool enabled) override;
   void enableSubtitles() override;
   void disableSubtitles() override;

/* internal signals */
signals:
   void int_mediaStatusChanged( mdk::MediaStatus newStatus);
   void int_playerStateChanged( mdk::State newState);
   void int_videoAvailableChanged( bool available);

private slots:
   void onTimerTick();
   void onMediaStatusChanged( mdk::MediaStatus newStatus);
   void onPlayerStateChanged( mdk::State newState);
   void onDurationChanged( int64_t duration_ms);
   void onVideoAvailable( bool available);
   void onAudioOnlyRequest();

private:
   void int_stop();
   void evaluateDisplayShow();
   void deletePixmap();

private:
   Fader & m_fader;
   StatusDisplay & m_logger;
   QTimer m_tickTimer;
   FullScreenMediaWidget_IF & m_displayWidget;
   mdk::Player m_player;
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
};


#endif // MEDIAENGINE_MDK_H
