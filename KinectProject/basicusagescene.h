#ifndef BASICUSAGESCENE_H
#define BASICUSAGESCENE_H

#include <AbstractScene.h>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <NiTE.h>
#include <OpenNI.h>
#include <QObject>
#include <skeleton.h>

class QOpenGLShaderProgram;

class BasicUsageScene : public QObject, public AbstractScene
{
    Q_OBJECT

    Q_PROPERTY(bool drawBoundingBox  READ getDrawBoundingBoxFlag   WRITE setDrawBoundingBoxFlag  NOTIFY changeOfStatus)
    Q_PROPERTY(bool drawCenterOfMass READ getDrawCenterOfMassFlag  WRITE setDrawCenterOfMassFlag NOTIFY changeOfStatus)
    Q_PROPERTY(bool drawSkeleton     READ getDrawSkeletonFlag      WRITE setDrawSkeletonFlag     NOTIFY changeOfStatus)

public:
    explicit BasicUsageScene();
    virtual ~BasicUsageScene();
    void initialise();
    void update(float t);
    void render();
    void resize(float w, float h );
    void setMatrix(QMatrix4x4& m_matrix);
    void setUser(const nite::UserData& m_user);
    void setNativeResolution(int width, int height);
    void setUserTrackerPointer(nite::UserTracker *pUserTracker);
    dai::Skeleton& getSkeleton() {return m_skeleton;}

    // Property Getters
    bool getDrawSkeletonFlag() const {return g_drawSkeleton;}
    bool getDrawCenterOfMassFlag() const {return g_drawCenterOfMass;}
    bool getDrawBoundingBoxFlag() const {return g_drawBoundingBox;}

public slots:
    // Property Setters
    void setDrawSkeletonFlag(bool value);
    void setDrawCenterOfMassFlag(bool value);
    void setDrawBoundingBoxFlag(bool value);

signals:
    void changeOfStatus();

private:
    void prepareShaderProgram();
    void DrawCenterOfMass(nite::UserTracker* pUserTracker, const nite::UserData& m_user);
    void DrawBoundingBox(const nite::UserData& m_user);
    void DrawLimb(nite::UserTracker* pUserTracker, const nite::SkeletonJoint& joint1, const nite::SkeletonJoint& joint2, int color);
    void DrawSkeleton(nite::UserTracker* pUserTracker, const nite::UserData& userData);

    QVector3D               colors[4];
    const int               colorCount;


    nite::UserTracker*      m_pUserTracker;
    QOpenGLShaderProgram*   m_shaderProgram;
    GLuint                  m_posAttr;
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_pointSize;
    QMatrix4x4              m_matrix;
    int                     m_nativeWidth;
    int                     m_nativeHeight;
    float                   m_width;
    float                   m_height;

    dai::Skeleton           m_skeleton;

    // Settings Flags
    bool                    g_drawSkeleton;
    bool                    g_drawCenterOfMass;
    bool                    g_drawBoundingBox;
    nite::UserData          m_user;
};

#endif // BASICUSAGESCENE_H
