# g923mac

`g923mac` is a plugin for [SCS games](https://www.scssoft.com/) which uses the [TelemetrySDK](https://modding.scssoft.com/wiki/Documentation/Engine/SDK/Telemetry) to read truck telemetry data, construct realistic force feedback effects, and play them on your Logitech G923. This plugin is a fork of [fffb](https://github.com/eddieavd/fffb).

## Building from source

To build `g923mac`:  

```bash
# clone the repo
git clone https://github.com/ivonunes/g923mac && cd g923mac

# create build directory
mkdir build && cd build

# configure and build project
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8

# create plugin directory
## should be in same directory as ets2/ats executable
## this should be the default path
mkdir ~/Library/Application\ Support/Steam/steamapps/common/American\ Truck\ Simulator/American\ Truck\ Simulator.app/Contents/MacOS/plugins

# copy plugin to plugin directory
cp libg923mac.dylib ~/Library/Application\ Support/Steam/steamapps/common/American\ Truck\ Simulator/American\ Truck\ Simulator.app/Contents/MacOS/plugins
```

Now you can launch ETS2/ATS.

Upon launch, you'll see the advanced SDK features popup, hit OK and the plugin initialization starts.  

If the wheel leds start flashing and the wheel turns to the right and back, wheel initialization was successful and you should be good to go!  

If you don't see the LEDs flash, reload the plugin by running `sdk reinit` in the in-game console.

## Compatibility

- **Games**: American Truck Simulator, Euro Truck Simulator 2
- **Platform**: macOS only (where G923 force feedback is not natively supported)
- **Hardware**: Logitech G923 steering wheel
