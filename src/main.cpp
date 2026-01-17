#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <EEPROM.h>

struct Ball
{
	int x;
	int y;
	int speed;
	int size;
	bool active;
};

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define pot_pin A0
#define buzzer_pin 3
void spawnBall();
void moveBall();
void deathCheck();
void drawScore();
void drawHearts();

unsigned int bestScore;


void setup()
{
	Serial.begin(9600);

	display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
	display.clearDisplay();
	display.display();
	display.setTextSize(1);
	display.setTextColor(WHITE, BLACK);

	pinMode(buzzer_pin, OUTPUT);

	EEPROM.get(0, bestScore);

	for (size_t i = 3; i > 0; i--)
	{
		display.setCursor(60, 14);
		display.print(i);
		display.display();
		delay(1000);
	}
	
	display.setCursor(60, 14);
	display.print("GO!");
	display.display();
	delay(1000);
	display.clearDisplay();

	drawScore();
	drawHearts();	
	display.display();
}

const unsigned char heart_bmp [] PROGMEM = {
  0b01100110, 
  0b11111111, 
  0b01111110, 
  0b00111100, 
  0b00011000, 
};

unsigned long lastMoveTime = 0;
unsigned long lastSpawnTime = 0;
int pot_value_in_screen_width;
int old_pot_value = 0;
unsigned int score = 0;
unsigned int health = 3;
bool isScoreBeated = false;
Ball balls[5];

void loop()
{
	deathCheck();
	unsigned long currentTime = millis();


	if (currentTime - lastSpawnTime >= 1500)
	{
		lastSpawnTime = currentTime;
		spawnBall();
	}

	if (currentTime - lastMoveTime >= 100)
	{
		lastMoveTime = currentTime;
		moveBall();
	}

	// platform move

	int current_pot_value = analogRead(pot_pin);

	pot_value_in_screen_width = map(current_pot_value, 0, 1023, 128, 0);
	int old_pot_value_in_screen_width = map(old_pot_value, 0, 1023, 128, 0);

	if (old_pot_value != 0)
	{
		display.drawRoundRect(old_pot_value_in_screen_width, 28, 15, 3, 50, SSD1306_BLACK);
	}
	old_pot_value = current_pot_value;
	display.drawRoundRect(pot_value_in_screen_width, 28, 15, 3, 50, SSD1306_WHITE);
	display.display();
}

void spawnBall()
{
	for (int i = 0; i < 5; i++)
	{
		if (balls[i].active == false)
		{
			balls[i].active = true;
			balls[i].x = random(5, 120);
			balls[i].y = 9;
			balls[i].speed = random(1, 3);
			balls[i].size = random(1, 4);

			break;
		}
	}
}
void moveBall()
{
	for (int i = 0; i < 5; i++)
	{
		if (balls[i].active)
		{
			display.drawCircle(balls[i].x, balls[i].y, balls[i].size, SSD1306_BLACK);
			balls[i].y += balls[i].speed;
			display.drawCircle(balls[i].x, balls[i].y, balls[i].size, SSD1306_WHITE);
			if (balls[i].y + balls[i].size >= 28 && balls[i].y + balls[i].size <= 31) 
			{
    			if (balls[i].x + balls[i].size >= pot_value_in_screen_width && balls[i].x - balls[i].size <= pot_value_in_screen_width + 15) {
        			display.drawCircle(balls[i].x, balls[i].y, balls[i].size, SSD1306_BLACK);
        			balls[i].active = false;
        			score++;
					drawScore();
					digitalWrite(buzzer_pin, HIGH);
					delay(10);
					digitalWrite(buzzer_pin, LOW);
					if (isScoreBeated == false && score > bestScore) {
						isScoreBeated = true;
						for(int i=0; i<3; i++) {
  							digitalWrite(buzzer_pin, HIGH); delay(50);
  							digitalWrite(buzzer_pin, LOW);  delay(50);
						}
					}
    			}
			}
			if (balls[i].y > 34)
			{
				display.drawCircle(balls[i].x, balls[i].y, balls[i].size, SSD1306_BLACK);
				balls[i].active = false;
				health--;
				drawHearts();
				digitalWrite(buzzer_pin, HIGH);
				delay(100);
				digitalWrite(buzzer_pin, LOW);
			}
		}
	}
}
void deathCheck () {
	if (health == 0) {
		display.clearDisplay(); 
        
		
        display.setCursor(10, 5); 
        display.setTextSize(1);
        display.print("SCORE: ");
        display.println(score);

        if (score > bestScore) {
            EEPROM.put(0, score);
            display.setCursor(10, 15);
            display.print("NEW BEST: ");
            display.println(score);
        } else {
            display.setCursor(10, 15);
            display.print("BEST SCORE: ");
            display.println(bestScore);
        }
		digitalWrite(buzzer_pin, HIGH);
		delay(500);
		digitalWrite(buzzer_pin, LOW);
        display.display(); 
        
        while (1); 
	}
}
void drawScore() {
	display.setCursor(0, 1);
	display.setTextColor(WHITE, BLACK);
	display.print("SCORE: ");
	display.println(score);
	display.display();
}
void drawHearts () {
	display.fillRect(100, 0, 28, 8, BLACK);
	for (unsigned i = 0; i < health; i++)
	{
		display.drawBitmap(120 - i * 8, 1, heart_bmp, 8, 5, WHITE);
	}
	display.display();
}