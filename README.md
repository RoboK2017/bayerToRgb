# bayerToRgb
This project is a simple program that converts Bayer format(MT9V034 specifically) to RGB

user rgbreader.exe plus options to parse raw bayer files:

-f/--file: -f [fileName] or --file=[fileName] to specify the raw bayerfile
-s/--show: to turn on/off display using opencv
-d/--debug: to turn on saving of the original bayer file and the generated rgb picture for each image. Extra logging information will be dumpped into standard input
-p/--patch: -p [number] or --patch=[number] to specify how many channels does the raw bayer file contain. i.e. 4 channels means there will be four images taken at one time.

Exmaple:
rgbreader.exe -f data2 -s -d -p 4

rgbreader.exe -f data2 -s -p 4