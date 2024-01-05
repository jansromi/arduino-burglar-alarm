
// Class for the distance sensor
class DistanceSensor {
private:
  const int SENSOR_PIN;
  // how many cycles to use when initializing the sensor
  const int INIT_CYCLES = 5;
  long initial_value;
  long measurement;
    // how much the measurement can deviate from the initial value
  double deviation = 0.10;

  // return the distance in centimeters
  // Pins are initialized, followed by sending a signal 
  // and receiving the reflected sound wave.
  // https://docs.arduino.cc/built-in-examples/sensors/Ping
  long measureDistance(){
    long duration;
    pinMode(SENSOR_PIN, OUTPUT);
    digitalWrite(SENSOR_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(SENSOR_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(SENSOR_PIN, LOW);

    pinMode(SENSOR_PIN, INPUT);
    duration = pulseIn(SENSOR_PIN, HIGH);
    return microsecondsToCentimeters(duration);
  }
  
  // " The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled."
  // 
  long microsecondsToCentimeters(long microseconds) {
    return microseconds / 29 / 2;
  }

public:
  DistanceSensor(int pin) : SENSOR_PIN(pin) {
    pinMode(SENSOR_PIN, OUTPUT);
  }

  long getDistance() {
    return measurement;
  }

  // Method that performs a new measurement and updates the measurement value.
  void update() {
    measurement = measureDistance();
  }

  // checks whether the set standard value has been exceeded or not.
  // if within deviation, return false, if not, 
  // return true
  bool hasChanged() {
    double lower_limit = initial_value - (initial_value * deviation);
    double upper_limit = initial_value + (initial_value * deviation);
    if (measurement >= lower_limit && measurement <= upper_limit) {
      return false; 
    } else {
      return true;
    }
  }

  // set the initial value to the average of INIT_CYCLES measurements
  void initialize() {
    long sum = 0;

    for (int i = 0; i < INIT_CYCLES; ++i) {
        // measure the distance for INIT_CYCLES times and sum them up
        long measurement_value = measureDistance();
        sum += measurement_value;
    }

    initial_value = sum / INIT_CYCLES;
    measurement = initial_value; 
  }
};

// Class for the piezo buzzer
class Piezo {
  private:
    const int PIEZO_PIN;
    const int POT_PIN;

  public:
    Piezo(int piezoPin, int potPin) : PIEZO_PIN(piezoPin), POT_PIN(potPin) {
      pinMode(piezoPin, OUTPUT);
      pinMode(potPin, INPUT);
    }

    // play some noise
    void play() {
      // read the potentiometer value
      int potValue = analogRead(POT_PIN);
      // map it a value between 0 and 2000
      unsigned long duration = map(potValue, 0, 1027, 0, 2000);
      
      if (duration == 0) {
        return;
      }
      // make some noise 
      tone(PIEZO_PIN, 432, duration);
    }
};

// Very basic class for the RGB LED
class Light {
  private:
    const int RED_PIN;
    const int GREEN_PIN;
    const int BLUE_PIN;

  public:
    Light(int redPin, int grnPin, int bluePin) : RED_PIN(redPin), GREEN_PIN(grnPin), BLUE_PIN(bluePin) {
        pinMode(RED_PIN, OUTPUT);
        pinMode(GREEN_PIN, OUTPUT);
        pinMode(BLUE_PIN, OUTPUT);
    }

    void setColor(int red, int green, int blue) {
        digitalWrite(RED_PIN, red);
        digitalWrite(GREEN_PIN, green);
        digitalWrite(BLUE_PIN, blue);
    }
};

// BurglarAlarm class combines the other classes together
// and provides the main functionality of the device
class BurglarAlarm {
  private:
    // ie. is the alarm on or off
    bool alarmMode;
    bool activeAlarm;
    Light light;
    Piezo piezo;
    DistanceSensor distancesensor;

    // raise hell
    void alarm() {
      // if the alarm is already active, do nothing
      // as the piezo would be playing already and too many
      // calls to play() will mess it up
      if (activeAlarm) { return; }

      activeAlarm = true;
      Serial.println("halytys aktivoitu!");
      piezo.play();
      // set a red light
      light.setColor(255, 0, 0);
    }

  public:
    BurglarAlarm(Piezo p, DistanceSensor e, Light v) : piezo(p), distancesensor(e), light(v) {
      alarmMode = true;
      activeAlarm = false;
    }

    // ie. shut down or turn on
    void changeState() {
      alarmMode = !alarmMode;
    }

    // Interrupt handler for the acknowledge button
    void acknowledgeAlarm() {
      activeAlarm = false;
      light.setColor(0, 255, 0);
      Serial.println("Halytys kuitattu!");
    }

    // activate the alarm
    void activate() {
      Serial.println("Halytystila aktiivinen");
      alarmMode = true;
    }

    // turn off the alarm
    void turnOff() {
      Serial.println("Halytystila suljettu");
      alarmMode = false;
    }

    // initialize the device by initializing the distance sensor
    // and setting the alarm mode
    void initialize(bool tila){
      Serial.println("Alustetaan varashalytin...");
      distancesensor.initialize();
      alarmMode = tila;
      Serial.println("Varashalytin alustettu!");
    }

    // main method that is called in the main loop
    void monitor(){

      if (activeAlarm) {
        Serial.println("Halytys aktiivinen!");
        return;
        }

      if (!alarmMode) {
        light.setColor(0, 255, 0);
        return;
      }

      // new readings from the distance sensor,
      // if the distance has changed, raise alarm
      distancesensor.update();
      if (distancesensor.hasChanged()) {
        alarm();
      } else {
        light.setColor(0, 0, 255);
      }
    }
};

DistanceSensor distancesensor(7);
Light light(6, 5, 4);
Piezo piezo(8, A0);

BurglarAlarm b_a(piezo, distancesensor, light);

const int ACKNOWLEDGE_PIN = 3;
const int ON_OFF_PIN = 2;

bool is_running;

void setup() {
  pinMode(ACKNOWLEDGE_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ACKNOWLEDGE_PIN), acknowledge, RISING);
  pinMode(ON_OFF_PIN, INPUT_PULLUP);

  is_running = digitalRead(ON_OFF_PIN);
  attachInterrupt(digitalPinToInterrupt(ON_OFF_PIN), set, CHANGE);
  Serial.begin(9600);
  b_a.initialize(is_running);
}

void loop() {
  b_a.monitor();
  delay(200);
}

// pass the acknowledge interrupt to the BurglarAlarm class.
// it is done here because the interrupt handler cannot be a member function
void acknowledge() {
  b_a.acknowledgeAlarm();
  b_a.initialize(is_running);
}

// slider control for turning the device on and off
void set() {
   is_running ? shut_down() : start();
}

// turn on the device
void start() {
  is_running = true;
  b_a.activate();
  b_a.initialize(is_running);
}

// turn off the device
void shut_down() {
  is_running = false;
  b_a.turnOff();
}