#include "PlaybackNotifier.h"
#include "PlaybackControl.h"
#include "PlaybackWorker.h"
#include <QDebug>
#include <QThread>


namespace dai {

PlaybackNotifier::PlaybackNotifier(PlaybackControl* parent, PlaybackWorker* worker)
{
    m_parent = parent;
    m_worker = worker;
}

void PlaybackNotifier::notifyListeners(QList<shared_ptr<BaseInstance> > instances)
{
    m_parent->notifyListeners(instances);
    m_worker->sync();
}

} // End namespace
