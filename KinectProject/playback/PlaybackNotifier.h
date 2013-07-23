#ifndef PLAYBACKNOTIFIER_H
#define PLAYBACKNOTIFIER_H

#include <QObject>
#include "types/StreamInstance.h"
#include <memory>

using namespace std;

namespace dai {

class PlaybackControl;

class PlaybackNotifier : public QObject
{
    Q_OBJECT

public:
    PlaybackNotifier(PlaybackControl* parent);

public slots:
    void notifyListeners(QList<shared_ptr<StreamInstance>> instances);

signals:
    void finished();

private:
    PlaybackControl*    m_parent;
};

} // End namespace

#endif // PLAYBACKNOTIFIER_H
