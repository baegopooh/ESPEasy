//#######################################################################################################
//#################### Plugin 101 GDK101 I2C Gamma Ray(Radone) Sensor      #######################
//#######################################################################################################


#define PLUGIN_101
#define PLUGIN_ID_101        101
#define PLUGIN_NAME_101       "Environment - GDK101"
#define PLUGIN_VALUENAME1_101 "Gamma Ray(μSv/h), 10min avg"
#define PLUGIN_VALUENAME2_101 "Gamma Ray(μSv/h), 1min avg"

enum
{
//   gdk101_REGISTER_DIG_T1              = 0x88,
//   gdk101_REGISTER_DIG_T2              = 0x8A,
//   gdk101_REGISTER_DIG_T3              = 0x8C,
//
//   gdk101_REGISTER_DIG_P1              = 0x8E,
//   gdk101_REGISTER_DIG_P2              = 0x90,
//   gdk101_REGISTER_DIG_P3              = 0x92,
//   gdk101_REGISTER_DIG_P4              = 0x94,
//   gdk101_REGISTER_DIG_P5              = 0x96,
//   gdk101_REGISTER_DIG_P6              = 0x98,
//   gdk101_REGISTER_DIG_P7              = 0x9A,
//   gdk101_REGISTER_DIG_P8              = 0x9C,
//   gdk101_REGISTER_DIG_P9              = 0x9E,
//
//   gdk101_REGISTER_CHIPID             = 0xD0,
  gdk101_REGISTER_VERSION            = 0xB4,
//   gdk101_REGISTER_SOFTRESET          = 0xE0,
//
//   gdk101_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0
//
//   gdk101_REGISTER_CONTROL            = 0xF4,
//   gdk101_REGISTER_CONFIG             = 0xF5,
//   gdk101_REGISTER_PRESSUREDATA       = 0xF7,
//   gdk101_REGISTER_TEMPDATA           = 0xFA,
//
//   gdk101_CONTROL_SETTING             = 0x57, // Oversampling: 16x P, 2x T, normal mode
//   gdk101_CONFIG_SETTING              = 0xE0, // Tstandby 1000ms, filter 16, 3-wire SPI Disable
};

// typedef struct
// {
//   uint16_t dig_T1;
//   int16_t  dig_T2;
//   int16_t  dig_T3;
//
//   uint16_t dig_P1;
//   int16_t  dig_P2;
//   int16_t  dig_P3;
//   int16_t  dig_P4;
//   int16_t  dig_P5;
//   int16_t  dig_P6;
//   int16_t  dig_P7;
//   int16_t  dig_P8;
//   int16_t  dig_P9;
// } gdk101_calib_data;

// gdk101_calib_data _gdk101_calib[2];

uint8_t gdk101_i2caddr;
// uint16_t Plugin_101_day,Plugin_101_hour,Plugin_101_min,Plugin_101_sec = 0;
uint8_t Plugin_101_buffer[2] = {0,0};
// uint16_t Plugin_101_status = 0;

// boolean Plugin_101_init[4] = {false, false, false, false};

boolean Plugin_101(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_101;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_101);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_101));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_101));
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        /*String options[2] = { F("0x76 - default settings (SDO Low)"), F("0x77 - alternate settings (SDO HIGH)") };*/
        int optionValues[4] = { 0x18, 0x19, 0x1a, 0x1b };
        addFormSelectorI2C(string, F("plugin_101_gdk101_i2c"), 4, optionValues, choice);
        addFormNote(string, F("* Default Address :: 0x18  A0 Open, A1 Short :: 0x19 A0 Short, A1 Open :: 0x1A  A0 Open, A1 Open :: 0x1B"));

        // addFormNumericBox(string, F("Altitude"), F("plugin_101_gdk101_elev"), Settings.TaskDevicePluginConfig[event->TaskIndex][1]);
        // addUnit(string, F("m"));

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_101_gdk101_i2c"));
        // Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_101_gdk101_elev"));
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        gdk101_i2caddr = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String log = F("Radone Sensor : Address: 0x");
        log += String(gdk101_i2caddr,HEX);
        addLog(LOG_LEVEL_INFO, log);

        Plugin_101_command(0xb0);  //read sensor status
        if (Plugin_101_buffer[0]==0 || Plugin_101_buffer[0]==1) {
          addLog(LOG_LEVEL_INFO, F("Radone Sensor : Not ready to measure(wait 10min after power up)"));
          break;
        }
        else if (Plugin_101_buffer[1]==1){
          addLog(LOG_LEVEL_INFO, F("Radone Sensor : Vibration Detected. No new Measure"));
          break;
        }
        Plugin_101_command(0xb2) ; // read sensor value(10min avg)
        UserVar[event->BaseVarIndex] = Plugin_101_buffer[0] + (float)Plugin_101_buffer[1]/100;
        log = F("Radone Sensor : Gamma Ray(μSv/h), 10min avg: ");
        log += Plugin_101_buffer[0] + (float)Plugin_101_buffer[1]/100;
        addLog(LOG_LEVEL_INFO, log);
        Plugin_101_command(0xb3);  // read sensor value(10min avg)
        UserVar[event->BaseVarIndex+1] = Plugin_101_buffer[0] + (float)Plugin_101_buffer[1]/100;
        log = F("Radone Sensor :Gamma Ray(μSv/h), 1min avg: ");
        log += Plugin_101_buffer[0] + (float)Plugin_101_buffer[1]/100;
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }

  }
  return success;
}


//**************************************************************************/
// Transmission command and read value over i2c
//**************************************************************************/
void Plugin_101_command(int cmd){
 /* Begin Write Sequence */
  Wire.beginTransmission(gdk101_i2caddr);
  Wire.write(cmd);
  Wire.endTransmission();
/* End Write Sequence */
  delay(10);
/* Begin Read Sequence */
  Wire.requestFrom(gdk101_i2caddr, (byte)2);
  byte i = 0;
  while(Wire.available())
  {
    Plugin_101_buffer[i] = Wire.read();
    i++;
  }
  // addLog(LOG_LEVEL_INFO, "1byte:" + Plugin_101_buffer[0]);
  // addLog(LOG_LEVEL_INFO, "2byte:" + Plugin_101_buffer[1]);
}
// //**************************************************************************/
// // Calculate Measuring time
// //**************************************************************************/
// void Cal_Measuring_Time(){
//    if(Plugin_101_sec == 60) { Plugin_101_sec = 0; Plugin_101_min++; }
//    if(Plugin_101_min == 60) { Plugin_101_min = 0; Plugin_101_hour++; }
//    if(Plugin_101_hour == 24) { Plugin_101_hour = 0; Plugin_101_day++; }
//    Serial.print("Measuring Time\t\t\t");
//    Serial.print(Plugin_101_day); Serial.print("d ");
//    if(Plugin_101_hour < 10) Serial.print("0");
//    Serial.print(Plugin_101_hour); Serial.print(":");
//    if(Plugin_101_min < 10) Serial.print("0");
//    Serial.print(Plugin_101_min); Serial.print(":");
//    if(Plugin_101_sec < 10) Serial.print("0");
//    Serial.println(Plugin_101_sec);
// }
//
//
// //**************************************************************************/
// // result
// //**************************************************************************/
// void print_result(int cmd){
//   float value = 0.0f;
//   switch(cmd){
//     case 0xA0:
//       Serial.print("Reset Response\t\t\t");
//       if(Plugin_101_buffer[0]== 1) Serial.println("Reset Success.");
//       else Serial.println("Reset Fail(Status - Ready).");
//       break;
//     case 0xB0:
//       Serial.print("Status\t\t\t\t");
//       switch(Plugin_101_buffer[0]){
//         case 0: Serial.println("Ready"); break;
//         case 1: Serial.println("10min Waiting"); break;
//         case 2: Serial.println("Normal"); break;
//       }
//       Plugin_101_status = Plugin_101_buffer[0];
//       Serial.print("VIB Status\t\t\t");
//       switch(Plugin_101_buffer[1]){
//         case 0: Serial.println("OFF"); break;
//         case 1: Serial.println("ON"); break;
//       }
//       break;
//     case 0xB1:
//       if(Plugin_101_status > 0){
//         Plugin_101_sec++;
//         Cal_Measuring_Time();
//       }
//       break;
//     case 0xB2:
//       Serial.print("Measuring Value(10min avg)\t");
//       value = Plugin_101_buffer[0] + (float)Plugin_101_buffer[1]/100;
//       Serial.print(value); Serial.println(" uSv/hr");
//       break;
//     case 0xB3:
//       Serial.print("Measuring Value(1min avg)\t");
//       value = Plugin_101_buffer[0] + (float)Plugin_101_buffer[1]/100;
//       Serial.print(value); Serial.println(" uSv/hr");
//       break;
//     case 0xB4:
//       Serial.print("FW Version\t\t\t");
//       Serial.print("V"); Serial.print(Plugin_101_buffer[0]);
//       Serial.print("."); Serial.println(Plugin_101_buffer[1]);
//       break;
//   }
// }
