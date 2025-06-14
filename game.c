#include <GL/glut.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>
#include <stdbool.h>
#include <string.h>


#define MAX_KEYS 2
#define MAX_PERMITS 2
#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 600
#define GHOST_COUNT 4
#define ROWS 50
#define COLS 59
#define TILE_SIZE 10
#define MAZE_WIDTH (COLS * TILE_SIZE)
#define MAZE_HEIGHT (ROWS * TILE_SIZE)
#define OFFSET_X ((WINDOW_WIDTH - MAZE_WIDTH) / 2)
#define OFFSET_Y ((WINDOW_HEIGHT - MAZE_HEIGHT) / 2)
#define POWER_PELLET_DURATION 300 

//sem_t board_lock;  // To manage read/write access to the game board
pthread_cond_t power_pellet_lock;
pthread_mutex_t power_pallete_mutex;
sem_t ghost_house_key;
sem_t ghost_exit_permit;
sem_t speed_boost;
int available_keys = MAX_KEYS;
int available_permits = MAX_PERMITS;
pthread_mutex_t key_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t permit_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t key_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t permit_available = PTHREAD_COND_INITIALIZER;
sem_t power_pellet_sem;
int lives = 3;
int gameOver = 0;
int score = 0;
int dotRemaining = 0;
int pacmanDirection = 0;
float pacmanMouthAngle = 20.0f;
int mouthOpen = 1;
int running = 1;
int currentOption=0;  // 0 = Start, 1 = Instructions, 2 = Exit
int showInstructions = 0;  // 0 = menu/game, 1 = showing instructions
bool powerPalleteEaten=false;
int power_pellet_timer = 0;
float side = 10.0f;
const float PI = 3.14159265f;
// 0 = pallete, 1 = wall, 2 = empty, 3=powerPalletes
int board[ROWS][COLS]={

{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,2,2,2,2,2,2,1,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,2,2,0,2,0,2,0,2,0,2,1,0,2,0,2,2,0,2,0,2,0,2,0,2,2,2,0,2,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,2,2,2,2,2,1,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1},
{1,2,0,2,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,0,2,1,0,2,0,2,0,2,0,2,1,2,0,2,1,1,1,1,1,2,0,2,0,2,1,1,1,2,0,2,1,2,0,2,2,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,1,2,2,2,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,0,2,0,2,1,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,1,2,0,2,1,2,2,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,1,2,2,2,1,1,1,1,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,2,1,0,2,0,2,0,2,0,2,2,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,2,2,2,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
{1,1,1,1,1,1,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,1,1,1,1,1,1,1,1,1,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,1},
{1,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,1,2,0,2,1},
{1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,1},
{1,2,0,2,0,2,0,2,1,1,1,1,1,1,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,1,2,0,2,1},
{1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1,1,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,2,0,2,0,2,0,2,0,2,1,0,2,0,2,0,2,0,2,2,2,2,2,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1,1,1,1,1,1},
{1,1,1,1,1,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,2,0,2,0,2,0,2,0,2,1,0,2,0,2,0,2,0,2,1,2,2,2,2,2},
{2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1,2,2,2,2,2},
{2,2,2,2,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,0,2,0,2,0,2,0,2,1,2,2,2,2,2},
{2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2},
{1,1,1,1,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,2,0,2,1,1,1,1,1,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,1,2,1,2,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,1,1,1,1,1,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,2,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,1},
{1,2,0,2,0,2,1,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,2,0,2,0,2,0,2,1,2,0,2,1},
{1,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1,2,2,2,1},
{1,2,0,2,0,2,1,2,0,2,0,2,0,2,1,1,1,1,1,1,0,2,0,2,0,2,1,1,1,1,1,1,1,0,2,0,2,0,2,1,1,1,1,1,1,1,1,2,0,2,0,2,0,2,1,2,0,2,1},
{1,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,1},
{1,2,0,2,0,2,1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
{1,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,1,1,1,1,1,1,1,1,0,2,0,2,0,2,0,2,0,2,0,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1}

};

typedef struct {
    float x, y;  // Actual positions
    int gridRow, gridCol; // positions in grid
    bool isAlive;
} Entity;

Entity pacman;
Entity ghosts[GHOST_COUNT];

pthread_t game_engine_thread;
pthread_t ui_thread;
pthread_t ghost_threads[GHOST_COUNT];
pthread_mutex_t board_lock;




const char *menuItems[] = { "Start Game", "Instructions", "Exit" };
const int numItems = 3;

void drawTextt(float x, float y, const char *text, void *font) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++)
        glutBitmapCharacter(font, text[i]);
}

void displayMenu() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glColor3f(1.0, 1.0, 0.0);
    drawTextt(240, 500, "PAC-MAN GAME", GLUT_BITMAP_TIMES_ROMAN_24);

    for (int i = 0; i < numItems; i++) {
         if (i == currentOption)
            glColor3f(1.0, 0.0, 0.0);  // Highlight selected item
        else
            glColor3f(1.0, 1.0, 1.0);

        drawTextt(260, 400 - i * 60, menuItems[i], GLUT_BITMAP_HELVETICA_18);
    }

    glutSwapBuffers();
}


// Count the initial number of pellets
void countPellets() {
    dotRemaining = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (board[i][j] == 0) {
                dotRemaining++;
            }
        }
    }
}

// ==== Drawing functions ====
void drawBox(float x, float y, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y); //bottom left corner
    glVertex2f(x + TILE_SIZE, y); //bottom right
    glVertex2f(x + TILE_SIZE, y + TILE_SIZE); //top right
    glVertex2f(x, y + TILE_SIZE); //top left
    glEnd();
}

// Drawing ghost
void drawGhost(float x, float y, float radius, int ghostId) {
  if(ghosts[ghostId].isAlive==false){return;};
    int numSegments = 100;
    float colors[4][3]= {
          {1.0f, 0.0f, 0.0f},    // Red
          {0.0f, 1.0f, 1.0f},    // Cyan
          {1.0f, 0.65f, 0.0f},   // Orange
          {1.0f, 0.41f, 0.71f}   // Pink
          };
    if(powerPalleteEaten){
   float newColors[4][3] = {
          {0.678f, 0.847f, 0.902f}, // Blue
          {0.678f, 0.847f, 0.902f}, // Blue
          {0.678f, 0.847f, 0.902f}, // Blue
          {0.678f, 0.847f, 0.902f}  // Blue
    };
    memcpy(colors, newColors, sizeof(colors));
    }
    
    // Draw semicircular head
    glBegin(GL_TRIANGLE_FAN);
    glColor3fv(colors[ghostId]);
    glVertex2f(x, y); // center
    for (int i = 0; i <= numSegments; i++) {
        float angle = PI * i / numSegments;
        glVertex2f(x + cos(angle) * radius, y + sin(angle) * radius);
    }
    glEnd();

    // Draw body rectangle
    glBegin(GL_QUADS);
    glVertex2f(x - radius, y); 
    glVertex2f(x + radius, y);
    glVertex2f(x + radius, y - radius);
    glVertex2f(x - radius, y - radius);
    glEnd();

    // Draw wavy bottom using triangles
    int waves = 4;
    float waveWidth = (2 * radius) / waves;
    for (int i = 0; i < waves; ++i) {
        float x1 = x - radius + i * waveWidth;
        float x2 = x1 + waveWidth;
        float xc = (x1 + x2) / 2.0f;
        float bottom = y - radius;

        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(xc, bottom - radius / 4);
        glVertex2f(x1, bottom);
        glVertex2f(x2, bottom);
        glEnd();
    }
    
    // Draw eyes
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(x - radius/2 - radius/4, y - radius/3);
    glVertex2f(x - radius/2 + radius/4, y - radius/3);
    glVertex2f(x - radius/2 + radius/4, y - radius/3 - radius/4);
    glVertex2f(x - radius/2 - radius/4, y - radius/3 - radius/4);
    
    glVertex2f(x + radius/2 - radius/4, y - radius/3);
    glVertex2f(x + radius/2 + radius/4, y - radius/3);
    glVertex2f(x + radius/2 + radius/4, y - radius/3 - radius/4);
    glVertex2f(x + radius/2 - radius/4, y - radius/3 - radius/4);
    glEnd();
    
    // Draw pupils
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(x - radius/2 - radius/8, y - radius/3 - radius/8);
    glVertex2f(x - radius/2 + radius/8, y - radius/3 - radius/8);
    glVertex2f(x - radius/2 + radius/8, y - radius/3 - radius/4);
    glVertex2f(x - radius/2 - radius/8, y - radius/3 - radius/4);
    
    glVertex2f(x + radius/2 - radius/8, y - radius/3 - radius/8);
    glVertex2f(x + radius/2 + radius/8, y - radius/3 - radius/8);
    glVertex2f(x + radius/2 + radius/8, y - radius/3 - radius/4);
    glVertex2f(x + radius/2 - radius/8, y - radius/3 - radius/4);
    glEnd();
}

// Drawing pacman
void drawPacman(float x, float y, float radius, float mouthAngleDegrees, int direction) {
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow color

    float startAngle;
    float endAngle;

    // Direction: 0 = Right, 1 = Up, 2 = Left, 3 = Down
    float rotation = 0.0f;
    switch (direction) {
        case 0: {
        rotation = 0.0f; 
        break;    // Right
        }
        case 1: {
        rotation = 270.0f; 
        break;   // Up
        }
        case 2:{
        rotation = 180.0f;
        break;  // Left
        }
        case 3: {
        rotation = 90.0f;
        break;  // Down
        }
    }

    startAngle = mouthAngleDegrees;
    endAngle = 360.0f - mouthAngleDegrees;

    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(rotation, 0, 0, 1);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0); // Center of Pac-Man
    for (float angle = startAngle; angle <= endAngle; angle += 2.0f) {
        float rad = angle * PI / 180.0f;
        glVertex2f(cos(rad) * radius, sin(rad) * radius);
    }
    glEnd();

    glPopMatrix();
}

// Pellets
void drawCircle(float x, float y, float radius) {
    const int Segments = 100; // More segments = smoother circle
    glPushMatrix();
    glTranslatef(x, y, 0);
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0); // Center of circle
    for(int i = 0; i <= Segments; ++i) {
        float angle = i * 2.0f * PI / Segments;
        float dx = radius * cosf(angle);
        float dy = radius * sinf(angle);
        glVertex2f(dx, dy);
    }
    glEnd();
    
    glPopMatrix();
}

void drawText(const char* text, float x, float y) {
    glColor3f(1.0f, 1.0f, 1.0f); // White text
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

// Board UI
void drawBoard() {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            float x = col * TILE_SIZE + OFFSET_X;
            float y = row * TILE_SIZE + OFFSET_Y;
            if (board[row][col] == 1) {
                drawBox(x, y, 0.0f, 0.0f, 1.0f); // Wall (blue)
            } 
            else if (board[row][col] == 0) {
                glColor3f(1.0f, 1.0f, 1.0f); // White color for pellets
                drawCircle(x + TILE_SIZE/2, y + TILE_SIZE/2, 2.0f); // Pellet
            }
            else if (board[row][col] == 3) {
                glColor3f(1.0f, 0.0f, 0.0f); // Red color for pellets
                drawCircle(x + TILE_SIZE/2, y + TILE_SIZE/2, 5.0f); // Power Pallete;
            }
        }
    }
}

// Display Game
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    
    pthread_mutex_lock(&board_lock);
    
    // Display score and lives
    char scoreText[20];
    sprintf(scoreText, "Score: %d", score);
    drawText(scoreText, 20, 20);
    
    char livesText[20];
    sprintf(livesText, "Lives: %d", lives);
    drawText(livesText, WINDOW_WIDTH - 100, 20);
    
    char pelletsText[30];
    sprintf(pelletsText, "Pellets: %d", dotRemaining);
    drawText(pelletsText, WINDOW_WIDTH/2 - 50, 20);

    // If game over, display message
    if (gameOver) {
        char gameOverText[20];
        if (dotRemaining <= 0) {
            sprintf(gameOverText, "You Win! Score: %d", score);
        } else {
            sprintf(gameOverText, "Game Over! Score: %d", score);
        }
        drawText(gameOverText, WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2);
        pthread_mutex_unlock(&board_lock);
        glutSwapBuffers();
        return; // skip drawing rest of scene
    }
    
    drawBoard(); // Draw walls and pellets
    
    // Animate Pac-Man's mouth
    pacmanMouthAngle += (mouthOpen ? -1.0f : 1.0f);
    if (pacmanMouthAngle <= 5.0f || pacmanMouthAngle >= 40.0f) {
        mouthOpen = !mouthOpen;
    }
    
    // Draw Pac-Man at his position
    drawPacman(pacman.x + TILE_SIZE/2, pacman.y + TILE_SIZE/2, TILE_SIZE/2, pacmanMouthAngle, pacmanDirection);
    
    // Draw ghosts
    for (int i = 0; i < GHOST_COUNT; i++) {
        drawGhost(ghosts[i].x + TILE_SIZE/2, ghosts[i].y + TILE_SIZE/2, TILE_SIZE/2, i);
    }
    
    pthread_mutex_unlock(&board_lock);
    glutSwapBuffers();
}

/*void* RespawnPallete(void*arg){
    sleep(20); // Wait 20 seconds before respawning
    board[powerPalleteX][powerPalleteY]=2;
     glutPostRedisplay();
     return NULL;
}*/
void* ghostRespawn(void* arg) {
    int ghostId = *(int*)arg;
    free(arg);
    sleep(10); // Wait 10 seconds before respawning
    // Respawn at initial position or "ghost house"
    ghosts[ghostId].x = OFFSET_X + TILE_SIZE * 29;
    ghosts[ghostId].y = OFFSET_Y + TILE_SIZE * 23;
    ghosts[ghostId].gridRow = 23;
    ghosts[ghostId].gridCol = 29;
    ghosts[ghostId].isAlive=true;
    powerPalleteEaten=false;
    return NULL;
}

// Check if Pac-Man collides with any ghost
void checkGhostCollision() {
    if (gameOver) return;
    
    for (int i = 0; i < GHOST_COUNT; i++) {
    
    // Only check collisions with active ghosts
        if (!ghosts[i].isAlive) {
        continue;
        }
        // Check if PacMan and ghost are in the same grid cell
        if (pacman.gridRow == ghosts[i].gridRow && pacman.gridCol == ghosts[i].gridCol) {
            if(powerPalleteEaten){
            score+=200;
            pthread_t respawnThread;
            int* ghostId = malloc(sizeof(int));
            *ghostId = i;
            ghosts[i].isAlive=false;
            glutPostRedisplay();
            pthread_create(&respawnThread, NULL, ghostRespawn, ghostId);
            break;
            }
            else{
            lives--;
            if (lives <= 0) {
                gameOver = 1;
            } else {
                // Reset Pac-Man to starting position
                pacman.x = OFFSET_X + TILE_SIZE;
                pacman.y = OFFSET_Y + TILE_SIZE;
                pacman.gridRow = 1;
                pacman.gridCol = 1;
             }
            }
            break;
        }
    }
}



void* gameEngine(void* arg) {
    while (running) {
        pthread_mutex_lock(&board_lock);
        //glutPostRedisplay();
        
        //Power Pallete Timer Count
         if (powerPalleteEaten && power_pellet_timer > 0) {
            power_pellet_timer--;
            if (power_pellet_timer <= 0) {
                powerPalleteEaten = false;
                // Reset ghosts to normal state
                glutPostRedisplay(); // Force redraw to update ghost colors
            }
        }
        
         checkGhostCollision();
        
        // Check if player won
        if (dotRemaining <= 0) {
            gameOver = 1;
        }
          pthread_mutex_unlock(&board_lock);
        
        usleep(50000); // 50ms delay for animation
    }
    return NULL;
}


// Move Pac-Man in the specified direction
void movePacman(int direction) {
    if (gameOver) return;
    
    float newX = pacman.x;
    float newY = pacman.y;
    int newRow = pacman.gridRow;
    int newCol = pacman.gridCol;
    
    // Calculate new position based on direction
    switch (direction) {
        case 0: // Right
            newX += TILE_SIZE;
            newCol++;
            break;
        case 1: // Up
            newY -= TILE_SIZE;
            newRow--;
            break;
        case 2: // Left
            newX -= TILE_SIZE;
            newCol--;
            break;
        case 3: // Down
            newY += TILE_SIZE;
            newRow++;
            break;
    }
    
    // Check if new position is valid (not a wall)
    if (newRow >= 0 && newRow < ROWS && newCol >= 0 && newCol < COLS && board[newRow][newCol] != 1) {
        // Valid move
        pacman.x = newX;
        pacman.y = newY;
        pacman.gridRow = newRow;
        pacman.gridCol = newCol;
        pacmanDirection = direction;
        
        // Check if Pac-Man eats a pellet
        if (board[newRow][newCol] == 0) {
            board[newRow][newCol] = 2; // Mark pellet as eaten
            score += 10;
            dotRemaining--;
            system("aplay -q /home/dev/Downloads/pacman-chomp.wav &");

        }
         else if (board[newRow][newCol] == 3) {
        //pthread_mutex_lock(&power_pallete_mutex);
        sem_wait(&power_pellet_sem);
            if(powerPalleteEaten==false){
            board[newRow][newCol] = -1; // Mark pellet as eaten
            score += 50;
            dotRemaining--;
            powerPalleteEaten=true;
            power_pellet_timer = POWER_PELLET_DURATION;
              //pthread_cond_signal(&power_pellet_lock);
              
              // Force redraw to update ghost colors
            glutPostRedisplay();
          }
       //pthread_mutex_lock(&power_pallete_mutex);
          sem_post(&power_pellet_sem);   
        }
    }
}
// Ghost movement AI
void* ghostController(void* ghost_id_ptr) {
    int id = *((int*)ghost_id_ptr);
    int fast = id % 2 == 0; // even numbered ghosts are faster
    free(ghost_id_ptr); // Free allocated memory
    
    // Each ghost has a specific movement pattern and behavior
    int baseDelay = 150000 + (id * 20000); // Different speeds for each ghost
    int moveDelay = baseDelay; // Current delay, can be modified by power-ups
    
    //scenario 3
    sem_wait(&ghost_house_key);
    sem_wait(&ghost_exit_permit);
    sleep(1);
    sem_post(&ghost_exit_permit);
    sem_post(&ghost_house_key);
    
    
    //scenario 4
    if (fast) {
        sem_wait(&speed_boost);
        moveDelay = baseDelay / 3; // Make the ghost 3x faster
        //printf("Ghost %d activated speed boost! Delay: %d\n", id, moveDelay);
        sleep(2);  // Use the boost
        moveDelay = baseDelay; // Reset to original speed
        //printf("Ghost %d speed boost ended. Delay: %d\n", id, moveDelay);
        sem_post(&speed_boost);
    }
    
    while (1) {
        if (gameOver) break;
        usleep(moveDelay);
        
        // Simple ghost AI: try to move towards Pac-Man with some randomness
        int possibleDirs[4] = {0, 1, 2, 3}; // Right, Up, Left, Down
        int validDirs[4]={0};
        int validCount = 0;
        
        // Calculate distances to Pac-Man for each direction
        for (int i = 0; i < 4; i++) {
            int newRow = ghosts[id].gridRow;
            int newCol = ghosts[id].gridCol;
            
            switch (possibleDirs[i]) {
                case 0:{
                newCol++; 
                break; // Right
                }
                case 1:{
                newRow--; 
                break; // Up
                }
                case 2:{
                newCol--; 
                break; // Left
                }
                case 3: {
                newRow++; 
                break; // Down
                }
            }
            
            // Check if direction is valid (not a wall)
            if (newRow >= 0 && newRow < ROWS && newCol >= 0 && newCol < COLS && board[newRow][newCol] != 1) {
                validDirs[validCount++] = possibleDirs[i];
            }
        }
        
       pthread_mutex_lock(&board_lock);
       
       // Skip if this ghost is not alive
        if (!ghosts[id].isAlive) {
            pthread_mutex_unlock(&board_lock);
            continue;
        }
        
        // If there are valid directions, choose one
        if (validCount > 0) {
            int randIndex = rand() % validCount;
            int chosenDir = validDirs[randIndex];
            
            // Add some bias towards Pac-Man
            if (rand() % 4 < 2) { // 50% chance to move towards Pac-Man
                int bestDir = -1;
                float bestDist = powerPalleteEaten ? -1 : 1000000;
                
                for (int i = 0; i < validCount; i++) {
                    int dir = validDirs[i];
                    int newRow = ghosts[id].gridRow;
                    int newCol = ghosts[id].gridCol;
                    
                    switch (dir) {
                        case 0:{
                        newCol++;
                        break;
                        }
                        case 1:{
                        newRow--;
                        break;
                        }
                        case 2: {
                        newCol--;
                        break;
                        }
                        case 3:{
                        newRow++;
                        break;
                        }
                    }
                    
                    // Calculate distance to Pac-Man
                    float dist = sqrt(pow(newRow - pacman.gridRow, 2) + pow(newCol - pacman.gridCol, 2));
                    
                    if(powerPalleteEaten) {
                      // If power pellet active, try to move away from PacMan
                      if (dist > bestDist) {
                          bestDist = dist;
                          bestDir = dir;
                      }
                      }
                      else{
                       if (dist < bestDist) {
                              bestDist = dist;
                              bestDir = dir;
                          }
                      }
                    }
                if (bestDir != -1) {
                    chosenDir = bestDir;
               }
            }
            
            // Move ghost in chosen direction
            switch (chosenDir) {
                case 0: // Right
                    ghosts[id].x += TILE_SIZE;
                    ghosts[id].gridCol++;
                    break;
                case 1: // Up
                    ghosts[id].y -= TILE_SIZE;
                    ghosts[id].gridRow--;
                    break;
                case 2: // Left
                    ghosts[id].x -= TILE_SIZE;
                    ghosts[id].gridCol--;
                    break;
                case 3: // Down
                    ghosts[id].y += TILE_SIZE;
                    ghosts[id].gridRow++;
                    break;
            }
             
        }
        pthread_mutex_unlock(&board_lock);
    }
    
    return NULL;
}

void keyboard(unsigned char key, int x, int y) {
    pthread_mutex_lock(&board_lock);
    
    switch (key) {
        case 'q':
        case 'Q':
        case 27: // ESC key
            running = 0;
            exit(0);
            break;
        case 'r':
        case 'R':
            if (gameOver) {
                // Reset game
                gameOver = 0;
                lives = 3;
                score = 0;
                
                // Reset pellets
                for (int i = 0; i < ROWS; i++) {
                    for (int j = 0; j < COLS; j++) {
                        if (board[i][j] == 2) {
                            board[i][j] = 0; // Restore eaten pellets
                        }
                    }
                }
                countPellets();
                
                // Reset positions
                pacman.x = OFFSET_X + TILE_SIZE;
                pacman.y = OFFSET_Y + TILE_SIZE;
                pacman.gridRow = 1;
                pacman.gridCol = 1;
                
                // Reset ghosts
                for (int i = 0; i < GHOST_COUNT; i++) {
                    ghosts[i].x = OFFSET_X + TILE_SIZE * (COLS/2 - 2 + i%2*4);
                    ghosts[i].y = OFFSET_Y + TILE_SIZE * (ROWS/2 - 2 + i/2*4);
                    ghosts[i].gridRow = ROWS/2 - 2 + i/2*4;
                    ghosts[i].gridCol = COLS/2 - 2 + i%2*4;
                }
            }
            break;
    }
    
    pthread_mutex_unlock(&board_lock);
}

void specialKeyboard(int key, int x, int y) {
    pthread_mutex_lock(&board_lock);
    
    switch (key) {
        case GLUT_KEY_RIGHT:
            movePacman(0);
            break;
        case GLUT_KEY_UP:
            movePacman(3);
            break;
        case GLUT_KEY_LEFT:
            movePacman(2);
            break;
        case GLUT_KEY_DOWN:
            movePacman(1);
            break;
    }
    
    pthread_mutex_unlock(&board_lock);
    glutPostRedisplay();
}






void initOpenGL() {
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
}

void initGame() {
    //Power Palletes
    int countPowerPalletes=rand()%6+4; //range between 4 to 6 for power Palletes in the whole game
    for (int i=0;i<countPowerPalletes;++i){
    int x_pp;
    int y_pp;
    do{
    x_pp=rand()%(ROWS-2)+2;
    y_pp=rand()%(COLS-2)+2;
    if(board[x_pp][y_pp]!=1){
    board[x_pp][y_pp]=3;//3 for power palletes
    }
    }while(board[x_pp][y_pp]==1);
    }
    // Initialize Pac-Man position
    pacman.x = OFFSET_X + TILE_SIZE;
    pacman.y = OFFSET_Y + TILE_SIZE;
    pacman.gridRow = 1;
    pacman.gridCol = 1;
    pacman.isAlive=true;
    // Initialize ghost positions
    for (int i = 0; i < GHOST_COUNT; i++) {
        ghosts[i].x = OFFSET_X + TILE_SIZE * (COLS/2 - 2 + i%2*4);
        ghosts[i].y = OFFSET_Y + TILE_SIZE * (ROWS/2 -2 + i/2*4);
        ghosts[i].gridRow = ROWS/2 - 2 + i/2*4;
        ghosts[i].gridCol = COLS/2 - 2 + i%2*4;
        ghosts[i].isAlive=true;
    }
    
    // Count initial pellets
    countPellets();
}

void* uiThread(void* arg) {
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    return NULL;
}

void displayInstructions() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glColor3f(1.0, 1.0, 0.0);
    drawTextt(220, 500, "INSTRUCTIONS", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(1.0, 1.0, 1.0);
    drawTextt(100, 400, "1. Use Arrow Keys to move Pac-Man.", GLUT_BITMAP_HELVETICA_18);
    drawTextt(100, 360, "2. Eat all dots to win the level.", GLUT_BITMAP_HELVETICA_18);
    drawTextt(100, 320, "3. Avoid ghosts or you'll lose a life.", GLUT_BITMAP_HELVETICA_18);
    drawTextt(100, 280, "4. Eat power pellets to make ghosts vulnerable.", GLUT_BITMAP_HELVETICA_18);
    glColor3f(1.0, 1.0, 0.0);
    drawTextt(180, 200, "Press 'm' to go back to the main menu.", GLUT_BITMAP_TIMES_ROMAN_24);

    glutSwapBuffers();
}

void* displayUpdater(void* arg) {
    while (running) {
        glutPostRedisplay();
        usleep(16000); // ~60 FPS
    }
    return NULL;
}



void startGame() {
    // Register actual game display and input handlers
     // Initialize OpenGL
    initOpenGL();
    
    // Initialize game state
    initGame();
sem_init(&ghost_house_key, 0, 3);         // 3 keys
sem_init(&ghost_exit_permit, 0, 3);
sem_init(&speed_boost, 0, 2);  //2 speed boosts
sem_init(&power_pellet_sem, 0, 1);


    // Start game logic threads
    pthread_create(&ui_thread, NULL, uiThread, NULL);
    pthread_create(&game_engine_thread, NULL, gameEngine, NULL);

    for (int i = 0; i < GHOST_COUNT; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_create(&ghost_threads[i], NULL, ghostController, id);
    }
pthread_t displayThread;
 pthread_create(&displayThread, NULL, displayUpdater, NULL);
    glutPostRedisplay(); // Trigger screen refresh
   
}

void specialKeyHandler(int key, int x, int y) {
    if (key == GLUT_KEY_UP) {
        currentOption = (currentOption - 1 + numItems) % numItems;
    } else if (key == GLUT_KEY_DOWN) {
        currentOption = (currentOption + 1) % numItems;
    }
    glutPostRedisplay();
    
}



void keyboardHandler(unsigned char key, int x, int y) {
    if (showInstructions) {
        if (key == 'm' || key == 'M') {
            showInstructions = 0;
            glutDisplayFunc(displayMenu);
            glutKeyboardFunc(keyboardHandler); // Reassign same handler
            glutPostRedisplay();
        }
        return;
    }

    if (key == 13) { // Enter key
        switch (currentOption) {
            case 0:
                startGame();
                break;
            case 1:
                showInstructions = 1;
                glutDisplayFunc(displayInstructions);
                glutKeyboardFunc(keyboardHandler);
                glutPostRedisplay();
                break;
            case 2:
                exit(0);
                break;
        }
    }
}



int main(int argc, char** argv) {
  
   // Seed RNG
    srand(time(NULL));

    // Initialize mutexes and condition variables
    pthread_mutex_init(&board_lock, NULL);
    pthread_mutex_init(&power_pallete_mutex, NULL);
    pthread_cond_init(&power_pellet_lock, NULL);

  

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Pac-Man Menu");

    // Initialize OpenGL
    initOpenGL();

    // Set menu as initial screen
    glutDisplayFunc(displayMenu);
    glutKeyboardFunc(keyboardHandler);
    glutSpecialFunc(specialKeyHandler);
    glutMainLoop();

    // Clean up (usually won't reach here)
    pthread_mutex_destroy(&board_lock);
    pthread_mutex_destroy(&power_pallete_mutex);
    pthread_cond_destroy(&power_pellet_lock);

    sem_destroy(&ghost_house_key);    
    sem_destroy(&ghost_exit_permit); 
    sem_destroy(&speed_boost); 
    
    return 0;
}
