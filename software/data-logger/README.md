# Nix

    nix-shell -p platformio avrdude python2Packages.pyserial
    ln -s "$(which avrdude)" ~/.platformio/packages/tool-avrdude/avrdude
    platformio run --upload-port /dev/ttyUSB0 -t upload
    python2 -m serial.tools.miniterm /dev/ttyUSB0  -b 9600

