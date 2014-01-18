#ifndef OGREPOINTCLOUD_H
#define OGREPOINTCLOUD_H

#include <QString>
#include "Ogre.h"

namespace dai {

class OgrePointCloud
{
public:
    OgrePointCloud(const QString& name, const QString& resourceGroup, int numPoints);
    virtual ~OgrePointCloud();
    void initialise();
    void updateVertexPositions(float *points, int size);
    void updateVertexColours(float *colours, int size);

protected:
    void initialiseVertexBuffer();
    void initialiseColorBuffer();

private:
    Ogre::MeshPtr m_pointCloud;
    Ogre::SubMesh* m_subMesh;
    Ogre::VertexDeclaration* m_declaration;
    Ogre::HardwareVertexBufferSharedPtr m_vertexBuffer;
    Ogre::HardwareVertexBufferSharedPtr m_colorBuffer;
};

} // End Namespace

#endif // OGREPOINTCLOUD_H
