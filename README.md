# southpole-vcvrack

![All](./doc/sp-all-2018-01-22.png)

Southpole VCV Rack Modules

These modules are mainly reskins of the existing Audible Instruments /
Arable Instruments VCV Rack versions of Mutable Instruments eurorack modules.
- The skins are inspired by hardware micro versions of various MI modules.
- Some skins do not exist yet in hardware.
- Other modules did not yet exist in VCV Rack versions.
- Compiled agaist parasites firmware where possible. (Required a few fixes which is why it is checked in here) 

- [Mutable Instruments](https://mutable-instruments.net/)
- [Audible Instruments](https://github.com/VCVRack/AudibleInstruments/)
- [ArableInstruments](https://github.com/adbrant/ArableInstruments/)

## Building

Compile Rack from source, following the instructions at https://github.com/VCVRack/Rack.

After checking out in the `plugins/` directory, get external dependencies with

	git submodule update --init --recursive


## Modules copied from Audible Instruments / Arable Instruments

### Cornrows - Macro Oscillator
Based on [Braids](https://mutable-instruments.net/modules/braids), [Manual](https://mutable-instruments.net/modules/braids/manual/)

### Splash - Tidal Modulator
Based on [Tides](https://mutable-instruments.net/modules/tides), [Manual](https://mutable-instruments.net/modules/tides/manual/)

- eurorack firmware
- does not compile against parasites for now, need to use Makefile.eurorack

### Lambs - Wavetable Oscillator
Based on [Sheep](https://mutable-instruments.net/modules/tides/firmware/) (Tides alternative firmware)

- eurorack firmware
- does not compile against parasites for now, need to use Makefile.eurorack

### Smoke - Texture Synthesizer
Based on [Clouds](https://mutable-instruments.net/modules/clouds), [Manual](https://mutable-instruments.net/modules/clouds/manual/)

- parasites firmware

### Resonator
Based on [Rings](https://mutable-instruments.net/modules/rings), [Manual](https://mutable-instruments.net/modules/rings/manual/)

### Quad VC-polarizer
Based on [Blinds](https://mutable-instruments.net/modules/blinds), [Manual](https://mutable-instruments.net/modules/blinds/manual/)

### Quad VCA
Based on [Veils](https://mutable-instruments.net/modules/veils), [Manual](https://mutable-instruments.net/modules/veils/manual/)

## New modules

### ETAGERE EQ Filter - inspired by [Shelves](https://mutable-instruments.net/modules/shelves)
[Manual](https://mutable-instruments.net/modules/shelves/manual/)

- experimental - does not fully reproduce all characteristics of Shelves!
- filter code copied from RJModules Filters
