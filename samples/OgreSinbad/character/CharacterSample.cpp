#include "CharacterSample.h"
#include "character/SinbadCharacterController.h"
#include "common/SdkTrays.h"

Sample_Character::Sample_Character()
{

}

void Sample_Character::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");
    mCamera->setNearClipDistance(5);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
}

void Sample_Character::createScene(void)
{
    // set background and some fog
    mViewport->setBackgroundColour(ColourValue(1.0f, 1.0f, 0.8f));
    mSceneMgr->setFog(Ogre::FOG_LINEAR, ColourValue(1.0f, 1.0f, 0.8f), 0,15, 100);

    // set shadow properties
    mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
    mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
    mSceneMgr->setShadowTextureSize(1024);
    mSceneMgr->setShadowTextureCount(1);

    // disable default camera control so the character can do its own
    mCameraMan->setStyle(OgreBites::CS_MANUAL);

    // use a small amount of ambient lighting
    mSceneMgr->setAmbientLight(ColourValue(0.3, 0.3, 0.3));

    // add a bright light above the scene
    Light* light = mSceneMgr->createLight();
    light->setType(Light::LT_POINT);
    light->setPosition(-10, 40, 20);
    light->setSpecularColour(ColourValue::White);

    // create a floor mesh resource
    MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Plane(Vector3::UNIT_Y, 0),100, 100, 10, 10, true, 1, 10, 10, Vector3::UNIT_Z);

    // create a floor entity, give it a material, and place it at the origin
    Entity* floor = mSceneMgr->createEntity("Floor", "floor");
    floor->setMaterialName("Examples/Rockwall");
    floor->setCastShadows(false);
    mSceneMgr->getRootSceneNode()->attachObject(floor);

    // create our character controller
    mChara = new SinbadCharacterController(mCamera);

    SetupDepthMaterial();
    mDepthPanel = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel","DepthPanel");
    mDepthPanel->setMaterialName("DepthTextureMaterial");
    mDepthPanel->setMetricsMode(Ogre::GMM_RELATIVE);
    mDepthPanel->setWidth(0.25);
    mDepthPanel->setHeight(0.25*m_Height/m_Width);
    mDepthPanel->setHorizontalAlignment(GHA_RIGHT);
    mDepthPanel->setVerticalAlignment(GVA_BOTTOM);
    mDepthPanel->setLeft(-mDepthPanel->getWidth());
    mDepthPanel->setTop(-mDepthPanel->getHeight());

    mTrayMgr->getTraysLayer()->add2D((Ogre::OverlayContainer*)mDepthPanel);

    mDepthPanel->show();
    mTrayMgr->hideLogo();
}

void Sample_Character::destroyScene(void)
{
    // clean up character controller and the floor mesh
    if (mChara) delete mChara;
    MeshManager::getSingleton().remove("floor");
}

bool Sample_Character::frameRenderingQueued(const FrameEvent& evt)
{
    // let character update animations and camera
    mChara->addTime(evt.timeSinceLastFrame);
    return BaseApplication::frameRenderingQueued(evt);
}

bool Sample_Character::keyPressed(const OIS::KeyEvent& evt)
{
    // relay input events to character controller
    if (!mTrayMgr->isDialogVisible()) mChara->injectKeyDown(evt);
    return BaseApplication::keyPressed(evt);
}

bool Sample_Character::keyReleased(const OIS::KeyEvent& evt)
{
    return BaseApplication::keyReleased(evt);
}

bool Sample_Character::mouseMoved(const OIS::MouseEvent& evt)
{
    // relay input events to character controller
    if (!mTrayMgr->isDialogVisible()) mChara->injectMouseMove(evt);
    return BaseApplication::mouseMoved(evt);
}

bool Sample_Character::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
{
    // relay input events to character controller
    if (!mTrayMgr->isDialogVisible()) mChara->injectMouseDown(evt, id);
    return BaseApplication::mousePressed(evt, id);
}

void Sample_Character::SetupDepthMaterial()
{
    // Create the texture
    TexturePtr texture = TextureManager::getSingleton().createManual(
            "MyDepthTexture", // name
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            TEX_TYPE_2D,      // type
            m_Width, m_Height,         // width & height
            0,                // number of mipmaps
            PF_BYTE_BGRA,     // pixel format
            TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);


    // Create a material using the texture
    MaterialPtr material = MaterialManager::getSingleton().create(
            "DepthTextureMaterial", // name
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    material->getTechnique(0)->getPass(0)->createTextureUnitState("MyDepthTexture");
    material->getTechnique(0)->getPass(0)->setSceneBlending(SBT_TRANSPARENT_ALPHA);
}
