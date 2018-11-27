//
// Walbi, the walking biped
//
// a project by The Inner Geek / Pedro Tavares Silva
//
// Tutorial at https://releasetheinnergeek.wordpress.com/
//
// Hardware: - Arduino Nano
//           - LewanSoul LX16-A serial servos x10
//           - LewanSoul Debug Board
//
// GNU General Public License v2.0
//
// This program replays a series os poses that can be recorded with Walbi_record.ino and 
// stored in the poseAngles array.
//

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // pin 10 = Arduino soft RX (connects to debug board TX)
                                 // pin 11 = Arduino soft TX (debub board RX)

//
// This code by LewanSoul to control LX16-A serial servos (not provided as library)
//

#define GET_LOW_BYTE(A) (byte)((A))
#define GET_HIGH_BYTE(A) (byte)((A) >> 8)
#define BYTE_TO_HW(A, B) ((((unsigned int)(A)) << 8) | (byte)(B))
#define LOBOT_SERVO_FRAME_HEADER          0x55
#define LOBOT_SERVO_MOVE_TIME_WRITE       1

byte buf[10];

byte LobotCheckSum()
{
  byte i;
  unsigned int temp = 0;
  for (i = 2; i < buf[3] + 2; i++) {
    temp += buf[i];
  }
  temp = ~temp;
  i = (byte)temp;
  return i;
}

void moveServo(byte id, unsigned int position, unsigned int time)
{
  buf[0] = buf[1] = LOBOT_SERVO_FRAME_HEADER;
  buf[2] = id;
  buf[3] = 7;
  buf[4] = 1; //LOBOT_SERVO_MOVE_TIME_WRITE
  buf[5] = GET_LOW_BYTE(position);
  buf[6] = GET_HIGH_BYTE(position);
  buf[7] = GET_LOW_BYTE(time);
  buf[8] = GET_HIGH_BYTE(time);
  buf[9] = LobotCheckSum();
  mySerial.write(buf, 10);
}

//
// End of code by LewanSoul
//

void setup() {

  Serial.begin(115200); // hardware serial - USB cable from Arduino to PC running Arduino IDE Serial Monitor
  while (!Serial) {}

  mySerial.begin(115200); // SoftwareSerial - connects Arduino to Debug Board serial pins (RX->TX, TX->RX, GND->GND)

  Serial.println("Walby play VERSION 1.0.0");
}

int timeToMove = 1500;

const byte numPoses = 5; // replaying a sequence of 5 poses, using 3 different poses recorded previously

unsigned int duration [numPoses] = {timeToMove, // how many miliseconds the robot has to move from pose 0 > 1
                                    timeToMove, // from pose 1 > 2
                                    timeToMove, // ...
                                    timeToMove,
                                    timeToMove
                                   };

boolean runOnce = true; // when the program starts the robot goes immediatly to the first pose

byte pose [numPoses] = {0, 1, 2, 1, 0}; // this is the order the poses will be played back

unsigned int poseAngles [3] [10] = {
  {565, 576, 538, 505, 357, 418, 489, 489, 641, 479}, // each pose has the angle value for each of the 10 servos
  {372, 575, 894, 504, 533, 605, 499, 164, 641, 292},
  {294, 577, 1004, 505, 586, 672, 495, 48, 640, 234}
};

void loop() {

  byte incomingByte;

  if (runOnce)
  {
    for (int b = 0; b < 10; b++)
      moveServo( b + 1, poseAngles [0] [b], 500 - b );
    Serial.println("Starting posture");
    runOnce = false;
  }

  Serial.println("Enter the number of the last pose to replay (first is zero, last is 4)");
  do {} while (Serial.available() == 0);

  incomingByte = Serial.read() - '0';

  if ((incomingByte >= 0) and (incomingByte < numPoses))
  {
    Serial.print("\nPlaying movements 0 to ");
    Serial.println(incomingByte);
    for (int a = 0; a < incomingByte + 1; a++)
    {
      for (int b = 0; b < 10; b++)
        moveServo(b + 1, poseAngles[ pose [a] ] [b], duration [a] - b );
      Serial.print("Pose: ");
      Serial.println( pose [a] );
      delay( duration [a] - 10 );
    };
  } else
    Serial.println("\nInvalid command!");

}
