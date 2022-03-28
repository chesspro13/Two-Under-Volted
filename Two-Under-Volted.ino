#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define FEET_PER_REVOLUTION 6.9
#define MILE 5280
#define SPEED_SENSOR 2
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SPEED SENSOR===================================================================================================================
// Data
short tireResolution = 4; // Number of magnets that make a full resolution. More is better
int revolutionsPerSecond = 0; // Counts how many revolutions have happened in one second. This is constantly changing
double wheelHeightWithoutDeflation = 26.5;
double wheelHeightWithDeflation = 26.25;
double feetPerMagnet = FEET_PER_REVOLUTION / tireResolution;


// Inturrupt
volatile bool takeReading = false;


// Time
double now;
double updateFrequancy = 1;
double lastUpdate;
long lastInturrupt;
double timeSinceLastTick = 0;
double refreshRate = 0.25;
long lastRefresh;


// Display Data
double curMPH = 88;


long totalDistance;
short hours;
short minutes;
short seconds;

int miles;
short partial;


void setup() {
  Serial.begin(9600);

  // SPEED SENSOR===================================================================================================================
  pinMode( SPEED_SENSOR, INPUT_PULLUP );
  attachInterrupt(digitalPinToInterrupt(SPEED_SENSOR), record, FALLING);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
}

void loop() {

	// Get current time in millis for time calculations.
	// 	Used to determin if it is time to update display, take reading
  now = millis();

	// Check to see if it has been long enough to refresh the Oled display
  if( now >= lastRefresh + (refreshRate * 1000) )
    display.display();

	// Check to see if it is time to update all of the calculatons
  if( now >= lastUpdate + (updateFrequancy * 1000) )
  {
    {
      curMPH = (((feetPerMagnet*(revolutionsPerSecond/tireResolution))/((updateFrequancy)))*0.6818);
    }
    lastUpdate = now;
    revolutionsPerSecond = 0;
  }
  
	// Inutrrupt has triggered which means the sensor has found a magnet. Start gathering data
  if( takeReading == true )
  {    
    timeSinceLastTick = now - lastInturrupt;
    
    revolutionsPerSecond += 1;
    totalDistance += 1;

    // Reset
    lastInturrupt = now;
    takeReading = false;
  }

  printToOled();
}


void record()
{
  takeReading = true;
}


	// Speed graph
void speedGraph(int offset) {
  display.fillRect(96+10, 40 - (offset * 10), 20, 8, SSD1306_INVERSE);
}


	// Used to mirror the speed graph until I can get data for the battery pack voltage.
void batteryGraph(int offset) {
  display.fillRect(78+10, 40 - (offset * 10), 20, 8, SSD1306_INVERSE);
}


	// Get data
void gatherData()
{
  // Graphs
  int bars = curMPH/5 ;
  
  for( int i = 0; i <= bars; i++ )
  {
    speedGraph(i);
    batteryGraph(i);
  }

  // Time
  long now = millis()/1000;
  
  hours = (now/60)/60;
  now -= hours*60;
  minutes = now/60;
  now -= minutes*60;
  seconds = now;

  // Distance

  now = millis();
  
  totalDistance = (now/1000)*6.9;
  miles = int(totalDistance / MILE);
  partial = int(double((totalDistance - miles*double(MILE))/double(MILE))*10);
}


void printToOled() {
  display.clearDisplay();

  gatherData();

  // Setup
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Speed
  display.setTextSize(6);
  display.setCursor(0, 0);
  if( curMPH < 10 )
    display.println("0" + String(int(curMPH)));
  else
    display.println(String(int(curMPH)));
  

  // Distance
  display.setTextSize(3);
    // Whole Miles
  display.setCursor(0, 44);     
  if( miles < 10 )
    display.println("0" + String(miles));
  else
    display.println(String(miles));
    // Period
  display.fillRect(35, 60, 4, 4, SSD1306_WHITE);
    // Partial Mile
  display.setCursor(41, 44);
  display.println(String(int(partial)));

  // Time
    // Setup
  display.setTextSize(2);    
    // HOUR
  display.setCursor(64, 50);
  display.println(String(hours));
    // COLON
  display.drawRect(75, 54, 2, 2, SSD1306_WHITE);
  display.drawRect(75, 58, 2, 2, SSD1306_WHITE);
    // MINUTE
  display.setCursor(78, 50);
  if( minutes < 10 )
    display.println("0" + String(minutes));
  else
    display.println(String(minutes));
    // COLON
  display.drawRect(101, 54, 2, 2, SSD1306_WHITE);
  display.drawRect(101, 58, 2, 2, SSD1306_WHITE);
    // SECOND
  display.setCursor(104, 50);
  if( seconds < 10 )
    display.println("0" + String(seconds));
  else
    display.println(String(seconds));

  
  display.setTextSize(1);    
  display.setCursor(68, 0);
  display.println("MPH");
  
  display.display();
}
