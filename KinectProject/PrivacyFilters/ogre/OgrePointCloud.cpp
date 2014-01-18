#include "OgrePointCloud.h"
#include <QDebug>

namespace dai {

OgrePointCloud::OgrePointCloud(const QString& name, const QString& resourceGroup, int numPoints)
{    
    m_pointCloud = Ogre::MeshManager::getSingleton().createManual(name.toStdString(), resourceGroup.toStdString());
    m_subMesh = m_pointCloud->createSubMesh();

    // Shared vertex data between sub meshes
    m_pointCloud->sharedVertexData = new Ogre::VertexData();
    m_pointCloud->sharedVertexData->vertexCount = numPoints;

    // Init my vertex declaration
    m_declaration = m_pointCloud->sharedVertexData->vertexDeclaration;
}

OgrePointCloud::~OgrePointCloud()
{
    Ogre::MeshManager::getSingleton().remove(m_pointCloud->getName());
}

void OgrePointCloud::initialise()
{
    initialiseVertexBuffer();
    initialiseColorBuffer();

    m_subMesh->useSharedVertices = true;
    m_subMesh->operationType = Ogre::RenderOperation::OT_POINT_LIST;
    m_pointCloud->_setBounds(Ogre::AxisAlignedBox(-100, -100, -100, 100, 100, 100), true);
    m_pointCloud->load();
}

void OgrePointCloud::initialiseVertexBuffer()
{
    // Declaration of buffer formats
    m_declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);

    // Create vertex buffer
    m_vertexBuffer = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(m_declaration->getVertexSize(0),
                                                                                    m_pointCloud->sharedVertexData->vertexCount,
                                                                                    Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
    // Bind Buffer
    Ogre::VertexBufferBinding* bind = m_pointCloud->sharedVertexData->vertexBufferBinding;
    bind->setBinding(0, m_vertexBuffer);
}

void OgrePointCloud::initialiseColorBuffer()
{
    // Declaration of buffer formats
    m_declaration->addElement(1, 0, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);

    // Create color buffer
    m_colorBuffer = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR),
                                                                                   m_pointCloud->sharedVertexData->vertexCount,
                                                                                   Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
    // Bind Buffer
    Ogre::VertexBufferBinding* bind = m_pointCloud->sharedVertexData->vertexBufferBinding;
    bind->setBinding(1, m_colorBuffer);
}

void OgrePointCloud::updateVertexPositions(float *points, int size)
{
    float *pPArray = static_cast<float*>(m_vertexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

    for (int i=0; i<size*3; i+=3) {
        pPArray[i] = points[i];
        pPArray[i+1] = points[i+1];
        pPArray[i+2] = points[i+2];
    }

    m_vertexBuffer->unlock();
}

void OgrePointCloud::updateVertexColours(float *colours, int size)
{
    Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
    Ogre::RGBA *pCArray = static_cast<Ogre::RGBA*>(m_colorBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

    for (int i=0, k=0; i<size*3 && k<size; i+=3, k++) {
        rs->convertColourValue(Ogre::ColourValue(colours[i],colours[i+1],colours[i+2]), &pCArray[k]);
    }

    m_colorBuffer->unlock();
}

} // End Namespace
