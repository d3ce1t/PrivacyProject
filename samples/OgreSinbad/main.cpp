#include "character/CharacterSample.h"
#include <QtGlobal>

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    // Create application object
    Sample_Character app;

    try {
        app.go();
    }
    catch(Ogre::Exception&) {

    }

    return 0;
}
