# TODO tasks remaining before initial alpha release
We define alpha releases as: Not feature complete, but functional and probably safe.

# TODO tasks for beta release
We define beta releases as: Feature complete, only fixing bugs from that point until release 1.0.
Note: there will probably be a few alpha releases based on user bug reports and the following work items.  Once this list is complete we
will declare beta.

* Figure out how to protect github repos from accidental pushes
* test with 0.20 motor code
* move config/technical info to the info screen (and stop using DrawText fields for showing most data fields)
* Implement cruise mode (also missing in 850C)
* Implement offroad mode(also missing in 850C)
* show temp warnings on main screen
* show motor faults promenantly on main screen
* suspend the 100ms timer tick during copy_layer_2_layer_3_vars
* let user edit maxpower from the mainscreen 
* fix power fields to blink as needed
* uncomment offroad mode - and fix the config editing of the div25 field it use
* show motor temp alerts
* show power limiting alerts (due to PWM or temp or whatever)
* successful installation/usage report from at least one alpha user/dev
* add a watchdog handler
* turn bluetooth back on and have it implement bicycle power/speed/cadence profile and test with Strava app - https://devzone.nordicsemi.com/f/nordic-q-a/3233/anybody-wrote-ble_cps-c-for-cycling-power - https://www.bluetooth.com/specifications/gatt/services/ 
and https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v12.3.0%2Fble_sdk_app_csc.html&cp=5_5_7_4_2_2_5
* make a shutdown screen
* pack & align eeprom 
* implement a watchdog function

# Tasks for release 1.1
* report pedal power via strava
* fix bluetooth notifications (so battery SOC/cadence periodically updates in android app)

# Tasks for future releases
After the initial 1.0 release the following features can go into 1.1

* add the concept of Subscreens, so that the battery bar at the top and the status bar at the bottom can be shared across all screens
* setup the local analog comparator to compare Vbat to a min voltage (19V or whatever).  If it falls below that voltage assume user just killed the power at the battery and quickly write settings to flash.  Only feasible if oscope timing shows we have enough time before the CPU voltage fails for this to be worth bothering with.
* let user completely customize which fields show in the various layout positions of the screens.  said differently: make fields fully customizable like the garmin UI or this note from casainho: https://github.com/OpenSource-EBike-firmware/SW102_LCD_Bluetooth/issues/3#issuecomment-518039673
* add a graph field type which can be used to graph any parameter vs time.  Allow this new graph type to be plopped into any of the standard layouts/screens
* dim screen when the headlight is on
* Make a better implementation for APP_ERROR_CHECK, that includes FILE and LINENO of the caller
* merge with 850C code somewhat? (sharing behavior - just different UX layer and HAL)
* clean up button handling and take advantage of extra button on the SW102
* don't bother wasting CPU cycles to update Fields that are currently not being shown to the user
* clean up buttons_clock by treating all buttons uniformly and getting rid of the enormous copypasta switches
* FIXME - pingpong between two rx buffers, current implementation allows ISR to overwrite the buffer being used by
the GUI thread.  Use two buffers + a ptr.

# Completed TODO work items

* DONE merge 850C style rx comms code with the existing SW102 code
* DONE merge the 850C style tx comms code with the existing SW102 code
* DONE merge the 850C eeprom code, keeping as much as possible in common
* DONE call the rx/tx comms loop every 20ms from the main loop (needed to fix app timers first)
* DONE make the rtc stubs do something
* DONE turn on eeprom writing
* DONE semi-cruftily port over the 850C button code 
* DONE make accel button presses work
* DONE make all other button presses (excluding config) work
* DONE add a logical buttons abstraction and callbacks for screen button handling
* DONE turn on system power / turn off system power per button press
* DONE wait for last flash write to complete before powering down
* DONE show current selection in selection menus
* DONE make config entry presses work
* DONE handle hierarchical menu entry/exit correctly
* DONE Make short press of UP-DOWN (really should be power) - cycle to next screen.  make screens a loop: main, config, power tuning?, fault (optional)
* DONE fix exititing editor via pwr press
* DONE make walk assist press work
* DONE test bootloader with pixel 3 
* DONE test to make sure only one EEPROM record is getting written
* DONE test with real motor (after triple checking wiring color questions)
* DONE make config system (hopefully usable on other platforms)
* DONE make width 0, and height 0 mean "auto select based on display size"
* DONE make release_build task to generate end user usable bins
* DONE figure out why assert(gc_done) fails in eeprom_hw.c - it requires softdevice to be running
* DONE make selection cursor and editor box blink
* DONE make editor presses work correctly
* make all fields white on black
* add a border line between each config entry
* add motorsim so I can move my real motor to my bike but still continue dev
* make a boot screen with version info, battery voltage (detected locally), and "looking for motor" (show until we've had motor comms for 5 seconds)
* soc battery capacity needs a larger stepsize
* install on kevin's bike
* use battery icon
* Run layer2 from a 100ms timer tick
* BUG: Soc is wrong on bike, debug in simulator (starts at 0% SOC and goes up to 200% after 20% of battery consumed)
* improve sim a bit for soc dev
* show volts or SOC in top left
* idle shutdown timer
* retest SOC on bike
* retest walk assist - power boost is way too low
* let button press exit fault screen (or have it timeout?)
* review new 850C changes from new 850C branch, changes after 7/22
* use a bigger font for motor power (more like the spec)
* when editables are selected invert the entire background, not just each character (current approach has an ugly black line between chars)  possibly just fix the bug in ugui putstring
* make selection in menus prettier
* fix hang when saving after turning on motor temp - something seems wonky in FDS land? 
* test that we only allow walk mode when speeds are legal
* fix startup boost config entries
* save ODO to flash on shutdown
* put secondary data on a new "info" screen
* move trip distance to info screen
* move motor temp to info screen
* put speed back into main screen
* add cadence back to main screen (if it fits readably, otherwise move the info screen)
* investigate to see if OLED is pwmed, becuase it flickers in my camera.  i.e. if we change the pwm interval we can make it brighter
* do eeprom GC if flash is full (currently only done at boot)
* LowPerformer idea to only use the numbers for bigger fonts to save flash. (USE_FONT_24X40 costs 30KB of flash space for instance)
* Currently we run OLED at 100% brightness, lower it back to 0xbf by default and let the user have a setting to select what they want.  Or possibly dim the screen if the headlight is on.
* assist level not getting saved on real bike
* include release notes: how to install (with openocd links), working features, not yet working features, etc...
* to support readonly (but dynamic) config values, if value changes set dirty on the field (only need to check the small number of visible editables)
* move font selection out of Field and into FieldLayout
* delete dead code in mainscreen.c
* merge walk/brake into a single status line with other faults
* show units
* include units on speed
* save 15Kish of flash by turning off USE_FONT_10X16 and pulling just the digits from that font into a new less flash consuming font
* label assist on main screen (#26)
* Stop using the redundent tx buffer #24 

# Misc notes from kevin not yet formatted

* install eabi toolchain from https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update/+download/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2
per https://launchpadlibrarian.net/218827232/How-to-build-toolchain.pdf

* nrf on linux instructions here: https://gustavovelascoh.wordpress.com/2017/01/23/starting-development-with-nordic-nrf5x-and-gcc-on-linux/

stlink setup:
You have to connect the following 4 lines: SWDIO, SWCLK, VCC and GND.
https://devzone.nordicsemi.com/f/nordic-q-a/13869/openocd-promgram-nrf51822-with-st-link-v2-mini
https://devzone.nordicsemi.com/f/nordic-q-a/4984/st-link-nrf51822-and-openocd <- best next steps
https://devzone.nordicsemi.com/f/nordic-q-a/20872/how-to-load-hex-to-nrf51-with-openocd

eclipse debug setup:
http://embeddedsoftdev.blogspot.com/p/ehal-nrf51.html
