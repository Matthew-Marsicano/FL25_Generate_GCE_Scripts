// Interactive DRV8434S stepper motor driver control via serial monitor

#include <SPI.h>
#include <DRV8434S.h>

const uint8_t CSPin = 10;

DRV8434S sd;

// Motor control variables
bool motorRunning = false;
uint8_t motorDirection = 0;  // 0 = forward, 1 = reverse
uint16_t stepPeriodUs = 2000;  // Speed control (lower = faster)
unsigned long lastStepTime = 0;

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) { ; }
  
  Serial.println("DRV8434S Interactive Stepper Driver");
  Serial.println("====================================");
  
  SPI.begin();
  sd.setChipSelectPin(CSPin);

  delay(1);

  sd.resetSettings();
  sd.clearFaults();
  
  Serial.println("Driver reset complete");

  sd.setCurrentMilliamps(1000);
  Serial.println("Current limit set to 1000 mA");
  
  sd.setStepMode(DRV8434SStepMode::MicroStep32);
  Serial.println("Step mode: 1/32 microstep");

  sd.enableSPIDirection();
  sd.enableSPIStep();
  Serial.println("SPI control enabled");

  sd.enableDriver();
  Serial.println("Driver enabled");
  
  printDriverStatus();
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
      sd.step();
      lastStepTime = currentTime;
    }
  }
}

void processCommand(String cmd)
{
  if (cmd == "start" || cmd == "s")
  {
    motorRunning = true;
    sd.setDirection(motorDirection);
    Serial.println("Motor STARTED");
    Serial.print("Direction: ");
    Serial.println(motorDirection == 0 ? "Forward" : "Reverse");
    Serial.print("Speed (period): ");
    Serial.print(stepPeriodUs);
    Serial.println(" us");
  }
  else if (cmd == "stop" || cmd == "x")
  {
    motorRunning = false;
    Serial.println("Motor STOPPED");
  }
  else if (cmd == "forward" || cmd == "f")
  {
    motorDirection = 0;
    sd.setDirection(motorDirection);
    Serial.println("Direction set to FORWARD");
    if (motorRunning) {
      Serial.println("(Change will take effect immediately)");
    }
  }
  else if (cmd == "reverse" || cmd == "r")
  {
    motorDirection = 1;
    sd.setDirection(motorDirection);
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
  else if (cmd == "fault")
  {
    printDriverStatus();
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
  Serial.println("fault           - Check driver fault status");
  Serial.println("help (h)        - Show this help menu");
  Serial.println("================\n");
}

void printStatus()
{
  Serial.println("\n=== MOTOR STATUS ===");
  Serial.print("Running: ");
  Serial.println(motorRunning ? "YES" : "NO");
  Serial.print("Direction: ");
  Serial.println(motorDirection == 0 ? "Forward" : "Reverse");
  Serial.print("Speed Period: ");
  Serial.print(stepPeriodUs);
  Serial.println(" us");
  Serial.print("Approximate RPM: ");
  float stepsPerSecond = 1000000.0 / stepPeriodUs;
  float rpm = (stepsPerSecond * 60.0) / (200.0 * 32.0); // 200 steps/rev * 32 microsteps
  Serial.println(rpm, 2);
  Serial.println("====================\n");
}

void printDriverStatus()
{
  // Read fault status
  uint16_t faultStatus = sd.readFault();
  
  Serial.print("Fault Status: 0x");
  Serial.print(faultStatus, HEX);
  
  if (faultStatus == 0)
  {
    Serial.println(" (No faults)");
  }
  else
  {
    Serial.println();
    if (faultStatus & 0x01) Serial.println("  - Fault detected");
    if (faultStatus & 0x02) Serial.println("  - SPI Error");
    if (faultStatus & 0x04) Serial.println("  - Undervoltage lockout");
    if (faultStatus & 0x08) Serial.println("  - Charge pump undervoltage");
    if (faultStatus & 0x10) Serial.println("  - Overtemperature warning");
    if (faultStatus & 0x20) Serial.println("  - Overtemperature shutdown");
    if (faultStatus & 0x40) Serial.println("  - Channel A overcurrent");
    if (faultStatus & 0x80) Serial.println("  - Channel B overcurrent");
  }
  
  Serial.println();
}
/*
## How to Use:

Open the Serial Monitor and type these commands:

**Control Commands:**
- `start` or `s` - Start the motor
- `stop` or `x` - Stop the motor
- `forward` or `f` - Set direction to forward
- `reverse` or `r` - Set direction to reverse
- `speed 1000` - Set speed (100-50000 microseconds, lower = faster)

**Information Commands:**
- `status` or `?` - Show current motor status
- `fault` - Check driver faults
- `help` or `h` - Show all commands

**Examples:**
```
speed 1000
forward
start
(motor runs forward at speed 1000)
stop
reverse
speed 5000
start
(motor runs reverse at slower speed)

speed 925 = 10.14 RPM



*/