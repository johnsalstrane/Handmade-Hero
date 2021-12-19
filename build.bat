@echo off

cl -DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 -Zi -FC win32_handmade.cpp user32.lib Gdi32.lib