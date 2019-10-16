#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

class Button
{
public:
	Button(int pin) : m_pin(pin) {}

	bool Update(uint32_t time)
	{
		bool state = digitalRead(m_pin) == LOW;
		
		if (m_locked)
		{
			if (time < m_unlockTime)
			return false;

			m_locked = false;
		}

		if (state != m_state)
		{
			m_locked = true;
			m_unlockTime = time + LockDuration;
			m_state = state;
			return state;
		}

		return false;
	}

private:
	int m_pin;
	bool m_state = false;
	bool m_locked = false;
	uint32_t m_unlockTime = 0;
	const uint32_t LockDuration = 100000;
};

namespace Output
{
	const int Lights = 6;
}

namespace Input
{
	const int Colour = 8;
	const int Mode = 9;
	const int Speed = A0;
	const int Brightness = A1;
}

const uint32_t Colours[] = 
{
	Adafruit_NeoPixel::Color(255, 255, 255),
	Adafruit_NeoPixel::Color(255, 0, 0),
	Adafruit_NeoPixel::Color(0, 255, 0),
	Adafruit_NeoPixel::Color(255, 255, 0),
	Adafruit_NeoPixel::Color(0, 0, 255),
	Adafruit_NeoPixel::Color(255, 0, 255),
	Adafruit_NeoPixel::Color(0, 255, 255),
};

const int TotalPixelCount = 60;
const int PixelCount = 40;
const int StartPixel = (TotalPixelCount - PixelCount) / 2;
const int ColourCount = sizeof Colours / sizeof Colours[0];
int currentColour = 0;
int currentIndex = -1;
int nextIndex = 0;
uint32_t nextPixelTime = 0;
uint32_t pixelDuration = 40000;
int mode;
double durations[PixelCount];

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(TotalPixelCount, Output::Lights, NEO_GRB + NEO_KHZ800);
Button colourButton(Input::Colour);
Button modeButton(Input::Mode);

void setup() 
{
	pixels.begin();
	pixels.setBrightness(0);
	pixels.show();

	pinMode(Input::Colour, INPUT_PULLUP);
	pinMode(Input::Mode, INPUT_PULLUP);

	Serial.begin(9600);
	
	double lastT = 0;
	for (int i = 1; i <= PixelCount / 2; ++i)
	{
		double y = 1.0 - i / (PixelCount / 2.0);
		double t = acos(y) / 1.57; // [0, 1]

		// Serial.println(y);
		// Serial.println(t);
		
		durations[i - 1] = durations[PixelCount - i] = t - lastT; 
		
		lastT = t;
	}
	
	// for (int i = 0; i < PixelCount; ++i)
	// {
		// Serial.println(durations[i]);
	// }
}

int getPixelIndex(int index)
{
	return (PixelCount - 1) - abs(index - (PixelCount - 1));
}

uint32_t getPixelDuration()
{
	switch (mode)
	{
		case 0: return pixelDuration;
		case 1: return pixelDuration * PixelCount / 2 * durations[getPixelIndex(currentIndex)];
	}
}

void loop() 
{
	uint32_t now = micros();

	if (colourButton.Update(now))
	currentColour = (currentColour + 1) % ColourCount;
	
	if (modeButton.Update(now))
	{
		mode = (mode + 1) % 2;
	}

	uint64_t durationVal = analogRead(Input::Speed);
	uint64_t brightnessVal = analogRead(Input::Brightness);

	int brightness = map((brightnessVal * brightnessVal) >> 10, 0, 1023, 3, 255);
	pixelDuration = map(durationVal, 0, 1023, 50000, 5000);
	
	const uint32_t colour = Colours[currentColour];

	if (now > nextPixelTime)
	{
		if (currentIndex >= 0)
			pixels.setPixelColor(StartPixel + getPixelIndex(currentIndex), pixels.Color(0,0,0));

		currentIndex = nextIndex;
		
		pixels.setPixelColor(StartPixel + getPixelIndex(currentIndex), colour);

		pixels.setBrightness(brightness);
		pixels.show();
		
		nextIndex = (currentIndex + 1) % (PixelCount * 2 - 2);
		nextPixelTime = now + getPixelDuration();
	}
}
