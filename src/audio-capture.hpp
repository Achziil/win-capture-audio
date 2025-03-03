#pragma once

#include <cstdio>
#include <optional>
#include <tuple>

#include <windows.h>
#include <wil/resource.h>

#include <obs.h>
#include <util/darray.h>

#include "common.hpp"
#include "audio-capture-helper.hpp"
#include "session-monitor.hpp"

/* clang-format off */

#define SETTING_MODE                   "mode"
#define SETTING_SESSION                "session"
#define SETTING_EXCLUDE                "exclude"

#define TEXT_NAME                      obs_module_text("Name")

#define TEXT_MODE                      obs_module_text("Mode")
#define TEXT_MODE_SESSION              obs_module_text("Mode.Session")
#define TEXT_MODE_HOTKEY               obs_module_text("Mode.Hotkey")

#define TEXT_SESSION                   obs_module_text("Session")
#define TEXT_EXCLUDE                   obs_module_text("Exclude")

#define TEXT_HOTKEY_START              obs_module_text("Hotkey.Start")
#define TEXT_HOTKEY_STOP               obs_module_text("Hotkey.Stop")

#define HOTKEY_START                   "hotkey_start"
#define HOTKEY_STOP                    "hotkey_stop"

/* clang-format on */

namespace CaptureEvents {
enum CaptureEvents { Shutdown = WM_USER, Update, SessionAdded, SessionExpired };
}

enum mode { MODE_SESSION, MODE_HOTKEY };

struct AudioCaptureConfig {
	enum mode mode = MODE_SESSION;

	std::optional<std::string> executable;
	HWND hotkey_window = NULL;

	bool exclude = false;
};

class AudioCapture {
private:
	std::thread worker_thread;
	DWORD worker_tid;
	wil::unique_event worker_ready{wil::EventOptions::ManualReset};

	wil::critical_section config_section;
	AudioCaptureConfig config;

	obs_hotkey_pair_id hotkey_pair;
	obs_source_t *source;

	WAVEFORMATEX format;
	std::optional<Mixer> mixer;

	std::optional<SessionMonitor> session_monitor;
	std::unordered_map<DWORD, AudioCaptureHelper> helpers;

	wil::critical_section sessions_section;
	std::unordered_map<SessionKey, std::string> sessions;

	void StartCapture(const std::set<DWORD> &new_pids);
	void StopCapture();

	void AddSession(const MSG &msg);
	void RemoveSession(const MSG &msg);

	void WorkerUpdate();

	bool Tick(const MSG &msg);
	void Run();

public:
	void Update(obs_data_t *settings);
	std::unordered_map<SessionKey, std::string> GetSessions();

	static std::tuple<std::string, std::string>
	MakeSessionOptionStrings(std::set<DWORD> pids,
				 const std::string &executable);

	bool IsUwpWindow(HWND window);
	HWND GetUwpActualWindow(HWND parent_window);

	void HotkeyStart();
	void HotkeyStop();

	AudioCapture(obs_data_t *settings, obs_source_t *source);
	~AudioCapture();
};