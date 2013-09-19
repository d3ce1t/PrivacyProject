#ifndef PLAYBACKNOTIFIER_H
#define PLAYBACKNOTIFIER_H

#include <QObject>
#include "types/StreamInstance.h"
#include <memory>

using namespace std;

namespace dai {

class PlaybackControl;
class PlaybackWorker;

class PlaybackNotifier : public QObject
{
    Q_OBJECT

public:
    PlaybackNotifier(PlaybackControl* parent, PlaybackWorker* worker);

public slots:
    void notifyListeners(QList<shared_ptr<BaseInstance>> instances);

private:
    PlaybackControl*    m_parent;
    PlaybackWorker*     m_worker;
};

} // End namespace

#endif // PLAYBACKNOTIFIER_H
