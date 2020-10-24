// Includes
#include <Servo.h>

// TF-Mini or TF-Luna
#define TFMINI_BAUDRATE 115200 // bauds
#define TFMINI_DATARATE 10.0f // ms
int distance = 0;
int strength = 0;
float temp = 0;

// Servos
Servo servo_g, servo_d;
#define SERVO_G_PIN PB0
#define SERVO_D_PIN PB1
#define SERVO_POS_NEUTRAL 90
#define SERVO_POS_MIN SERVO_POS_NEUTRAL - 40
#define SERVO_POS_MAX SERVO_POS_NEUTRAL + 40
int servo_angle = SERVO_POS_MIN; // current servo pos

// Stepper
#define DIR_PIN PB15 
#define STEP_PIN PA8
#define PULSE_PER_REV 2400 // PPR_motor * Gear_reduction * Microstepping = 200*3/2*16
#define TIME_PER_REV 500.0f // ms
const int TIME_PER_PULSE = (TIME_PER_REV * 1000.0f)/ PULSE_PER_REV; // us
const int PULSE_PER_DATAPOINT = (TFMINI_DATARATE * 1000.0f) / TIME_PER_PULSE;

// External communication
#define EXTERNAL_BAUDRATE 115200 // bauds
char serial_buffer[15];
float theta,phi,rho;

// the setup function runs once when you press reset or power the board
void setup() {
  // Stepper
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(DIR_PIN,HIGH);
  
  // Servos
  servo_g.attach(SERVO_G_PIN);
  servo_d.attach(SERVO_D_PIN);
  servo_pos(servo_angle);
  
  // Serial ports
  Serial.begin(EXTERNAL_BAUDRATE); // USB
  Serial3.begin(TFMINI_BAUDRATE); // TF mini
  flushSerial3();
  
  // Center servos
  //servo_pos(SERVO_POS_MAX   );
  //for(;;){}
}

// Scanning fuction. Adapt to your needs!
uint16_t pulses = 0;
int8_t servo_dir = 1;
int8_t offset = 0;
void loop() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(TIME_PER_PULSE);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(TIME_PER_PULSE);
  pulses ++;

  if((pulses + offset)%PULSE_PER_DATAPOINT == 0){
    getTFminiData(&distance, &strength, &temp);
    send_pos();
  }

  if(pulses == PULSE_PER_REV) {
  pulses = 0;
  servo_angle += servo_dir;
  if(servo_angle > SERVO_POS_MAX || servo_angle < SERVO_POS_MIN){
    servo_dir = -servo_dir;
  }
  servo_pos(servo_angle);
  }
}

// Send X Y Z STRENGTH to the computer
// @TODO: reverse kinematics
void send_pos(){
  if(distance == 0) return;
  
  theta = (float)pulses*360.0f/PULSE_PER_REV; //compute angle from motor pulses
  theta *= PI / 180.0f; // convert to radians
  
  phi = (float)servo_angle*-0.3f+71; // compute angle from servo geometry
  phi = 2*phi - 90.0f; // compute mirror bounce angle
  phi *= PI / 180.0f; // convert to radians
  
  rho = distance - 5.5f; // rho is the LIDAR distance
  
  sprintf(serial_buffer,"%d\t%d\t%d\t%d\n\0",(int)(rho*cos(phi)*cos(theta)),(int)(rho*cos(phi)*sin(theta)),(int)(rho*sin(phi)),strength);
  Serial.print(serial_buffer);
}

// Move both servos to change the mirror angle
void servo_pos(int angle){
  if(angle > SERVO_POS_MAX || angle < SERVO_POS_MIN)
  return;
    servo_g.write(180 - angle);
    servo_d.write(angle);
}

// Fetchs data from the Lidar
void getTFminiData(int* distance, int* strength, float* temp) {
  static char i = 0;
  char j = 0;
  int checksum = 0;
  static int rx[9];
  while(1){
  if(Serial3.available())
  { 
      rx[i] = Serial3.read();
//      Serial.println(rx[i],HEX);
      if(rx[0] != 0x59) {
        i = 0;
      } else if(i == 1 && rx[1] != 0x59) {
        i = 0;
      } else if(i == 8) {
        for(j = 0; j < 8; j++) {
          checksum += rx[j];
        }
        if(rx[8] == (checksum % 256)) {
          *distance = rx[2] + rx[3] * 256;
          *strength = rx[4] + rx[5] * 256;
          *temp = (rx[6] + rx[7] * 256.0f)/8 - 256;
        }
        i = 0;
        break;
      } else
      {
        i++;
      }
    }
  }
  flushSerial3();
}

// Flushes the INPUT serial buffer
void flushSerial3(){
  while(Serial3.available()){Serial3.read();}
}
