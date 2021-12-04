#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define FPS 100 // define the frames per second that you'd like to play the game at 

///////////game info storage 
#define num_boxes 37
#define num_platforms 1
int boxes[num_boxes][2];    // stores the x pos, y pos and number of consecutive boxes for the whole map  
int boundaries[num_platforms][4];        // this stores the values of (x_1, y_1, x_2, y_2) of all platforms other than the bottom platform 
int boxes_p = 0;           // zeroth platform is always the bottom platform
int game_state = 1;         // 0 is the play game page (to be added), 1 is the game, 2 is player 1 won, 3 is player 2 won 

float death_state[] = {0, 1};

float x[2][2] = {{0, 1},{1, 0}}; 
float z[2][2] = {{1, 0},{0, -1}}; 
float h[2][2] = {{1/sqrt(2), 1/sqrt(2)}, {1/sqrt(2), -1/sqrt(2)}};

///////////pins
////p1
const int jt1_sw_pin = 12; //digial pin 
const int jt1_x_pin = 0;  // analog pin 
const int jt1_y_pin = 1;  // analog pin 
const int p1_jump_pin = 4; 
const int p1_shoot_pin = 5; 
const int p1_gate_pin = 6; 
const int p1_b4_pin = 7; 

boolean p1_jump_button = 0; 
boolean p1_jump_button_prev = 0; 
boolean p1_shoot_button = 0; 
boolean p1_shoot_button_prev = 0; 
boolean p1_gate_button = 0; 
boolean p1_gate_button_prev = 0; 

////p2
const int jt2_sw_pin = 13; //digital pin 
const int jt2_x_pin = 2; // analog pin
const int jt2_y_pin = 3; // analog pin
const int p2_jump_pin = 8; 
const int p2_shoot_pin = 9; 
const int p2_gate_pin = 10; 
const int p2_b4_pin = 11;


boolean p2_jump_button = 0; 
boolean p2_jump_button_prev = 0; 
boolean p2_shoot_button = 0; 
boolean p2_shoot_button_prev = 0; 
boolean p2_gate_button = 0; 
boolean p2_gate_button_prev = 0; 

///////////////////////////////////defining some variables////////////////////////////
/////////////time 
//unsigned long currentTime; 
//unsigned long startTime;
//float loop_time = 1000/FPS;

///////////dim
//player 1 dimensions 
byte player_width = 8;
byte player_height = 8;

///////////pos 
//player positions
struct player {
  int player_number; 
  int x; 
  int y; 
  int vx; 
  int vy; 
  int right; 
  int up; 
  int ammo; 
  int health; 
  float alpha; 
  float beta; 
  byte gate; 
  int bullet_x; 
  int bullet_y; 
  int bullet_vx;
  int bullet_vy; 
  int bullet_gate; //what gate are you shooting
  int bullet_exists; // so that the bullet doesnt randomly respawn when clicked again. Rather you can only shoot when the buttlet hits a player or goes out of the screen
}; 
////
//p1 = {20, 40 - player_height, 0, 0, 1, 0, 1, 0, 0};
//p2 = {40, 60 - player_height, 0, 0, 1, 0, 1, 0, 0};
//x pos, y pos, vx, vy, right, up, alpha, beta, gate (0 = X, 1 = Z, 2 = H) 
//
//p1_bullet={128, 64, 0, 0};
//p2_bullet={128, 64, 0, 0};

// -1 - left, 0 - none, 1 - right 
// 1 - up, 0 - none, -1 - down

player p1 = {1, 20, 40 - player_height, 0, 0, 1, 0, 20, 100, 1, 0, 0, 128, 64, 0, 0, 0, 0}; // reason for 60 - player height is cause the box takes up 4 pixels from the total of 64 
player p2 = {2, 40, 60 - player_height, 0, 0, 1, 0, 20, 100, 1, 0, 0, 128, 64, 0, 0, 0, 0}; 
 

/////////////pace of game
//bullet speed 
const int bullet_speed = 6; 
const int player_speed = 4; 
const int jump_speed = 5; 
const int gravity = 1; 

static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

static const unsigned char PROGMEM h_gate[] = {
  B11111111, B10000000,
  B10000000, B10000000,
  B10100010, B10000000,
  B10100010, B10000000,
  B10111110,B10000000,
  B10100010,B10000000,
  B10100010,B10000000,
  B10000000,B10000000,
  B11111111,B10000000,
};

static const unsigned char PROGMEM x_gate[] = {
  B11111111,B10000000,
  B10000000,B10000000,
  B10100010,B10000000,
  B10010100,B10000000,
  B10001000,B10000000,
  B10010100,B10000000,
  B10100010,B10000000,
  B10000000,B10000000,
  B11111111,B10000000,
};

static const unsigned char PROGMEM z_gate[] = {
  B11111111,B10000000,
  B10000000,B10000000,
  B10111100,B10000000,
  B10000100,B10000000,
  B10001000,B10000000,
  B10010000,B10000000,
  B10011110,B10000000,
  B10000000,B10000000,
  B11111111,B10000000,
};

#define box_weight 4
#define box_height 4
static const unsigned char PROGMEM box[] = {
  B11111000,
  B10001000,
  B10011000,
  B11001000,
  B11111000,
};

static const unsigned char PROGMEM box4[] = {
  B11110000,
  B10010000,
  B11010000,
  B11110000,
};

static const unsigned char PROGMEM player_sprite_r[] =
{ B00000000, 
  B00000000,
  B00011000, 
  B00011000, 
  B00010110, 
  B00010100, 
  B00111000, 
  B00101000, 
  };

static const unsigned char PROGMEM player_sprite_ru[] =
{ B00000010, 
  B00000100,
  B01101000, 
  B01110100, 
  B00010000, 
  B00010000, 
  B00111000, 
  B00101000, 
  };

static const unsigned char PROGMEM player_sprite_u[] =
{ B00010000, 
  B00011000,
  B01100000, 
  B01110000, 
  B00010000, 
  B00010000, 
  B00111000, 
  B00101000, 
  };

static const unsigned char PROGMEM player_sprite_lu[] =
{ B10000000, 
  B01000000,
  B00101100, 
  B01011100, 
  B00010000, 
  B00010000, 
  B00111000, 
  B00101000, 
  };

static const unsigned char PROGMEM player_sprite_l[] =
{ B00000000, 
  B00000000,
  B00110000, 
  B00110000, 
  B11010000, 
  B01010000, 
  B00111000, 
  B00101000, 
  };

static const unsigned char PROGMEM player_sprite_ld[] =
{ B00000000, 
  B00000000,
  B01100000, 
  B01110000, 
  B00010000, 
  B01010000, 
  B10111000, 
  B00101000, 
  };

static const unsigned char PROGMEM player_sprite_d[] =
{ B00000000, 
  B00010000,
  B00111000, 
  B00111000, 
  B00010000, 
  B00010000, 
  B00111000, 
  B00101000, 
  };

static const unsigned char PROGMEM player_sprite_rd[] =
{ B00000000, 
  B00000000,
  B00001100, 
  B00011100, 
  B00010000, 
  B00010100, 
  B00111010, 
  B00101000, 
  };

////////////////some function declarations/////////////
void init_boxes(void);
//////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  //////////////define pins 
  pinMode(jt1_sw_pin, INPUT); 
  pinMode(jt2_sw_pin, INPUT); 
  digitalWrite(jt1_sw_pin, HIGH); 
  digitalWrite(jt2_sw_pin, HIGH); 
  
  pinMode(p1_jump_pin, INPUT_PULLUP); 
  pinMode(p1_shoot_pin, INPUT_PULLUP); 
  pinMode(p1_gate_pin, INPUT_PULLUP); 

  pinMode(p2_jump_pin, INPUT_PULLUP); 
  pinMode(p2_shoot_pin, INPUT_PULLUP); 
  pinMode(p2_gate_pin, INPUT_PULLUP); 
  
  pinMode(13, OUTPUT); 
  digitalWrite(13, LOW); 
  init_boxes();
  delay(1000); 

}


///////////////////////////////////////////////////////loop starts/////////////////////////////////
void loop() {
  if (game_state==1){ 
  //   accounting for fps by recording start time 
//     startTime = millis();
  //   put your main code here, to run repeatedly:

    // read the pins
     p1_jump_button = digitalRead(p1_jump_pin); 
     p1_shoot_button = digitalRead(p1_shoot_pin); 
     p1_gate_button = digitalRead(p1_gate_pin);  

     p2_jump_button = digitalRead(p2_jump_pin); 
     p2_shoot_button = digitalRead(p2_shoot_pin); 
     p2_gate_button = digitalRead(p2_gate_pin); 
     
     display.clearDisplay();
     draw_level(); 
     player_status(&p2, jt2_x_pin, jt2_y_pin, p2_jump_pin, p2_shoot_pin, p2_gate_pin, p2_b4_pin);
     player_status(&p1, jt1_x_pin, jt1_y_pin, p1_jump_pin, p1_shoot_pin, p1_gate_pin, p1_b4_pin);
     draw_player(p1);
     draw_player(p2);
     display.display();

     p1_jump_button_prev = p1_jump_button; 
     p1_shoot_button_prev = p1_shoot_button;
     p1_gate_button_prev = p1_gate_button; 

     p2_jump_button_prev = p2_jump_button; 
     p2_shoot_button_prev = p2_shoot_button;
     p2_gate_button_prev = p2_gate_button; 
    
  //  Serial.print("Switch:  "); 
  //  Serial.print(digitalRead(jt1_sw_pin)); 
  //  Serial.print("\n"); 
  //  Serial.print("X Value:  "); 
  //  Serial.print(analogRead(jt1_x_pin)); 
  //  Serial.print("\n"); 
  //  Serial.print("Y Value:  "); 
  //  Serial.print(analogRead(jt1_y_pin)); 
  //  Serial.print("\n");
  //  Serial.print("button value:   ");
  //  Serial.print(digitalRead(p1_jump_pin)); 
  //  Serial.print("\n"); 
  //  Serial.print("Box value:   ");
  //  Serial.print(boxes[1][0]); 
  //  Serial.print(", ");
  //  Serial.print(boxes[1][1]);
  //  Serial.print("\n"); 
  //  if(digitalRead(p1_jump_pin)==0){
  //      digitalWrite(13, HIGH); 
  //    }
  //  else{
  //      digitalWrite(13, LOW);   
  ////    }
  //  int val = is_in_a_box(10, 10); 
  //  Serial.println(val); 
  //  for(int i=0; i < 1000; i++){
  //      is_in_a_box(10, 10); 
  ////      Serial.print(","); 
  //    }
  //  Serial.print("val: ");
  //  Serial.print(val); 
//  //  Serial.print("\n");  
//    Serial.print("Time taken for a single loop: ");
//    Serial.print(millis() - startTime); 
//    Serial.print("\n"); 
//  //    delay(50); // enable only while debugging
//     while(millis() - startTime<loop_time){
//      Serial.print("Hey there, do we ever go into this looop?,\n");
//      }
//    Serial.print("Time taken at end of loop: ");
//    Serial.print(millis() - startTime); 
//    Serial.print("\n"); 
  }
  if (game_state == 2){
    display.setCursor(30,30); 
    display.setTextSize(1); 
    display.setTextColor(WHITE);
    display.println("Player 1 wins!");
    }
  if (game_state == 3){
    display.setCursor(30,30); 
    display.setTextSize(1); 
    display.setTextColor(WHITE);
    display.println("Player 2 wins!");
    }
}

///////////////////////////////////////////////////////loop ends/////////////////////////////////

void draw_level(void){
    draw_platform(0, 60, 32); 
//    draw_platform(10, 15, 5);
    // drawing level platforms 
//    draw_platform(40, 40, 10);
    display.drawFastHLine(0, 12, 128, 1);
    display.setCursor(50,1); 
    display.setTextSize(1); 
    display.setTextColor(WHITE);
    display.println("|1>");
  }


void draw_platform(int x, int y, int num_blocks){
    // this function draws a platform with num_blocks number of blocks at (x,y) continuing onto the right and printing below
    boundaries[boxes_p][0] = x;
    boundaries[boxes_p][1] = y;
    boundaries[boxes_p][2] = x + 4*num_blocks;
    boundaries[boxes_p][3] = y;
    boxes_p++;

    for(int i = 0; i < num_blocks; i++){
      display.drawBitmap(x + 4*i, y, box4, box_height, 4, 1);
//      boxes[boxes_p][0] = x + 4*i; 
//      boxes[boxes_p][1] = y; 
      }
  }

void draw_player(player p) {
  // orientation: right = 1 means that the player is pointing to the right, right = -1 the player is pointing left. 0 is none
  // -1 - left, 0 - none, 0 - right
  // 1 - up, 0 - none, -1 - down 
  if(p.right == 1 && p.up == 0){
      display.drawBitmap(p.x, p.y, player_sprite_r, player_width, player_height, 1);
    }
  if(p.right == 1 && p.up == 1){
      display.drawBitmap(p.x, p.y, player_sprite_ru, player_width, player_height, 1);
    }
  if(p.right == 0 && p.up == 1){
      display.drawBitmap(p.x, p.y, player_sprite_u, player_width, player_height, 1);
    }
  if(p.right == -1 && p.up == 1){
      display.drawBitmap(p.x, p.y, player_sprite_lu, player_width, player_height, 1);
    }
  if(p.right == -1 && p.up == 0){
      display.drawBitmap(p.x, p.y, player_sprite_l, player_width, player_height, 1);
    }
  if(p.right == -1 && p.up == -1){
      display.drawBitmap(p.x, p.y, player_sprite_ld, player_width, player_height, 1);
    }
  if(p.right == 0 && p.up == -1){
      display.drawBitmap(p.x, p.y, player_sprite_d, player_width, player_height, 1);
    }
  if(p.right == 1 && p.up == -1){
      display.drawBitmap(p.x, p.y, player_sprite_rd, player_width, player_height, 1);
    }
  if(p.right == 0 && p.up == 0){
      display.drawBitmap(p.x, p.y, player_sprite_r, player_width, player_height, 1);
  }
}

void player_status( struct player *p, int x_rot, int y_rot, int button1, int button2, int button3, int button4){
  // define changes based on joystick rotation 
  int x_value, y_value; 
  if ((*p).player_number == 1){
    x_value = analogRead(jt1_x_pin); 
    y_value = analogRead(jt1_y_pin); 
  }
  else{
    x_value = analogRead(jt2_x_pin); 
    y_value = analogRead(jt2_y_pin); 
  }

  if(x_value>=256 && x_value<=768 && y_value<256){
    //pointing right 
    (*p).right = 1; 
    (*p).up = 0; 
    (*p).vx = (*p).right * player_speed; 
  }
  else if(x_value < 256 && y_value < 256){
    // pointing right up 
    (*p).right = 1; 
    (*p).up = 1; 
    (*p).vx =(*p).right * player_speed; 
  }
  else if(x_value < 256 && y_value >=256 && y_value <= 768){
    // pointing up
    (*p).right = 0; 
    (*p).up = 1; 
    (*p).vx = (*p).right * player_speed; 
  }
  else if(x_value < 256 && y_value > 768){
    // pointing left up 
    (*p).right = -1; 
    (*p).up = 1; 
    (*p).vx = (*p).right * player_speed; 
  }
  else if(x_value>= 256 && x_value <=768 && y_value >768){
    // pointing left 
    (*p).right = -1; 
    (*p).up = 0; 
    (*p).vx = (*p).right * player_speed; 
  }
  else if(x_value > 768 && y_value > 768){
    //pointing left down 
    (*p).right = -1; 
    (*p).up = -1; 
    (*p).vx = (*p).right * player_speed; 
  }
  else if(x_value > 768 && y_value >=256 && y_value <= 768){
    // pointing down
    (*p).right = 0;
    (*p).up = -1; 
    (*p).vx = (*p).right * player_speed; 
  }
  else if(x_value > 768 && y_value < 256){
    // pointing right down 
    (*p).right = 1; 
    (*p).up = -1; 
    (*p).vx = (*p).right * player_speed; 
  }
  else{
    (*p).right = 1; 
    (*p).up = 0; 
    (*p).vx = 0; 
  }

//  // check horizontal collisions - take a total of 6 points, 3 on each side of the player to decide as the player has a size of 6 and a box size is 4.
//  if(is_in_a_box((*p).x+1+(*p).vx, (*p).y+2+(*p).vy)
//  || is_in_a_box((*p).x+1+(*p).vx, (*p).y+4+(*p).vy) 
//  || is_in_a_box((*p).x+1+(*p).vx, (*p).y+7+(*p).vy) 
//  || is_in_a_box((*p).x+3+(*p).vx, (*p).y+2+(*p).vy) 
//  || is_in_a_box((*p).x+3+(*p).vx, (*p).y+4+(*p).vy)
//  || is_in_a_box((*p).x+3+(*p).vx, (*p).y+7+(*p).vy)){
//    (*p).vx = 0; 
//  }
//
 // check bottom collision, i.e, is the player standing on a box? if not, apply gravity 
// if((is_in_a_box((*p).x+1+(*p).vx, (*p).y+8+(*p).vy) == 1) || (is_in_a_box((*p).x+3+(*p).vx, (*p).y+8+(*p).vy)) == 1){
//   (*p).vy = 0; 
// }else{
//   (*p).vy = (*p).vy + gravity; 
// }

boolean collided = false;
//
//  for(int i=0; i < num_platforms; i++){
//      if ((*p).y + (*p).vy + 8 >= boundaries[i][1]){
//        if (((*p).x <= boundaries[i][2]) && ((*p).x + 8 >= boundaries[i][0])){
//          (*p).vy = 0;  
//        }else{
//          collided = true;
////          (*p).vy = (*p).vy + gravity;
//        }
//      }else{
//        collided = true;
////        (*p).vy = (*p).vy + gravity;
//      }
//   }
//boundaries[0][1]

//   for(int i=0; i < num_platforms; i++){
      if ((*p).y + (*p).vy + 8 >= 60){
          (*p).vy = 0;  
//          (*p).vy = (*p).vy + gravity;
       
      }else{
//        collided = true;
        (*p).vy = (*p).vy + gravity;
      }
//   }

//  if(collided == true){
//      (*p).vy = (*p).vy + gravity;
//    }
   
  // alternate bottom collision checking on the ground

//  if ((*p).y + (*p).vy + 8 >= 60){
//    (*p).vy = 0;
//  }else{
//    (*p).vy = (*p).vy + gravity;
//  }

  if ((*p).x + (*p).vx <= 0){
    (*p).x = 1; 
    (*p).vx = 0; 
    }

  if ((*p).x + (*p).vx >= 120){
    (*p).x = 119; 
    (*p).vx = 0; 
    }

/////////////jump///////////////

  // if the jump button is pressed 
//  if((*p).vy == 0 && digitalRead(button1)==0){
//    (*p).vy = -1*jump_speed; 
//  }
  if ((*p).player_number == 1){
      if((*p).vy == 0 && p1_jump_button != p1_jump_button_prev && p1_jump_button == 0){
        (*p).vy = -1*jump_speed; 
      }
  }else{
      if((*p).vy == 0 && p2_jump_button != p2_jump_button_prev && p2_jump_button == 0){
        (*p).vy = -1*jump_speed; 
      }
    }

////////////change gates and print them above/////////////

//update
//0 = X, 1 = Z, 2 = H
if ((*p).player_number == 1){
      if(p1_gate_button != p1_gate_button_prev && p1_gate_button == 0){
        if((*p).gate == 2){
            (*p).gate = 0;
          }
          else{
              (*p).gate = (*p).gate + 1; 
            }
      }   
  }else{
      if(p2_gate_button != p2_gate_button_prev && p2_gate_button == 0){
        if((*p).gate == 2){
            (*p).gate = 0;
          }
          else{
              (*p).gate = (*p).gate + 1; 
            }
      }
    }

//draw gates 

drawgate((*p).player_number, (*p).gate);

if ((*p).player_number == 1){
      if((*p).vy == 0 && p1_jump_button != p1_jump_button_prev && p1_jump_button == 0){
        (*p).vy = -1*jump_speed; 
      }
  }else{
      if((*p).vy == 0 && p2_jump_button != p2_jump_button_prev && p2_jump_button == 0){
        (*p).vy = -1*jump_speed; 
      }
    }

// initiating and propogating bullet 
if((*p).player_number == 1){
    if(p1_shoot_button != p1_shoot_button_prev && p1_shoot_button == 0){
      (*p).bullet_x = (*p).x + 4; 
      (*p).bullet_y = (*p).y + 4;
      (*p).bullet_vx =(*p).right*bullet_speed; // gotta change this 
      (*p).bullet_vy = -1 * (*p).up*bullet_speed;
      (*p).bullet_gate = (*p).gate;
      display.drawPixel((*p).bullet_x, (*p).bullet_y, 1);
    }
  }else{
      if(p2_shoot_button != p2_shoot_button_prev && p2_shoot_button == 0){
      (*p).bullet_x = (*p).x + 4; 
      (*p).bullet_y = (*p).y + 4;
      (*p).bullet_vx = (*p).right*bullet_speed; 
      (*p).bullet_vy = -1 * (*p).up*bullet_speed;
      (*p).bullet_gate = (*p).gate; 
      display.drawPixel((*p).bullet_x, (*p).bullet_y, 1);
  }
    }
//update bullet position 

//edge cases
if ((*p).bullet_x <= 0 || (*p).bullet_x >= 128){
    (*p).bullet_x = 128; 
    (*p).bullet_y = 64; 
    (*p).bullet_vx = 0; 
    (*p).bullet_vy = 0;  
    }
if ((*p).bullet_y <= 0 || (*p).bullet_y >= 64){
    (*p).bullet_x = 128; 
    (*p).bullet_y = 64; 
    (*p).bullet_vx = 0; 
    (*p).bullet_vy = 0;  
    }

//collision with opponent
if ((*p).player_number == 1){
  if((*p).bullet_x <= p2.x + 8 && (*p).bullet_x >= p2.x && (*p).bullet_y >= p2.y && (*p).bullet_y <= p2.y + 8){
      (*p).bullet_x = 128; 
      (*p).bullet_y = 64; 
      (*p).bullet_vx = 0; 
      (*p).bullet_vy = 0;

      applygate(2, (*p).gate);
    }
  }else{
      if((*p).bullet_x <= p1.x + 8 && (*p).bullet_x >= p1.x && (*p).bullet_y >= p1.y && (*p).bullet_y <= p1.y + 8){
          (*p).bullet_x = 128; 
          (*p).bullet_y = 64; 
          (*p).bullet_vx = 0; 
          (*p).bullet_vy = 0;
    
          applygate(1, (*p).gate);
        }
    
    }

(*p).bullet_x = (*p).bullet_x + (*p).bullet_vx;
(*p).bullet_y = (*p).bullet_y + (*p).bullet_vy;
  
display.drawPixel((*p).bullet_x, (*p).bullet_y, 1);

//  // define changes based on other buttons pressed 
//
//  // check for top collisions
//  if((*p).vy < 0 && (is_in_a_box((*p).x+1+(*p).vx, (*p).y+2+(*p).vy) || is_in_a_box((*p).x+3+(*p).vx, (*p).y+2+(*p).vy))){
//    (*p).vy = 0; 
//  }
//
  // updating coordinates
  (*p).x = (*p).x + (*p).vx; 
  (*p).y = (*p).y + (*p).vy; 

//check if game ended
if((*p).alpha == death_state[0] && (*p).beta == death_state[1]){
  if((*p).player_number == 1){
      game_state = 3; 
    }
  if((*p).player_number == 2){
      game_state = 2; 
    }
  }

}

void applygate(int player_number, int gate_number){
    if(player_number==1){
        if(gate_number == 0){
            float temp1 = x[0][0]*p1.alpha + x[0][1]*p1.beta;
            float temp2 =  x[1][0]*p1.alpha + x[1][1]*p1.beta;
            p1.alpha = temp1; 
            p1.beta = temp2; 
          }
          if(gate_number == 1){
            float temp1 = z[0][0]*p1.alpha + z[0][1]*p1.beta;
            float temp2 =  z[1][0]*p1.alpha + z[1][1]*p1.beta;
            p1.alpha = temp1; 
            p1.beta = temp2; 
          }
          if(gate_number == 1){
            float temp1 = h[0][0]*p1.alpha + h[0][1]*p1.beta;
            float temp2 =  h[1][0]*p1.alpha + h[1][1]*p1.beta;
            p1.alpha = temp1; 
            p1.beta = temp2; 
          }
      }else{
        if(gate_number == 0){
            float temp1 = x[0][0]*p2.alpha + x[0][1]*p2.beta;
            float temp2 =  x[1][0]*p2.alpha + x[1][1]*p2.beta;
            p2.alpha = temp1; 
            p2.beta = temp2; 
          }
          if(gate_number == 1){
            float temp1 = z[0][0]*p2.alpha + z[0][1]*p2.beta;
            float temp2 =  z[1][0]*p2.alpha + z[1][1]*p2.beta;
            p2.alpha = temp1; 
            p2.beta = temp2; 
          }
          if(gate_number == 1){
            float temp1 = h[0][0]*p2.alpha + h[0][1]*p2.beta;
            float temp2 =  h[1][0]*p2.alpha + h[1][1]*p2.beta;
            p1.alpha = temp1; 
            p1.beta = temp2; 
          }
        }
  }

void drawgate(int player_number, int gate_number){
  int gate_xpos = 0; 
  int gate_ypos = 0;

  if (player_number == 1){
      gate_xpos = 1; 
      gate_ypos = 1; 
    }else{
      gate_xpos = 118; 
      gate_ypos = 1;   
    }
    
  if(gate_number == 0){
      display.drawBitmap(gate_xpos, gate_ypos, x_gate, 9, 9, 1);
    }
  if(gate_number == 1){
      display.drawBitmap(gate_xpos, gate_ypos, z_gate, 9, 9, 1);
    }
  if(gate_number == 2){
      display.drawBitmap(gate_xpos, gate_ypos, h_gate, 9, 9, 1);
    }
  
  }

int is_in_a_box(int x, int y){
  int in_a_box = 0; 
  for(int i = 0; i < num_boxes; i++){
    if((boxes[i][0] <= x) && (boxes[i][0] + 4 >= x) && (boxes[i][1] <= y) && (boxes[i][1] + 4 >= y)){
      in_a_box = 1; 
    }
  }
  return in_a_box; 
}

void init_boxes(void){
  for(int i=0; i < num_boxes; i++){
    boxes[i][0] = -10; 
    boxes[i][1] = -10; 
  }
}
