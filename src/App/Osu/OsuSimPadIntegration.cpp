#include <AnimationHandler.h>
#include <Engine.h>
#include "OsuSimPadIntegration.h"

// hid_write seems to be horribly slow, so this runs on another thread
void* OsuSimPadIntegration::ThreadProc(void* data) {
	auto* self = (OsuSimPadIntegration*)data;
	while (!self->m_bStopThread) {
		for (int i = 0; i < OsuSimPadIntegration::KEYS; i++) {
			if (self->m_curColor[i] == self->m_lastColor[i])
				continue;

			float alpha = COLOR_GET_Af(self->m_curColor[i]);
			unsigned char cmd[6];
			cmd[0] = 0x00; // Command type (special)
			cmd[1] = 0x03; // Subcommand (set LED)
			cmd[2] = i + 1; // LED index
			cmd[3] = (int)(COLOR_GET_Ri(self->m_curColor[i]) * alpha);
			cmd[4] = (int)(COLOR_GET_Gi(self->m_curColor[i]) * alpha);
			cmd[5] = (int)(COLOR_GET_Bi(self->m_curColor[i]) * alpha);

			self->sendData(cmd);
		}
	}

	return NULL;
}

OsuSimPadIntegration::OsuSimPadIntegration() {
	m_device = NULL;
	for (int i = 0; i < OsuSimPadIntegration::KEYS; i++) {
		m_wishColor[i] = 0xFFFFFFFF;
		m_fAlpha[i] = 1.0;
		m_curColor[i] = 0xFFFFFFFF;
		m_lastColor[i] = 0xFFFFFFFF;
	}

	m_bStopThread = false;
	m_updateThread = nullptr;
}

OsuSimPadIntegration::~OsuSimPadIntegration() {
	stop();
}

void OsuSimPadIntegration::start() {
	stop();

	// Enumerate USB interfaces to find the one that actually controls the lights
	// Hardcoded to SimPad v2 Anniversary Edition
	auto* enum_list = hid_enumerate(0x8088, 0x0006);
	if (!enum_list) {
		debugLog("hid_enumerate failed: %ls", hid_error(NULL));
		stop();
		return;
	}

	hid_device* handle = NULL;
	for (auto* dev = enum_list; dev; dev = dev->next) {
		if (dev->interface_number == 1) {
			handle = hid_open_path(dev->path);
			if (!handle) {
				debugLog("hid_open_path failed: %ls", hid_error(NULL));
				stop();
				return;
			}
		}
	}

	if (handle == nullptr) {
		debugLog("failed to find device");
		stop();
		return;
	}

	m_device = handle;
	m_bStopThread = false;
	m_updateThread = new McThread(OsuSimPadIntegration::ThreadProc, (void*)this);
}

void OsuSimPadIntegration::stop() {
	m_bStopThread = true;
	SAFE_DELETE(m_updateThread);

	if (m_device)
		hid_close(m_device);
	m_device = NULL;
	hid_exit();
}

void OsuSimPadIntegration::update() {
	for (int i = 0; i < OsuSimPadIntegration::KEYS; i++) {
		m_curColor[i] = COLOR(
			(int)(clamp<float>(COLOR_GET_Af(m_wishColor[i]) * m_fAlpha[i], 0.0, 1.0) * 255.0f),
			COLOR_GET_Ri(m_wishColor[i]),
			COLOR_GET_Gi(m_wishColor[i]),
			COLOR_GET_Bi(m_wishColor[i])
		);
	}
}

void OsuSimPadIntegration::setColor(size_t key, Color color) {
	if (key >= OsuSimPadIntegration::KEYS)
		return;

	anim->deleteExistingAnimation(&m_fAlpha[key]);
	m_fAlpha[key] = 1.0f;

	// Need to do sRGB -> linear, otherwise some colors look too washed out
	m_wishColor[key] = COLOR(
		COLOR_GET_Ai(color),
		(int)(clamp<float>(std::powf(COLOR_GET_Rf(color), 2.2f), 0.0f, 1.0f) * 255.0f),
		(int)(clamp<float>(std::powf(COLOR_GET_Gf(color), 2.2f), 0.0f, 1.0f) * 255.0f),
		(int)(clamp<float>(std::powf(COLOR_GET_Bf(color), 2.2f), 0.0f, 1.0f) * 255.0f)
	);
}

void OsuSimPadIntegration::startFade(size_t key, float duration) {
	if (key >= OsuSimPadIntegration::KEYS)
		return;

	m_fAlpha[key] = 1.0f;
	anim->moveLinear(&m_fAlpha[key], 0.0, duration, true);
}

bool OsuSimPadIntegration::sendData(unsigned char data[6]) {
	unsigned char real_data[7];
	real_data[0] = 0; // Report ID
	memcpy(&real_data[1], data, 6);

	return hid_write(m_device, real_data, sizeof(real_data)) != -1;
}