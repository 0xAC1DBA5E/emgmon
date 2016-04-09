/* 2016-01-24 Alistair Robertson
 * Sends packets over serial port at 250Hz
 * Packets are little-endian.
 * See Data_u::packet for packet spec
 * 
 * Expect data to stream at 4.352 kB/s when using all 6 channels
 * Data rate is set to 57600 bps. 
 * 
 * Client writes 'start' to serial port to begin streaming.
 */

static const unsigned UPDATEFREQUENCY = 250;
static const unsigned UPDATEPERIOD = 1000/UPDATEFREQUENCY;
static const unsigned NUMCHANNELS = 1; // using 1 shield

unsigned long lastwritetime;

union Data_u {
  struct {
    uint8_t start; // start byte for checking alignment
    uint32_t ms; // <- millis() when writeSerial() is called
    uint16_t channel[NUMCHANNELS];
  } packet;
  unsigned char buffer[sizeof(packet)];
};
volatile Data_u data{};

// only sample and send data after 'start' message is received
bool isrunning;
void serialEvent() {
  isrunning = Serial.readString() == "start";
}

// sample the sensors
void sample() {
  for(unsigned i = 0; i < NUMCHANNELS; i++) {
    data.packet.channel[i] = analogRead(i);
  }
}

// send packet
void writeSerial() {
  data.packet.ms = lastwritetime;
  Serial.write(const_cast<unsigned char*>(data.buffer), sizeof(data.buffer));
  Serial.flush();
}

void setup() {
  isrunning = false;
  // turn off LED
  pinMode(13, OUTPUT);
  digitalWrite(13,LOW);
  // init data
  data.packet.start = 255;
  data.packet.ms = 0;
  for(unsigned i = 0; i < NUMCHANNELS; i++)
    data.packet.channel[i] = 0;
  // set serial data rate
  Serial.begin(57600);
}

void loop() {
  if(isrunning) {
    unsigned long t = millis();
    if (t - lastwritetime >= UPDATEPERIOD) {
      lastwritetime = t;
      sample();
      writeSerial();
    }
  } 
}


