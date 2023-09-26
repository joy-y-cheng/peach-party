#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Board.h"
#include <string>
#include <list>

class Actor;
class Player;
class Vortex;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    bool squareHasCoordinates(int x, int y);
    Actor* chooseRandomSquare();
    
    Player* getPeach() const;
    Player* getYoshi() const;
    Player* getOtherPlayer(Player* player) const;
    
    int getBank() const;
    void changeBank(int coins);
    
    void depositDropping(int dropX, int dropY);

    void shootVortex(int vortexX, int vortexY, int dir);
    bool checkVortexOverlap(Vortex* vortex);
private:
    std::list<Actor*> m_actorContainer;
    Player* m_peach;
    Player* m_yoshi;
    int m_bank;
};

#endif // STUDENTWORLD_H_
