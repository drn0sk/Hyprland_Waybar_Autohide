#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/desktop/view/LayerSurface.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/event/EventBus.hpp>

inline HANDLE PHANDLE = nullptr;

// Do NOT Change
APICALL EXPORT std::string PLUGIN_API_VERSION() {
	return HYPRLAND_API_VERSION;
}

CBox bar;
bool hidden = true;
// 0 for auto hiding, 1 to lock in the shown state, -1 to lock in the hidden state
char lock_bar = 0, lock_bar_prev = 0, lock_bar_prev2 = 0;

void toggleBar() {
	// killall -SIGUSR1 waybar
	hidden = !hidden;
	HyprlandAPI::invokeHyprctlCommand("dispatch", "exec killall -SIGUSR1 waybar", "");
}

void updateBar() {
	if(hidden != (lock_bar < 0)) {
		toggleBar();
	}
	if(!lock_bar) {
		std::string pos_str = HyprlandAPI::invokeHyprctlCommand("cursorpos", "", "");
		std::string::size_type c = pos_str.find(", ");
		double x,y;
		std::istringstream s1(pos_str.substr(0, c)), s2(pos_str.substr(c+2));
		s1 >> x;
		s2 >> y;
		Vector2D pos(x,y);
		if(bar.containsPoint(pos) == hidden) {
			toggleBar();
		}
	}
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE h) {
	PHANDLE = h;

	// check version
	const std::string COMPOSITOR_HASH = __hyprland_api_get_hash();
	const std::string CLIENT_HASH = __hyprland_api_get_client_hash();
	if(COMPOSITOR_HASH != CLIENT_HASH) {
		HyprlandAPI::addNotification(PHANDLE, "[waybar_autohide] Mismatched headers", CHyprColor{1.0,0.2,0.2,1.0}, 5000);
		throw std::runtime_error("[waybar_autohide] Version mismatch");
	}

	if(!HyprlandAPI::addDispatcherV2(PHANDLE, "waybar_autohide:lock", [&](std::string arg){
				if(arg == "off" || arg == "no" || arg == "0") {
					lock_bar_prev2 = 1;
					lock_bar = 0;
				} else if(arg == "show" || arg == "1") {
					lock_bar_prev2 = 1;
					lock_bar = 1;
				} else if(arg == "hide" || arg == "-1") {
					lock_bar_prev2 = 1;
					lock_bar = -1;
				} else {
					if(lock_bar != 1) {
						lock_bar_prev2 = lock_bar;
						lock_bar = 1;
					} else {
						lock_bar = (lock_bar_prev2 == 1) ? 0 : lock_bar_prev2;
						lock_bar_prev2 = 1;
					}
				}
				updateBar();
				return (struct SDispatchResult){};
			})) {
		HyprlandAPI::addNotification(PHANDLE, "[waybar_autohide] couldn't add dispatcher", CHyprColor{1.0,0.2,0.2,1.0}, 5000);
		throw std::runtime_error("[waybar_autohide] failed to add dispatcher");
	}
	static auto P1 = Event::bus()->m_events.layer.opened.listen([&](PHLLS layer){
			if(layer->m_namespace == "waybar") {
				bar = layer->m_geometry;
			}
		});
	static auto P2 = Event::bus()->m_events.input.mouse.move.listen([&](Vector2D pos, Event::SCallbackInfo &c){
			if(!lock_bar) {
				// (bar.containsPoint(pos) && hidden) || (!bar.containsPoint(pos) && !hidden)
				if(bar.containsPoint(pos) == hidden) {
					toggleBar();
				}
			}
		});
	static auto P3 = Event::bus()->m_events.window.fullscreen.listen([&](PHLWINDOW win){
			// lock hidden (lock_bar = -1) and save prev state (in lock_bar_prev) if fullscreen, revert to prev state otherwise
			if(win->m_fullscreenState.internal == FSMODE_FULLSCREEN || win->m_fullscreenState.internal == FSMODE_MAX) {
				// fullscreen
				if(lock_bar != -1) {
					lock_bar_prev = lock_bar;
					lock_bar = -1;
				}
			} else {
				// not fullscreen
				lock_bar = lock_bar_prev;
				lock_bar_prev = 0;
			}
			// change bar state as necessary
			updateBar();
		});
	if(!P1 || !P2 || !P3) {
		HyprlandAPI::addNotification(PHANDLE, "[waybar_autohide] failed to add event listeners", CHyprColor{1.0,0.2,0.2,1.0}, 5000);
		throw std::runtime_error("[waybar_autohide] failed to register event listener.");
	}

	HyprlandAPI::invokeHyprctlCommand("dispatch", "exec killall -SIGUSR2 waybar", "");

	return {"waybar_autohide", "Shows waybar when the mouse is near it, hides it when the mouse goes away.", "drn0sk", "1.0"};
}
