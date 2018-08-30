const int START_PIN = 2;

int currentIndex = -1;

int getHighPin(int index)
{
	return START_PIN + 7 - (index >> 2);
}

int getLowPin(int index)
{
	return START_PIN + 3 - (index & 3);
}

void unsetCurrent()
{
	if (currentIndex >= 0)
	{
		const int low = getLowPin(currentIndex);
		const int high = getHighPin(currentIndex);
		pinMode(low, INPUT);
		pinMode(high, INPUT);
	}
}

void setPin(int index)
{
	currentIndex = index;
	
	const int low = getLowPin(index);
	const int high = getHighPin(index);
	
	pinMode(low, OUTPUT);
	pinMode(high, OUTPUT);
	digitalWrite(low, LOW);
	digitalWrite(high, HIGH);
}

void setup()
{
	for (int i = START_PIN; i < START_PIN + 8; ++i)
	{
		pinMode(i, INPUT);
		digitalWrite(i, LOW);
	}
}

void light(int index)
{
	unsetCurrent();
	setPin(index);
	delay(40);
}

void loop()
{
	for (int i = 0; i < 14; ++i)
		light(i);

	for (int i = 14; i > 0; --i)
		light(i);
}

