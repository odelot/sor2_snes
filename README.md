# sor2_snes
Street of Rage 2 port for SNES

This repo is a study on the subject of create games for SNES.

I'm sharing it because it can help others creating their games ^.^ probably I won't work on it anymore

## Compile

To compile you will need to prepare your dev environment using __pvsneslib__ https://github.com/alekmaul/pvsneslib

With the configured environment just type __make__ to compile it

## Test ROM

You can download the file __sor2_snes.sfc__ and try it on an emulator or on real hardware using a flashcart

## Features

* 3 background layers with scrolling
* optimized hud layer storage on VRAM
* state machine to animate characters
* queue to upload its assets to VRAM
* Y order sort 
* handle hitbox to cause damage
* using soundeffects for damange and punch
* using background music

## Caveats

I've cut a lot of corners to have this work. These are:
* the sprites are 64x64 - I am not using meta-sprites. It is not efficient memory wise.
* there are inefficient piece of code all around
* part of the program are in Portuguese. 

## Background Music

The music was made for snes by Edson Leal -> more of his work https://soundcloud.com/edinhu/tracks 
