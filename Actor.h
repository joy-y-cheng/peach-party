#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;
class Player;

class Actor : public GraphObject
{
public:
    Actor(StudentWorld* world, int imageID, int startX, int startY, int dir = right, int depth = 0);
    virtual void doSomething() = 0;
    virtual bool isSquare() const = 0;
    virtual bool canGetHitByVortex() const = 0;
    virtual void hitByVortex();
    StudentWorld* getWorld() const;
    bool isAlive() const;
    void setDead();
    bool sharesCoordsWith(Actor* otherActor) const;
    bool overlapsWith(Actor* otherActor) const;
    
    bool getActivation(int playerNum);
    void changeActivation(int playerNum, bool status);
    void activateOnPlayers(int mustLand);
    virtual void doActivity(Player* player);
    virtual void doActivity2(Player* player);
private:
    bool m_alive;
    StudentWorld* m_world;
    
    bool m_activatedOnPeach;
    bool m_activatedOnYoshi;
};

// MOVER

class Mover : public Actor
{
public:
    Mover(StudentWorld* world, int imageID, int startX, int startY);
    virtual void doSomething() = 0;
    virtual bool canGetHitByVortex() const = 0;
    virtual bool isSquare() const;
    
    bool isWalking() const;
    void setWalking(bool walking);
    
    int getWalkDir() const;
    void setWalkDir(int dir);
    bool canGoInDir(int dir) const;
    int countValidDirs() const;
    void setAutomaticWalkDir();
    int chooseRandomDir() const;
    
    int getTicks() const;
    void changeTicks(int ticks);
    
    void swap(Mover* otherMover);
    virtual void teleport();
private:
    bool m_walking;
    int m_walkDir;
    int m_ticksToMove;
};

// PLAYER

class Player : public Mover
{
public:
    Player(StudentWorld* world, int startX, int startY, int playerNum);
    virtual void doSomething();
    virtual bool canGetHitByVortex() const;
    virtual void teleport();
    
    void setDirectedBySquare();
    bool justLanded() const;
    int squaresToMove() const;
    
    int getCoins() const;
    int changeCoins(int coins);
    void swapCoins(Player* otherPlayer);
    
    int getStars() const;
    int changeStars(int stars);
    void swapStars(Player* otherPlayer);
    
    bool hasVortex() const;
    void changeVortex(bool hasVortex);
private:
    int m_playerNum;
    int m_stars;
    int m_coins;
    bool m_justLanded;
    bool m_transported;
    bool m_hasVortex;
    bool m_directedBySquare;
};

// VORTEX

class Vortex : public Mover
{
public:
    Vortex(StudentWorld* world, int startX, int startY, int dir);
    virtual void doSomething();
    virtual bool canGetHitByVortex() const;
};

// ENEMIES

class Enemy : public Mover
{
public:
    Enemy(StudentWorld* world, int imageID, int startX, int startY, int maxSquares);
    virtual void doSomething();
    virtual bool canGetHitByVortex() const;
    virtual void hitByVortex();
    
    int getPauseCounter() const;
    void changePauseCounter(int pauses);
    
    virtual void doWalkingActivity();
private:
    int m_pauseCounter;
    int m_maxSquaresToMove;
};

class Bowser : public Enemy
{
public:
    Bowser(StudentWorld* world, int startX, int startY);
    virtual void doActivity(Player* player);
    virtual void doWalkingActivity();
};

class Boo : public Enemy
{
public:
    Boo(StudentWorld* world, int startX, int startY);
    virtual void doActivity(Player* player);
};

// SQUARES

class Square : public Actor
{
public:
    Square(StudentWorld* world, int imageID, int startX, int startY, int mustLand, int dir = right);
    virtual void doSomething();
    virtual bool isSquare() const;
    virtual bool canGetHitByVortex() const;
private:
    int m_mustLand;
};

class CoinSquare : public Square
{
public:
    CoinSquare(StudentWorld* world, int startX, int startY, bool grant);
    virtual void doActivity(Player* player);
private:
    bool m_grant;
};

class StarSquare : public Square
{
public:
    StarSquare(StudentWorld* world, int startX, int startY);
    virtual void doActivity(Player* player);
};

class DirSquare : public Square
{
public:
    DirSquare(StudentWorld* world, int startX, int startY, int dir);
    virtual void doActivity(Player* player);
};

class BankSquare : public Square
{
public:
    BankSquare(StudentWorld* world, int startX, int startY);
    virtual void doActivity(Player* player);
    virtual void doActivity2(Player* player);
};

class EventSquare : public Square
{
public:
    EventSquare(StudentWorld* world, int startX, int startY);
    virtual void doActivity(Player* player);
};

class DroppingSquare : public Square
{
public:
    DroppingSquare(StudentWorld* world, int startX, int startY);
    virtual void doActivity(Player* player);
};

#endif // ACTOR_H_
