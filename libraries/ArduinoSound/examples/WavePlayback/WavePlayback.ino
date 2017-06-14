/*
 This reads a wave file from an SD card and plays it using the I2S interface to
 a MAX08357 I2S Amp Breakout board.

 Circuit:
 * Arduino/Genuino Zero, MKRZero or MKR1000 board
 * SD breakout or shield connected
 * MAX08357:
   * GND connected GND
   * VIN connected 5V
   * LRC connected to pin 0 (Zero) or pin 3 (MKR1000, MKRZero)
   * BCLK connected to pin 1 (Zero) or pin 2 (MKR1000, MKRZero)
   * DIN connected to pin 9 (Zero) or pin A6 (MKR1000, MKRZero)

 created 15 November 2016
 by Sandeep Mistry
 */

#include <SD.h>
#include <ArduinoSound.h>

// filename of wave file to play
const char filename[] = "A20020~2.WAV";

// variable representing the Wave File
SDWaveFile waveFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial1.begin(115200);
  //while (!Serial1) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  // }

  // setup the SD card, depending on your shield of breakout board
  // you may need to pass a pin number in begin for SS
  Serial1.print("Initializing SD card...");
  if (!SD.begin()) {
    Serial1.println("initialization failed!");
    return;
  }
  Serial1.println("initialization done.");

  // create a SDWaveFile
  waveFile = SDWaveFile(filename);

  // check if the WaveFile is valid
  if (!waveFile) {
    Serial1.println("wave file is invalid!");
    while (1); // do nothing
  }

  // print out some info. about the wave file
  Serial1.print("Bits per sample = ");
  Serial1.println(waveFile.bitsPerSample());

  long channels = waveFile.channels();
  Serial1.print("Channels = ");
  Serial1.println(channels);

  long sampleRate = waveFile.sampleRate();
  Serial1.print("Sample rate = ");
  Serial1.print(sampleRate);
  Serial1.println(" Hz");

  long duration = waveFile.duration();
  Serial1.print("Duration = ");
  Serial1.print(duration);
  Serial1.println(" seconds");

  // adjust the playback volume
  AudioOutI2S.volume(7);

  // check if the I2S output can play the wave file
  if (!AudioOutI2S.canPlay(waveFile)) {
    Serial1.println("unable to play wave file using I2S!");
    while (1); // do nothing
  }

  // start playback
  Serial1.println("starting playback");
  AudioOutI2S.play(waveFile);
}

void loop() {
  // check if playback is still going on
  if (!AudioOutI2S.isPlaying()) {
    // playback has stopped

    Serial1.println("playback stopped");
    while (1); // do nothing
  }
}
