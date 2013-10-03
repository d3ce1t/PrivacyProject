#ifndef CHARACTERSAMPLE_H
#define CHARACTERSAMPLE_H

#include "BaseApplication.h"

class SinbadCharacterController;

class Sample_Character : public BaseApplication
{
public:

    friend class SinbadCharacterController;

    const unsigned int  m_Width = 640;
    const unsigned int m_Height = 480;

    Ogre::OverlayElement* mDepthPanel;

    Sample_Character();
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) override;

protected:
    virtual void createCamera(void) override;
    virtual void createScene(void) override;
    virtual void destroyScene(void) override;
    virtual bool keyPressed(const OIS::KeyEvent& evt) override;
    virtual bool keyReleased(const OIS::KeyEvent& evt) override;
    virtual bool mouseMoved(const OIS::MouseEvent& evt) override;
    virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id) override;
    void SetupDepthMaterial();
    void cleanupContent();

private:
	SinbadCharacterController* mChara;
};

#endif
