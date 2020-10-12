Chip-8 Emulator
===

## Cloning this repository

This repository uses submodules which must be retrieved during the clone.
```bash
git clone --recurse-submodules https://github.com/SteelCityKeyPuncher/chip8.git
```

## Running the Emulator

Run the program by passing the path to a ROM as the first argument.

Run from the main directory because the path to the shaders (in `assets/`) are relative to this directory.

```bash
./build/chip8 roms/15PUZZLE
```

The `-r` switch can be used to configure the CPU rate, which is the number of instructions executed per second. If not specified, 500 Hz will be used by default.

```bash
./build/chip8 roms/MAZE -r 100
```

### Keymap

Chip-8 programs use the following hex keypad:
```
1 2 3 C
4 5 6 D
7 8 9 E
A 0 B F
```

This maps to the following on a keyboard.
```
1 2 3 4
Q W E R
A S D F
Z X C V
```

Other keys:
* `ESC`: exit the emulator.
* `Page Up`: increase the CPU speed.
* `Page Down`: decrease the CPU speed.

## Building the Emulator

This project requires that you have CMake in your path.

```bash
mkdir build
cd build
cmake ..
```

On Linux, simply run `make`. I'm not sure if this runs on Windows yet.
