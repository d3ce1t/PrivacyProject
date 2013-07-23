#include "PlaybackNotifier.h"
#include "PlaybackControl.h"

namespace dai {

PlaybackNotifier::PlaybackNotifier(PlaybackControl* parent)
{
    m_parent = parent;
}

void PlaybackNotifier::notifyListeners(QList<shared_ptr<StreamInstance>> instances)
{
    m_parent->notifyListeners(instances);
    emit finished();
}

} // End namespace
