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
const int PixelCount = 60;
const int ColourCount = sizeof Colours / sizeof Colours[0];
int currentColour = 0;
int currentIndex = -1;
int nextIndex = 0;
uint32_t nextPixelTime = 0;
uint32_t pixelDuration = 40000;
double durations[PixelCount];

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PixelCount, Output::Lights, NEO_GRB + NEO_KHZ800);
Button colourButton(Input::Colour);
Button modeButton(Input::Mode);

// [0, 1, 0]
/*double getPhase(int pixelIndex)
{
	double y = fabs(pixelIndex - (PixelCount / 2) / double(PixelCount / 2));
	return acos(y) / 1.57;
}

uint32_t getPixelDuration()
{
	//return pixelDuration;

	double t0 = getPhase(getPixelIndex(currentIndex));
	double t1 = getPhase(getPixelIndex(nextIndex));
	
	return pixelDuration * PixelCount * 2 * (fabs(t1 - t0)) / 6.28;
}
*/

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

		Serial.println(y);
		Serial.println(t);
		
		durations[i - 1] = durations[PixelCount - i] = t - lastT; 
		
		lastT = t;
	}
	
	for (int i = 0; i < PixelCount; ++i)
	{
		Serial.println(durations[i]);
	}
}

int getPixelIndex(int index)
{
	return (TotalPixelCount - PixelCount) / 2 + (PixelCount - 1) - abs(index - (PixelCount - 1));
}

// [0, 1, 0]
double getPhase(int pixelIndex)
{
	double y = fabs(pixelIndex - (PixelCount / 2) / double(PixelCount / 2));
	return acos(y) / 1.57;
}

uint32_t getPixelDuration()
{
	//return pixelDuration;

	return pixelDuration * PixelCount / 2 * durations[getPixelIndex(currentIndex)];
	
/*	double t0 = getPhase(getPixelIndex(currentIndex));
	double t1 = getPhase(getPixelIndex(nextIndex));
	
	return pixelDuration * PixelCount * 2 * (fabs(t1 - t0)) / 6.28;
*/
}

void loop() 
{
	uint32_t now = micros();

	if (colourButton.Update(now))
	currentColour = (currentColour + 1) % ColourCount;
	
	if (modeButton.Update(now))
	{
	}

	uint64_t durationVal = analogRead(Input::Speed);
	uint64_t brightnessVal = analogRead(Input::Brightness);

	int brightness = map((brightnessVal * brightnessVal) >> 10, 0, 1023, 3, 255);
	pixelDuration = map(durationVal, 0, 1023, 50000, 5000);
	
	const uint32_t colour = Colours[currentColour];

	if (now > nextPixelTime)
	{
		if (currentIndex >= 0)
			pixels.setPixelColor(getPixelIndex(currentIndex), pixels.Color(0,0,0));

		currentIndex = nextIndex;
		
		pixels.setPixelColor(getPixelIndex(currentIndex), colour);

		pixels.setBrightness(brightness);
		pixels.show();
		
		nextIndex = (currentIndex + 1) % (PixelCount * 2 - 2);
		nextPixelTime = now + getPixelDuration();
	}
}
