//JAKUB ROMANOWSKI

#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define STUDENT_DATA "Jakub Romanowski s203681"

#define NUMLEVELS 4
#define ENTER 10

#define GRASS_COL 1
#define ROAD_COL 2
#define META_COL 3
#define BLACK_COL 4
#define FRIENDLY_CAR_COL 5
#define BUSH_COL 6

#define BUSH_SYMBOL 'O'
#define PLAYER_SYMBOL '@'
#define Player_SPEED 4
#define ATTACH_KEY 'c'

#define CAR_CHANGE_SPEED_PROB 100 // chance of car changing speed in a tick
#define CHANGES_SPEED_PROB 3       // chance that a car can change speed at all

#define PLAYWIN_Y 5
#define PLAYWIN_X 10

#define FRAME_TIME 50       //MILISECONDS

#define RA(min, max) ( (min) + rand() % ((max) - (min) + 1) )       //used from ball.c


// OBJECTS OBJECTS OBJECTS
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
    int direction;
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
} Level;

// PROTOTYPES
void Ranking(WINDOW *win, Player *p, int levnum, char *pname, int score);

// WINDOWS FUNCTIONS
int GenerateRoads(WINDOW *win, Level l, int border, int yMax, int xMax)
{
    int i, j;
    int lanes=0;
    int bush_prob = l.bush_prob;
    if(border)
        box(win, 0, 0);
    for(i=border; i < yMax-border; ++i)
    {
        if(i == border)
            wattron(win, COLOR_PAIR(META_COL));             //top lane is the meta
        else if(i   %2==0 || i>yMax-3)
            wattron(win, COLOR_PAIR(GRASS_COL));            //even lanes are grass 
        else
            {
                wattron(win, COLOR_PAIR(ROAD_COL));         //uneven lanes are road
                lanes++;
            }               
        for(j=border; j < xMax-border; ++j)
        {
            if((RA(0, bush_prob) % bush_prob == 0) && (i%2 == 0) && 
                (i>0) && (i<yMax-3) && (i>1) && (i<xMax))
            {
                wattron(win, COLOR_PAIR(BUSH_COL));
                mvwaddch(win, i, j, BUSH_SYMBOL);
                wattron(win, COLOR_PAIR(GRASS_COL));
            }
            else
            {
                //wattron(win, COLOR_PAIR(GRASS_COL));
                mvwaddch(win, i, j, ' ');
            }
            // mvwaddch(win, i, j, '0'+bush_prob);
        
        }
    }
    return lanes;
}
void RefreshWindow(WINDOW *win, int border, int yMax, int xMax)
{
    int i, j;
    if(border)
        box(win, 0, 0);
    for(i=border; i < yMax-border; ++i)
    {
        if(i == border)
            wattron(win, COLOR_PAIR(META_COL));             //top lane is the meta
        else if(i%2==0 || i>yMax-3)
            wattron(win, COLOR_PAIR(GRASS_COL));            //even lanes are grass 
        else
            wattron(win, COLOR_PAIR(ROAD_COL));             //uneven lanes are road
        for(j=border; j < xMax-border; ++j)
        {
            if(mvwinch(win, i, j) == ' ')
                mvwaddch(win, i, j, ' ');
        }
    }
}
void ClearWindow(WINDOW *win, int border, int yMax, int xMax)
{
    int i, j;
    if(border)
        box(win, 0, 0);
    wattron(win, COLOR_PAIR(BLACK_COL));
    for(i=border; i < yMax-border; ++i)
    {
        for(j=border; j < xMax-border; ++j)
        {
            mvwaddch(win, i, j, ' ');
        }
    }
    wrefresh(win);
}

void GameOver(WINDOW *win, Player *p, Timer t, Level l, int gameResult)
{
    refresh();
    flushinp();
    usleep(1000*1000*1);
    ClearWindow(win, 0, p->yMax, p->xMax);

    int score = t.time * 100;
    char name[] = "___";
    
    // remove the "game is woking!" from the top
    mvprintw(1, 0, "                        ");
    refresh();

    // gameResult == 1      ->     game LOST
    // gameResult == 2      ->     game WON
    if(gameResult == 1)
        mvwprintw(win, 0, 0, "GAME OVER!                              ");
        
    else if(gameResult == 2)
    {
        mvwprintw(win, 0, 0, "You WON!                                ");
        mvwprintw(win, 1, 0, "SCORE:  %d                         ", score);
        mvwprintw(win, 3, 0, "enter your name...");
        mvwprintw(win, 4, 0, "%s", name);
        wrefresh(win);
        nodelay(win, false);
        char key;
        int i=0;
        while(1)
        {
            flushinp();
            key = wgetch(win);
            if((key == 127) && i > 0)
            {
                name[i-1] = '_';
                i--;
                mvwprintw(win, 4, 0, "%s", name);
                wrefresh(win);
                continue;
            }
            else if((key >= 'a') && (key <= 'z') && i<3)
            {
                name[i] = key -'a' + 'A';
                mvwprintw(win, 4, 0, "%s", name);
                wrefresh(win);
                i++;
            }
            if(key == 10 && i==3)
                break;
        }
    }
    mvwprintw(win, 7, 0, "Press any key to countinue... ");
    wrefresh(win);    
    flushinp();
    getch();
    // mvwprintw(win, 3, 0, "                               ");
    // mvwprintw(win, 6, 0, "                               ");
    if(gameResult == 2)
    {
        Ranking(win, p, l.lev_num, name, score);
        wrefresh(win);
        getch();
    }
    ClearWindow(win, 0, p->yMax, p->xMax);
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

    return win;
}

// MENU FUNCTIONS
int Choice(int numlevels)
{
    mvprintw(0, 0,"Choose the level:                 ");
    mvprintw(1, 0,"                                  ");
    refresh();
    WINDOW *menuwin = newwin(numlevels+2, 20, 5, 10);
    box(menuwin, 0, 0);

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
        default:
            break;
    }
    }
    return 0;
}

// LEVELS FUNCTIONS

// COLOR FUNCTIONS
void SetGoodColor(WINDOW *win, Player *p)
{
    if(p->yPos % 2 == 1 && p->yPos <= p->yMax-3)
        wattron(win, COLOR_PAIR(ROAD_COL));
    else if(p->yPos >0)
        wattron(win, COLOR_PAIR(GRASS_COL));
    else
        wattron(win, COLOR_PAIR(META_COL));
}

// OBJECT FUNCTIONS
Player CreatePlayer(int yPos, int xPos, int yMax, int xMax, char symbol)
{
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
    Timer t;
    t.time = 0;
    t.frame_n = 0;
    return t;
}

Car CreateCar(int head, int lane, int length, int speed, int direction,
              int stops, int isFriendly, int index, int changesSpeed, int lastFrame)
{
    Car car;
    car.head = head;
    car.lane = lane;
    car.length = length;
    car.speed = speed;
    car.direction = direction;
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
    car.direction = 1;
    car.stops = !randomNotStops;
    car.isFriendly = !randomIsNotFriendly;
    car.index = index;
    car.holdsPlayer = 0;
    car.changesSpeed = !randomNotChangesSpeed;
    car.lastFrameMoved = lastFrame;
    return car;
}

//this is the array of all cars
Car **CreateCars(int numroads, Level l)
{
    Car **cars = (Car **)malloc(numroads * sizeof(Car *));
    for(int i=0; i<numroads; ++i)
    {
        //int randomSpeed = RA(CAR_MIN_SPEED, CAR_MAX_SPEED);
        cars[i] = (Car *)malloc(l.cars_per_lane * sizeof(Car));
        for(int j=0; j<l.cars_per_lane; ++j)
        {
            int randomSpeed = RA(l.car_min_speed, l.car_max_speed);
            int randomLength = RA(l.min_car_len, l.max_car_len);
            int randomHead = RA(j*l.map_width/l.cars_per_lane, (j+1)*l.map_width/l.cars_per_lane - randomLength);
            int randomNotStops = RA(0, l.car_stops_prob)%l.car_stops_prob;
            int randomNotFriendly = RA(0, l.car_friendly_prob)%l.car_friendly_prob;
            int randomNotChangesSpeed = RA(0, CHANGES_SPEED_PROB)%CHANGES_SPEED_PROB;
            cars[i][j] = CreateCar(randomHead, 1+(2*i), randomLength, randomSpeed, 1, !randomNotStops,
                                   !randomNotFriendly, j, randomNotChangesSpeed, 0);
        }
    }
    return cars;
}

Level CreateLevel(int lev_num, int bush_prob, int min_car_len, int max_car_len, int cars_per_lane, int car_min_speed,
                  int car_max_speed, int car_stops_prob, int car_friendly_prob, int remove_car_prob, int map_height, int map_width)
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
    return l;
}


void PlaceCar(WINDOW *win, Car *car, Player *p)
{
    wattron(win, COLOR_PAIR(ROAD_COL));
    if(car->direction == 1)
    {
        for(int i=car->head; i>car->head-car->length+1; --i)
            if(i >= 0)
                mvwaddch(win, car->lane, i, '>');
    }
    if(car->direction == -1)
    {
        for(int i=car->head; i<car->head + car->length; ++i)
            if(i+car->length <= p->xMax)
                mvwaddch(win, car->lane, i, '<');
    }
}

// car movement
void MoveCar(WINDOW *win, Player *p, Car *car, Timer t, Level l)
{
    //srand(time(NULL));
    if(t.frame_n - car->lastFrameMoved>= car->speed)
    {
        car->head += car->direction;
        if(((mvwinch(win, car->lane, car->head - car->length) & A_CHARTEXT)  != PLAYER_SYMBOL))  // || p->isAttached
            mvwaddch(win, car->lane, car->head-car->length, ' ');
        if(car -> head <= p->xMax + car->length)
        {    
            if (car->isFriendly)
                wattron(win, COLOR_PAIR(FRIENDLY_CAR_COL));
            else
                wattron(win, COLOR_PAIR(ROAD_COL));
            mvwaddch(win, car->lane, car->head, '>');
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
            // MAKE IT A FUNCTION
    // attach player to the car i he presses a key and is close
    if(car->isFriendly && (p->xPos > car->head-car->length && p->xPos <= car->head) && (p->yPos == car->lane+1) && key == ATTACH_KEY && !p->isAttached && t.frame_n - p->lastFrameMoved >= Player_SPEED)
    {
        wattron(win, COLOR_PAIR(GRASS_COL));
        mvwaddch(win, p->yPos, p->xPos, ' ');
        p->isAttached = 1;
        car->holdsPlayer = 1;
        p -> attachedCarIndex = car->index;
        p->attachedCarIndex = car->index;
        p->yPos--;
        p->lastFrameMoved = t.frame_n +  (1000/FRAME_TIME);
        wattron(win, COLOR_PAIR(FRIENDLY_CAR_COL));
    }
    // MAKE IT A FUNCTION

    if(p->isAttached && (p->attachedCarIndex == car->index) && (p->yPos == car->lane))
    {
        p->xPos = car->head-1;
        wattron(win, COLOR_PAIR(ROAD_COL));
        //mvwaddch(win, p->yPos, p->xPos-1, ' ');
        mvwaddch(win, p->yPos, p->xPos, PLAYER_SYMBOL);
    }
}



//car movement
int DisplayCar(WINDOW *win, Car *car, Player *p, Timer t, char key, Level l)
{
    if(car->isFriendly)
        wattron(win, COLOR_PAIR(FRIENDLY_CAR_COL));
    else
        wattron(win, COLOR_PAIR(ROAD_COL));

    // this loop reassures that when one car is surpassing another, there will be no blank space between them on screen
    for(int i=car->head; i>car->head-car->length+1; --i)
        if(i >= 0)
            mvwaddch(win, car->lane, i, '>');

    // some cars stops when frog is close to them
    if(((mvwinch(win, car->lane+1, car->head+2) & A_CHARTEXT) == PLAYER_SYMBOL) && car->stops && !car->isFriendly)
    {
        car->lastFrameMoved = t.frame_n + (1000/FRAME_TIME);
        car->stops = 0;
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
    if(!hide)
    {
        mvprintw(yMax+PLAYWIN_Y+2, PLAYWIN_X, "current time:  %.2f   ", t->time);
        mvprintw(yMax+PLAYWIN_Y+3, PLAYWIN_X, "current frame: %d     ", t->frame_n);
    }
    else
    {
        mvprintw(yMax+PLAYWIN_Y+2, PLAYWIN_X, "                      ");
        mvprintw(yMax+PLAYWIN_Y+3, PLAYWIN_X, "                      ");
    }
    wrefresh(win);
    t->time += (float)FRAME_TIME/1000;
    t->frame_n++;
}
    
    //PLAYER MOVEMENT FUNCTIONS
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

    int DisplayPlayer(WINDOW *win, Player *p, Timer t, Car **cars, char key)
    {
        if(t.frame_n - p->lastFrameMoved >= Player_SPEED)
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
                    if((p->isAttached) && (mvwinch(win, p->yPos-1, p->xPos) & A_CHARTEXT) != BUSH_SYMBOL)
                    {
                        MvPlayerUp(win, p);
                        p->isAttached = 0;
                        cars[(p->yPos-1)/2][p->attachedCarIndex].holdsPlayer=0;
                        p->lastFrameMoved = t.frame_n + 300/FRAME_TIME;
                    }
                    break;
                default:
                    break;
            }
        }

        // these lines make colors of the map stay as they should be
        SetGoodColor(win, p);

        // Cars are updated after the player, so if there is a car symbol at player's position before he is displayed, that means
        // he ran into a car in the previous frame
        if(((mvwinch(win, p->yPos, p->xPos) & A_CHARTEXT) == '>') && !p->isAttached)
        {
            mvwaddch(win, p->yPos, p->xPos, p->symbol);
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
        if(!p->isAttached)
            mvwaddch(win, p->yPos, p->xPos, p->symbol);

        //RefreshWindow(win, 0, p->yMax, p->xMax);
        flushinp();         //to 'unclog' the input (so it does not stack up)
        return 0;
    }
// READING / WRITING TO FILES

void ReadLevelConfig(int choice, Level *l)
{
    char level_name[] = "levelX.txt";
    char level_number = '0'+choice;
    level_name[5] = level_number;
    FILE *f = fopen(level_name, "r");
    if( f == NULL)
    {
        printw("ERROR: config file not found! Check i a file level%d.txt' exists in the game's folder", choice);
        endwin();
        return;
    }
        // Read level's attributes
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

    *l = CreateLevel(choice, bush_prob, min_car_len, max_car_len, cars_per_lane,
                    car_min_speed, car_max_speed, car_stops_prob, car_friendly_prob,
                    remove_car_prob, map_height, map_width);

    fclose(f);
}

void Ranking(WINDOW *win, Player *p, int levnum, char *pname, int score)
{
    char fname[12] = "rank_.txt";
    fname[4] = levnum + '0';
    char names[5][4];
    int scores[5];
    int score_used = 0;
    FILE *f = fopen(fname, "r");
    
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
    ClearWindow(win, 0, p->yMax, p->xMax);
    mvwprintw(win, 0, 0, "Best scores in level %d:", levnum);
    f = fopen(fname, "r");
    for(int i=0; i<5; ++i)
    {
        char name[4];
        int score;
        fscanf(f, "%s %d", names[i], &scores[i]);
        mvwprintw(win, i+1, 0, "%d. %s: %d\n", i+1, names[i], scores[i]);
    }
    mvwprintw(win, 7, 0, "Press any key to countinue... ");
    fclose(f);
}


// MAIN LOOP

int MainLoop(WINDOW *win, Player *player, Timer timer, Car **cars, int numroads, Level l)
{
    char key;
    do
    {
        // function 'DisplayPlayer' returns 0 if game should still be running,
        // 1 if game is lost, 2 if game is won
        int gameResult = DisplayPlayer(win, player, timer, cars, key);
        if(gameResult)
        {
            DisplayTimerInfo(stdscr, &timer, player->yMax, gameResult);   
            GameOver(win, player, timer, l, gameResult);
            break;
        }
        DisplayTimerInfo(stdscr, &timer, player->yMax, gameResult);       
        for(int i=0; i<numroads; ++i)
        {
            for(int j=0; j<l.cars_per_lane; ++j)
            {
                DisplayCar(win, &cars[i][j], player, timer, key, l);
            }
        }
        // a little brute-force aproach. This line is nescesary because cars used to cover the player symbol
        if(player->isAttached)
            {
                wattron(win, COLOR_PAIR(ROAD_COL));
                mvwaddch(win, player->yPos, player->xPos, PLAYER_SYMBOL);
            }
        wrefresh(win);
        usleep(1000 * FRAME_TIME);
    }while(key = wgetch(win));
    
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
    //use_default_colors();

    // choosing the level
    int choice = Choice(NUMLEVELS);         // choice -> a variable   Choice -> a function
    Level l;
    ReadLevelConfig(choice, &l);


    WINDOW *levelwin = StartLevel(choice, l);
    int yMax, xMax;                          // level window dimensions
    getmaxyx(levelwin, yMax, xMax);
    nodelay(levelwin, true);
    int numRoads = GenerateRoads(levelwin, l, 0, yMax, xMax);      //number of road lanes

    Player player;
    player = CreatePlayer(yMax-2, xMax/2, yMax, xMax, PLAYER_SYMBOL);
    Timer timer;
    timer = CreateTimer();

    Car **cars = CreateCars(numRoads, l);
    for(int i=0; i<numRoads; ++i)
    {
        for(int j=0; j<l.cars_per_lane; ++j)
        {
            PlaceCar(levelwin, &cars[i][j], &player);
        }
    }
    mvprintw(0, 0,"LEVEL %d             ", choice);
    mvprintw(1, 0,"FROG GAME IS WORKING!");
    mvprintw(PLAYWIN_Y + yMax + 5, PLAYWIN_X, STUDENT_DATA);
    refresh();
    wrefresh(levelwin);
    MainLoop(levelwin, &player, timer, cars, numRoads, l);
    //getch();
    endwin();
    }
    return 0;
}