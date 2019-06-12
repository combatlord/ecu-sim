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
      //unsigned char thisResp[] = {0x04, 0x43, 0x04, 0x46, 0x00, 0x46, 0x02}; //This gives ones valid code (C0600)
      //unsigned char thisResp[] = {0x06, 0x43, 0x00, 0x16, 0x00, 0x06, 0x01}; //sends two (P0601 and P1600), first digit is length
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
        unsigned char thisResp[8];
        int writePos =0;
        thisResp[writePos++] = 7;//length
        thisResp[writePos++] = 0x43; 
        thisResp[writePos++] = 0x10 | (totalCodesLen >> 8); //length
        thisResp[writePos++] = (totalCodesLen) & 0xFF; //length        
        for(int i=0; i < 2; i++){
          thisResp[writePos++] = (currentTroubleCodes[i] >> 8) & 0xFF;
          thisResp[writePos++] = currentTroubleCodes[i] & 0xFF;
        }
        CAN.sendMsgBuf(0x7E8, 0, sizeof(thisResp), thisResp);        
      }
            
}
