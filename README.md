# fffb  

### macOS fake force feedback plugin

## what?

`fffb` is a plugin for scs games which uses the TelemetrySDK to read truck telemetry data, constructs custom forces and plays them on your logitech G923  

## why?

- logitech GHUB is a broken piece of `    `
- force feedback doesn't work on macOS
- i like how the wheel shakes when i press the gas pedal

## how?

the scs related stuff is ripped from the examples bundled with the TelemetrySDK.  
to interact with the wheel, i use `flt`, a small library i wrote for interacting with the G923 from macOS. it allows us to enable/disable and configure autocentering, play custom forces and have fun with the leds.  

## usage

to build the plugin:  

```bash
# clone the repo
git clone https://github.com/eddieavd/fffb && cd fffb

# create build directory
mkdir build && cd build

# configure and build fffb
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8

# create plugin directory
## should be in same directory as ets2/ats executable
## this should be the default path
mkdir ~/Library/Application\ Support/Steam/steamapps/common/American\ Truck\ Simulator/American\ Truck\ Simulator.app/Contents/MacOS/plugins

# copy plugin to plugin directory
cp libfffb.dylib ~/Library/Application\ Support/Steam/steamapps/common/American\ Truck\ Simulator/American\ Truck\ Simulator.app/Contents/MacOS/plugins
```

now you can launch ets2/ats.  
upon launch, you'll see the advanced sdk features popup, hit OK and the plugin initialization starts.  
if the wheel leds start flashing and the wheel turns to the right and back, wheel initialization was successful and you should be good to go!  
however, wheel initialization is a bit unstable so if you don't see the leds flash, reload the plugin by running `sdk reinit` in the in-game console

## disclaimer

should work on any apple silicon mac, not tested on x86_64  
works only with Logitech G923 PS  
the ffb protocol is similar on other logitech wheels so it should be easy to extend support, but i don't have any other wheels and don't feel like writing code i can't test.  
