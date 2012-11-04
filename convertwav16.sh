mplayer -af resample=8000,channels=1,format=s16le -ao pcm:nowaveheader:file=output.raw input.wav
gcc bin2c.c -o bin2c
./bin2c output.raw soundraw > soundraw.c

