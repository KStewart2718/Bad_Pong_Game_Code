#include "mbed.h"
#include "LCD.h"
#include <stdint.h>

//led pins
DigitalOut blue_led(PB_10);
DigitalOut green_led(PC_7);
DigitalOut yellow_led(PA_9);
DigitalOut red_led(PA_8);
DigitalOut white_led(PB_14);

//digits on the 7 seg display, starting from 1 on the rhs
DigitalOut seg1(PB_12);
DigitalOut seg2(PB_11);
DigitalOut seg3(PB_1);
DigitalOut seg4(PA_4);

//button pins
DigitalIn button1(PD_8);
DigitalIn button2(PB_13);
InterruptIn button(PD_8);

//analog pins
AnalogIn seedPin(PA_3);

//create shiftout object
//ShiftOut shiftout(D13, D10, D12, D14, D11);

/*--------------------------------------------------------------------------------------------
  Enums for clearer code
---------------------------------------------------------------------------------------------*/
typedef enum : uint8_t
{
  LEFT,
  RIGHT,
} LeftRight;

enum Rows : uint8_t
{
  TOPROW,
  BOTTOMROW
};

enum Chars : uint8_t
{
  LEFT_PADDLE,
  RIGHT_PADDLE,
  BALL,
};

enum Coords : uint8_t
{
  X,
  Y 
};

/*
  Custom Character Data
*/

uint8_t LHSPaddleChar[8] = 
{
  0b00111,
  0b00111,
  0b00111,
  0b00111,
  0b00111,
  0b00111,
  0b00111,
  0b00111,
};

uint8_t RHSPaddleChar[8] =
{
  0b11100,
  0b11100,
  0b11100,
  0b11100,
  0b11100,
  0b11100,
  0b11100,
  0b11100,
};

uint8_t ballChar[8] =
{
  0b00000,
  0b00000,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
  0b00000,
  0b00000,
};

/*
  Objects
*/

class gameObject
{
  public:
    int8_t x_Pos;
    volatile int8_t y_Pos;
    LeftRight dir;

  gameObject(){};

  gameObject(LeftRight direction)
  {
    dir = direction;
  }
};


gameObject leftPaddle(LEFT);
gameObject rightPaddle(RIGHT);
gameObject ball;

/*
    Global Variables
*/
uint8_t leftScore = 0;
uint8_t rightScore = 0;
uint8_t maxScore = 3;

uint8_t ballDrawTicker = 0;
uint8_t delay_value = 80;
uint8_t ballDrawTickerLimit = delay_value % 9;

LCD lcd(PC_5, PC_4, PA_10, PB_3, PB_5, PB_4);

/*
    Function Declarations
*/

void ISR_Button();
void loadCustomChars();
void welcomeScreen();
void gameInit();
void paddleAI();
void moveBall(gameObject *obj);
void drawBall(gameObject *obj);
void drawPaddle(gameObject *obj);
void collisionDetect(gameObject *obj1, gameObject *obj2);
void endGame();

// main() runs in its own thread in the OS
int main()
{
    button.fall(&ISR_Button);
    srand(6/*seedPin.read()*/);
    loadCustomChars();
    welcomeScreen();
    gameInit();

    while(true) 
    {
        if(leftScore >= maxScore || rightScore >= maxScore)
        {
            endGame();
        }

        if(ballDrawTicker == ballDrawTickerLimit)
        {
            ballDrawTicker = 0;
            
            if(!(rand() % 3))
            {
            paddleAI();    
            }

            moveBall(&ball);
            drawBall(&ball);
        }

        drawPaddle(&rightPaddle);
        drawPaddle(&leftPaddle);
        
        collisionDetect(&leftPaddle, &ball);
        collisionDetect(&rightPaddle, &ball);

        ballDrawTicker++;
        
        thread_sleep_for(delay_value);
    }
}

void ISR_Button()
{
  if(leftPaddle.y_Pos)
  {
    leftPaddle.y_Pos--;
  }
  else 
  {
    leftPaddle.y_Pos++;
  }
}

void loadCustomChars()
{
  lcd.create(LEFT_PADDLE, LHSPaddleChar);
  lcd.create(RIGHT_PADDLE, RHSPaddleChar);
  lcd.create(BALL, ballChar);
}

void welcomeScreen()
{
  lcd.cls();
  lcd.locate(4, 0);
  lcd.printf("BAD PONG");
  thread_sleep_for(1000);
  lcd.locate(0, 1);
  lcd.printf("Start in:");
  lcd.locate(10, 1);
  thread_sleep_for(1000);
  lcd.printf("3 ");
  lcd.locate(12, 1);
  thread_sleep_for(1000);
  lcd.printf("2 ");
  lcd.locate(14, 1);
  thread_sleep_for(1000);
  lcd.printf("1");
  thread_sleep_for(1000);
  lcd.cls();
  lcd.printf("GO!   GO!   GO! ");
  lcd.locate(0, 1);
  lcd.printf("   GO!   GO!    ");
  thread_sleep_for(1000);
}

void gameInit()
{
  leftPaddle.x_Pos = 1;
  leftPaddle.y_Pos = 1;

  rightPaddle.x_Pos = 14;
  rightPaddle.y_Pos = 0;

  ball.x_Pos = 8;
  ball.y_Pos = rand() % 2;

  lcd.cls();
}

void moveBall(gameObject *obj)
{
  switch(obj->dir)
  {
    case LEFT:
      obj->x_Pos--;

      if(obj->y_Pos)
      {
        obj->y_Pos--;
      }
      else 
      {
        obj->y_Pos++;
      }
      break;

    case RIGHT:
      obj->x_Pos++;

      if(obj->y_Pos)
      {
        obj->y_Pos--;
      }
      else 
      {
        obj->y_Pos++;
      }
      break;

    default:
      break;
  }

  if(obj->x_Pos < 0)
  {
    rightScore++;
    drawBall(&ball);
    obj->x_Pos = 8;
    obj->y_Pos = rand() % 2;
    obj->dir = static_cast<LeftRight>(rand() % 2);
    thread_sleep_for(1000);
  }
  if(obj-> x_Pos > 15)
  {
    leftScore++;
    drawBall(&ball);
    obj->x_Pos = 8;
    obj->y_Pos = rand() % 2;
    obj->dir = static_cast<LeftRight>(rand() % 2);
    thread_sleep_for(1000);
  }
}

void drawPaddle(gameObject *obj)
{
  
  if(obj->dir == LEFT)
  {
    lcd.character(obj->x_Pos, obj->y_Pos, LEFT_PADDLE);
  }
  else if(obj->dir == RIGHT)
  {
    lcd.character(obj->x_Pos, obj->y_Pos, RIGHT_PADDLE);
  }

  if(obj->y_Pos)
  {
    lcd.locate(obj->x_Pos, obj->y_Pos - 1);
    lcd.printf(" ");
  }
  else
  {
    lcd.locate(obj->x_Pos, obj->y_Pos + 1);
    lcd.printf(" ");
  }
}

void drawBall(gameObject *obj)
{
  lcd.character(obj->x_Pos, obj->y_Pos, BALL);

  switch(obj->dir)
  {
    case LEFT:
      if(obj->y_Pos)
      {
        lcd.locate(obj->x_Pos + 1, obj->y_Pos - 1);
        lcd.printf(" ");
      }
      else
      {
        lcd.locate(obj->x_Pos + 1, obj->y_Pos + 1);
        lcd.printf(" ");
      }
      break;
    case RIGHT:
      if(obj->y_Pos)
      {
        lcd.locate(obj->x_Pos - 1, obj->y_Pos - 1);
        lcd.printf(" ");
      }
      else
      {
        lcd.locate(obj->x_Pos - 1, obj->y_Pos + 1);
        lcd.printf(" ");
      }
      break;
    default:
      break;
  }
}

void paddleAI()
{
  if(rightPaddle.y_Pos)
  {
    rightPaddle.y_Pos--;
  }
  else 
  {
    rightPaddle.y_Pos++;
  }
}

void collisionDetect(gameObject *obj1, gameObject *obj2)
{
  switch (obj1->dir) 
  {
    case LEFT:
      if(obj1->y_Pos == obj2->y_Pos && obj1->x_Pos + 1 == obj2->x_Pos)
      {
        obj2->dir = RIGHT;
      }
      break;
    case RIGHT:
      if(obj1->y_Pos == obj2->y_Pos && obj1->x_Pos - 1 == obj2->x_Pos)
      {
        obj2->dir = LEFT;
      }
      break;
    default:
      break;
  }
}

void endGame()
{
  lcd.cls();

  if(leftScore > rightScore)
  {
    lcd.printf("YOU WIN!");
    thread_sleep_for(1500);
    lcd.locate(0, 1);
    lcd.printf("...yay, I guess?");
  }
  else
  {
    lcd.printf("YOU LOSE!");
    thread_sleep_for(1500);
    lcd.locate(0, 1);
    lcd.printf("GOOD DAY SIR!");
  }
  
  while(1);
}
