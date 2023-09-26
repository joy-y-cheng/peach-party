#include "Actor.h"
#include "GameConstants.h"
#include "StudentWorld.h"
#include <vector>
using namespace std;

// ACTOR IMPLEMENTATION

Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, int dir, int depth)
 : GraphObject(imageID, startX, startY, dir, depth)
{
    m_alive = true;
    m_world = world;
    
    m_activatedOnPeach = false;
    m_activatedOnYoshi = false;
}

void Actor::hitByVortex()
{
}

StudentWorld* Actor::getWorld() const
{
    return m_world;
}

bool Actor::isAlive() const
{
    return m_alive;
}

void Actor::setDead()
{
    m_alive = false;
}

bool Actor::sharesCoordsWith(Actor* otherActor) const
{
    if (getX() == otherActor->getX() && getY() == otherActor->getY())
        return true;
    return false;
}

bool Actor::overlapsWith(Actor* otherActor) const
{
    if (abs(getX() - otherActor->getX()) < 16 && abs(getY() - otherActor->getY()) < 16)
        return true;
    return false;
}

bool Actor::getActivation(int playerNum)
{
    if (playerNum == 1)
        return m_activatedOnPeach;
    return m_activatedOnYoshi;
}

void Actor::changeActivation(int playerNum, bool status)
{
    if (playerNum == 1)
        m_activatedOnPeach = status;
    else
        m_activatedOnYoshi = status;
}

// mustLand of 1 means player must not be in the walking state for activity to occur
// mustLand of 2 means player can be in either the walking state or the waiting to roll state for the activity to occur
// mustland of 3 means different activities occur depending on state of player
void Actor::activateOnPlayers(int mustLand)
{
    for (int playerNum = 1; playerNum <= 2; playerNum++)
    {
        Player* player = (playerNum == 1) ? getWorld()->getPeach() : getWorld()->getYoshi();
        if (!sharesCoordsWith(player))
            changeActivation(playerNum, false);
        if (sharesCoordsWith(player) && !getActivation(playerNum))
        {
            if (mustLand == 1 || mustLand == 3)
            {
                if (!player->isWalking())
                {
                    doActivity(player);
                    changeActivation(playerNum, true);
                }
                // If player is in the walking state and mustLand is 3...
                else if (mustLand == 3)
                {
                    // Do secondary activity (relevant for BankSquare)
                    doActivity2(player);
                    changeActivation(playerNum, true);
                }
            }
            else if (mustLand == 2)
            {
                doActivity(player);
                changeActivation(playerNum, true);
            }
        }
    }
}

void Actor::doActivity(Player* player)
{
}

void Actor::doActivity2(Player* player)
{
}

// MOVER IMPLEMENTATION

Mover::Mover(StudentWorld* world, int imageID, int startX, int startY)
 : Actor(world, imageID, startX, startY)
{
    m_walking = false;
    m_walkDir = right;
    m_ticksToMove = 0;
}

bool Mover::isSquare() const
{
    return false;
}

bool Mover::isWalking() const
{
    return m_walking;
}

void Mover::setWalking(bool walking)
{
    m_walking = walking;
}

int Mover::getWalkDir() const
{
    return m_walkDir;
}

void Mover::setWalkDir(int dir)
{
    m_walkDir = dir;
    if (dir == left)
        setDirection(left);
    else
        setDirection(right);
}

bool Mover::canGoInDir(int dir) const
{
    int xNew, yNew;
    getPositionInThisDirection(dir, 16, xNew, yNew);
    if (getWorld()->squareHasCoordinates(getX(), getY()) && !(getWorld()->squareHasCoordinates(xNew, yNew)))
        return false;
    return true;
}

int Mover::countValidDirs() const
{
    int dirs[4] = {right, left, up, down};
    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        if (canGoInDir(dirs[i]))
            count++;
    }
    return count;
}

void Mover::setAutomaticWalkDir()
{
    if (getWalkDir() == right || getWalkDir() == left)
    {
        if (canGoInDir(up))
            setWalkDir(up);
        else
            setWalkDir(down);
    }
    else
    {
        if (canGoInDir(right))
            setWalkDir(right);
        else
            setWalkDir(left);
    }
}

int Mover::chooseRandomDir() const
{
    int dirs[4] = {right, left, up, down};
    vector<int> validDirs;
    for (int i = 0; i < 4; i++)
    {
        if (canGoInDir(dirs[i]))
            validDirs.push_back(dirs[i]);
    }
    return validDirs[randInt(0, static_cast<int>(validDirs.size()) - 1)];
}

int Mover::getTicks() const
{
    return m_ticksToMove;
}

void Mover::changeTicks(int ticks)
{
    m_ticksToMove += ticks;
}

void Mover::swap(Mover* otherMover)
{
    int tempX = getX();
    int tempY = getY();
    int tempWalking = m_walking;
    int tempTicks = m_ticksToMove;
    int tempWalkDir = m_walkDir;
    int tempDir = getDirection();
    moveTo(otherMover->getX(), otherMover->getY());
    m_walking = otherMover->m_walking;
    m_ticksToMove = otherMover->m_ticksToMove;
    m_walkDir = otherMover->m_walkDir;
    setDirection(otherMover->getDirection());
    otherMover->moveTo(tempX, tempY);
    otherMover->m_walking = tempWalking;
    otherMover->m_ticksToMove = tempTicks;
    otherMover->m_walkDir = tempWalkDir;
    otherMover->setDirection(tempDir);
}

void Mover::teleport()
{
    Actor* randSquare = getWorld()->chooseRandomSquare();
    moveTo(randSquare->getX(), randSquare->getY());
}

// PLAYER IMPLEMENTATION

Player::Player(StudentWorld* world, int startX, int startY, int playerNum)
 : Mover(world, (playerNum == 1) ? IID_PEACH : IID_YOSHI, startX, startY)
{
    m_playerNum = playerNum;
    m_stars = 0;
    m_coins = 0;
    m_hasVortex = false;
    m_justLanded = true;
    m_transported = true;
    m_directedBySquare = false;
}

void Player::doSomething()
{
    m_justLanded = false;
    
    if (!isWalking())
    {
        switch (getWorld()->getAction(m_playerNum))
        {
            case ACTION_ROLL:
            {
                int dieRoll = randInt(1, 10);
                changeTicks(dieRoll * 8);
                setWalking(true);
                break;
            }
            case ACTION_FIRE:
            {
                // Get square directly in front of player
                if (hasVortex())
                {
                    int vortexX;
                    int vortexY;
                    getPositionInThisDirection(getWalkDir(), 16, vortexX, vortexY);
                    getWorld()->shootVortex(vortexX, vortexY, getWalkDir());
                    getWorld()->playSound(SOUND_PLAYER_FIRE);
                    changeVortex(false);
                }
                break;
            }
            default:
                return;
        }
    }
    if (isWalking())
    {
        if (!m_directedBySquare)
        {
            if (getWorld()->squareHasCoordinates(getX(), getY()) && countValidDirs() > 2)
            {
                switch (getWorld()->getAction(m_playerNum))
                {
                    case ACTION_UP:
                    {
                        if (getWalkDir() != down && canGoInDir(up))
                            setWalkDir(up);
                        else
                            return;
                        break;
                    }
                    case ACTION_DOWN:
                    {
                        if (getWalkDir() != up && canGoInDir(down))
                            setWalkDir(down);
                        else
                            return;
                        break;
                    }
                    case ACTION_RIGHT:
                    {
                        if (getWalkDir() != left && canGoInDir(right))
                            setWalkDir(right);
                        else
                            return;
                        break;
                    }
                    case ACTION_LEFT:
                    {
                        if (getWalkDir() != right && canGoInDir(left))
                            setWalkDir(left);
                        else
                            return;
                        break;
                    }
                    default:
                        return;
                }
            }
            if (!canGoInDir(getWalkDir()))
                // Set new walk direction
                setAutomaticWalkDir();
        }
        m_directedBySquare = false;
        
        // Move 2 pixels in the walk direction
        moveAtAngle(getWalkDir(), 2);
        
        // Decrement ticks left
        changeTicks(-1);
        
        if (getTicks() == 0)
        {
            setWalking(false);
            m_justLanded = true;
        }
    }
}

bool Player::canGetHitByVortex() const
{
    return false;
}

void Player::teleport()
{
    Mover::teleport();
    // Set random new valid walk direction
    setWalkDir(chooseRandomDir());
    cerr << "Current random dir: " << getWalkDir() << endl;
}

void Player::setDirectedBySquare()
{
    m_directedBySquare = true;
}

bool Player::justLanded() const
{
    return m_justLanded;
}

int Player::squaresToMove() const
{
    return getTicks() / 8;
}

int Player::getCoins() const
{
    return m_coins;
}

int Player::changeCoins(int coins)
{
    int coinsAdded;
    if (m_coins + coins < 0)
    {
        coinsAdded = -m_coins;
        m_coins = 0;
    }
    else
    {
        coinsAdded = coins;
        m_coins += coins;
    }
    cerr << m_coins << ", " << coinsAdded << " added" << endl;
    return coinsAdded;
}

void Player::swapCoins(Player* otherPlayer)
{
    int tempCoins = m_coins;
    m_coins = otherPlayer->m_coins;
    otherPlayer->m_coins = tempCoins;
}

int Player::getStars() const
{
    return m_stars;
}

int Player::changeStars(int stars)
{
    int starsAdded;
    if (m_stars + stars < 0)
    {
        starsAdded = -m_stars;
        m_stars = 0;
    }
    else
    {
        starsAdded = stars;
        m_stars += stars;
    }
    cerr << m_stars << ", " << starsAdded << " added" << endl;
    return starsAdded;
}

void Player::swapStars(Player* otherPlayer)
{
    int tempStars = m_stars;
    m_stars = otherPlayer->m_stars;
    otherPlayer->m_stars = tempStars;
}

bool Player::hasVortex() const
{
    return m_hasVortex;
}

void Player::changeVortex(bool hasVortex)
{
    m_hasVortex = hasVortex;
}

// VORTEX IMPLEMENTATION

Vortex::Vortex(StudentWorld* world, int startX, int startY, int fireDir)
 : Mover(world, IID_VORTEX, startX, startY)
{
    setWalkDir(fireDir);
}

void Vortex::doSomething()
{
    if (!isAlive())
        return;
    moveAtAngle(getWalkDir(), 2);
    if (getX() < 0 || getX() >= VIEW_WIDTH || getY() < 0 || getY() >= VIEW_HEIGHT)
    {
        cerr << "Passed screen boundaries\n";
        setDead();
    }
    if (getWorld()->checkVortexOverlap(this))
    {
        setDead();
        getWorld()->playSound(SOUND_HIT_BY_VORTEX);
    }
}

bool Vortex::canGetHitByVortex() const
{
    return false;
}

// ENEMY IMPLEMENTATION

Enemy::Enemy(StudentWorld* world, int imageID, int startX, int startY, int maxSquares)
 : Mover(world, imageID, startX, startY)
{
    m_pauseCounter = 180;
    m_maxSquaresToMove = maxSquares;
}

void Enemy::doSomething()
{
    if (!isWalking())
    {
        activateOnPlayers(1);
        changePauseCounter(-1);
        if (getPauseCounter() == 0)
        {
            int squaresToMove = randInt(1, m_maxSquaresToMove);
            changeTicks(squaresToMove * 8);
            setWalkDir(chooseRandomDir());
            setWalking(true);
        }
    }
    if (isWalking())
    {
        if (getWorld()->squareHasCoordinates(getX(), getY()) && countValidDirs() > 2)
            setWalkDir(chooseRandomDir());
        
        if (!canGoInDir(getWalkDir()))
            // Set new walk direction
            setAutomaticWalkDir();
        
        // Move 2 pixels in the walk direction
        moveAtAngle(getWalkDir(), 2);
        
        // Decrement ticks left
        changeTicks(-1);
        
        if (getTicks() == 0)
        {
            setWalking(false);
            changePauseCounter(180);
            doWalkingActivity();
        }
    }
}

bool Enemy::canGetHitByVortex() const
{
    return true;
}

void Enemy::hitByVortex()
{
    cerr << "Actor was hit by vortex\n";
    setWalking(false);
    setWalkDir(right);
    changePauseCounter(-getPauseCounter() + 180);
    teleport();
}

int Enemy::getPauseCounter() const
{
    return m_pauseCounter;
}

void Enemy::changePauseCounter(int pauses)
{
    m_pauseCounter += pauses;
}

void Enemy::doWalkingActivity()
{
}

// BOWSER IMPLEMENTATION

Bowser::Bowser(StudentWorld* world, int startX, int startY)
 : Enemy(world, IID_BOWSER, startX, startY, 10)
{
}

void Bowser::doActivity(Player* player)
{
    int lose = randInt(0, 1);
    if (lose == 1)
    {
        player->changeStars(-player->getStars());
        player->changeCoins(-player->getCoins());
        getWorld()->playSound(SOUND_BOWSER_ACTIVATE);
        cerr << "Bowser robbed " << ((player == getWorld()->getPeach()) ? "Peach" : "Yoshi") << "\n";
    }
}

void Bowser::doWalkingActivity()
{
    // Possibly deposit a dropping
    int dropping = randInt(0, 3);
    if (dropping == 3)
    {
        getWorld()->depositDropping(getX(), getY());
        getWorld()->playSound(SOUND_DROPPING_SQUARE_CREATED);
    }
}

// BOO IMPLEMENTATION

Boo::Boo(StudentWorld* world, int startX, int startY)
: Enemy(world, IID_BOO, startX, startY, 3)
{
}

void Boo::doActivity(Player* player)
{
    int swapItem = randInt(0, 1);
    if (swapItem == 0)
    {
        player->swapCoins(getWorld()->getOtherPlayer(player));
        cerr << "Boo swapped coins\n";
    }
    else
    {
        player->swapStars(getWorld()->getOtherPlayer(player));
        cerr << "Boo swapped stars\n";
    }
    getWorld()->playSound(SOUND_BOO_ACTIVATE);
}

// SQUARE IMPLEMENTATION

Square::Square(StudentWorld* world, int imageID, int startX, int startY, int mustLand, int dir)
 : Actor(world, imageID, startX, startY, dir, 1)
{
    m_mustLand = mustLand;
}

void Square::doSomething()
{
    activateOnPlayers(m_mustLand);
}

bool Square::isSquare() const
{
    return true;
}

bool Square::canGetHitByVortex() const
{
    return false;
}

// COIN SQUARE IMPLEMENTATION

CoinSquare::CoinSquare(StudentWorld* world, int startX, int startY, bool grant)
 : Square(world, (grant == true) ? IID_BLUE_COIN_SQUARE : IID_RED_COIN_SQUARE, startX, startY, 1)
{
    m_grant = grant;
}
 
void CoinSquare::doActivity(Player* player)
{
    int numCoins;
    int sound;
    
    if (m_grant)
    {
        numCoins = 3;
        sound = SOUND_GIVE_COIN;
    }
    else
    {
        numCoins = -3;
        sound = SOUND_TAKE_COIN;
    }
    cerr << "Coins: ";
    player->changeCoins(numCoins);
    getWorld()->playSound(sound);
}

// STAR SQUARE IMPLEMENTATION

StarSquare::StarSquare(StudentWorld* world, int startX, int startY)
 : Square(world, IID_STAR_SQUARE, startX, startY, 2)
{
}

void StarSquare::doActivity(Player* player)
{
    if (player->getCoins() >= 20)
    {
        cerr << "Coins: ";
        player->changeCoins(-20);
        
        cerr << "Stars: ";
        player->changeStars(1);
        
        getWorld()->playSound(SOUND_GIVE_STAR);
    }
}

// DIRECTIONAL SQUARE IMPLEMENTATION

DirSquare::DirSquare(StudentWorld* world, int startX, int startY, int dir)
 : Square(world, IID_DIR_SQUARE, startX, startY, 2, dir)
{
}

void DirSquare::doActivity(Player* player)
{
    player->setWalkDir(getDirection());
    player->setDirectedBySquare();
}

// BANK SQUARE IMPLEMENTATION

BankSquare::BankSquare(StudentWorld* world, int startX, int startY)
 : Square(world, IID_BANK_SQUARE, startX, startY, 3)
{
}

void BankSquare::doActivity(Player* player)
{
    cerr << "Coins: ";
    int coinsToGive = getWorld()->getBank();
    player->changeCoins(coinsToGive);
    
    cerr << "Bank coins: ";
    getWorld()->changeBank(-coinsToGive);
    getWorld()->playSound(SOUND_WITHDRAW_BANK);
}

void BankSquare::doActivity2(Player* player)
{
    cerr << "Coins: ";
    int coinsToAdd = player->changeCoins(-5);
    
    cerr << "Bank coins: ";
    getWorld()->changeBank(-coinsToAdd);
    getWorld()->playSound(SOUND_DEPOSIT_BANK);
}

// EVENT SQUARE IMPLEMENTATION

EventSquare::EventSquare(StudentWorld* world, int startX, int startY)
 : Square(world, IID_EVENT_SQUARE, startX, startY, 1)
{
}

void EventSquare::doActivity(Player* player)
{
    if (player->justLanded())
    {
        int action = randInt(1, 3);
        if (action == 1)
        {
            player->teleport();
            getWorld()->playSound(SOUND_PLAYER_TELEPORT);
            cerr << "Teleported\n";
        }
        else if (action == 2)
        {
            player->swap(getWorld()->getOtherPlayer(player));
            getWorld()->playSound(SOUND_PLAYER_TELEPORT);
            cerr << "Swapped\n";
        }
        else
        {
            player->changeVortex(true);
            getWorld()->playSound(SOUND_GIVE_VORTEX);
            cerr << "Gave a vortex\n";
        }
    }
}

// DROPPING SQUARE IMPLEMENTATION

DroppingSquare::DroppingSquare(StudentWorld* world, int startX, int startY)
 : Square(world, IID_DROPPING_SQUARE, startX, startY, 1)
{
}

void DroppingSquare::doActivity(Player* player)
{
    int action = randInt(1, 2);
    if (action == 1)
    {
        cerr << "Coins: ";
        player->changeCoins(-10);
    }
    else
    {
        cerr << "Stars: ";
        player->changeStars(-1);
    }
    getWorld()->playSound(SOUND_DROPPING_SQUARE_ACTIVATE);
}
