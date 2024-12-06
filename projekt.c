// JAKUB ROMANOWSKI s203681
// i only used the "RA" macro from ball.c (the example code from Moodle)
// function name 'MainLoop' is also the same, but the function itself is different

#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define STUDENT_DATA "Jakub Romanowski s203681"

#define NUMLEVELS 4         // number of levels

#define ENTER 10            // non-letter keys in more readable form
#define BACKSPACE 123

#define GRASS_COL 1
#define ROAD_COL 2
#define META_COL 3
#define BLACK_COL 4               // defining colors
#define FRIENDLY_CAR_COL 5
#define BUSH_COL 6
#define STORK_GRASS 7
#define STORK_ROAD 8
#define STORK_COL 9

#define BUSH_SYMBOL 'O'
#define PLAYER_SYMBOL '@'               // defining symbols
#define STORK_SYMBOL 'V'
#define CAR '>'

#define Player_SPEED 4                  // defining speeds
#define STORK_SPEED 8

#define ATTACH_KEY 'c'              // defining keys
#define QUITKEY 27                  // 27 == 'ESC'      

#define CAR_CHANGE_SPEED_PROB 100 // chance of car changing speed in a tick
#define CHANGES_SPEED_PROB 3       // chance that a car can change speed at all

#define PLAYWIN_Y 5             // (0,0) dimensions of the window that displays the game
#define PLAYWIN_X 10

#define FRAME_TIME 50       // each frame lasts this many miliseconds
#define CAR_STOP_TIME 1500      // car stops for this many miliseconds to let the player pass the street

#define RA(min, max) ( (min) + rand() % ((max) - (min) + 1) )       //used from ball.c


// -------------OBJECTS OBJECTS OBJECTS-----------------
typedef struct{
    int xPos;
    int yPos;
    int yMax;
    int xMax;
    int lastFrameMoved;
    int isAttached;
    int attachedCarIndex;
    char symbol;

} Player;

typedef struct{
    float time;
    int frame_n;
} Timer;

typedef struct{
    int head;
    int lane;
    int length;
    int speed;
    int stops;
    int isFriendly;
    int holdsPlayer;
    int changesSpeed;
    int index;
    int lastFrameMoved;         //frame at which the car has moved for the last time (used for changing car speeds)
} Car;

typedef struct{
    int lev_num;
    int bush_prob;
    int min_car_len;
    int max_car_len;
    int cars_per_lane;
    int car_min_speed;
    int car_max_speed;
    int car_stops_prob;
    int car_friendly_prob;
    int remove_car_prob;
    int map_height;
    int map_width;
    int isStork;
} Level;

typedef struct{
    int yPos;
    int xPos;
    int lastFrameMoved;
    char lastSymbol;
} Stork;

// ------------------PROTOTYPES-----------------------
void Ranking(WINDOW *win, Player *p, int levnum, char *pname, int score);

void SetGoodColorReplay(WINDOW *win, Player *p, int y, char ch);

// -----------------WINDOW FUNCTIONS-----------------
int GenerateRoads(WINDOW *win, Level l, int yMax, int xMax)     // this function paints every lane the correct color
{                                                               // and returns the number of lanes
    int i, j;
    int lanes=0;
    int bush_prob = l.bush_prob;

    // panting the lanes correct colors
    for(i=0; i < yMax; ++i)
    {
        if(i == 0)
            wattron(win, COLOR_PAIR(META_COL));             // top lane is the meta
        else if(i   %2==0 || i>yMax-3)
            wattron(win, COLOR_PAIR(GRASS_COL));            // even lanes are grass 
        else
            {
                wattron(win, COLOR_PAIR(ROAD_COL));         // uneven lanes are road
                lanes++;                                    // if a road is being created, increase the number of roads
            }  
                     
        for(j=0; j < xMax; ++j)
        {
            // generate bushes of grass lanes, but not on the bottom/meta
            if((RA(0, bush_prob) % bush_prob == 0) && (i%2 == 0) && (i>0) && (i<yMax-3) && (i>1) && (i<xMax))
            {
                wattron(win, COLOR_PAIR(BUSH_COL));
                mvwaddch(win, i, j, BUSH_SYMBOL);
                wattron(win, COLOR_PAIR(GRASS_COL));
            }
            else
                mvwaddch(win, i, j, ' ');       // if not a bush, a ' ' is added
        
        }
    }
    return lanes;
}

void ClearWindow(WINDOW *win, int yMax, int xMax)       // function makes the whole window blank
{
    int i, j;
    wattron(win, COLOR_PAIR(BLACK_COL));
    for(i=0; i < yMax; ++i)
    {
        for(j=0; j < xMax; ++j)
        {
            mvwaddch(win, i, j, ' ');
        }
    }
    wrefresh(win);
}

// what happens if game is over (either WON or LOST)
void GameOver(WINDOW *win, Player *p, Timer t, Level l, int gameResult)
{
    refresh();
    flushinp();
    usleep(1000*1000*1);
    ClearWindow(win, p->yMax, p->xMax);

    // calculate the score
    int score = t.time * 100;
    
    // remove the "game is working!" from the top
    mvprintw(1, 0, "                        ");
    refresh();

    // gameResult == 1      ->     game LOST
    // gameResult == 2      ->     game WON

    // if game lost
    if(gameResult == 1)
        mvwprintw(win, 0, 0, "GAME OVER!                              ");
        
    //if game won
    else if(gameResult == 2)
    {
        char name[] = "_____";          // initial state of the name that will be inputed into the ranking

        mvwprintw(win, 0, 0, "You WON!                                ");
        mvwprintw(win, 1, 0, "SCORE:  %d                         ", score);
        mvwprintw(win, 3, 0, "enter your name...");
        mvwprintw(win, 4, 0, "%s", name);
        wrefresh(win);
        nodelay(win, false);        
        char key;
        int i=0;

        // inputing the name
        while(1)
        {
            flushinp();
            key = wgetch(win);
            if((key == BACKSPACE) && i > 0)               // if BACKSPACE pressed, remove a character from the name     
            {
                name[i-1] = '_';
                i--;
                mvwprintw(win, 4, 0, "%s", name);
                wrefresh(win);
                continue;
            }
            else if((key >= 'a') && (key <= 'z') && i<5)        // characters must be in the alphabet
            {
                name[i] = key -'a' + 'A';                       // make the character uppercase
                mvwprintw(win, 4, 0, "%s", name);
                wrefresh(win);
                i++;
            }
            if(key == 10 && i==5)
                break;
        }
        Ranking(win, p, l.lev_num, name, score);            // upload the name and the score to the ranking
    }

    mvwprintw(win, 7, 0, "Press any key to countinue... ");
    wrefresh(win);    
    flushinp();
    getch();
    // mvwprintw(win, 3, 0, "                               ");
    // mvwprintw(win, 6, 0, "                               ");
    ClearWindow(win, p->yMax, p->xMax);
}

WINDOW *StartLevel(int choice, Level l)
{
    WINDOW *win = newwin(l.map_height, l.map_width, PLAYWIN_Y, PLAYWIN_X);        //PLACEHOLDER VALUES
    //box(win, 0, 0);

    init_pair(GRASS_COL, COLOR_WHITE, COLOR_GREEN);
    init_pair(ROAD_COL, COLOR_WHITE, COLOR_YELLOW);
    init_pair(META_COL, COLOR_WHITE, COLOR_RED);
    init_pair(BLACK_COL, COLOR_WHITE, COLOR_BLACK);
    init_pair(FRIENDLY_CAR_COL, COLOR_BLUE, COLOR_YELLOW);
    init_pair(BUSH_COL, COLOR_BLACK, COLOR_GREEN);
    init_color(STORK_COL, 1000, 1000, 0); // Yellow color with full red and green

    init_pair(STORK_GRASS, STORK_COL, COLOR_GREEN);
    init_pair(STORK_ROAD, STORK_COL, COLOR_YELLOW);

    return win;
}

void AddReplayFrame(WINDOW *win, Player *p, FILE *f)
{
    for(int i=0; i<p->yMax; ++i)
    {
        for(int j=0; j<p->xMax; ++j)
        {
            char ch = (mvwinch(win, i, j) & A_CHARTEXT);
            fprintf(f, "%c", ch);
        }
        fprintf(f, "\n");
    }
}

void PlayReplay(WINDOW *win, Player *p, FILE *f)
{
    f = fopen("replay.txt", "r");
    char ch;
    ClearWindow(win, p->yMax, p->xMax);
    while(1)
    {
        for(int i=0; i<p->yMax; ++i)
        {
            for(int j=0; j<=p->xMax; ++j)
            {   
                fscanf(f, "%c", &ch);
                SetGoodColorReplay(win, p, i, ch);
                mvwaddch(win, i, j, ch);
                // if(i == p->yMax - 1 && j== p->xMax)
                //     return;
            }
        }
        wrefresh(win);
        usleep(1000 * FRAME_TIME);
    }
    fclose(f);
}

// --------------------MENU FUNCTIONS-----------------------
int Choice(int numlevels)
{
    mvprintw(0, 0,"Choose the level:                 ");
    mvprintw(1, 0,"Press ESC to leave the game                                                           ");
    refresh();
    
    // create a menu window with a box around it
    WINDOW *menuwin = newwin(numlevels+2, 20, 5, 10);
    box(menuwin, 0, 0);

    // choosing the level
    int highlight =0;
    while(1)
    {
        for(int i=0; i < numlevels; ++i)
        {
            if(i==highlight)
                wattron(menuwin, A_REVERSE);
            mvwprintw(menuwin, 1+i, 2, "level %d", i);
            wattroff(menuwin, A_REVERSE);
        }
        int key = wgetch(menuwin);
        switch(key)
        {
            case 'w':
                if(highlight > 0)
                    highlight--;
                break;
            case 's':
                if(highlight < numlevels-1)
                    highlight++;
                break;
            case ENTER:                // if ENTER pressed
                delwin(menuwin);
                return highlight;
            case QUITKEY:
                return -1;
            default:
                break;
        }
    }
    return 0;
}

// -------------------COLOR FUNCTIONS--------------------------
// Two similar functions. They set the right color, useful when chaning position of player/stork
// For better understanding, check GenerateRoads function
void SetGoodColor(WINDOW *win, Player *p)
{
    if(p->yPos % 2 == 1 && p->yPos <= p->yMax-3)
        wattron(win, COLOR_PAIR(ROAD_COL));
    else if(p->yPos >0)
        wattron(win, COLOR_PAIR(GRASS_COL));
    else
        wattron(win, COLOR_PAIR(META_COL));
}
void SetGoodColorStork(WINDOW *win, Player *p, Stork *s)
{
    if(s->yPos % 2 == 1 && s->yPos <= p->yMax-3)
        wattron(win, COLOR_PAIR(STORK_ROAD));
    else if(s->yPos >0)
        wattron(win, COLOR_PAIR(STORK_GRASS));
    else
        wattron(win, COLOR_PAIR(META_COL));
}
void SetGoodColorReplay(WINDOW *win, Player *p, int y, char ch)
{
    if(y % 2 == 1 && y <= p->yMax-3)
        {
            if(ch == STORK_SYMBOL)
                wattron(win, COLOR_PAIR(STORK_ROAD));
            else
                wattron(win, COLOR_PAIR(ROAD_COL));
        }

    else if(y >0)
        {
            if(ch == BUSH_SYMBOL)
                wattron(win, COLOR_PAIR(BUSH_COL));
            else if(ch == STORK_SYMBOL)
                wattron(win, COLOR_PAIR(STORK_GRASS));
            else 
            wattron(win, COLOR_PAIR(GRASS_COL));
        }
    else
        {
            wattron(win, COLOR_PAIR(META_COL));
        }
}

// -----------------OBJECT FUNCTIONS------------------------
Player CreatePlayer(int yPos, int xPos, int yMax, int xMax, char symbol)
{
    // create player
    Player p;
    p.yPos = yPos;
    p.xPos = xPos;
    p.yMax = yMax;
    p.xMax = xMax;
    p.lastFrameMoved = 0;
    p.isAttached = 0;
    p.attachedCarIndex = 0;
    p.symbol = symbol;
    return p;
}

Timer CreateTimer()
{
    // create timer
    Timer t;
    t.time = 0;
    t.frame_n = 0;
    return t;
}

Car CreateCar(int head, int lane, int length, int speed, int stops, int isFriendly, int index, int changesSpeed, int lastFrame)
{
    // create a car with given parameters
    Car car;
    car.head = head;
    car.lane = lane;
    car.length = length;
    car.speed = speed;
    car.stops = stops;
    car.isFriendly = isFriendly;
    car.index = index;
    car.holdsPlayer = 0;
    car.changesSpeed = changesSpeed;
    car.lastFrameMoved = lastFrame;

    return car;
}

Car CreateRandomCar(int lane, int lastFrame, int index, Level l)
{
    // create random parameters, return a car with those parameters
    int randomSpeed = RA(l.car_min_speed, l.car_max_speed);
    int randomLength = RA(l.min_car_len, l.max_car_len);
    int randomNotStops = RA(0, l.car_stops_prob)%l.car_stops_prob;
    int randomIsNotFriendly = RA(0, l.car_friendly_prob)%l.car_friendly_prob;
    int randomNotChangesSpeed = RA(0, CHANGES_SPEED_PROB)%CHANGES_SPEED_PROB;
    Car car;
    car.head = 0;
    car.lane = lane;
    car.length = randomLength;
    car.speed = randomSpeed;
    car.stops = !randomNotStops;
    car.isFriendly = !randomIsNotFriendly;
    car.index = index;
    car.holdsPlayer = 0;
    car.changesSpeed = !randomNotChangesSpeed;
    car.lastFrameMoved = lastFrame;
    return car;
}

Stork CreateStork(int yPos, int xPos)
{
    Stork s;
    s.lastFrameMoved = 0;
    s.lastSymbol = ' ';
    s.yPos = yPos;
    s.xPos = xPos;
    //s.lastAttr = COLOR_PAIR(GRASS_COL);
    return s;
}

// this is the array of all cars
Car **CreateCars(int numroads, Level l)
{
    Car **cars = (Car **)malloc(numroads * sizeof(Car *));  // allocate memory for rows of cars' matrix (one row for every road lane)
    for(int i=0; i<numroads; ++i)
    {
        cars[i] = (Car *)malloc(l.cars_per_lane * sizeof(Car));     // numbers of cars in a row should equal car_per_lane (number from config file)

        // creating a random car
        for(int j=0; j<l.cars_per_lane; ++j)
        {
            int randomSpeed = RA(l.car_min_speed, l.car_max_speed);
            int randomLength = RA(l.min_car_len, l.max_car_len);
            int randomHead = RA(j*l.map_width/l.cars_per_lane, (j+1)*l.map_width/l.cars_per_lane - randomLength);   // cars are placed randomly, but this weird equation makes the layou more even
            int randomNotStops = RA(0, l.car_stops_prob)%l.car_stops_prob;
            int randomNotFriendly = RA(0, l.car_friendly_prob)%l.car_friendly_prob;
            int randomNotChangesSpeed = RA(0, CHANGES_SPEED_PROB)%CHANGES_SPEED_PROB;
            cars[i][j] = CreateCar(randomHead, 1+(2*i), randomLength, randomSpeed, !randomNotStops,
                                   !randomNotFriendly, j, randomNotChangesSpeed, 0);
        }
    }
    return cars;
}

// this function creates a level with all the parameters from the inpu
Level CreateLevel(int lev_num, int bush_prob, int min_car_len, int max_car_len, int cars_per_lane, int car_min_speed,
                  int car_max_speed, int car_stops_prob, int car_friendly_prob, int remove_car_prob, int map_height,
                  int map_width, int isStork)
{
    Level l;
    l.lev_num = lev_num;
    l.bush_prob = bush_prob;
    l.min_car_len = min_car_len;
    l.max_car_len = max_car_len;
    l.cars_per_lane = cars_per_lane;
    l.car_min_speed = car_min_speed;
    l.car_max_speed = car_max_speed;
    l.car_stops_prob = car_stops_prob;
    l.car_friendly_prob = car_friendly_prob;
    l.remove_car_prob = remove_car_prob;
    l.map_height = map_height;
    l.map_width = map_width;
    l.isStork = isStork;
    return l;
}

// placing the cars at start
// (otherwise they would be 1 character long at first)
void PlaceCar(WINDOW *win, Car *car, Player *p)
{
    wattron(win, COLOR_PAIR(ROAD_COL));
    for(int i=car->head; i>car->head-car->length+1; --i)
        if(i >= 0)
            mvwaddch(win, car->lane, i, CAR);
}

// car movement
void MoveCar(WINDOW *win, Player *p, Car *car, Timer t, Level l)
{
    // move the car only if enough time has passed
    if(t.frame_n - car->lastFrameMoved>= car->speed)
    {
        car->head++;
        char ch_rear= (mvwinch(win, car->lane, car->head - car->length) & A_CHARTEXT);
        char ch_front= (mvwinch(win, car->lane, car->head) & A_CHARTEXT);
        if((ch_rear!= PLAYER_SYMBOL) && ch_rear!= STORK_SYMBOL)
            mvwaddch(win, car->lane, car->head-car->length, ' ');
        if(car -> head <= p->xMax + car->length)
        {    
            if (car->isFriendly)
                wattron(win, COLOR_PAIR(FRIENDLY_CAR_COL));
            else
                wattron(win, COLOR_PAIR(ROAD_COL));
            if(ch_front != STORK_SYMBOL)
                mvwaddch(win, car->lane, car->head, CAR);
        }
        else
        {
            if(RA(0, l.remove_car_prob) % l.remove_car_prob == 0 && !car->holdsPlayer)
                *car = CreateRandomCar(car->lane, car->lastFrameMoved, car->index, l);
            else
                car->head = -1;
        }
        car->lastFrameMoved = t.frame_n;
    }
    // a small chance of car changing speed during the game
    if((RA(0, CAR_CHANGE_SPEED_PROB)%CAR_CHANGE_SPEED_PROB == 0) && car->changesSpeed)
        car->speed = RA(l.car_min_speed, l.car_max_speed);
}


// function related to moving player while he is attached to the car / while he is being attached
void HandleAttachment(WINDOW *win,Car *car, Player *p, Timer t, char key)
{
    // attach player to the car i he presses a key and is close
    if(car->isFriendly && (p->xPos > car->head-car->length && p->xPos <= car->head) && (p->yPos == car->lane+1)
       && key == ATTACH_KEY && !p->isAttached && t.frame_n - p->lastFrameMoved >= Player_SPEED)
    {
        wattron(win, COLOR_PAIR(GRASS_COL));        // so player does not leave a spot of street's color behind
        mvwaddch(win, p->yPos, p->xPos, ' ');       // remove player from his previous position
        p->isAttached = 1;                          // Player is inside a car (so he cannot be hit by a car or captured by the stork)
        car->holdsPlayer = 1;                       // car is holding the player (the progam needs to know this f.e to not destroy the car)
        p -> attachedCarIndex = car->index;         // so the program knows which car the player is attached to
        p->yPos--;                                  // move player up
        p->lastFrameMoved = t.frame_n + (Player_SPEED);     // add delay to players movement so he cannot jump out instantly
        wattron(win, COLOR_PAIR(FRIENDLY_CAR_COL));
    }

    // if player is attached, move him alongside the car
    if(p->isAttached && (p->attachedCarIndex == car->index) && (p->yPos == car->lane))
    {
        p->xPos = car->head-1;
        wattron(win, COLOR_PAIR(ROAD_COL));
        mvwaddch(win, p->yPos, p->xPos, PLAYER_SYMBOL);
    }
}



//car movement
int DisplayCar(WINDOW *win, Car *car, Player *p, Timer t, char key, Level l)
{
    // chosing the right color
    if(car->isFriendly)
        wattron(win, COLOR_PAIR(FRIENDLY_CAR_COL));
    else
        wattron(win, COLOR_PAIR(ROAD_COL));

    // this loop reassures that when one car is surpassing another, there will be no blank space between them on screen
    for(int i=car->head; i>car->head-car->length+1; --i)
        if(i >= 0 && ((mvwinch(win, car->lane, i) & A_CHARTEXT) != STORK_SYMBOL))       // don't cover the stork - it should be visible all the time
            mvwaddch(win, car->lane, i, CAR);
 
    // some cars stop when the frog is trying to get across the street
    // Checking if player is next to the car, below it
    char ch = (mvwinch(win, car->lane+1, car->head+2) & A_CHARTEXT);
    if((ch == PLAYER_SYMBOL) && car->stops && !car->isFriendly)         // friendly cars don't stop (ironically), so the player can jump inside them easily
    {
        car->lastFrameMoved = t.frame_n + (CAR_STOP_TIME/FRAME_TIME);   // delay cars movement
        car->stops = 0;                                                 // car stops only once for the player (then it gets annoyed) 
        return 0;
    }

    // move the car
    MoveCar(win, p, car, t, l);
    
    // Handle what happens when player wants to attach / is attached
    HandleAttachment(win, car, p, t, key);


    //wrefresh(win);
    return 0;
}


void DisplayTimerInfo(WINDOW *win, Timer *t, int yMax, int hide)
{
    // if the timer should not be hidden, display it, otherwise hide it
    if(!hide)
        mvprintw(yMax+PLAYWIN_Y+2, PLAYWIN_X, "current time:  %.2f   ", t->time);
    else
        mvprintw(yMax+PLAYWIN_Y+2, PLAYWIN_X, "                      ");

    wrefresh(win);
    t->time += (float)FRAME_TIME/1000;  // update the time
    t->frame_n++;                       // update the current frame
}
    
    //------------------PLAYER MOVEMENT FUNCTIONS---------------------
    // the if statements prevent running outside the map or into a bush
void MvPlayerUp(WINDOW *win, Player *p)
{
if((p->yPos > 0) && (mvwinch(win, p->yPos-1, p->xPos) & A_CHARTEXT) != BUSH_SYMBOL)
    {
        SetGoodColor(win, p);
        mvwaddch(win, p->yPos, p->xPos, ' ');
        p->yPos--;
    }
}
void MvPlayerDown(WINDOW *win, Player *p)
{
    if((p->yPos < p->yMax-1) && (mvwinch(win, p->yPos+1, p->xPos) & A_CHARTEXT) != BUSH_SYMBOL)
    {
        SetGoodColor(win, p);
        mvwaddch(win, p->yPos, p->xPos, ' ');
        p->yPos++;
    }
}
void MvPlayerRight(WINDOW *win, Player *p)
{
    if((p->xPos < p->xMax-1) && (mvwinch(win, p->yPos, p->xPos + 1) & A_CHARTEXT) != BUSH_SYMBOL)
    {
        SetGoodColor(win, p);
        mvwaddch(win, p->yPos, p->xPos, ' ');
        p->xPos++;
    }
}
void MvPlayerLeft(WINDOW *win, Player *p)
{
    if((p->xPos > 0) && (mvwinch(win, p->yPos, p->xPos - 1) & A_CHARTEXT) != BUSH_SYMBOL)
    {
        SetGoodColor(win, p);
        mvwaddch(win, p->yPos, p->xPos, ' ');
        p->xPos--;
    }
}
void MvPlayerGeneral(WINDOW *win, Player *p, Timer t, Car **cars, char key)
{
    switch(key)
    {
        case 'w':
            if(!p->isAttached)
            {
            MvPlayerUp(win, p);
            p->lastFrameMoved = t.frame_n;
            }
            break;
        case 's':
            if(!p->isAttached)
            {
            MvPlayerDown(win, p);
            p->lastFrameMoved = t.frame_n;
            }
            break;
        case 'a':
            if(!p->isAttached)
            {
            MvPlayerLeft(win, p);
            p->lastFrameMoved = t.frame_n;
            }
            break;
        case 'd':
            if(!p->isAttached)
            {
            MvPlayerRight(win, p);
            p->lastFrameMoved = t.frame_n;
            }
            break;
        case ATTACH_KEY:
            // this long if statement check if player is attached (only then he can jump out), would not jump into a bush and if he is inside the map's bonds
            if((p->isAttached) && ((mvwinch(win, p->yPos-1, p->xPos) & A_CHARTEXT) != BUSH_SYMBOL) && (p->xPos >=0) && (p->xPos < p->xMax))
            {
                MvPlayerUp(win, p);
                p->isAttached = 0;
                cars[(p->yPos-1)/2][p->attachedCarIndex].holdsPlayer=0;
                p->lastFrameMoved = t.frame_n + Player_SPEED;
            }
            break;
        default:
            break;
    }
}
int DisplayPlayer(WINDOW *win, Player *p, Timer t, Car **cars, char key)
{
    if(t.frame_n - p->lastFrameMoved >= Player_SPEED)       // move only when enough time has passed since the last movement
    {
        MvPlayerGeneral(win, p, t, cars, key);
        if(key == QUITKEY)
            return -1;
    }
    SetGoodColor(win, p);

    // Cars are updated after the player, so if there is a car symbol at player's position before he is displayed, that means
    // he ran into a car in the previous frame
    char ch = (mvwinch(win, p->yPos, p->xPos) & A_CHARTEXT);

    // if there is a car or stork in players position before he is placed, it means the the game has been lost
    if(((ch == CAR) || (ch == STORK_SYMBOL)) && !p->isAttached)
    {
        mvwaddch(win, p->yPos, p->xPos, p->symbol);     // display the player so his last position is known
        wrefresh(win);
        return 1;
    }
    // return 2 if game won
    if(p->yPos == 0)
    {
        mvwaddch(win, p->yPos, p->xPos, p->symbol);
        wrefresh(win);
        return 2; 
    }
    // display player if he is not attached
    // otherwise, he will be displayed by a function that moves a car he is attached to
    if(!p->isAttached)
        mvwaddch(win, p->yPos, p->xPos, p->symbol);

    flushinp();         //to 'unclog' the input
    return 0;
}

// STORK movement
void MoveStork(WINDOW *win, Stork *s, Player *p, Timer t)
{
    if(t.frame_n - s->lastFrameMoved >= STORK_SPEED)
    {
    int yMovement, xMovement;

    if(s->yPos > p->yPos)
        yMovement = -1;
    else if(s->yPos < p->yPos)
        yMovement = 1;
    else
        yMovement = 0;

    if(s->xPos > p->xPos)
        xMovement = -1;
    else if (s->xPos < p->xPos)
        xMovement = 1;
    else
        xMovement = 0;

    // remove stork's trace before moving it
    SetGoodColorStork(win, p, s);
    if(s->lastSymbol == BUSH_SYMBOL)
        wattron(win, COLOR_PAIR(BUSH_COL));
    if(s->lastSymbol != CAR)
        mvwaddch(win, s->yPos, s->xPos, s->lastSymbol);
    else
        mvwaddch(win, s->yPos, s->xPos, ' ');

    // update stork's position, display it
    s->yPos += yMovement;
    s->xPos += xMovement;
    s->lastSymbol = (mvwinch(win, s->yPos, s->xPos) & A_CHARTEXT);
    SetGoodColorStork(win, p, s);
    wattron(win, A_BOLD);
    mvwaddch(win, s->yPos, s->xPos, STORK_SYMBOL);
    wattroff(win, A_BOLD);
    s->lastFrameMoved = t.frame_n;      // update storks timer
    }
}


// READING / WRITING TO FILES

int ReadLevelConfig(int choice, Level *l)      // this function is 1005 characters long as of making this comment. Probably could be done shorter but it works.
{
    char level_name[] = "levelX.txt";
    char level_number = '0'+choice;           // put levels number in place of X
    level_name[5] = level_number;
    FILE *f = fopen(level_name, "r");         // open a file called level[number of level].txt

    // handling errors
    if( f == NULL)
    {
        mvprintw(1, 0, "ERROR: config file not found! Check if a file level%d.txt' exists in the game's folder", choice);
        getch();
        endwin();
        return -1;
    }
    // level's attributes
    int bush_prob;
    int min_car_len;
    int max_car_len;
    int cars_per_lane;
    int car_min_speed;
    int car_max_speed;
    int car_stops_prob;
    int car_friendly_prob;
    int remove_car_prob;
    int map_height;
    int map_width;
    int isStork;
    //skip the first string in a file input, since it's a variable name
    fscanf(f, "%*s%d", &bush_prob);
    fscanf(f, "%*s%d", &min_car_len);
    fscanf(f, "%*s%d", &max_car_len);
    fscanf(f, "%*s%d", &cars_per_lane);
    fscanf(f, "%*s%d", &car_min_speed);
    fscanf(f, "%*s%d", &car_max_speed);
    fscanf(f, "%*s%d", &car_stops_prob);
    fscanf(f, "%*s%d", &car_friendly_prob);
    fscanf(f, "%*s%d", &remove_car_prob);
    fscanf(f, "%*s%d", &map_height);
    fscanf(f, "%*s%d", &map_width);
    fscanf(f, "%*s%d", &isStork);

    *l = CreateLevel(choice, bush_prob, min_car_len, max_car_len, cars_per_lane,
                    car_min_speed, car_max_speed, car_stops_prob, car_friendly_prob,
                    remove_car_prob, map_height, map_width, isStork);

    fclose(f);
}

void Ranking(WINDOW *win, Player *p, int levnum, char *pname, int score)
{
    // similar process as in the function before (ReadLevelConfig)
    char fname[12] = "rankX.txt";
    fname[4] = levnum + '0';
    FILE *f = fopen(fname, "r");
    // handling errors
    if( f == NULL)
    {
        mvprintw(0, 5, "ERROR: config file not found! Check if a file rank%d.txt' exists in the game's folder", levnum);
        endwin();
        return;
    }

    char names[5][6];       // matrix storing top 5 players and their names
    int scores[5];          // array storing top 5 scores
    int score_used = 0;     // whether or not score has been used already (will be useful when printing the ranking out)
    
    //read ranking from file
    for(int i=0; i<5; ++i)
    {
        fscanf(f, "%s %d", names[i], &scores[i]);
    }
    fclose(f);

    // update the ranking
    f = fopen(fname, "w");
    for(int i=0; i<5; ++i)
    {
        if(score < scores[i] && !score_used)
        {
            fprintf(f, "%s %d\n", pname, score);
            score_used = 1;
            i--;
        }
        else
            fprintf(f, "%s %d\n", names[i], scores[i]);
    }
    fclose(f);

    //print the ranking out
    ClearWindow(win, p->yMax, p->xMax);
    mvwprintw(win, 0, 0, "Best scores in level %d:", levnum);
    f = fopen(fname, "r");
    for(int i=0; i<5; ++i)
    {
        char name[6];
        int score;
        fscanf(f, "%s %d", name, &score);
        mvwprintw(win, i+1, 0, "%d. %s: %d\n", i+1, name, score);
    }

    mvwprintw(win, 7, 0, "Press any key to countinue... ");
    fclose(f);
}


// ------------------MAIN LOOP---------------------

int MainLoop(WINDOW *win, Player *player, Timer timer, Car **cars, int numroads, Level l, Stork *s, FILE *f)
{
    char key;
    do
    {
        // function 'DisplayPlayer' returns 0 if game should still be running,
        // 1 if game is lost, 2 if game is won, -1 if player wants to EXIT
        int gameResult = DisplayPlayer(win, player, timer, cars, key);
        if(gameResult == -1)
            return -1;
        
        // if game is won/lost
        else if(gameResult)
        {
            DisplayTimerInfo(stdscr, &timer, player->yMax, gameResult);   
            GameOver(win, player, timer, l, gameResult);
            break;
        }
        // display current time
        DisplayTimerInfo(stdscr, &timer, player->yMax, gameResult);       
        for(int i=0; i<numroads; ++i)
        {
            for(int j=0; j<l.cars_per_lane; ++j)
            {
                DisplayCar(win, &cars[i][j], player, timer, key, l);
            }
        }
        // move the stork, if it should be present in the game
        if(l.isStork)
            MoveStork(win, s, player, timer);

        // a little brute-force aproach. This line is nescesary because cars used to cover the player symbol
        // when he was attached to a car
        if(player->isAttached)
            {
                wattron(win, COLOR_PAIR(ROAD_COL));
                mvwaddch(win, player->yPos, player->xPos, PLAYER_SYMBOL);
            }
        wrefresh(win);
        usleep(1000 * FRAME_TIME);
        AddReplayFrame(win, player, f);
    }while(key = wgetch(win));
    PlayReplay(win, player, f);
}

int main()
{
    while(1)
    {
    //ncurses start
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    start_color();

    // choosing the level
    int choice = Choice(NUMLEVELS);         // choice -> a variable   Choice -> a function
    if(choice == -1)                        //choice returns -1 if QUITKEY pressed
        break;
    Level l;
    if (ReadLevelConfig(choice, &l) == -1)
        continue;

    // making the level window (dimensions taken from the config file)
    WINDOW *levelwin = StartLevel(choice, l);
    int yMax, xMax;                          // level window dimensions
    getmaxyx(levelwin, yMax, xMax);
    nodelay(levelwin, true);                 // disable delay for playability
    int numRoads = GenerateRoads(levelwin, l, yMax, xMax);      //number of road lanes

    // creating player
    Player player;
    player = CreatePlayer(yMax-2, xMax/2, yMax, xMax, PLAYER_SYMBOL);

    // creating timer
    Timer timer;
    timer = CreateTimer();

    // creating stork
    Stork s;
    s = CreateStork(0, 160);

    // creating cars
    Car **cars = CreateCars(numRoads, l);

    // placing cars in their initial position
    for(int i=0; i<numRoads; ++i)
    {
        for(int j=0; j<l.cars_per_lane; ++j)
        {
            PlaceCar(levelwin, &cars[i][j], &player);
        }
    }
    
    // printing some info on the top of the screen
    mvprintw(0, 0,"LEVEL %d             ", choice);
    mvprintw(1, 0,"FROG GAME IS WORKING!        ");
    mvprintw(PLAYWIN_Y + yMax + 5, PLAYWIN_X, STUDENT_DATA);
    refresh();
    wrefresh(levelwin);

    FILE *f = fopen("replay.txt", "w");

    // check i player wants to leave the game (while playing)
    int quit = MainLoop(levelwin, &player, timer, cars, numRoads, l, &s, f);
    if(quit == -1)
        break;
    //getch();
    endwin();
    }

    endwin();
    return 0;
}