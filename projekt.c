#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>

#define CONFIG_FNAME "config.txt"
#define ENTER 10

#define GRASS_COL 1

#define PLAYER_SYMBOL '@'

#define PLAYWIN_Y 5
#define PLAYWIN_X 10

#define FRAME_TIME 25       //MILISECONDS


// READING / WRITING TO FILES

void readconfig(int *numlevels)
{
    FILE *f = fopen(CONFIG_FNAME, "r");
    if( f == NULL)
    {
        printw("ERROR: config file not found! Check the name of CONFIG_FNAME");
        endwin();
        return;
    }
    int result = fscanf(f, "%*s%d", numlevels);     //skip the first string in a file input, since it's a variable name
    // if(result != 1)
    // {
    //     printw("ERROR: Failed to read config file.");
    //     endwin();
    //     fclose(f);
    //     return;
    // }
    fclose(f);
}

// GENERATING MAP (key)
char *GenerateMap(int map_length, int grassprob, int bouncyprob)            // G -> grass
{                                                                           // H -> road(cars dissappear)
    srand(time(NULL));                                                      // B -> cars are bouncy
    char *map = (char *)malloc((map_length+1) * sizeof(char));
    for(int i=0; i<5; ++i)
    {
        map[i] = 'G';
    }
    for(int i=5; i<map_length; ++i)
    {
        int a=rand();
        int b=rand();
        if(a%grassprob == 0)
            map[i]='G';
        else
        {
            if(b%bouncyprob == 0)
            {
                map[i] = 'B';
            }
            else
                map[i] = 'H';
        }
    }
    map[map_length] = '\0';
    return map;
}

// WINDOWS FUNCTIONS
void ColorWindow(WINDOW *win, int color, int border, int yMax, int xMax)
{
    int i, j;
    if(border)
        box(win, 0, 0);
    wattron(win, COLOR_PAIR(color));
    for(i=border; i < yMax-border; ++i)
    {
        for(j=border; j < xMax-border; ++j)
        {
            mvwaddch(win, i, j, ' ');
        }
    }
}

WINDOW *StartLevel(int choice)
{
    WINDOW *win = newwin(20, 80, PLAYWIN_Y, PLAYWIN_X);        //PLACEHOLDER VALUES
    //box(win, 0, 0);

    init_pair(GRASS_COL, COLOR_WHITE, COLOR_GREEN);

    return win;
}


// MENU FUNCTIONS
int Choice(int numlevels)
{
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



// OBJECTS OBJECTS OBJECTS
typedef struct{
    int xPos;
    int yPos;
    int yMax;
    int xMax;
    char symbol;

} Player;

typedef struct{
    float time;
    int frame_n;
} Timer;


// OBJECT FUNCTIONS
Player CreatePlayer(int xPos, int yPos, int yMax, int xMax, char symbol)
{
    Player p;
    p.xPos = xPos;
    p.yPos = yPos;
    p.yMax = yMax;
    p.xMax = xMax;
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

void DisplayTimerInfo(WINDOW *win, Timer *t, int yMax)
{
    mvprintw(yMax+PLAYWIN_Y+2, PLAYWIN_X, "current time:  %.2f   ", t->time);
    mvprintw(yMax+PLAYWIN_Y+3, PLAYWIN_X, "current frame: %d     ", t->frame_n);
    wrefresh(win);
    t->time += (float)FRAME_TIME/1000;
    t->frame_n++;
}
    
    //PLAYER MOVEMENT FUNCTIONS
    void MvPlayerUp(WINDOW *win, Player *p)
    {
    if(p->yPos > 0)
        {
            mvwaddch(win, p->yPos, p->xPos, ' ');
            p->yPos--;
        }
    }
    void MvPlayerDown(WINDOW *win, Player *p)
    {
        if(p->yPos < p->yMax-1)
        {
            mvwaddch(win, p->yPos, p->xPos, ' ');
            p->yPos++;
        }
    }
    void MvPlayerRight(WINDOW *win, Player *p)
    {
        if(p->xPos < p->xMax-1)
        {
            mvwaddch(win, p->yPos, p->xPos, ' ');
            p->xPos++;
        }
    }
    void MvPlayerLeft(WINDOW *win, Player *p)
    {
        if(p->xPos > 0)
        {
            mvwaddch(win, p->yPos, p->xPos, ' ');
            p->xPos--;
        }
    }
    void DisplayPlayer(WINDOW *win, Player *p, char key)
    {
        switch(key)
        {
            case 'w':
                MvPlayerUp(win, p);
                break;
            case 's':
                MvPlayerDown(win, p);
                break;
            case 'a':
                MvPlayerLeft(win, p);
                break;
            case 'd':
                MvPlayerRight(win, p);
                break;
            default:
                break;
        }
        mvwaddch(win, p->yPos, p->xPos, p->symbol);
        flushinp();         //to 'unclog' the input (so it does not stack up)
    }


// MAIN LOOP

int MainLoop(WINDOW *win, Player *player, Timer timer)
{
    char key;
    do
    {
        DisplayPlayer(win, player, key);
        DisplayTimerInfo(stdscr, &timer, player->yMax);

        usleep(1000 * FRAME_TIME);
    }while(key = wgetch(win));
}

int main()
{
    //ncurses start
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    start_color();
    
    // variables
    int numlevels = 0;   //number of levels

    // load from config file
    readconfig(&numlevels);
    //printw("number of levels: %d\n", numlevels);

    // choosing the level
    int choice = Choice(numlevels);         // choice -> a variable   Choice -> a function
    int yMax, xMax;                          // level window dimensions

    WINDOW *levelwin = StartLevel(choice);
    getmaxyx(levelwin, yMax, xMax);
    nodelay(levelwin, true);
    ColorWindow(levelwin, GRASS_COL, 0, yMax, xMax);
    refresh();
    wrefresh(levelwin);

    Player player;
    player = CreatePlayer(2, 2, yMax, xMax, PLAYER_SYMBOL);
    Timer timer;
    timer = CreateTimer();

    printw("FROG GAME IS WORKING!\nLEVEL %d", choice);
    MainLoop(levelwin, &player, timer);
    getch();
    endwin();
    return 0;
}