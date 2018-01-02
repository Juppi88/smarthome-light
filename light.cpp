#include "light.h"
#include "config.h"
#include "Arduino.h"

void debug_message(const char *message);

Light::Light(int pin)
{
	hardwarePin = pin;
	maxBrightness = MAX_BRIGHTNESS_VALUE;
	transitionTime = 1000; // Use a transition time of 1 second by default

	// Enable the light pin for writing.
	pinMode(pin, OUTPUT);
}

void Light::Process(void)
{
	if (!IsTransitionInProgress()) {
		return;
	}

	// How long has the transition been running?
	int t = millis() - transitionStarted;

	// Check whether the transition has finished.
	if (t >= transitionTime) {
		t = transitionTime;
		EndTransition();
	}

	// Calculate and set the actual brightness of the light.
	brightness = map(t, 0, transitionTime, brightnessFrom, brightnessTo);
	analogWrite(hardwarePin, brightness);
}

void Light::SetToggled(bool toggle)
{
	if (isToggled != toggle) {

		// If the state of the light changed, transition its brightness smoothly to either max or 0.
		isToggled = toggle;
		SetBrightness(ShouldBeToggledOn() ? (isToggled ? maxBrightness : minBrightness) : 0);
	}
}

void Light::SetMinBrightness(int value)
{
	if (minBrightness != value) {
		minBrightness = (value < MAX_BRIGHTNESS_VALUE ? value : MAX_BRIGHTNESS_VALUE);

		// If the light should be toggled on, update its brightness immediately.
		SetBrightness(ShouldBeToggledOn() ? (isToggled ? maxBrightness : minBrightness) : 0);
	}
}

void Light::SetMaxBrightness(int value)
{
	if (maxBrightness != value) {
		maxBrightness = (value < MAX_BRIGHTNESS_VALUE ? value : MAX_BRIGHTNESS_VALUE);

		// If the light is already toggled on, update its brightness immediately.
		if (ShouldBeToggledOn()) {
			SetBrightness(isToggled ? maxBrightness : minBrightness);
		}
	}
}

void Light::SetBrightness(int value)
{
	// If the transition time has been disabled, set the requested brightness value immediately.
	if (transitionTime == 0) {
		
		brightness = value;
		analogWrite(hardwarePin, brightness);

		return;
	}

	// Initialize a smooth brightness transition over time.
	transitionStarted = millis();

	brightnessFrom = brightness;
	brightnessTo = value;
}
