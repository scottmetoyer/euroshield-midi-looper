# Midi Looper

Midi Looper is a four channel tape-style looping performance utility for the 1010music Euroshield with the PJRC Teensy 3.2.

It allows you to layer up to four loops of incoming notes to create evolving, polyrhythmic sequences.

## Getting Started

### Installation

Download the .ino file and install it to a [Teensy 3.2 board](https://www.pjrc.com/store/teensy32.html) connected to the [1010music Euroshield](https://1010music.com/product/euroshield1) using your preferred method. I use [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html).

Connect MIDI in and out devices to the Euroshield using Type B TRS adapters.

### Usage

Midi Looper records incoming note data (no CC or other MIDI data just yet) into a series of 4 looping MIDI sequences that play on channels 1 to 4. Sequences can be any length and are not currently clocked or linked in any way to allow polyrhythmic sequences to be created.

To arm recording, press the Euroshield button. The Teensy amber LED will light to let you know a sequence is ready to be recorded.

Play some notes. As soon as the Euroshield receives incoming note data it will begin recording.

Press the Euroshield button again to stop recording. The recorded MIDI notes will immediately begin looping on the recorded channel, and the recording channel will be incremented by 1 (or set to channel 1 if channel 4 was recorded).

Press the button again to arm recording on the next channel. Repeat as desired to fill the 4 sequences.

Long press (more than one second) to clear all sequences and set the recording channel back to 1.

## Authors

**Scott Metoyer** - [Scott Metoyer](http://music.scottmetoyer.com)

## License

This project is licensed under the MIT License.


