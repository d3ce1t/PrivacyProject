#ifndef SKELETON_H
#define SKELETON_H

#include <QObject>
#include <QVector2D>
#include <NiTE.h>

namespace Dataset
{
    class Skeleton : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QVector2D        jointHead          READ jointHead          NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointNeck          READ jointNeck          NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointLeftShoulder  READ jointLeftShoulder  NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointRightShoulder READ jointRightShoulder NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointLeftElbow     READ jointLeftElbow     NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointRightElbow    READ jointRightElbow    NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointLeftHand      READ jointLeftHand      NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointRightHand     READ jointRightHand     NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointTorso         READ jointTorso         NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointLeftHip       READ jointLeftHip       NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointRightLip      READ jointRightLip      NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointLeftKnee      READ jointLeftKnee      NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointRightKnee     READ jointRightKnee     NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointLeftFoot      READ jointLeftFoot      NOTIFY jointsChanged)
        Q_PROPERTY(QVector2D        jointRightFoot     READ jointRightFoot     NOTIFY jointsChanged)
        Q_PROPERTY(char             state              READ getState           WRITE setState NOTIFY jointsChanged)

    public:
        explicit Skeleton(QObject *parent = 0);
        void setNativeResolution(int width, int height);
        void setResolution(int width, int height);
        QVector2D jointHead() const;
        QVector2D jointNeck() const;
        QVector2D jointLeftShoulder() const;
        QVector2D jointRightShoulder() const;
        QVector2D jointLeftElbow() const;
        QVector2D jointRightElbow() const;
        QVector2D jointLeftHand() const;
        QVector2D jointRightHand() const;
        QVector2D jointTorso() const;
        QVector2D jointLeftHip() const;
        QVector2D jointRightLip() const;
        QVector2D jointLeftKnee() const;
        QVector2D jointRightKnee() const;
        QVector2D jointLeftFoot() const;
        QVector2D jointRightFoot() const;
        char getState() const;

        void setSkeleton(const nite::Skeleton& skeleton, nite::UserTracker* pUserTracker);

    signals:
        void jointsChanged();

    public slots:
        void setState(char value);

    private:
        QVector2D getJoint(nite::JointType type) const;

        nite::Skeleton* m_skeleton;
        nite::UserTracker* m_pUserTracker;
        char m_state; // 0 -> NONE, 1 -> CALIBRATING, 2 -> Tracked
        int                     m_nativeWidth;
        int                     m_nativeHeight;
        float                   m_width;
        float                   m_height;

    };
}

#endif // SKELETON_H
