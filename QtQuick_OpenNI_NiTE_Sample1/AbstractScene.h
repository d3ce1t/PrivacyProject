#ifndef ABSTRACTSCENE_H
#define ABSTRACTSCENE_H

class AbstractScene
{
public:
    virtual void initialise() = 0;
    virtual void update(float t) = 0;
    virtual void render() = 0;
    virtual void resize( int w, int h ) = 0;
};

#endif // ABSTRACTSCENE_H
