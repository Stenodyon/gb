# GUEMBOI

This GameBoy emulator runs several games (like the Pokémon games and Link's
Awakening) but still crashes on others (Kirby's Dream Land grr). Has sound
support but does not emulate GameBoy Color features (some games require it).

![screenshots](https://github.com/Stenodyon/gb/raw/master/mosaic.png)

## Building & Using

I wouldn't recomment it but if you insist… You can build it on Linux, you'll
need `make`, `g++` and the `SDL2` library. Just `make -j4` and you should be
good.

Controls:
* `c`: Start/unpause the emulator
* `x`: Pause the emulator
* `w, a, s, d`: directional keys
* `t`: Select
* `y`: Start
* `k`: Button A
* `j`: Button B
