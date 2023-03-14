#pragma once

#include <hidapi.h>
#include "Engine.h"
#include "Thread.h"

class OsuSimPadIntegration {
public:
	static constexpr size_t KEYS = 2;

	OsuSimPadIntegration();
	~OsuSimPadIntegration();

	void start();
	void stop();
	void update();

	void setColor(size_t key, Color color);
	void startFade(size_t key, float duration = 0.25f);

	bool isRunning() {
		return m_device != NULL;
	}

private:
	static void* ThreadProc(void*);
	bool sendData(unsigned char data[6]);

	McThread* m_updateThread;
	bool m_bStopThread;

	hid_device* m_device;

	Color m_wishColor[KEYS];
	float m_fAlpha[KEYS];
	Color m_curColor[KEYS];
	Color m_lastColor[KEYS];
};