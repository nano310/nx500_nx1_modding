# Samsung NX500 / NX1 Modding

## About

This is a repository hosting information for those interested in altering the functionality of their Samsung NX500 and NX1 cameras.

These cameras run a full blown Linux distribution (Tizen) with shell, X, networking stack, debugging and testing tools - the works. This enables us to alter functionality of the cameras and ... well - just play around - it's fun! Seeing xeyes on your camera is such a heart warming experience :)

Inital work was done and presented on DPReview Samsung Forum [here](http://www.dpreview.com/forums/thread/3964253) and [here](http://www.dpreview.com/forums/thread/3971805) and EOSHD [here](http://www.eoshd.com/comments/topic/19099-petition-for-samsung-nx1-hack/?do=findComment&comment=134256). Special thanks to tecnoworld, Vasile, Kino Seed, outerbeat, arspr and many others.

## TL;DR - I want my camera hacked - what to do?

Resources in this repository are aimed more at "developers" or "advanced users" than "regular users". If you just want to quickly use all these mods, there are two ways to take:
  - Use a mod pack made by @Vasile-gh [here](https://github.com/ottokiksmaler/nx500_nx1_modding/tree/master/video-bitrate-mods/nx-patch). You will need a one-time install of [this](https://github.com/ottokiksmaler/nx500_nx1_modding/tree/master/nx-on-wake), though. This mod is integrated into the camera hibernation image and does not require Bluetooth for startup.
  - Use a mod with many many integrated options packed together by @KinoSeed: [Read about it at facebook.com/NXKS2](https://www.facebook.com/NXKS2) or download directly from http://authors-vault.com/NX-KS2.zip - this mod is launched by enabling Bluetooth.

## TL;DR - I want to remove 30 min movie recording limit without hacking my camera

Read these instructions: [Removing 30 min Movie Recording Limit Without Hacking the Camera](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/Removing_Movie_Recording_Limit_Without_Hack.md)

## TL;DR - I want to check second hand camera (or my own, but I don't want to hack it) for shutter count, etc, What to do?

Read these instructions: [Checking Shutter Count, Power-on Count, etc. Without Hacking the Camera](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/Checking_shutter_count.md)

## Status

Things that work:
  - [Higher bitrates for any video mode - developed by @vasile-gh] (https://github.com/ottokiksmaler/nx500_nx1_modding/tree/master/video-bitrate-mods/nx-patch) 
  - [Persistent modding of camera without use of the Bluetooth - developed by @vasile-gh] (https://github.com/ottokiksmaler/nx500_nx1_modding/tree/master/nx-on-wake) 
  - [Customizing functions of various keys](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/Customizing%20keys.md) for introduction.
  - [Focus stacking](https://github.com/ottokiksmaler/nx500/blob/master/focus_stack.c)
    - Focus on near point - press "Near"
    - Focus on far point - press "Far"
    - Click "Stack" and wait for it to finish
    - Files are in the usual place - remember to fix exposure/awb/etc as you normally would
    - Precompiled binary is [here](https://github.com/ottokiksmaler/nx500/blob/master/focus_stack)
    - Command line options are:
      ```focus_stack [ help | sweep | number_of_photos [ delay_between_photos [ button_height [ button_width ] ] ] ]```
  - [NX500 - Fully electronic shutter - silent](https://github.com/ottokiksmaler/nx500/blob/master/NX500%20Silent%20shooting.md)
  - [Shutter count and power on count](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/Shutter_count_power_count.md)
  - [Prefman tool - access to full preferences in usable form](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/Prefman_tool.md)
  - Collection of small utilities for bash scripting:
    - ```popup_timeout ["text message"] [timeout_in_seconds]``` - displays a popup message for a given number of seconds
    - ```popup_ok [message] [ok label] [cancel label]``` - displays a OK / Cancel dialog, returns 0 if OK, 255 if Cancel
    - ```popup_entry [message] [OK label] [cancel label] [sample entry] [number]``` - displays an entry box with OK/Cancel buttons and prefilled sample text, returns entered text and 0 code for OK or just 255 for cancel. If the fifth parameter is "number" then the NUMBERONLY keyboard is shown.
    - ```onscreen_key [Label] [button] [x y] [width height]``` - Displays a button at given coordinates (default 400 0) of given size (default 120 60) with given label (default REC) and clicks given button (default rec). To dismiss long-click it for at least five seconds. Record - rec, OK - ok, Half-shutter - S1, Full-shutter - S2, Menu - menu, etc.
    - ```shutter_to_key``` - remaps half-shutter and full shutter to user specified keys, for example: ```shutter_to_key ael rec``` will remap half-shutter to AEL and full-shutter to REC so you can start/stop video and lock exposure while camera is in gimbal.
    - ```shutter_to_rec``` - remaps half-shutter to OK (video pause) and full shutter to REC (video start/stop)
    - ```log_watcher``` - restart video before file corruption, generate lists of selected/displayed files in gallery
  - [Running scripts without entering factory mode by starting Bluetooth](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/Running_scripts_without_factory_mode_BT.md)
  - [Running an SSH/SFTP/SCP server on camera](https://github.com/ottokiksmaler/nx500/blob/master/ssh-server/README.md)
  - [Running a FTP server on camera](https://github.com/ottokiksmaler/nx500/blob/master/Running%20FTP%20server%20on%20the%20camera.md)
  - [Enabling PTP mode on the camera](https://github.com/ottokiksmaler/nx500/blob/master/Enable-PTP-on-NX500-NX1.md)
  - [Capturing screenshots of LCD](https://github.com/ottokiksmaler/nx500/blob/master/Enable-Screenshot.md)
  - [Running arbitrary shell scripts from SD card](https://github.com/ottokiksmaler/nx500/blob/master/Running-shell-scripts-from-SD-card.md)
  - [Running a telnet server for remote access to camera](https://github.com/ottokiksmaler/nx500/blob/master/Running-telnet-server-on-camera.md)
  - Running a fully functional browser on camera (run **browser** from script or telnet session)
  - [Controlling the camera](https://github.com/ottokiksmaler/nx500/blob/master/Control-camera-from-command-line.md) over telnet or from scripts (two different ways) including changing modes, exposure parameters, capturing images, setting resolution and quality, etc.
  - Not a real 120fps - sorry - [**NX500** - **1920x1080 video at 120fps and ~40Mbps**](https://github.com/ottokiksmaler/nx500/blob/master/Enable-1080p-120fps-video-on-NX500.md) (already present in NX1)
  - [Working with key events](https://github.com/ottokiksmaler/nx500/blob/master/Working-with-key-events.md) (detecting and sending) - also covers touch
  - Working with generic X - xeyes work, xmessage works, etc.
  - [Running debugger](https://github.com/ottokiksmaler/nx500/blob/master/Running-gdb.md) - gdb on device and gdb server on device
  - [Description of working with focus from command line](https://github.com/ottokiksmaler/nx500/blob/master/Working%20with%20focus.md)
  - [Description of working with lens](https://github.com/ottokiksmaler/nx500/blob/master/Working%20with%20lens.md)
  - [Focus buttons](https://github.com/ottokiksmaler/nx500/blob/master/focus_buttons.c) - Program that displays series of buttons on top of the screen 
    - long press (1+s) to save current focus position
    - click to restore saved focus position
    - Can be used for studio shooting, astrophotography (focus on infinity during day, recall the focus during the night), etc.
    - Precompiled binary is [here](https://github.com/ottokiksmaler/nx500/blob/master/focus_buttons)
  - [Remove 29:59 time limit for video via DEV Mode](https://github.com/ottokiksmaler/nx500/blob/master/DEV%20and%20CS%20modes.md) - original work by [Vasile](http://www.dpreview.com/forums/thread/3979382)
  - [Controlling LCD / EVF](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/Control_LCD_EVF.md)
  - [Utility for direct read/write access to RAM of another process](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/Poker.md) - useful for in place patching of already loaded binaries
  - [Boot process and speeding up the mods](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/doc/Boot_Process_and_Speeding_up_Mods.md)
  - [Compiling against camera library](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/doc/Compile_against_camera_library.md)
  - [Updating the Hibernation Image](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/doc/Update_hibernation_image.md)
  - [Permanent bitrate mods to hibernation image](https://github.com/ottokiksmaler/nx500_nx1_modding/blob/master/Permanent_bitrate_mods.md) 

## To do ...

Things that should work given time:
  - [x] Removing video recording time limit - thanks to Vasile
  - [ ] SSH on camera - client should work just fine, have to test server
  - [ ] Automatic backup - have to decide how to implement - related work
    - http://lemmster.de/auto-backup-from-nx300-via-ftp.html
    - https://cedarandthistle.wordpress.com/2014/11/01/autobackup-to-linux-from-the-samsung-nx300m/
  - [ ] Enabling additional kernel modules - should work similar to  [what's described here](http://www.lemmster.de/cross-compile-kernel-module-samsung-nx300-ubnut-14.04.html)
  - [ ] Crypto like here [NX Crypto Photography](https://sites.google.com/site/nxcryptophotography/)

Things that could work with a bit of luck
  - [ ] USB audio
  - [ ] Removing sharpening and noise reduction in video
  - [ ] Adding support for different Gamma/LUT in video on NX500 (perhaps on NX1 as well)

Not likely - but might be possible
  - [ ] No crop 4k video
  - [ ] Higher framerate 1080p or 720p (sensor can be read at 240fps but that's that - not implemented anywhere else in camera)
  - [ ] Full sensor video at any rate
  - [x] Silent shutter (fully electronic) - NX500 only for now

# Donations

Q: So, where's the **Donate** button?
A: Nowhere. Find a suitable charity and donate. If you cannot afford it, find a local charity and see if they need help in person - it will do you a world of good.
