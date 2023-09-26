#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
 : GameWorld(assetPath)
{
    m_peach = nullptr;
    m_yoshi = nullptr;
    m_bank = 0;
}

StudentWorld::~StudentWorld()
{
    cleanUp();
}

int StudentWorld::init()
{
    Board bd;
    
    // Get filepath to board data file
    ostringstream oss;
    oss << assetPath() << "board0" << getBoardNumber() << ".txt";
    string board_file = oss.str();
    
    // Load board
    Board::LoadResult result = bd.loadBoard(board_file);
    if (result == Board::load_fail_file_not_found)
    {
        cerr << "Could not find data file\n";
        return GWSTATUS_BOARD_ERROR;
    }
    else if (result == Board::load_fail_bad_format)
    {
        cerr << "Your board was improperly formatted\n";
        return GWSTATUS_BOARD_ERROR;
    }
    cerr << "Successfully loaded board\n";
    
    // Populate board with actors
    for (int i = 0; i < BOARD_WIDTH; i++)
    {
        for (int j = 0; j < BOARD_HEIGHT; j++)
        {
            Board::GridEntry ge = bd.getContentsOf(i, j);
            switch (ge)
            {
                case Board::player:
                {
                    m_peach = new Player(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, 1);
                    m_actorContainer.push_back(m_peach);
                    m_yoshi = new Player(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, 2);
                    m_actorContainer.push_back(m_yoshi);
                    m_actorContainer.push_back(new CoinSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, true));
                    break;
                }
                case Board::blue_coin_square:
                {
                    m_actorContainer.push_back(new CoinSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, true));
                    break;
                }
                case Board::red_coin_square:
                {
                    m_actorContainer.push_back(new CoinSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, false));
                    break;
                }
                case Board::star_square:
                {
                    m_actorContainer.push_back(new StarSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j));
                    break;
                }
                case Board::up_dir_square:
                {
                    m_actorContainer.push_back(new DirSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, DirSquare::up));
                    break;
                }
                case Board::down_dir_square:
                {
                    m_actorContainer.push_back(new DirSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, DirSquare::down));
                    break;
                }
                case Board::left_dir_square:
                {
                    m_actorContainer.push_back(new DirSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, DirSquare::left));
                    break;
                }
                case Board::right_dir_square:
                {
                    m_actorContainer.push_back(new DirSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, DirSquare::right));
                    break;
                }
                case Board::bank_square:
                {
                    m_actorContainer.push_back(new BankSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j));
                    break;
                }
                case Board::event_square:
                {
                    m_actorContainer.push_back(new EventSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j));
                    break;
                }
                case Board::bowser:
                {
                    m_actorContainer.push_back(new Bowser(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j));
                    m_actorContainer.push_back(new CoinSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, true));
                    break;
                }
                case Board::boo:
                {
                    m_actorContainer.push_back(new Boo(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j));
                    m_actorContainer.push_back(new CoinSquare(this, SPRITE_WIDTH * i, SPRITE_HEIGHT * j, true));
                    break;
                }
                case Board::empty:
                    break;
            }
        }
    }
	startCountdownTimer(99);
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    // Ask all actors to do something
    list<Actor*>::iterator p = m_actorContainer.begin();
    while (p != m_actorContainer.end())
    {
        (*p)->doSomething();
        p++;
    }
    
    // Remove inactive/dead game objects
    list<Actor*>::iterator d = m_actorContainer.begin();
    while (d != m_actorContainer.end())
    {
        if (!(*d)->isAlive())
        {
            delete *d;
            d = m_actorContainer.erase(d);
            cerr << "Deleted object\n";
        }
        else
            d++;
    }
    
    // Determine if Peach and Yoshi have vortexes
    string peachVortex = "";
    if (m_peach->hasVortex())
        peachVortex = " VOR";
    string yoshiVortex = "";
    if (m_yoshi->hasVortex())
        yoshiVortex = " VOR";
    
    // Update text
    ostringstream oss;
    oss << "P1 Roll: " << m_peach->squaresToMove() << " Stars: " << m_peach->getStars() << " $$: " << m_peach->getCoins() << peachVortex << " | Time: " << timeRemaining() << " | Bank: " << m_bank << " | P2 Roll: " << m_yoshi->squaresToMove() << " Stars: " << m_yoshi->getStars() << " $$: " << m_yoshi->getCoins() << yoshiVortex;
    string text = oss.str();
    setGameStatText(text);
    
    // Check if game is over
    if (timeRemaining() <= 0)
    {
        playSound(SOUND_GAME_FINISHED);
        
        // Determine winner
        int winner;
        if (m_peach->getStars() > m_yoshi->getStars())
            winner = 1;
        else if (m_yoshi->getStars() > m_peach->getStars())
            winner = 2;
        else
        {
            if (m_peach->getCoins() > m_yoshi->getCoins())
                winner = 1;
            else if (m_yoshi->getCoins() > m_peach->getCoins())
                winner = 2;
            else
                winner = randInt(1, 2);
        }
        
        // Set final score and return winner
        if (winner == 1)
        {
            setFinalScore(m_peach->getStars(), m_peach->getCoins());
            return GWSTATUS_PEACH_WON;
        }
        else
        {
            setFinalScore(m_yoshi->getStars(), m_yoshi->getCoins());
            return GWSTATUS_YOSHI_WON;
        }
    }
    
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    // Remove all actors
    list<Actor*>::iterator p = m_actorContainer.begin();
    while (p != m_actorContainer.end())
    {
        delete *p;
        p = m_actorContainer.erase(p);
    }
}

bool StudentWorld::squareHasCoordinates(int x, int y)
{
    list<Actor*>::iterator p = m_actorContainer.begin();
    while (p != m_actorContainer.end())
    {
        // If a square's coordinates match arguments, return true
        if ((*p)->isSquare() && (*p)->getX() == x && (*p)->getY() == y)
            return true;
        p++;
    }
    return false;
}

Actor* StudentWorld::chooseRandomSquare()
{
    // Create a temporary vector of just squares
    list<Actor*>::iterator p = m_actorContainer.begin();
    vector<Actor*> squareContainer;
    while (p != m_actorContainer.end())
    {
        if ((*p)->isSquare())
            squareContainer.push_back(*p);
        p++;
    }
    // Choose and return random square
    int squareIndex = randInt(1, static_cast<int>(squareContainer.size()));
    return squareContainer[squareIndex - 1];
}

Player* StudentWorld::getPeach() const
{
    return m_peach;
}

Player* StudentWorld::getYoshi() const
{
    return m_yoshi;
}

Player* StudentWorld::getOtherPlayer(Player* player) const
{
    if (player == m_peach)
        return m_yoshi;
    return m_peach;
}

int StudentWorld::getBank() const
{
    return m_bank;
}

void StudentWorld::changeBank(int coins)
{
    m_bank += coins;
    cerr << m_bank << endl;
}

void StudentWorld::depositDropping(int dropX, int dropY)
{
    list<Actor*>::iterator p = m_actorContainer.begin();
    while (p != m_actorContainer.end())
    {
        // If a square's coordinates match arguments, delete it
        if ((*p)->isSquare() && (*p)->getX() == dropX && (*p)->getY() == dropY)
            {
                delete *p;
                m_actorContainer.erase(p);
                break;
            }
        p++;
    }
    m_actorContainer.push_front(new DroppingSquare(this, dropX, dropY));
}

void StudentWorld::shootVortex(int vortexX, int vortexY, int dir)
{
    m_actorContainer.push_front(new Vortex(this, vortexX, vortexY, dir));
}

bool StudentWorld::checkVortexOverlap(Vortex* vortex)
{
    list<Actor*>::iterator p = m_actorContainer.begin();
    while (p != m_actorContainer.end())
    {
        if ((*p)->canGetHitByVortex() && (*p)->overlapsWith(vortex))
        {
            (*p)->hitByVortex();
            return true;
        }
        p++;
    }
    return false;
}
