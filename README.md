# NCurses Game Project

Welcome to the **NCurses Game**, a console-based game built using the `ncurses` library. This project demonstrates how to create an interactive and visually engaging game entirely within the terminal.

Video demonstration: https://youtu.be/RcCAvz4A3r0

## About the game

The game is about a frog trying to pass the street. The difficulties are:
- Cars driving across the streets.
- Static obstacles (bushes)
- Stork trying to capture the frog


## Features

- Terminal-based gameplay
- Dynamic visual elements using `ncurses`
- Concept of levels
- Levels are randomly generated (random cars, obstacles)
- Cars randomly change speed, some of the cars disapear when reaching the border, new random cars are generated.
- Friendly cars (player can jump inside them by pressing 'c', and travel inside them safely, 'c' to leave)
- Concept of points, ranking for every level (player can insert their name upon finishing the level)
- A replay of the whole game can be shown after the game is lost/won

## Installation and Compilation

To run the game, you'll need a C compiler and the `ncurses` library installed on your system. Follow these steps to compile and play:

1. **Install NCurses:**
   - On Debian-based systems (Ubuntu, etc.):  
     ```bash
     sudo apt-get install libncurses5-dev libncursesw5-dev
     ```
   - On Red Hat-based systems (Fedora, etc.):  
     ```bash
     sudo dnf install ncurses-devel
     ```

Compile the game using f.e: gcc -o projekt projekt.c -lncurses
Run the game: ./projekt
