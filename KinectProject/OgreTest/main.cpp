#include "character/CharacterSample.h"

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    // Create application object
    Sample_Character app;

    try {
        app.go();
    } catch( Ogre::Exception& e) {

    }

    return 0;
}
