#ifndef MEDIAENGINE_MDKSDL_H
#define MEDIAENGINE_MDKSDL_H

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
// TODO the name is now wrong !!! remove SDL!
class MediaEngineMdkSdl : public IF_MediaEngineInterface
{
   Q_OBJECT
public:
   explicit MediaEngineMdkSdl(Fader & fader,
                              FullScreenMediaWidget_IF & displayWidget,
                              StatusDisplay & logger,
                              QObject *parent = nullptr);
   ~MediaEngineMdkSdl() override;

   /** used to bind a "screen" widget to media engine implementation */
   void setWidgetForRender(QWidget * canvas) override;

public slots:
   void checkPlatform() override;
   void setCurrentSource( const AbstractMediaSource *source, bool dontStopFlag = false ) override;
   void play() override;
   void pause() override;
   void togglePlayPause() override;
   void stop() override;
   void rewind() override;
   void setStepSizeMs( int stepMs);
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
   void enableFadeIn( bool enabled) override;
   void setLoopPlayback( bool enabled) override;
   void enableSubtitles() override;
   void disableSubtitles() override;

private slots:
   void onTimerTick();
   void onMediaStatusChanged( mdk::MediaStatus newStatus);
   void onPlayerStateChanged( mdk::State newState);
   void onDurationChanged( int64_t duration_ms);
   void onVideoAvailable( bool available);
   void onStopAllRequest();
   void onAudioOnlyRequest();


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
};


#endif // MEDIAENGINE_MDKSDL_H
