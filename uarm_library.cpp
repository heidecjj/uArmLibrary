/******************************************************************************
* File Name          : uArm_library.cpp
* Author             : Joey Song  
* Updated            : Joey Song, Alex Tan, Dave Corboy
* Email              : joey@ufactory.cc
* Version            : V1.2 
* Date               : 12 Dec, 2014
* Modified Date      : 17 Dec, 2015
* Description        : 
* License            : 
* Copyright(C) 2014 UFactory Team. All right reserved.
*******************************************************************************/

#include "uarm_library.h"

uArmClass uarm;

uArmClass::uArmClass()
{
  g_servo_offset = 0.0;
}

void uArmClass::alert(byte times, byte runTime, byte stopTime)
{
  for(int ct=0; ct < times; ct++)
  {
    delay(stopTime);
    digitalWrite(BUZZER, HIGH);
    delay(runTime);
    digitalWrite(BUZZER, LOW);
  }
}

/* The code below is written by jerry song */

void uArmClass::writeAngle(byte servo_rot_angle, byte servo_left_angle, byte servo_right_angle, byte servo_hand_rot_angle, byte trigger)
{
  attachAll();
  g_servo_rot.write(round(servo_rot_angle));
  g_servo_left.write(round(servo_left_angle));
  g_servo_right.write(round(servo_right_angle));
  g_servo_hand_rot.write((round(servo_hand_rot_angle)));

  // refresh logical servo angle cache
  cur_rot = readAngle(SERVO_ROT_NUM);
  cur_left = readAngle(SERVO_LEFT_NUM);
  cur_right = readAngle(SERVO_RIGHT_NUM);
  cur_hand = readAngle(SERVO_HAND_ROT_NUM);
}

void uArmClass::writeAngle(double servo_rot_angle, double servo_left_angle, double servo_right_angle, double servo_hand_rot_angle)
{
  attachAll();
  
  if(servo_left_angle < 10) servo_left_angle = 10;
  if(servo_left_angle > 120) servo_left_angle = 120;
  if(servo_right_angle < 10) servo_right_angle = 10;
  if(servo_right_angle > 110) servo_right_angle = 110;

  if(servo_left_angle + servo_right_angle > 160) 
  {
    servo_right_angle = 160 - servo_left_angle;
    return;
  }

  int servo_rot_angle_execute = inputToReal(SERVO_ROT_NUM,servo_rot_angle);
  int servo_left_angle_execute = inputToReal(SERVO_LEFT_NUM,servo_left_angle);
  int servo_right_angle_execute = inputToReal(SERVO_RIGHT_NUM,servo_right_angle);
  int servo_hand_rot_angle_execute = inputToReal(SERVO_HAND_ROT_NUM,servo_hand_rot_angle);

#if 1
  g_servo_rot.write(servo_rot_angle_execute);
  g_servo_left.write(servo_left_angle_execute);
  g_servo_right.write(servo_right_angle_execute);
  g_servo_hand_rot.write(servo_hand_rot_angle_execute);
#else
  g_servo_rot.write(90);
  g_servo_left.write(80);
  g_servo_right.write(10);
  g_servo_hand_rot.write(90);
  Serial.print(F("Write logical angles: "));
  Serial.print(servo_rot_angle);
  Serial.print(F(", "));
  Serial.print(servo_left_angle);
  Serial.print(F(", "));
  Serial.print(servo_right_angle);
  Serial.print(F(", "));
  Serial.println(servo_hand_rot_angle);
#endif

  // refresh logical servo angle cache
  cur_rot = servo_rot_angle;
  cur_left = servo_left_angle;
  cur_right = servo_right_angle;
  cur_hand = servo_hand_rot_angle;
}

void uArmClass::attachAll()
{
  if (!g_servo_rot.attached()) {
    g_servo_rot.attach(11);
    cur_rot = readAngle(SERVO_ROT_NUM);
  }
  if (!g_servo_left.attached()) {
    g_servo_left.attach(13);
    cur_left = readAngle(SERVO_LEFT_NUM);
  }
  if (!g_servo_right.attached()) {
    g_servo_right.attach(12);
    cur_right = readAngle(SERVO_RIGHT_NUM);
  }
  if (!g_servo_hand_rot.attached()) {
    g_servo_hand_rot.attach(10);
    cur_hand = readAngle(SERVO_HAND_ROT_NUM);
  }
}

void uArmClass::detachAll()
{
  g_servo_rot.detach();
  g_servo_left.detach();
  g_servo_right.detach();
  g_servo_hand_rot.detach();
}

byte uArmClass::inputToReal(byte servo_num, double input_angle)
{
  return (byte)round(constrain((input_angle + readServoOffset(servo_num)),0,180));
}

double uArmClass::readServoOffset(byte servo_num)
{
  if ((servo_num == 1)||(servo_num == 2)||(servo_num == 3))
  {
    g_servo_offset = (EEPROM.read(LINEAR_START_ADDRESS + (servo_num-1)*2 +1))/10.00;

    if (EEPROM.read(LINEAR_START_ADDRESS + (servo_num-1)*2 ) == 0)
      {g_servo_offset = - g_servo_offset;}

    return g_servo_offset;
  } 
  else if (servo_num == 4)
    return 0;
  else {
      Serial.println(F("Incorrect"));
  }
}

void uArmClass::saveDataToRom(double data, int address)
{
  int dataWhole;

  byte Byte0;
  byte Byte1;
  byte Byte2;

  if(abs(data) < 1) {
    dataWhole = (int) (data*100);
  }
  else{
    dataWhole = (int) (data*10);
  }

  if (dataWhole > 0){ 
    Byte0 = 1;
  }
  else{ 
    Byte0 =0; 
  }

  dataWhole = abs(dataWhole);

  Byte1 = (( dataWhole >> 0) & 0xFF);
  Byte2 = (( dataWhole >> 8) & 0xFF);

  EEPROM.write( address, Byte0);
  EEPROM.write( address + 1, Byte1);
  EEPROM.write( address + 2, Byte2);
}

double uArmClass::readToAngle(double input_analog, byte servo_num, byte trigger)
{
  int address = OFFSET_START_ADDRESS +(servo_num-1)*6;

  double data_a = (EEPROM.read(address+1)+EEPROM.read(address+2)*256)/10.0;
  if (EEPROM.read(address)==0)
    {data_a = -data_a;}

  double data_b = (EEPROM.read(address+4)+EEPROM.read(address+5)*256)/100.0;
  if (EEPROM.read(address+3)==0)
    {data_b = -data_b;}

  if (trigger == 0){
    return (data_a + data_b*input_analog) - readServoOffset(servo_num);
  }
  if (trigger == 1){
    return (data_a + data_b*input_analog);
  }
}

double uArmClass::readAngle(byte servo_num)
{
  return readAngle(servo_num, 0);
}

double uArmClass::readAngle(byte servo_num, byte trigger)
{
  switch (servo_num)
  {
    case SERVO_ROT_NUM:
      return readToAngle(analogRead(SERVO_ROT_ANALOG_PIN),SERVO_ROT_NUM,trigger);
      break;

    case SERVO_LEFT_NUM:
      return readToAngle(analogRead(SERVO_LEFT_ANALOG_PIN),SERVO_LEFT_NUM,trigger);
      break;

    case SERVO_RIGHT_NUM:
      return readToAngle(analogRead(SERVO_RIGHT_ANALOG_PIN),SERVO_RIGHT_NUM,trigger);
      break;

    case SERVO_HAND_ROT_NUM:
      return readToAngle(analogRead(SERVO_HAND_ROT_ANALOG_PIN),SERVO_HAND_ROT_NUM,trigger);
      break;

    default:
      break;

  }
}

/*Action control */

void uArmClass::calAngles(double x, double y, double z)
{
   if (z > (MATH_L1 + MATH_L3 + TopOffset))
    {
        z = MATH_L1 + MATH_L3 + TopOffset;
    }

    if (z < (MATH_L1 - MATH_L4 + BottomOffset))
    {
        z = MATH_L1 - MATH_L4 + BottomOffset;
    }

  double x_in = 0.0;
  double y_in = 0.0;
  double z_in = 0.0;
  double right_all = 0.0;
  double right_all_2 = 0.0;
  double sqrt_z_x = 0.0;
  double sqrt_z_y = 0.0;
  double phi = 0.0;

  y_in = (-y-MATH_L2)/MATH_L3;
  z_in = (z - MATH_L1) / MATH_L3;
  right_all = (1 - y_in*y_in - z_in*z_in - MATH_L43*MATH_L43) / (2 * MATH_L43);
  sqrt_z_y = sqrt(z_in*z_in + y_in*y_in);

  if (x == 0)
  {
    // Calculate value of theta 1
    g_theta_1 = 90;

    // Calculate value of theta 3
    if (z_in == 0) {
      phi = 90;
    }

    else {
    phi = atan(-y_in / z_in)*MATH_TRANS;
    }

    if (phi > 0) phi = phi - 180;

    g_theta_3 = asin(right_all / sqrt_z_y)*MATH_TRANS - phi;
    if(g_theta_3<0)
      {
        g_theta_3 = 0;
      }

    // Calculate value of theta 2
    g_theta_2 = asin((z + MATH_L4*sin(g_theta_3 / MATH_TRANS) - MATH_L1) / MATH_L3)*MATH_TRANS;
  }
  else
  {
    // Calculate value of theta 1

    g_theta_1 = atan(y / x)*MATH_TRANS;

    if (y / x > 0) {
      g_theta_1 = g_theta_1;
    }
    if (y / x < 0) {
      g_theta_1 = g_theta_1 + 180;
    }
    if (y == 0) {
      if (x > 0) g_theta_1 = 180;
      else g_theta_1 = 0;       
    }

    // Calculate value of theta 3

    x_in = (-x / cos(g_theta_1 / MATH_TRANS) - MATH_L2) / MATH_L3;

    if (z_in == 0){ phi = 90; }

    else{ phi = atan(-x_in / z_in)*MATH_TRANS; }

    if (phi > 0) {phi = phi - 180;}  

    sqrt_z_x = sqrt(z_in*z_in + x_in*x_in);

    right_all_2 = -1 * (z_in*z_in + x_in*x_in + MATH_L43*MATH_L43 - 1) / (2 * MATH_L43);
    g_theta_3 = asin(right_all_2 / sqrt_z_x)*MATH_TRANS;
    g_theta_3 = g_theta_3 - phi;

    if (g_theta_3 <0 ) {
      g_theta_3 = 0;
    }

    // Calculate value of theta 2
    g_theta_2 = asin(z_in + MATH_L43*sin(abs(g_theta_3 / MATH_TRANS)))*MATH_TRANS;
  }

  g_theta_1 = abs(g_theta_1);
  g_theta_2 = abs(g_theta_2);

  if (g_theta_3 < 0 ){}
  else{
    if ((calYonly(g_theta_1,g_theta_2, g_theta_3)>y+0.1)||(calYonly(g_theta_1,g_theta_2, g_theta_3)<y-0.1))
    {
      g_theta_2 = 180 - g_theta_2;
    }  
  }

  if(isnan(g_theta_1)||isinf(g_theta_1))
    {g_theta_1 = uarm.readAngle(1);}
  if(isnan(g_theta_2)||isinf(g_theta_2))
    {g_theta_2 = uarm.readAngle(2);}
  if(isnan(g_theta_3)||isinf(g_theta_3)||(g_theta_3<0))
    {g_theta_3 = uarm.readAngle(3);}
}

void uArmClass::calXYZ(double theta_1, double theta_2, double theta_3)
{
  // g_l3_1 = MATH_L3 * cos(theta_2 / MATH_TRANS);
  // g_l4_1 = MATH_L4*cos(theta_3 / MATH_TRANS);
  double l5 = (MATH_L2 + MATH_L3*cos(theta_2 / MATH_TRANS) + MATH_L4*cos(theta_3 / MATH_TRANS));

  g_cal_x = -cos(abs(theta_1 / MATH_TRANS))*l5;
  g_cal_y = -sin(abs(theta_1 / MATH_TRANS))*l5;
  g_cal_z = MATH_L1 + MATH_L3*sin(abs(theta_2 / MATH_TRANS)) - MATH_L4*sin(abs(theta_3 / MATH_TRANS));
}

void uArmClass::calXYZ()
{
  calXYZ(
  uarm.readToAngle(analogRead(SERVO_ROT_ANALOG_PIN),SERVO_ROT_NUM,ABSOLUTE),
  uarm.readToAngle(analogRead(SERVO_LEFT_ANALOG_PIN),SERVO_LEFT_NUM,ABSOLUTE),
  uarm.readToAngle(analogRead(SERVO_RIGHT_ANALOG_PIN),SERVO_RIGHT_NUM,ABSOLUTE));
}

void uArmClass::interpolate(double start_val, double end_val, double (&interp_vals)[INTERP_INTVLS], byte ease_type) {
  double delta = end_val - start_val;
  for (byte f = 0; f < INTERP_INTVLS; f++) {
    switch (ease_type) {
      case INTERP_LINEAR :
        interp_vals[f] = delta * f / INTERP_INTVLS + start_val;
        break;
      case INTERP_EASE_INOUT :
        {
          float t = f / (INTERP_INTVLS / 2.0);
          if (t < 1) {
            interp_vals[f] = delta / 2 * t * t + start_val;
          } else {
            t--;
            interp_vals[f] = -delta / 2 * (t * (t - 2) - 1) + start_val;
          }
        }
        break;
      case INTERP_EASE_IN :
        {
          float t = (float)f / INTERP_INTVLS;
          interp_vals[f] = delta * t * t + start_val;
        }
        break;
      case INTERP_EASE_OUT :
        {
          float t = (float)f / INTERP_INTVLS;
          interp_vals[f] = -delta * t * (t - 2) + start_val;
        }
        break;
      case INTERP_EASE_INOUT_CUBIC :  // this is a compact version of Joey's original cubic ease-in/out
        {
          float t = (float)f / INTERP_INTVLS;
          interp_vals[f] = start_val + (3 * delta) * (t * t) + (-2 * delta) * (t * t * t);
        }
        break;
    }
  }
}

void uArmClass::moveToOpts(double x, double y, double z, double hand_angle, byte relative_flags, double time, byte path_type, byte ease_type) {

  attachAll();

  // find current position using cached servo values
  double current_x;
  double current_y;
  double current_z;
  getCalXYZ(cur_rot, cur_left, cur_right, current_x, current_y, current_z);

  // deal with relative xyz positioning
  byte posn_relative = (relative_flags & F_POSN_RELATIVE) ? 1 : 0;
  x = current_x * posn_relative + x;
  y = current_y * posn_relative + y;
  z = current_z * posn_relative + z;

  // find target angles
  double tgt_rot;
  double tgt_left;
  double tgt_right;
  uarm.getCalAngles(x, y, z, tgt_rot, tgt_left, tgt_right);

  // deal with relative hand orientation
  if (relative_flags & F_HAND_RELATIVE) {
    hand_angle += cur_hand;                                     // rotates a relative amount, ignoring base rotation
  } else if (relative_flags & F_HAND_ROT_REL) {
    hand_angle = hand_angle + cur_hand + (tgt_rot - cur_rot);   // rotates relative to base servo, 0 value keeps an object aligned through movement
  }
  
  if (time > 0) {
    if (path_type == PATH_ANGLES) {
      // we will calculate angle value targets
      double rot_array[INTERP_INTVLS];
      double left_array[INTERP_INTVLS];
      double right_array[INTERP_INTVLS];
      double hand_array[INTERP_INTVLS];

      interpolate(cur_rot, tgt_rot, rot_array, ease_type);
      interpolate(cur_left, tgt_left, left_array, ease_type);
      interpolate(cur_right, tgt_right, right_array, ease_type);
      interpolate(cur_hand, hand_angle, hand_array, ease_type);

      for (byte i = 0; i < INTERP_INTVLS; i++)
      {
        writeAngle(rot_array[i], left_array[i], right_array[i], hand_array[i]);
        delay(time * 1000 / INTERP_INTVLS);
      }
    } else if (path_type == PATH_LINEAR) {
      // we will calculate linear path targets
      double x_array[INTERP_INTVLS];
      double y_array[INTERP_INTVLS];
      double z_array[INTERP_INTVLS];
      double hand_array[INTERP_INTVLS];

      interpolate(current_x, x, x_array, ease_type);
      interpolate(current_y, y, y_array, ease_type);
      interpolate(current_z, z, z_array, ease_type);
      interpolate(cur_hand, hand_angle, hand_array, ease_type);

      for (byte i = 0; i < INTERP_INTVLS; i++)
      {
        double rot, left, right;
        getCalAngles(x_array[i], y_array[i], z_array[i], rot, left, right);
        writeAngle(rot, left, right, hand_array[i]);
        delay(time * 1000 / INTERP_INTVLS);
      }
    }
  }

  // set final target position at end of interpolation or "atOnce"
  writeAngle(tgt_rot, tgt_left, tgt_right, hand_angle);
}

void uArmClass::drawRec(double length_1, double length_2, double time_spend_per_length)
{
  moveTo(length_1,0,0,1,time_spend_per_length);
  moveTo(0,length_2,0,1,time_spend_per_length);
  moveTo(-length_1,0,0,1,time_spend_per_length);
  moveTo(0,-length_2,0,1,time_spend_per_length);
}

void uArmClass::drawCur(double length_1, double length_2, int angle, double time_spend)
{
  uarm.attachAll();
  double l_xp;
  double l_yp;

  calXYZ();
  double current_x = g_cal_x;
  double current_y = g_cal_y;
  double current_z = g_cal_z;
  double interp_arr[INTERP_INTVLS];

  interpolate(0, angle/MATH_TRANS, interp_arr, INTERP_EASE_INOUT_CUBIC); 

  for (byte i = 0; i < INTERP_INTVLS; i++){

    l_xp = length_1 * cos(interp_arr[i]);
    l_yp = length_2 * sin(interp_arr[i]);

    calAngles( l_xp + current_x - length_1 , l_yp+ current_y , current_z);
    uarm.writeAngle(g_theta_1, g_theta_2, g_theta_3,0);

    delay(time_spend*1000/INTERP_INTVLS);
  
  }

}

double uArmClass::calYonly(double theta_1, double theta_2, double theta_3)
{
    //g_l3_1_2 = MATH_L3 * cos(theta_2 / MATH_TRANS);
    //g_l4_1_2 = MATH_L4*cos(theta_3 / MATH_TRANS);
    double l5_2 = (MATH_L2 + MATH_L3*cos(theta_2 / MATH_TRANS) + MATH_L4*cos(theta_3 / MATH_TRANS));

    return -sin(abs(theta_1 / MATH_TRANS))*l5_2;
}

void uArmClass::gripperCatch()
{
  g_servo_hand.attach(SERVO_HAND);
  g_servo_hand.write(HAND_ANGLE_CLOSE);
  digitalWrite(VALVE_EN, LOW); // valve disable
  digitalWrite(PUMP_EN, HIGH); // pump enable
  g_gripper_reset = true;
}

void uArmClass::gripperRelease()
{
  if(g_gripper_reset)
  {
    g_servo_hand.attach(SERVO_HAND);
    g_servo_hand.write(HAND_ANGLE_OPEN);
    digitalWrite(VALVE_EN, HIGH); // valve enable, decompression
    digitalWrite(PUMP_EN, LOW);   // pump disable
    g_gripper_reset= false;
  }
}

/* Action Control */
void uArmClass::pumpOn()
{

   pinMode(PUMP_EN, OUTPUT);
   pinMode(VALVE_EN, OUTPUT);
   digitalWrite(VALVE_EN, LOW);
   digitalWrite(PUMP_EN, HIGH);
}

void uArmClass::pumpOff()
{
   pinMode(PUMP_EN, OUTPUT);
   pinMode(VALVE_EN, OUTPUT);
   digitalWrite(VALVE_EN, HIGH);
   digitalWrite(PUMP_EN, LOW);
   delay(50);
   digitalWrite(VALVE_EN,LOW);
}

/* Written and Added by Josh Heidecker*/
void uArmClass::moveToGround(){
	moveToGround(1);
}
void uArmClass::moveToGround(double rate){
	/*Moves head to ground at rate cm/.01sec*/
	pinMode(STOPPER, INPUT_PULLUP);
	int stopper=1;

	while (true){
		stopper = digitalRead(STOPPER);
		if (stopper == 0) break;
		uarm.moveTo(0, 0, -rate,1,.01);
	}
}

void uArmClass::init(){
	pinMode(STOPPER, INPUT_PULLUP);
}