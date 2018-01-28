# southpole-vcvrack

### Please note: Currently no stable / compilable tag is announced yet. Stay tuned!!!

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

- Based on [Braids](https://mutable-instruments.net/modules/braids), [Manual](https://mutable-instruments.net/modules/braids/manual/)

### Splash - Tidal Modulator / Lambs - Wavetable Oscillator

- Based on [Tides](https://mutable-instruments.net/modules/tides), [Manual](https://mutable-instruments.net/modules/tides/manual/)

- Based on [Sheep](https://mutable-instruments.net/modules/tides/firmware/) (Tides alternative firmware)

- Uses eurorack firmware, no parasites interface yet
- does not compile against parasites, need to use Makefile.eurorack then copy the library to an extra directory in /plugins

### Smoke - Texture Synthesizer
Based on [Clouds](https://mutable-instruments.net/modules/clouds), [Manual](https://mutable-instruments.net/modules/clouds/manual/)

- parasites firmware (Neil)

### Annuli - Resonator
- Based on [Rings](https://mutable-instruments.net/modules/rings), [Manual](https://mutable-instruments.net/modules/rings/manual/)

### Bandana - Quad VC-polarizer
- Based on [Blinds](https://mutable-instruments.net/modules/blinds), [Manual](https://mutable-instruments.net/modules/blinds/manual/)

### Balaclava - Quad VCA
- Based on [Veils](https://mutable-instruments.net/modules/veils), [Manual](https://mutable-instruments.net/modules/veils/manual/)

## New modules

### Etagère - EQ Filter
Inspired by [Shelves](https://mutable-instruments.net/modules/shelves)
[Manual](https://mutable-instruments.net/modules/shelves/manual/)

- experimental - does not fully reproduce all characteristics of Shelves!
- filter code copied from RJModules Filters

### But - Manual A/B Buss 

- inspired by DJ Steevio's modular method
- Either input A or B go to the output and the A/B busses
- Two summed identical outputs for A and B buss are provided.

### Abr - Manual A/B Switch 

- Inverse of But, either input A or B goes to output
- Sums of inputs set to A or B provided

### Piste - Drum Processor

- inspired by [Bastl Skis](http://www.bastl-instruments.com/modular/skis/)
- one-stop module to turn oscillators into drums / percussion 
- input gain + eq
- 2 independent decay envelopes (eg. body and accent) the sum of which is applied to input signal via internal VCA
- 2nd envelope is scaled to 1st
- triggers are scalable
- overdrive in output stage

### Fuse - Synchronized one-shot triggers

- tool to help keep time in 4/4 based music
- can be used to launch triggers ONCE at a given step on a 16 step counter
- intended to build a "queue next pattern" system like on groove boxes
- Arm triggers manually or via inputs
- for example, clock with bars, trigger to mute the kick for exactly 4 bars, trigger 4 bar later to drop

