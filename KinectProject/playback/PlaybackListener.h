#ifndef PLAYBACKLISTENER_H
#define PLAYBACKLISTENER_H

#include <memory>
#include <QObject>
#include <types/DataFrame.h>

using namespace std;

namespace dai {

class PlaybackControl;

class PlaybackListener : public QObject
{
    Q_OBJECT

    friend class PlaybackControl;

public:
    PlaybackListener();
    virtual ~PlaybackListener();
    PlaybackControl* playback();

public slots:
    void manageFrames(QList<shared_ptr<DataFrame>> frames);

protected:
    virtual void onNewFrame(const QList<shared_ptr<DataFrame>>& frames) = 0;
    virtual void onPlaybackStart() = 0;
    virtual void onPlaybackStop() = 0;

private:
    void setPlayback(PlaybackControl* playback);

    PlaybackControl* m_playback;
};

} // End Namespace

#endif // PLAYBACKLISTENER_H
