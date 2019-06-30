#include <mcp_can.h>
#include <SPI.h>

#define CS_PIN 10
#define NUM_SENSOR_VALUES 256
#define MAX_TROUBLE_CODES 128

unsigned char STATUS_STARTUP = 0x01;
unsigned char RETRYING = 0x03;
unsigned char STATUS_RUNNING = 0x02;
unsigned char RESP_OK = 0x00;
unsigned char RESP_FULL = 0x05;

MCP_CAN CAN(CS_PIN);                                      // Set CS to pin 10
                                  // Set CS to pin 9

unsigned int canId = 0x000;

unsigned char len = 0;
unsigned char buf[8];
char str[20];

String BuildMessage="";
int MSGIdentifier=0;

unsigned char connection_status = STATUS_STARTUP;

unsigned char currentValues[NUM_SENSOR_VALUES];
uint16_t currentTroubleCodes[MAX_TROUBLE_CODES];

uint16_t numTroubleCodes = 0;

void setup() {
  Serial.begin(115200);
 
  //Serial.println("start fake ECU");
  
  memset(currentValues, 0, sizeof(currentValues));


    while(true)
    {
      if(CAN_OK == CAN.begin(CAN_500KBPS))
        break;
      else
        delay(100);        
    }
}

void loop()
{      
    if(Serial.available() > 0){
      int messageType = Serial.read();
      switch(messageType){
        case 0x01:
          sendStatus();
          break;
        case 0x02:
          setSensorValue();
          break;
        case 0x03:
          setTroubleCode();
          break;
      }
    }

    
    if(CAN_MSGAVAIL == CAN.checkReceive())  
    {
      
      CAN.readMsgBuf(&len, buf); 
        canId = CAN.getCanId();

        switch(buf[1]){
          case 0x01:
              handleCurrentDataReq(buf);
              break;          
          case 0x03:
          case 0x07:
              handleTroubleCodeReq(buf);
              break;
          case 0x04:
              clearErrorCodes();
              break;
        }
    }
    
}


void sendStatus(){
}

void setSensorValue(){
  unsigned char valCode = Serial.read() & 0xFF;
  unsigned char valVal = Serial.read() & 0xFF;
  currentValues[valCode] = valVal;
  Serial.write(RESP_OK);
}

void setTroubleCode(){
  uint16_t codeValue = Serial.read() & 0xFF;
  codeValue = (codeValue << 8) | (Serial.read() & 0xFF);
  if(numTroubleCodes == MAX_TROUBLE_CODES){
    Serial.write(RESP_FULL);
    return;
  }

  currentTroubleCodes[numTroubleCodes++] = codeValue;
  
  Serial.write(RESP_OK);
  
}

void handleCurrentDataReq(unsigned char buf[8]){
    //TODO: implement this later
    switch(buf[2])
    {      
      case 0x01:
      {        
        char codeStatus = 0x00;
        if(numTroubleCodes > 0){
          codeStatus = 0x80 | numTroubleCodes;
        }
        unsigned char numTroubleCodesMsg[7] = {0x04, 0x41, buf[2], codeStatus, 0x00, 0x00, 0x00};
        CAN.sendMsgBuf(0x7E8, 0, sizeof(numTroubleCodesMsg), numTroubleCodesMsg);
        
      }
        break;
      default:          
        unsigned char thisResp[7] = {0x04, 0x41, buf[2], currentValues[buf[2]], 0, 185, 147};
        CAN.sendMsgBuf(0x7E8, 0, sizeof(thisResp), thisResp);
        break;
    } 
}

void handleTroubleCodeReq(unsigned char buf[8]){
    if(numTroubleCodes <= 2){
        unsigned char msgLen = 3 + 2*numTroubleCodes;
        int writePos =0;
        unsigned char thisResp[msgLen];
        thisResp[writePos++] = msgLen;
        thisResp[writePos++] = 0x43;
        thisResp[writePos++] = 0x00 | (2*numTroubleCodes);
        for(int i=0; i< numTroubleCodes; i++){
          thisResp[writePos++] = (currentTroubleCodes[i] >> 8) & 0xFF;
          thisResp[writePos++] = currentTroubleCodes[i] & 0xFF;
        }
        CAN.sendMsgBuf(0x7E8, 0, sizeof(thisResp), thisResp);        
    }
    else
    {
        //TODO: handle more than 2 trouble codes
        unsigned int totalCodesLen = 2*numTroubleCodes;
        unsigned char firstResp[8];
        int writePos =0;
        int codeBytesWritten = 0;
        firstResp[writePos++] = sizeof(firstResp) -1 ;//length
        firstResp[writePos++] = 0x43;
        for(; writePos <  sizeof(firstResp); writePos++){
          if(codeBytesWritten % 2 == 0)
          {
              firstResp[writePos] = (currentTroubleCodes[codeBytesWritten/2] >> 8) & 0xFF;
          }
          else
          {
              firstResp[writePos] = currentTroubleCodes[codeBytesWritten/2] & 0xFF;
          }
          codeBytesWritten++;
        }
        CAN.sendMsgBuf(0x7E8, 0, sizeof(firstResp), firstResp);
        int frameNum = 1;
        unsigned char nextResp[7];
        while(codeBytesWritten < totalCodesLen){
          memset(nextResp, 0, sizeof(nextResp));
          int writePos =0;
          int thisSize = (totalCodesLen - codeBytesWritten) + 3;
          thisSize = thisSize > (sizeof(nextResp)) ? sizeof(nextResp) : thisSize;
          nextResp[writePos++] = thisSize -1 ;//length
          nextResp[writePos++] = 0x43;
          nextResp[writePos++] = 0x20 | frameNum;
          for(; writePos < thisSize; writePos++){
            if(codeBytesWritten % 2 == 0)
              {
                  nextResp[writePos] = (currentTroubleCodes[codeBytesWritten/2] >> 8) & 0xFF;
              }
              else
              {
                  nextResp[writePos] = currentTroubleCodes[codeBytesWritten/2] & 0xFF;
              }
              codeBytesWritten++;
          }
          CAN.sendMsgBuf(0x7E8, 0, thisSize, nextResp);
          frameNum++;
        }
        
    }
}

void clearErrorCodes(){
  numTroubleCodes = 0;
}
