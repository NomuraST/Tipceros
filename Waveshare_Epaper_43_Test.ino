//http://www.jarzebski.pl/arduino/komponenty/e-papier-waveshare-4-3.html
//https://www.waveshare.com/wiki/4.3inch_e-Paper_UART_Module
//http://convert-my-image.com/ImageConverter_Fr


#include "myEPD.h"
#include <Zforce.h>

#define DATA_READY 3
#define BUZZER 2
#define TIMOUT_T 8
#define EDP_REFRESH_TIME 1000

//#define PRINT_TOUCH

int State = 0;

struct _TouchState
{
  int Count;
  int ID;
  int TimeOut;
  bool On;
  bool Ev;
  int X;
  int Y;
} Touch;

//*************************************************
void setup(void)
{
  Serial1.begin(115200);
  Serial.begin( 115200 );
  Serial.println("Avant Zforcesetup");
  ZforceSetup();
  Serial.println("Epaper Test => Reset()");
  epd_reset();
  delay(500);
  Serial.println("Reset Done => init()");
  epd_init();
  //epd_set_memory(MEM_NAND);
  epd_set_memory(MEM_TF);
  epd_screen_rotation (1);
  epd_clear();
  epd_set_en_font(ASCII32);
  epd_set_color (BLACK , WHITE);
  epd_disp_string("Setup Done!", 0, 0);
  DotLine();
  epd_udpate();
}

//*************************************************
void loop(void)
{
  
  static int PrevState = -1;
  bool Change = false;
  static long StateTimer=0;

  ZforceLoop();
  if (Touch.Ev)
  {
    Serial.print("Touch X=");
    Serial.print(Touch.X);
    Serial.print(",Y=");
    Serial.println(Touch.Y);
  }

  if(StateTimer<10000) StateTimer++;
  //Serial.println(StateTimer);
  if (State != PrevState) Change = true;
  PrevState = State;

  if (Change)
  {
    Serial.print("State: ");
    Serial.println(State);
    StateTimer=0;
  }

  switch (State)
  {
    case 0:
      if (Change)
      {
        epd_clear();
        epd_disp_bitmap("accueil.BMP", 0, 0);
        epd_udpate();
      }
      if (Touch.Ev && (StateTimer>EDP_REFRESH_TIME)) State = 1;
      break;

    case 1:
      if (Change)
      {
        epd_clear();
        epd_disp_bitmap("tips.BMP", 0, 0);
        epd_udpate();
      }
      if (Touch.Ev && (StateTimer>EDP_REFRESH_TIME)) State = 2;
      break;

    case 2:
      if (Change)
      {
        epd_clear();
        epd_disp_bitmap("flore1.BMP", 0, 0);
        epd_udpate();
      }
      //if ((Touch.X > 360) && (Touch.Y >440) && (StateTimer>EDP_REFRESH_TIME) ) State = 1;
      if (Touch.Ev && (StateTimer>EDP_REFRESH_TIME)) State = 1;
      break;
  }
}

//*************************************************
void Pictures(void)
{
  epd_disp_bitmap("TEST.BMP", 0, 0);
  epd_udpate();
  delay(5000);
  epd_disp_bitmap("PIC01.BMP", 0, 0);
  epd_udpate();
  delay(5000);
  epd_disp_bitmap("PICO02.BMP", 0, 0);
  epd_udpate();
  delay(5000);
}

void Shapes(void)
{
  epd_draw_circle(200, 150, 100);
  epd_draw_circle(300, 250, 100);
  epd_draw_circle(300, 250, 101);
  epd_fill_circle(400, 400, 100);
  epd_draw_triangle(0, 0, 20, 100, 100, 30);
  epd_draw_line(10, 500, 400, 390);
}

void Text(void)
{
  epd_set_en_font(ASCII32);
  epd_set_color (GRAY , WHITE);
  epd_disp_string("ASCII32: Hello, World!", 10, 600);

  epd_set_en_font(ASCII48);
  epd_set_color (DARK_GRAY , WHITE);
  epd_disp_string("ASCII48: Hello, World!", 10, 650);

  epd_set_color (BLACK , WHITE);
  epd_set_en_font(ASCII64);
  epd_disp_string("ASCII64: Hello, World!", 10, 720);

  epd_disp_char('*', 400, 550);

}

void DotLine()
{
  for (int i = 0; i < 600; i += 8)
  {
    epd_draw_pixel(i, 40);
    epd_draw_pixel(i + 1, 40);
    epd_draw_pixel(i + 2, 40);
    epd_draw_pixel(i + 4, 40);
  }
}
//**********************************************************
void ZforceSetup(void)
{
  zforce.Start(DATA_READY);
  Message* msg = zforce.GetMessage();
  zforce.Enable(true);
  msg = zforce.GetMessage();
  do
  {
    msg = zforce.GetMessage();
  } while (msg == NULL);
  zforce.DestroyMessage(msg);
  // Send and read ReverseX
  zforce.ReverseX(true);
  do
  {
    msg = zforce.GetMessage();
  } while (msg == NULL);
  zforce.DestroyMessage(msg);
  // Send and read ReverseY
  zforce.ReverseY(false);
  do
  {
    msg = zforce.GetMessage();
  } while (msg == NULL);
  zforce.DestroyMessage(msg);
  // Send and read Touch Active Area
  //zforce.TouchActiveArea(50, 50, 2000, 4000);
  zforce.TouchActiveArea(0, 50, 3000, 600);
  do
  {
    msg = zforce.GetMessage();
  } while (msg == NULL);

  if (msg->type == MessageType::TOUCHACTIVEAREATYPE)
  {
    Serial.print("minX is: ");
    Serial.println(((TouchActiveAreaMessage*)msg)->minX);
    Serial.print("minY is: ");
    Serial.println(((TouchActiveAreaMessage*)msg)->minY);
    Serial.print("maxX is: ");
    Serial.println(((TouchActiveAreaMessage*)msg)->maxX);
    Serial.print("maxY is: ");
    Serial.println(((TouchActiveAreaMessage*)msg)->maxY);
  }
  zforce.DestroyMessage(msg);
}
//******************************************************
void ZforceLoop()
{
  Touch.Count = 0;
  Touch.Ev = false;

  Message* touch = zforce.GetMessage();
  if (touch != NULL)
  {
    if (touch->type == MessageType::TOUCHTYPE)
    {
      if (Touch.TimeOut == 0) Touch.Ev = true;
      Touch.TimeOut = TIMOUT_T;
      Touch.Count = ((TouchMessage*)touch)->touchCount;
      Touch.ID = ((TouchMessage*)touch)->touchData[0].id;
      Touch.X = ((TouchMessage*)touch)->touchData[0].x;
      Touch.Y = ((TouchMessage*)touch)->touchData[0].y;

#ifdef PRINT_TOUCH

      Serial.print(" ");
      Serial.print("ID: ");
      Serial.print(Touch.ID);
      Serial.print(" ");
      Serial.print("X = ");
      Serial.print(Touch.X);
      Serial.print(" ");
      Serial.print("Y = ");
      Serial.print(Touch.Y);
      Serial.print(" | ");
      Serial.print(Touch.TimeOut);
      Serial.print(" | ");
      Serial.print("Count = ");
      Serial.print(Touch.Count);
      Serial.println();
#endif

      zforce.DestroyMessage(touch);
    }
  }

  if (Touch.TimeOut > 0)
  {
    Touch.TimeOut--;
    Touch.On = true;
  }
  else Touch.On = false;
  delay(2);
}
