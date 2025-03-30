# cards_scanner


# Build and run
```bash
see top level scripts
```
# Contract:
This is running on a raspi. 
- On start up a bin config (bin numbers to use, fullness of bins, card kind) should be sent to the device from a pc
- After ocr the name, set, collectors number, image hash, bin identifier should be sent by mqtt or similar to pc to check with database.


# Set up Raspi using ansible
### Prerequisite:
- SD card with recent raspian
- SD card is set up and expanded
- SSH Key was added for passwordless access using ssh-copy-id
- LC_ALL locale was set to utf-8


# TODO for isntall ansible
install conan
add to path
new profile detect


# Notes for HW build
- We want most likely Off-axis bright field lighting with an 18-20 degree tilt in the light to rmeove hot spots
