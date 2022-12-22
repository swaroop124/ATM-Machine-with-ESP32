
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>  
#include <ArduinoJson.h>
#include <global.h>

const char* NAME__OF__WiFi = nameWifi;
const char* PASSWORD__OF__WiFi = passWifi;

const char* TOKEN__BOT=botToken;  

//Our Chat Id
const char* User__CHAT__id = chatID;

String USER__Google_SCRIPT=userScript; 
String STATEMENT__Google_Id =statementScript;
 
String LEDGER__Google_SCRIPT=""; 

String Entered_username="";  
String Entered_password=""; 

WiFiClientSecure C_l_i_e_n_T;
UniversalTelegramBot TELEGRAM_bot(TOKEN__BOT, C_l_i_e_n_T); 

int bot__REQUEST_Delay = 1000; 
unsigned long Last_bot_RUN_TIME;  


bool RepeaT=true;
bool user__INCOMING=false;
bool password__INCOMING=false;
bool Transcation__REPEAT=true;
bool credit__INCOMING=false;
bool debit__INCOMING=false;

//Verifies user
void handle_verify(int NEW_messages) {
  for (int i=0; i<NEW_messages; i++) {
    String Obtained_User__ID = String(TELEGRAM_bot.messages[i].chat_id);
    
    if (Obtained_User__ID != User__CHAT__id){
      TELEGRAM_bot.sendMessage(Obtained_User__ID, "Unauthorized user", "");
      continue;
    }
    String response = TELEGRAM_bot.messages[i].text;  

    if (response == "/start") { 
      String s = "Welcome To Our Bank.\n";
      s += "Use the following commands to control your outputs.\n\n";
      s += "Please Enter Your Username:";
      user__INCOMING=true;
      TELEGRAM_bot.sendMessage(Obtained_User__ID, s, "");   
    }else{
      if(user__INCOMING){                       
        Entered_username=response;
        user__INCOMING=false;                          
        TELEGRAM_bot.sendMessage(Obtained_User__ID,"Please Enter Your Password:"); 
        password__INCOMING=true;                      

      }else if(password__INCOMING==true){
        Entered_password=response;                    
        password__INCOMING=false;

        
        if (WiFi.status() == WL_CONNECTED) {           
          HTTPClient http;                    
          String ACESSED__URL = "https://script.google.com/macros/s/" + USER__Google_SCRIPT + "/exec?data1="+Entered_username+"&data2="+Entered_password;
          http.begin(ACESSED__URL.c_str()); 
          http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
          int HTTP_return_Code = http.GET();   
          String RES;
          if (HTTP_return_Code > 0) { 
            RES = http.getString();

            if(RES==""){
              TELEGRAM_bot.sendMessage(Obtained_User__ID,"Wrong Username or Password"); 
              return;
            }else{
              LEDGER__Google_SCRIPT=RES;  
              TELEGRAM_bot.sendMessage(Obtained_User__ID, "Correct UserId and Password");
              RepeaT=false; 
              TELEGRAM_bot.sendMessage(Obtained_User__ID, "For credit,type /credit \n For debit,type /debit \n For Statement,type /statement \n To logout,type /logout");
              transcation();    
            }
          }
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      
   C_l_i_e_n_T.setTrustAnchors(&cert); 
  #endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(NAME__OF__WiFi, PASSWORD__OF__WiFi);
  #ifdef ESP32
   C_l_i_e_n_T.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
  #endif
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  
  Serial.println("Connected to WiFi Successfully!");

  verification();
}


void verification() {
  while(RepeaT==true){
    if (millis() > Last_bot_RUN_TIME + bot__REQUEST_Delay)  {
      int NEW_messages = TELEGRAM_bot.getUpdates(TELEGRAM_bot.last_message_received + 1);

      while(NEW_messages) {
        handle_verify(NEW_messages); 
        NEW_messages = TELEGRAM_bot.getUpdates(TELEGRAM_bot.last_message_received + 1);
      }
      Last_bot_RUN_TIME = millis();
    }
  }
}

void transcation(){    
    while(Transcation__REPEAT=true){
      if (millis() > Last_bot_RUN_TIME + bot__REQUEST_Delay)  {
        int NEW_messages = TELEGRAM_bot.getUpdates(TELEGRAM_bot.last_message_received + 1);

        while(NEW_messages) {
          handle_transcation(NEW_messages); 
          NEW_messages = TELEGRAM_bot.getUpdates(TELEGRAM_bot.last_message_received + 1);
        }
        Last_bot_RUN_TIME = millis();
      }
    }
}


void handle_transcation( int NEW_messages) {
  for (int i=0; i<NEW_messages; i++) { 
    String Obtained_User__ID = String(TELEGRAM_bot.messages[i].chat_id);    
    if (Obtained_User__ID != User__CHAT__id){   
      TELEGRAM_bot.sendMessage(Obtained_User__ID, "Unauthorized user", "");
      continue;
    }
    String response = TELEGRAM_bot.messages[i].text;
    Serial.println(response);

    if (response == "/credit") { 
      credit__INCOMING=true;    
      TELEGRAM_bot.sendMessage(Obtained_User__ID, "Please Enter the Credit Amount:"); 
    }else if(response=="/debit"){ 
      debit__INCOMING=true; 
      TELEGRAM_bot.sendMessage(Obtained_User__ID, "Please Enter the Debit Amount:"); 
    }else if(response=="/statement"){       
      if (WiFi.status() == WL_CONNECTED) {           
          HTTPClient http;
          String ACESSED__URL = "https://script.google.com/macros/s/" + STATEMENT__Google_Id + "/exec?data1="+LEDGER__Google_SCRIPT;
          http.begin(ACESSED__URL.c_str()); 
          http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
          int HTTP_return_Code = http.GET(); 
          String RES;
          if (HTTP_return_Code > 0) {
            RES = http.getString();
            ACESSED__URL="https://script.google.com/macros/s/" + RES + "/exec?read";
            http.begin(ACESSED__URL.c_str());
            http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
            HTTP_return_Code=http.GET();
            if(HTTP_return_Code>0){
              RES = http.getString();
              String t="   Credit Debit Balance \n 1.";
              int k=1;
              for(int q=0;q<RES.length();q++){
                if(RES[q]==';' && q!=RES.length()-1){
                  t+=" \n ";
                  k++;
                  t+=String(k);
                  t+=". ";
                }else{
                  t+=RES[q];
                }
              }
              Serial.println(t);
              TELEGRAM_bot.sendMessage(Obtained_User__ID, t);  
            }
          }
        }
    }else if( response=="/logout"){      
      LEDGER__Google_SCRIPT="";       
      TELEGRAM_bot.sendMessage(Obtained_User__ID, "Thanks for using our Bank.");
      verification();
    }else{
      if (WiFi.status() == WL_CONNECTED) {           
        HTTPClient http;
        String ACESSED__URL="";
        if(credit__INCOMING==true){
          ACESSED__URL = "https://script.google.com/macros/s/" + LEDGER__Google_SCRIPT + "/exec?data1="+response+"&data2=0";
          credit__INCOMING=false;
        }
        if(debit__INCOMING==true){
          ACESSED__URL = "https://script.google.com/macros/s/" + LEDGER__Google_SCRIPT + "/exec?data1=0&data2="+response;
          debit__INCOMING=false;
        }
        http.begin(ACESSED__URL.c_str()); 
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int HTTP_return_Code = http.GET();
        String RES;
        if (HTTP_return_Code > 0) { 
          RES = http.getString();
          Serial.println("Current Balance:"+RES);
          TELEGRAM_bot.sendMessage(Obtained_User__ID,"Your Current Balance is "+RES);            
        }
      }
    }  
  }
}

void loop(){
  transcation();
}