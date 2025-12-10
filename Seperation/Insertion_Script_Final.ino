const uint8_t STEP_PIN = 3;  
const uint8_t DIR_PIN = 2;  
const uint8_t ENABLE_PIN = 4; // Optional - tie to GND if not used

// Motor control variables
bool motorRunning = false;
bool motorDirection = false;  // false = forward, true = reverse
uint16_t stepPeriodUs = 2000; // Speed control (lower = faster)
unsigned long lastStepTime = 0;
bool stepPinState = false;

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) { ; }
  
  Serial.println("STEP/DIR Stepper Motor Controller");
  Serial.println("==================================");
  
  // Configure pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  
  // Initialize pins
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
  digitalWrite(ENABLE_PIN, HIGH);  // Active LOW to enable driver
  
  Serial.println("Pins configured");
  Serial.println("NOTE: Set microstep mode using DRV8434S MODE pins");
  Serial.println("(Refer to datasheet for MODE pin configuration)");
  
  delay(100);  // Let driver stabilize
  
  printHelp();
}

void loop()
{
  // Handle serial commands
  if (Serial.available() > 0)
  {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    processCommand(command);
  }
  
  // Step the motor if running
  if (motorRunning)
  {
    unsigned long currentTime = micros();
    if (currentTime - lastStepTime >= stepPeriodUs)
    {
      // Generate step pulse (rising edge triggers step)
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(6);  // Minimum pulse width (check datasheet)
      digitalWrite(STEP_PIN, LOW);
      
      lastStepTime = currentTime;
    }
  }
}

void processCommand(String cmd)
{
  if (cmd == "start" || cmd == "s")
  {
    motorRunning = true;
    digitalWrite(DIR_PIN, motorDirection);
    Serial.println("Motor STARTED");
    Serial.print("Direction: ");
    Serial.println(motorDirection ? "Reverse" : "Forward");
    Serial.print("Speed (period): ");
    Serial.print(stepPeriodUs);
    Serial.println(" us");
  }
  else if (cmd == "stop" || cmd == "x")
  {
    motorRunning = false;
    digitalWrite(STEP_PIN, LOW);  // Ensure step pin is low
    Serial.println("Motor STOPPED");
  }
  else if (cmd == "forward" || cmd == "f")
  {
    motorDirection = false;
    digitalWrite(DIR_PIN, motorDirection);
    Serial.println("Direction set to FORWARD");
    if (motorRunning) {
      Serial.println("(Change will take effect immediately)");
    }
  }
  else if (cmd == "reverse" || cmd == "r")
  {
    motorDirection = true;
    digitalWrite(DIR_PIN, motorDirection);
    Serial.println("Direction set to REVERSE");
    if (motorRunning) {
      Serial.println("(Change will take effect immediately)");
    }
  }
  else if (cmd.startsWith("speed "))
  {
    int newSpeed = cmd.substring(6).toInt();
    if (newSpeed >= 100 && newSpeed <= 50000)
    {
      stepPeriodUs = newSpeed;
      Serial.print("Speed set to ");
      Serial.print(stepPeriodUs);
      Serial.println(" us (lower = faster)");
    }
    else
    {
      Serial.println("ERROR: Speed must be between 100 and 50000");
    }
  }
  else if (cmd == "status" || cmd == "?")
  {
    printStatus();
  }
  else if (cmd == "help" || cmd == "h")
  {
    printHelp();
  }
  else if (cmd == "enable")
  {
    digitalWrite(ENABLE_PIN, HIGH);  // Active LOW
    Serial.println("Driver ENABLED");
  }
  else if (cmd == "disable")
  {
    digitalWrite(ENABLE_PIN, LOW);  // Active LOW
    motorRunning = false;
    Serial.println("Driver DISABLED");
  }
  else
  {
    Serial.println("Unknown command. Type 'help' for commands.");
  }
}

void printHelp()
{
  Serial.println("\n=== COMMANDS ===");
  Serial.println("start (s)       - Start motor");
  Serial.println("stop (x)        - Stop motor");
  Serial.println("forward (f)     - Set direction to forward");
  Serial.println("reverse (r)     - Set direction to reverse");
  Serial.println("speed <value>   - Set speed (100-50000 us, lower=faster)");
  Serial.println("                  Examples: 'speed 1000' or 'speed 5000'");
  Serial.println("status (?)      - Show current motor status");
  Serial.println("enable          - Enable driver");
  Serial.println("disable         - Disable driver");
  Serial.println("help (h)        - Show this help menu");
  Serial.println("================\n");
}

void printStatus()
{
  Serial.println("\n=== MOTOR STATUS ===");
  Serial.print("Running: ");
  Serial.println(motorRunning ? "YES" : "NO");
  Serial.print("Direction: ");
  Serial.println(motorDirection ? "Reverse" : "Forward");
  Serial.print("Speed Period: ");
  Serial.print(stepPeriodUs);
  Serial.println(" us");
  Serial.print("Approximate RPM: ");
  
  // Calculate RPM based on your microstep setting
  // Adjust the divisor based on your MODE pin configuration:
  // Full step = 200, 1/2 = 400, 1/4 = 800, 1/8 = 1600, 1/16 = 3200, 1/32 = 6400
  float stepsPerSecond = 1000000.0 / stepPeriodUs;
  float rpm = (stepsPerSecond * 60.0) / (200.0 * 32.0); // Assumes 1/32 microstep
  Serial.print(rpm, 2);
  Serial.println(" (Verify microstep setting!)");
  Serial.println("====================\n");
}

/*
## Hardware Setup:

1. Connect your Arduino pins to DRV8434S:
   - STEP_PIN → STEP
   - DIR_PIN → DIR
   - ENABLE_PIN → nSLEEP (or tie to VCC if not controlling sleep)
   
2. Configure microstep mode using MODE0 and MODE1 pins:
   (See DRV8434S datasheet Table 7-7)
   - Full step: MODE0=LOW, MODE1=LOW
   - 1/2 step: MODE0=HIGH, MODE1=LOW
   - 1/4 step: MODE0=Float, MODE1=LOW
   - 1/8 step: MODE0=LOW, MODE1=HIGH
   - 1/16 step: MODE0=HIGH, MODE1=HIGH
   - 1/32 step: MODE0=Float, MODE1=HIGH
   
3. Update the RPM calculation in printStatus() based on your mode

4. Motor power supply to VM, logic to VCC (3.3V or 5V)
*/