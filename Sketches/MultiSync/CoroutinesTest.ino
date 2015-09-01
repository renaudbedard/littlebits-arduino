#include "Coroutines.h"

Coroutines<5> coroutines;

void test(COROUTINE_CONTEXT(coroutine))
{
	COROUTINE_LOCAL(int, i);

	BEGIN_COROUTINE;
	coroutine.wait(1);
	COROUTINE_YIELD;
  update_off();

  END_COROUTINE;
}

void setup()
{
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  update_off();
  randomSeed(analogRead(0));

  R = new Pong{0, true, 2};
  G = new Pong{0, true, 3};
  B = new Pong{0, true, 4};

  initialize_pulser_settings();
}

void loop() 
{
  unsigned long time = millis();
  coroutines.update(time);

  FIRST = analogRead(PIEZO);

  if (FIRST > THRESHOLD) {
    delay(2);
    SECOND = analogRead(PIEZO);
    if (SECOND > FIRST) {
      coroutines.start(blink);
      delay(1);
      //fake_blink();
    }
  }
}