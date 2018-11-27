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
// This program reads and displays the angle of each servo of the robot
//

#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // 10 = Arduino soft RX, 11 = Arduino soft TX

String message = "";

class CommandFeedback
{
  public:
    byte servoId;
    byte command;
    byte parameter[7];
    byte numPar;
    boolean messageComplete = false;
    int angle(void) {
      return (int(this->parameter[1]) * 256 + int(this->parameter[0]));
    };
};

CommandFeedback response;

void setup() {

  Serial.begin(115200); // hardware serial - USB cable from Arduino to PC running Arduino IDE Serial Monitor
  while (!Serial) {}

  Serial.println("\n\nHardware serial ready");

  mySerial.begin(115200); // SoftwareSerial - connects Arduino to Debug Board serial pins (RX->TX, TX->RX, GND->GND)

  Serial.println("PROGRAM VERSION 1.1.5\n\nSoftware serial ready\n\nSend 'R' to read servos");

}

// ========== SEND COMMAND =====================================
// =============================================================

void sendReadCommandToServos(byte servoID)
{
  byte command[6];

  command [0] = 0x55;
  command [1] = 0x55;
  command [2] = servoID;
  command [3] = 3;
  command [4] = 28; // SERVO_POS_READ
  command [5] = (~(servoID + 3 + 28)) & 0xff;

  mySerial.write(command, sizeof(command));
}

void process() {

  if (sizeof(message)) {

    switch (message.length()) {
      case 1: // discard message if first byte is invalid
        if (message[0] != 85) {
          message = "";
          // Serial.println("Discarder invalid byte");
        } else
          // Serial.println("First byte ok");
          break;
      case 2: // discard message if second byte is invalid
        if (message[1] != 85) {
          message = "";
          // Serial.println("Discarded two invalid bytes");
        } else
          // Serial.println("Second byte ok");
          break;
      case 3:
      case 4:
      case 5:
      case 6: // not enough data, wait for more data
        // Serial.println("Waiting for more data");
        break;
      default: // 7 and above
        // Serial.println("Analyzing...");
        if (message.length() == (byte(message[3]) + 3)) { // servo message complete
          //          Serial.println("Got enough data.");
          if (false) {// will check checksum in next version
            // checksum = (~(id + length + cmd + [5..len-1])) & 0xff
            message = "";
            Serial.println("Bad checksum. Discarded data.");
          } else {
            response.servoId = int(message[2]);
            response.command = int(message[4]);

            for (int i = 0; i < (int(message[3]) - 3); i++)
              response.parameter[i] = message[5 + i];

            response.numPar = message[3] - 3;
            response.messageComplete = true;
            message = "";

// Serial.println("Good data. Loaded into object.");
          }
        } else {
          //          Serial.print("Not enough data. Needed ");
          //          Serial.print(int(message[3]) + 3);
          //          Serial.print(" bytes and got ");
          //          Serial.println(message.length());
        }
    }
  }
}

boolean listen(int timeOut)
{
  int wait = 0;
  char incomming;

  do {
    if (mySerial.available()) {
      incomming = mySerial.read();
      if (incomming == char(-1))
        Serial.println("ERROR reading SoftwareSerial");
      else {
        message += incomming;
        //        Serial.print("\nmessage is now ");
        //        Serial.print(message.length());
        //        Serial.println(" chars long\n");
        //        Serial.print("Going to process string <");
        //        Serial.print(message);
        //        Serial.println(">");
        process();
      }
    }
    wait++;
    delay(1);
  } while ((wait < timeOut) and (!response.messageComplete));

  if (wait == timeOut) {
    Serial.println("ERROR: timed-out");
    return (false);
  } else
    return (true);
}

void loop() {

  byte incomingByte;

  if (Serial.available() > 0) { // wait for command R

    incomingByte = Serial.read();

    if (char(incomingByte) == 'R') {

      for (int i = 1; i <= 10; i++) {


        for (int a = 1; a <= 10; a++) {
          sendReadCommandToServos(i); //read servo 1
          if (listen(20))
            break;
        }

        if (response.messageComplete) {
          Serial.print("Servo ");
          Serial.print(response.servoId);
          Serial.print(" angle: ");
          Serial.println(response.angle());
          response.messageComplete = false;
        }

      }
    }
    else {
      Serial.print("\n\nInvalid command received: ");
      Serial.print("(" + String(char(incomingByte)) + ")");
    }

  }

}
