#ifndef __LIGHT_H
#define __LIGHT_H

#define MAX_BRIGHTNESS_VALUE ((1 << LIGHT_PWM_BITS) - 1)

class Light
{
public:
	Light(int pin);

	void Process(void);

	void SetToggled(bool toggle);
	void SetMinBrightness(int value);
	void SetMaxBrightness(int value);
	void SetTransitionTime(int ms) { transitionTime = (ms < 0 ? 0 : ms); }

private:
	void SetBrightness(int value);

	bool ShouldBeToggledOn(void) const { return (isToggled || minBrightness != 0); }
	bool IsTransitionInProgress(void) const { return transitionStarted != 0; }
	void EndTransition(void) { transitionStarted = 0; }

private:
	bool isToggled;

	int hardwarePin;
	int minBrightness;
	int maxBrightness;
	int transitionTime;
	
	int transitionStarted;
	int brightness;
	int brightnessFrom;
	int brightnessTo;
};

#endif
