#define TEXTURE_SIZE	512
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

#include "SampleViewer.h"
#include <QtGui/QScreen>
#include <QApplication>
#include <iostream>
#include <QElapsedTimer>
#include <QKeyEvent>
#include "OniSampleUtilities.h"

using namespace std;

SampleViewer::SampleViewer()
    : m_streams(NULL), m_eViewState(DEFAULT_DISPLAY_MODE), m_pTexMap(NULL)
{
    initOpenNI();
}

SampleViewer::~SampleViewer()
{
    cout << "End of application" << endl;

    m_device.close();
    openni::OpenNI::shutdown();

    if (m_pTexMap != NULL) {
        delete[] m_pTexMap;
        m_pTexMap = NULL;
    }

    if (m_streams != NULL) {
        delete []m_streams;
        m_streams = NULL;
    }
}

void SampleViewer::initialize()
{
    /*glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);*/

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
}

void SampleViewer::initOpenNI()
{
    const char* deviceURI = openni::ANY_DEVICE;

    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (m_device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

        if (m_depthStream.create(m_device, openni::SENSOR_DEPTH) != openni::STATUS_OK)
            throw 3;

        if (m_colorStream.create(m_device, openni::SENSOR_COLOR) != openni::STATUS_OK)
            throw 4;

        if (m_device.setDepthColorSyncEnabled(true) == openni::STATUS_OK)
            cout << "Frame Sync enabled" << endl;

        if (m_depthStream.start() != openni::STATUS_OK)
            throw 5;

        if (m_colorStream.start() != openni::STATUS_OK)
            throw 6;

        if (!m_depthStream.isValid() || !m_colorStream.isValid())
            throw 7;

        if (m_device.isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR)) {
            cout << "Image Registration is Supported" << endl;
            cout << "Current Mode: " << m_device.getImageRegistrationMode() << endl;
        }

        // Check Resolution
        openni::VideoMode depthVideoMode = m_depthStream.getVideoMode();
        openni::VideoMode colorVideoMode = m_colorStream.getVideoMode();

        int depthWidth = depthVideoMode.getResolutionX();
        int depthHeight = depthVideoMode.getResolutionY();
        int colorWidth = colorVideoMode.getResolutionX();
        int colorHeight = colorVideoMode.getResolutionY();

        if (depthWidth != colorWidth || depthHeight != colorHeight) {
            printf("Error - expect color and depth to be in same resolution: D: %dx%d, C: %dx%d\n",
                                            depthWidth, depthHeight, colorWidth, colorHeight);
            throw 8;
        }

        m_width = depthWidth;
        m_height = depthHeight;
        cout << "Native resolution is " << m_width << "x" << m_height << endl;

        // Init Streams
        m_streams = new openni::VideoStream*[2];
        m_streams[0] = &m_depthStream;
        m_streams[1] = &m_colorStream;

        // Texture map init
        m_nTexMapX = MIN_CHUNKS_SIZE(m_width, TEXTURE_SIZE);
        m_nTexMapY = MIN_CHUNKS_SIZE(m_height, TEXTURE_SIZE);
        m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];
    }
    catch (int ex)
    {
        m_depthStream.destroy();
        m_colorStream.destroy();
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        openni::OpenNI::shutdown();
        throw ex;
    }
}

void SampleViewer::render()
{
    int changedIndex;


    openni::Status rc = openni::OpenNI::waitForAnyStream(m_streams, 2, &changedIndex);

    if (rc != openni::STATUS_OK)
    {
        printf("Wait failed\n");
        return;
    }

    switch (changedIndex)
    {
    case 0:
        m_depthStream.readFrame(&m_depthFrame); break;
    case 1:
        m_colorStream.readFrame(&m_colorFrame); break;
    default:
        printf("Error in wait\n");
    }

    glViewport(0, 0, width(), height());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1.0, 1.0);

    if (m_depthFrame.isValid())
    {
        calculateHistogram(m_pDepthHist, MAX_DEPTH, m_depthFrame);
    }

    memset(m_pTexMap, 0, m_nTexMapX*m_nTexMapY*sizeof(openni::RGB888Pixel));

    // check if we need to draw image frame to texture
    if ((m_eViewState == DISPLAY_MODE_OVERLAY ||
        m_eViewState == DISPLAY_MODE_IMAGE) && m_colorFrame.isValid())
    {
        const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*)m_colorFrame.getData();
        openni::RGB888Pixel* pTexRow = m_pTexMap + m_colorFrame.getCropOriginY() * m_nTexMapX;
        int rowSize = m_colorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel);

        for (int y = 0; y < m_colorFrame.getHeight(); ++y)
        {
            const openni::RGB888Pixel* pImage = pImageRow;
            openni::RGB888Pixel* pTex = pTexRow + m_colorFrame.getCropOriginX();

            for (int x = 0; x < m_colorFrame.getWidth(); ++x, ++pImage, ++pTex)
            {
                *pTex = *pImage;
            }

            pImageRow += rowSize;
            pTexRow += m_nTexMapX;
        }
    }

    // check if we need to draw depth frame to texture
    if ((m_eViewState == DISPLAY_MODE_OVERLAY ||
        m_eViewState == DISPLAY_MODE_DEPTH) && m_depthFrame.isValid())
    {
        const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)m_depthFrame.getData();
        openni::RGB888Pixel* pTexRow = m_pTexMap + m_depthFrame.getCropOriginY() * m_nTexMapX;
        int rowSize = m_depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);

        for (int y = 0; y < m_depthFrame.getHeight(); ++y)
        {
            const openni::DepthPixel* pDepth = pDepthRow;
            openni::RGB888Pixel* pTex = pTexRow + m_depthFrame.getCropOriginX();

            for (int x = 0; x < m_depthFrame.getWidth(); ++x, ++pDepth, ++pTex)
            {
                if (*pDepth != 0)
                {
                    int nHistValue = m_pDepthHist[*pDepth];
                    pTex->r = nHistValue;
                    pTex->g = nHistValue;
                    pTex->b = nHistValue;
                }
            }

            pDepthRow += rowSize;
            pTexRow += m_nTexMapX;
        }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_nTexMapX, m_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap);

    // Display the OpenGL texture map
    glColor4f(1,1,1,1);

    glBegin(GL_QUADS);

    int nXRes = m_width;
    int nYRes = m_height;

    // upper left
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    // upper right
    glTexCoord2f((float)nXRes/(float)m_nTexMapX, 0);
    glVertex2f(width(), 0);
    // bottom right
    glTexCoord2f((float)nXRes/(float)m_nTexMapX, (float)nYRes/(float)m_nTexMapY);
    glVertex2f(width(), height());
    // bottom left
    glTexCoord2f(0, (float)nYRes/(float)m_nTexMapY);
    glVertex2f(0, height());

    glEnd();
}

void SampleViewer::keyPressEvent(QKeyEvent* ev) {
    switch (ev->key())
    {
    case Qt::Key_Escape:
        QApplication::exit();
        break;
    case Qt::Key_1 :
        m_eViewState = DISPLAY_MODE_OVERLAY;
        m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
        break;
    case Qt::Key_2:
        m_eViewState = DISPLAY_MODE_DEPTH;
        m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
        break;
    case Qt::Key_3:
        m_eViewState = DISPLAY_MODE_IMAGE;
        m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_OFF);
        break;
    case Qt::Key_M:
        m_depthStream.setMirroringEnabled(!m_depthStream.getMirroringEnabled());
        m_colorStream.setMirroringEnabled(!m_colorStream.getMirroringEnabled());
        break;
    }
}
