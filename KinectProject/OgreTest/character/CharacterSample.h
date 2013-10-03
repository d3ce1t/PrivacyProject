#ifndef CHARACTERSAMPLE_H
#define CHARACTERSAMPLE_H

#include "SinbadCharacterController.h"
#include "BaseApplication.h"

class Sample_Character : public BaseApplication
{
public:

    friend class SinbadCharacterController;

    Ogre::OverlayElement* mDepthPanel;

    Sample_Character();
    virtual bool frameRenderingQueued(const FrameEvent& evt) override;

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
