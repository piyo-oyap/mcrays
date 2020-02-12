
byte water_level, light_intensity, fan_speed;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

void parseCommand(String cmd) {
  char c = cmd.charAt(0);
  switch (c) {
    case 'W':
      water_level = cmd.substring(1, cmd.length()).toInt();
      break;
    case 'F':
      fan_speed = cmd.substring(1, cmd.length()).toInt();
      break;
    case 'L':
      light_intensity = cmd.substring(1, cmd.length()).toInt();
      break;
    default:
      Serial.println("Invalid Command");
      break;
  }
}
