This is a plugin for Hyprland that makes waybar autohide.

Waybar is shown when the cursor hovers over it, and hidden otherwise.


You can also lock waybar in the shown state by using the "waybar_autohide:lock" dispatcher.
The waybar_autohide:lock dispatcher takes an optional argument that can be:
	"off"/"no"/"0": to go back to automatically hiding
	"show"/"1" : to lock in the shown state
	"hide"/"-1": to lock in the hidden state
With no argument it toggles between being locked in the shown state and returning to autohiding or being locked in the hidden state


This plugin also locks waybar in the hidden state when a window goes fullscreen.
