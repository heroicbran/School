#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <math.h>
#include <fstream>
#include <wiringSerial.h>
#include <sys/socket.h>
#include <stdint.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <string>
using namespace std;

#define DT 0.02         // [s/loop] loop period. 20ms
#define G_GAIN 0.070     // [deg/s/LSB]
#define RECEIVED_BYTES_SIZE 52
#define SENDING_DATA_SIZE 30

//#define MAC_ADDRESS "80:4E:81:20:FC:B8" // NOTE 5
//#define MAC_ADDRESS "BC:F5:AC:F3:97:87" // NEXUS 5
#define MAC_ADDRESS "BC:F5:AC:49:CF:5C" // NEXUS 5 BT

///////////
// posture fall detection variables
///////////

#define NORMAL	0
#define BACK	1
#define FRONT	2
#define LEFT	3
#define RIGHT	4
#define FALSEALARM 5
#define UPRIGHT	6
#define	SITTINGDOWN	7
#define	LYINGDOWN	8
//Default Thresholds here?

 typedef union converter
 {
	float value;
	uint8_t bytes[4];
 } float_byte_converter;

 char datetime[17];
 char datetime_wipcn_display[24];

 void currentDateTime()
{
        /* time_t now = time(0);
        struct tm tstruct;
        tstruct = *localtime(&now);
        strftime(datetime, sizeof(datetime), "%Y-%m-%d.%H:%M:%S", &tstruct); */

		struct timeval tv;
		struct tm *ptm;
		long milliseconds;

		gettimeofday(&tv, NULL);
		ptm = localtime(&tv.tv_sec);

		strftime(datetime_wipcn_display, sizeof(datetime_wipcn_display), "%Y-%m-%d.%H:%M:%S", ptm);

		milliseconds = tv.tv_usec / 1000;

		sprintf(datetime_wipcn_display,"%s.%03ld",datetime_wipcn_display,milliseconds);

		for(int i=0, cnt=0;i<strlen(datetime_wipcn_display);i++)
		{
			if (isalnum(datetime_wipcn_display[i]))
			{
				datetime[cnt++] = datetime_wipcn_display[i];
			}
		}

}

int mode_select()
{
  string mode_txt;
  printf("Please select your mode of operation. (Enter 'help' for a list of commands): ");
  cin >> mode_txt;

  if (mode_txt == "help")
    printf("The modes are 'calibration', 'normal', and 'config'. \n");

  if (mode_txt == "normal" || mode_txt == "n")
  {
    printf("Normal was selected. \n");
    return 1;
  }

  if (mode_txt == "calibration" || mode_txt == "ca")
  {
    printf("Calibration was selected. \n");
    return 0;
  }

  if (mode_txt == "config" || mode_txt == "co")
  {
    printf("Manual was selected. \n");
    return 2;
  }

  if (mode_txt == "quit" | mode_txt == "q")
   return 3;

  if (mode_txt == "display" | mode_txt == "d")
  return 4;

}

void calibrate_thresholds(int &chest_acc_thresh_x, int &chest_acc_thresh_z, int &thigh_acc_thresh_x,
  int &thigh_acc_thresh_z, float &gyro_thresh_x_hi, float &gyro_thresh_x_lo, float &gyro_thresh_z_hi, float &gyro_thresh_z_lo)
{
  printf("Now configuring thresholds manually...\n");
  printf("Set Chest Accelerometer X Threshold [Default: 400 |Current: %d]: ", chest_acc_thresh_x);
  cin >> chest_acc_thresh_x;
  cout <<endl;
  printf("Set Chest Accelerometer Z Threshold [Default: 600 |Current: %d]: ", chest_acc_thresh_z);
  cin >> chest_acc_thresh_z;
  cout <<endl;
  printf("Set Thigh Accelerometer X Threshold [Default: 200 |Current: %d]: ", thigh_acc_thresh_x);
  cin >> thigh_acc_thresh_x;
  cout <<endl;
  printf("Set Thigh Accelerometer Z Threshold [Default: 200 |Current: %d]: ", thigh_acc_thresh_z);
  cin >> thigh_acc_thresh_z;
  cout <<endl;
  printf("Set Gyroscope X Threshold (High) [Default: 1100 |Current: %f]: ", gyro_thresh_x_hi);
  cin >> gyro_thresh_x_hi;
  cout <<endl;
  printf("Set Gyroscope X Threshold (Low) [Default: 300 |Current: %f]: ", gyro_thresh_x_lo);
  cin >> gyro_thresh_x_lo;
  cout <<endl;
  printf("Set Gyroscope Z Threshold (High) [Default: 1100 |Current: %f]: ", gyro_thresh_z_hi);
  cin >> gyro_thresh_z_hi;
  cout <<endl;
  printf("Set Gyroscope Z Threshold (Low) [Default: 300 |Current: %f]: ", gyro_thresh_z_lo);
  cin >> gyro_thresh_z_lo;
  cout <<endl;
  cout <<endl;
  printf("Configuration Complete! \n");
}

void display_thresholds(int chest_acc_thresh_x, int chest_acc_thresh_z, int thigh_acc_thresh_x,
  int thigh_acc_thresh_z, float gyro_thresh_x_hi, float gyro_thresh_x_lo, float gyro_thresh_z_hi, float gyro_thresh_z_lo)
{
  printf("These are the current settings \n");
  printf("=================================\n");
  printf("Chest Accelerometer X Threshold: %d \n", chest_acc_thresh_x);
  printf("Chest Accelerometer Z Threshold: %d \n", chest_acc_thresh_z);
  printf("Thigh Accelerometer X Threshold: %d \n", thigh_acc_thresh_x);
  printf("Thigh Accelerometer Z Threshold: %d \n", thigh_acc_thresh_z);
  printf("Gyroscope X Threshold (High): %f \n", gyro_thresh_x_hi);
  printf("Gyroscope X Threshold (Low): %f \n", gyro_thresh_x_lo);
  printf("Gyroscope Z Threshold (High): %f \n", gyro_thresh_z_hi);
  printf("Gyroscope Z Threshold (Low): %f \n", gyro_thresh_z_lo);

}

//Used to read data from
void receive_data(int fd, uint8_t received_bytes[RECEIVED_BYTES_SIZE], int &current_cnt, int &time_to_fall, int &time_to_up_sit,
                  int &chest_acc_position, int chest_acc_thresh_x, int chest_acc_thresh_z, int &chest_gyro_position,
                  float gyro_thresh_z_hi, float gyro_thresh_z_lo, float gyro_thresh_x_hi, float gyro_thresh_x_lo, uint8_t data_to_send[SENDING_DATA_SIZE],
                  float_byte_converter &myTemperatureConverter, float_byte_converter &myECGConverter, float_byte_converter &myAltitudeConverter,
                  float &current_temperature, int &thigh_position, int thigh_acc_thresh_x, int thigh_acc_thresh_z, int &current_state, int &previous_state,
                  int &current_altitude, int &previous_altitude, int &current_bpm, int &previous_bpm, float &previous_temperature)
{
  int i = 0;
  int received_bytes_length, converted_received_bytes_length;
  int packet_length = 0;
  int cnt_7D = 0;

  if (read(fd, &received_bytes[i], 1) == 1) // if there's data on serial
  {
    if (received_bytes[i] == 0x7E) // checks the beginning of the sequence
    {
      packet_length++;
      // store until you see another 0x7E
      while(1)
      {
        packet_length += read(fd, &received_bytes[++i], 1);
        if (received_bytes[i] == 0x7E)
        {
          // if next packet is already here..
          packet_length--;
          break;
        }
        else if (received_bytes[i] == 0x7D ) // find how many 0x7D in the packet
        {
          cnt_7D++;
        }
      }


      // should have all packets received by here
      // first, convert 0x7D to original data
      uint8_t converted_received_bytes[packet_length-cnt_7D];

      for(received_bytes_length=0,converted_received_bytes_length=0;received_bytes_length<packet_length;received_bytes_length++,converted_received_bytes_length++)
      {
        if( received_bytes[received_bytes_length] == 0x7D )
          converted_received_bytes[converted_received_bytes_length] = received_bytes[++received_bytes_length] ^ 0x20;
        else
          converted_received_bytes[converted_received_bytes_length] = received_bytes[received_bytes_length];
      }

     /* for(int k=0;k<converted_received_bytes_length;k++)
       printf("%02X ", converted_received_bytes[k]);
     printf("\n"); */

     /////////////////////////////////////////////////////////
      ///// done with translating data to proper/original data
      /////////////////////////////////////////////////////////

      // differentiate data to sensors

      uint16_t device_id;

      // chest sensor data
      int16_t chest_acc_x, chest_acc_y, chest_acc_z;

      float chest_gyro_x, chest_gyro_y, chest_gyro_z;
      int16_t chest_gyro_x_int16, chest_gyro_y_int16, chest_gyro_z_int16;

      float chest_EKG;
      float chest_temperature, chest_altimeter;

      // thigh sensor data
      int16_t thigh_acc_x, thigh_acc_y, thigh_acc_z;


      // identify device_id from converted_received_bytes
      device_id = (converted_received_bytes[10] << 8) | converted_received_bytes[11];

      //printf("device : %X%X\t",converted_received_bytes[10],converted_received_bytes[11]);

      current_cnt++;

      if(device_id == 0x57AA) // device from chest
      {
        //printf("Chest Sensor NORMAL\n");

        // put data into the correct position
        // chest accelerometer

        // change to int16_t first
        chest_acc_x = (int16_t) ( converted_received_bytes[15] | (converted_received_bytes[16] << 8)) >> 4;
        chest_acc_y = (int16_t) ( converted_received_bytes[17] | (converted_received_bytes[18] << 8)) >> 4;
        chest_acc_z = (int16_t) ( converted_received_bytes[19] | (converted_received_bytes[20] << 8)) >> 4;

        // chest gyroscope
        chest_gyro_x_int16 = (int16_t) ( converted_received_bytes[21] | (converted_received_bytes[22] << 8));
        chest_gyro_y_int16 = (int16_t) ( converted_received_bytes[23] | (converted_received_bytes[24] << 8));
        chest_gyro_z_int16 = (int16_t) ( converted_received_bytes[25] | (converted_received_bytes[26] << 8));

        chest_gyro_x = (float) chest_gyro_x_int16 * G_GAIN;
        chest_gyro_y = (float) chest_gyro_y_int16 * G_GAIN;
        chest_gyro_z = (float) chest_gyro_z_int16 * G_GAIN;

        chest_gyro_x += chest_gyro_x * DT;
        chest_gyro_y += chest_gyro_y * DT;
        chest_gyro_z += chest_gyro_z * DT;

        //////////////////////////////////
        // posture and fall detection
        //////////////////////////////////

        // first, see accelerometer
        if (chest_acc_y > 990)
          chest_acc_position = NORMAL; // sensor facing front upright
        else
        {
          //if( (chest_acc_x < (-1 * chest_acc_thresh_x)) || (chest_acc_x > chest_acc_thresh_x) || (chest_acc_z < (-1 * chest_acc_thresh_z)) || (chest_acc_z > chest_acc_thresh_z) )
          //{
            if (chest_acc_x < (-1 * chest_acc_thresh_x))
              chest_acc_position = LEFT; // sensor tilted to left
            else if (chest_acc_x > chest_acc_thresh_x)
              chest_acc_position = RIGHT; // sensor tilted to right
            else if (chest_acc_z < (-1 * chest_acc_thresh_z))
              chest_acc_position = FRONT; // sensor tilted to front
            else if (chest_acc_z > chest_acc_thresh_z)
              chest_acc_position = BACK; // sensor tilted to back
          //}
        }
	printf("==============================\n");
        printf("Accel (): %d \n", chest_acc_position);

        // second, see gyroscope
        if ( ((chest_gyro_x < 20) && (chest_gyro_x > -20)) || ((chest_gyro_z < 20) && (chest_gyro_z > -20)) )
          chest_gyro_position = NORMAL;
        else
        {
          if ( (chest_gyro_z > (-1 * gyro_thresh_z_hi) ) && (chest_gyro_z < (-1 * gyro_thresh_z_lo) ))
            chest_gyro_position = LEFT;
          else if( (chest_gyro_z > gyro_thresh_z_lo ) && (chest_gyro_z < gyro_thresh_z_hi ) )
            chest_gyro_position = RIGHT;
          else if( (chest_gyro_x > gyro_thresh_x_lo) && (chest_gyro_x < gyro_thresh_x_hi ))
            chest_gyro_position = FRONT;
          else if( (chest_gyro_x > (-1 * gyro_thresh_x_hi)) && (chest_gyro_x < (-1 * gyro_thresh_x_lo) ))
            chest_gyro_position = BACK;
        }
        printf("Gyro (): %d \n", chest_gyro_position);
	printf("==============================\n");

        printf("CHEST_ACC_X : %d\tCHEST_ACC_Y : %d\tCHEST_ACC_Z : %d\n",chest_acc_x,chest_acc_y,chest_acc_z);
        //printf("THIGH_ACC_X : %d\tTHIGH_ACC_Y : %d\tTHIGH_ACC_Z : %d\n",thigh_acc_x,thigh_acc_y,thigh_acc_z);
        //printf("GYRO_X : %.2f\tGYRO_Y : %.2f\tGYRO_Z : %.2f\n", chest_gyro_x, chest_gyro_y, chest_gyro_z);
        //printf("CHEST_GYRO_POSITION : %d\n\n", chest_gyro_position);

        // chest temperature

        data_to_send[5] = myTemperatureConverter.bytes[0] = converted_received_bytes[27];
        data_to_send[6] = myTemperatureConverter.bytes[1] = converted_received_bytes[28];
        data_to_send[7] = myTemperatureConverter.bytes[2] = converted_received_bytes[29];
        data_to_send[8] = myTemperatureConverter.bytes[3] = converted_received_bytes[30];
        current_temperature = myTemperatureConverter.value;

        //printf("\nTemperature : %.2fC\n", myTemperatureConverter.value);


        // altimeter
        data_to_send[9] = myAltitudeConverter.bytes[0] = converted_received_bytes[31];
        data_to_send[10] = myAltitudeConverter.bytes[1] = converted_received_bytes[32];
        data_to_send[11] = myAltitudeConverter.bytes[2] = converted_received_bytes[33];
        data_to_send[12] = myAltitudeConverter.bytes[3] = converted_received_bytes[34];

        current_altitude = (int) myAltitudeConverter.value;

        //printf("Altitude : %dm\n\n", myAltitudeConverter.value);


        /////////////////////
        /// fall detection posture starts here
        ////////////////////

        if ( (thigh_position == NORMAL) && (chest_acc_position == NORMAL) && (chest_gyro_position == NORMAL) )
        {
          time_to_up_sit = current_cnt;

          //current_state = UPRIGHT;

          if ( (time_to_up_sit - time_to_fall) < 25 )
          {
            current_state = FALSEALARM;
          }
          else
          {
            current_state = UPRIGHT;
          }
        }
        else if ( (thigh_position == BACK ) && (chest_acc_position == NORMAL) )
        {
          //current_state = SITTINGDOWN;

          time_to_up_sit = current_cnt;
          if ( (time_to_up_sit - time_to_fall) < 25 )
          {
            current_state = FALSEALARM;
          }
          else
          {

            current_state = SITTINGDOWN;
          }

        }
        else
        {
          if ( (current_cnt - time_to_up_sit) < 10 )
          {
            time_to_fall = current_cnt;

               //if ( ((chest_acc_position == LEFT) && (thigh_position == LEFT)) || ((thigh_position == LEFT) && (chest_gyro_position == LEFT))|| ((chest_acc_position == LEFT) && (chest_gyro_position == LEFT)))
             if ( ((chest_acc_position == LEFT) || (thigh_position == LEFT)) && (chest_gyro_position == LEFT) )
             {
               current_state = LEFT;
             }
             //else if ( ((chest_acc_position == RIGHT) && (thigh_position == RIGHT)) || ((thigh_position == RIGHT) && (chest_gyro_position == RIGHT))|| ((chest_acc_position == RIGHT) && (chest_gyro_position == RIGHT)))
             else if ( ((chest_acc_position == RIGHT) || (thigh_position == RIGHT)) && (chest_gyro_position == RIGHT) )
             {
               current_state = RIGHT;
             }
             //else if ( ((chest_acc_position == FRONT) && (thigh_position == FRONT)) || ((thigh_position == FRONT) && (chest_gyro_position == FRONT))|| ((chest_acc_position == FRONT) && (chest_gyro_position == FRONT)))
             else if ( ((chest_acc_position == FRONT) || (thigh_position == FRONT)) && (chest_gyro_position == FRONT) )
             {
               current_state = FRONT;
             }
             //else if ( ((chest_acc_position == BACK) && (thigh_position == BACK)) || ((thigh_position == BACK) && (chest_gyro_position == BACK))|| ((chest_acc_position == BACK) && (chest_gyro_position == BACK)))
             else if ( ((chest_acc_position == BACK) || (thigh_position == BACK)) && (chest_gyro_position == BACK) )
             {
               current_state = BACK;
             }


          }
          else
          {
            current_state = LYINGDOWN;
          }
        }
      }
      else if (device_id == 0x57E6)
      {
        //printf("Thigh sensor\n");

        thigh_acc_x = (int16_t) ( converted_received_bytes[15] | converted_received_bytes[16] << 8 );
        thigh_acc_y = (int16_t) ( converted_received_bytes[17] | converted_received_bytes[18] << 8 );
        thigh_acc_z = (int16_t) ( converted_received_bytes[19] | converted_received_bytes[20] << 8 );

        //printf("ACC_X : %d\tACC_Y : %d\tACC_Z : %d\n\n", thigh_acc_x, thigh_acc_y, thigh_acc_z);

        if (thigh_acc_y < -200 )
        {
          // upright position in y direction
          thigh_position = NORMAL ; // sensor is facing front, upright
        }
        else
        {
          if( (thigh_acc_x < (-1 * thigh_acc_thresh_x)) || (thigh_acc_x > thigh_acc_thresh_x) || (thigh_acc_z < (-1 * thigh_acc_thresh_z)) || (thigh_acc_z > thigh_acc_thresh_z) )
          {
            if (thigh_acc_x < (-1 * thigh_acc_thresh_x))
              thigh_position = RIGHT; // sensor tilted to right (east)
            else if (thigh_acc_x > thigh_acc_thresh_x)
              thigh_position = LEFT; // sensor tilted to left (west)
            else if (thigh_acc_z < (-1 * thigh_acc_thresh_z))
              thigh_position = FRONT; // sensor tilted to front (north)
            else if (thigh_acc_z > thigh_acc_thresh_z)
              thigh_position = BACK; // sensor titled to back (south)
          }
        }

      }

      /*else if (device_id == 0xDB85)
      {
        //printf("ECG chest sensor\n");

        data_to_send[1] = myECGConverter.bytes[0] = converted_received_bytes[15];
        data_to_send[2] = myECGConverter.bytes[1] = converted_received_bytes[16];
        data_to_send[3] = myECGConverter.bytes[2] = converted_received_bytes[17];
        data_to_send[4] = myECGConverter.bytes[3] = converted_received_bytes[18];

        current_bpm = (int) myECGConverter.value;
      }*/

      // if any sensor data has been changed, then do the following

      if ( (current_state != previous_state) || (current_altitude != previous_altitude) || (current_bpm != previous_bpm) || (current_temperature != previous_temperature) )
      {
          //printf("current_cnt : %d\ttime_to_up : %d\ttime_to_fall : %d\t",current_cnt,time_to_up_sit,time_to_fall);
          if (current_state != previous_state)
          {
            previous_state = current_state;
            switch(current_state)
            {
              case LEFT:
               // assign current posture/fall direction to data_to_send
               data_to_send[0] = 0x03;
               break;

              case RIGHT:
               // assign current posture/fall direction to data_to_send
               data_to_send[0] = 0x04;

               break;

              case FRONT:
               // assign current posture/fall direction to data_to_send
               data_to_send[0] = 0x05;

               break;

              case BACK:
               // assign current posture/fall direction to data_to_send
               data_to_send[0] = 0x06;

               break;

              case UPRIGHT:
               // assign current posture/fall direction to data_to_send
               data_to_send[0] = 0x00;

               break;

              case SITTINGDOWN:
               // assign current posture/fall direction to data_to_send
               data_to_send[0] = 0x01;

               break;

              case LYINGDOWN:
               // assign current posture/fall direction to data_to_send
               data_to_send[0] = 0x02;

               break;

             case FALSEALARM:
               // assign current posture/fall direction to data_to_send
               data_to_send[0] = 0x07;

               break;

             default :
               break;

            }
          }
          else if ( current_altitude != previous_altitude )
            previous_altitude = current_altitude;
          else if ( current_bpm != previous_bpm )
            previous_bpm = current_bpm;
          else if ( current_temperature != previous_temperature )
            previous_temperature = current_temperature;

          // need to pack these into a meaningful data
          // sending to Android only if any sensor data is changed
          // 1. current_state : 0x00 (upright), 0x01 (sittingdown), 0x02 (lyingdown), 0x03 (fall_left), 0x04 (fall_right), 0x05 (fall_front), 0x06 (fall_back), 0x07 (falsealarm)
          // 2. BPM : 4 bytes from the original BPM data
          // 3. temperature : 4 bytes from the original BPM data (Celcius)
          // 4. altitude : 4 bytes from the original altitude data (meters)
          // 5. date : 17 bytes including format of YYYYMMDDHHmmSSsss

          currentDateTime();
          //printf("%s\t",datetime_wipcn_display);


          /*if(data_to_send[0] == 0x00)
            printf("UPRIGHT\t\t");
          else if (data_to_send[0] == 0x01)
            printf("SITTING DOWN\t\t");
          else if (data_to_send[0] == 0x02)
            printf("LYING DOWN\t\t");
          else if (data_to_send[0] == 0x03)
            printf("FALL LEFT\t\t");
          else if (data_to_send[0] == 0x04)
            printf("FALL RIGHT\t\t");
          else if (data_to_send[0] == 0x05)
            printf("FALL FRONT\t\t");
          else if (data_to_send[0] == 0x06)
            printf("FALL BACK\t\t");
          else if (data_to_send[0] == 0x07)
            printf("FALSE ALARM\t\t");

          printf("BPM: %d\t\t", current_bpm);
          printf("Temp.: %.2f C\t\t", current_temperature);
          printf("Altitude: %d m\n", current_altitude);*/

          data_to_send[13] = datetime[0]; // YYYY (4bytes)
          data_to_send[14] = datetime[1];
          data_to_send[15] = datetime[2];
          data_to_send[16] = datetime[3];
          data_to_send[17] = datetime[4]; // MM (2bytes)
          data_to_send[18] = datetime[5];
          data_to_send[19] = datetime[6]; // DD (2bytes)
          data_to_send[20] = datetime[7];
          data_to_send[21] = datetime[8]; // HH (2bytes)
          data_to_send[22] = datetime[9];
          data_to_send[23] = datetime[10]; // mm (2bytes)
          data_to_send[24] = datetime[11];
          data_to_send[25] = datetime[12]; // SS (2bytes)
          data_to_send[26] = datetime[13];
          data_to_send[27] = datetime[14]; // sss (3bytes)
          data_to_send[28] = datetime[15];
          data_to_send[29] = datetime[16];

          // sending to Android since at least one of the sensor values was changed
          /*bluetooth_status = write(blueooth_socket, data_to_send, SENDING_DATA_SIZE);
          if (bluetooth_status == 0)
            printf("Data could not be sent, bluetooth_status : %d\n", bluetooth_status);
          */
        }
    }
  }


}


void calibrate_data_chest(int fd, uint8_t received_bytes[RECEIVED_BYTES_SIZE], int &current_cnt,int &chest_acc_thresh_x, int &chest_acc_thresh_z,
                  float &gyro_thresh_z_hi, float &gyro_thresh_z_lo, float &gyro_thresh_x_hi, float &gyro_thresh_x_lo, int thigh_position, int &thigh_acc_thresh_x, int &thigh_acc_thresh_z)
{
  int i = 0;
  int received_bytes_length, converted_received_bytes_length;
  int packet_length = 0;
  int cnt_7D = 0;
  int first_measure; //Used to detect if val should be updated
  int dir; // 0 = front, 1 = back, 2 = left, 3 = right
  int duration;

  chest_acc_thresh_z = 1500;
  //chest_acc_thresh_x = 0;
  //thigh_acc_thresh_x = 0;
  //thigh_acc_thresh_z = 0;
  gyro_thresh_x_hi = 10;
  gyro_thresh_x_lo = 1500;
  //gyro_thresh_z_hi = 0;
  //gyro_thresh_z_lo = 0;

  for (int dir =0; dir != 2; dir ++)
  {
    first_measure = 0;

    switch(dir)
    {
      case 0:
        printf("Lean forward \n");
        break;

    /*  case 1:
        printf("Lean back \n");
        break;

      case 2:
        printf("Lean left \n");
        break;*/

      case 1:
        printf("Lean right \n");
        break;

      default:
	     printf("\n");

    }
    //sleep(5);

    for (duration = 2500; duration > 0; duration--)
    {
      i = 0;
      packet_length = 0;
      cnt_7D = 0;

      if (read(fd, &received_bytes[i], 1) == 1) // if there's data on serial
      {
        if (received_bytes[i] == 0x7E) // checks the beginning of the sequence
        {
          packet_length++;
          // store until you see another 0x7E
          while(1)
          {
            packet_length += read(fd, &received_bytes[++i], 1);
            if (received_bytes[i] == 0x7E)
            {
              // if next packet is already here..
              packet_length--;
              break;
            }
            else if (received_bytes[i] == 0x7D ) // find how many 0x7D in the packet
            {
              cnt_7D++;
            }
          }

          // should have all packets received by here
          // first, convert 0x7D to original data
          uint8_t converted_received_bytes[packet_length-cnt_7D];

          for(received_bytes_length=0,converted_received_bytes_length=0;received_bytes_length<packet_length;received_bytes_length++,converted_received_bytes_length++)
          {
            if( received_bytes[received_bytes_length] == 0x7D )
              converted_received_bytes[converted_received_bytes_length] = received_bytes[++received_bytes_length] ^ 0x20;
            else
              converted_received_bytes[converted_received_bytes_length] = received_bytes[received_bytes_length];
          }


          // differentiate data to sensors

          uint16_t device_id;

          // chest sensor data
          int16_t chest_acc_x, chest_acc_y, chest_acc_z;

          float chest_gyro_x, chest_gyro_y, chest_gyro_z;
          int16_t chest_gyro_x_int16, chest_gyro_y_int16, chest_gyro_z_int16;

          float chest_EKG;
          float chest_temperature, chest_altimeter;

          // thigh sensor data
          int16_t thigh_acc_x, thigh_acc_y, thigh_acc_z;


          // identify device_id from converted_received_bytes
          device_id = (converted_received_bytes[10] << 8) | converted_received_bytes[11];

          //printf("device : %X%X\t",converted_received_bytes[10],converted_received_bytes[11]);

          current_cnt++;

          if(device_id == 0x57AA) // device from chest
          {
            //printf("Chest Sensor\n");

            // put data into the correct position
            // chest accelerometer

            // change to int16_t first
            switch(dir)
            {
              case 0:  //Calibrate forward and back
                chest_acc_z = (int16_t) ( converted_received_bytes[19] | (converted_received_bytes[20] << 8)) >> 4;
                if (abs(chest_acc_z) < abs(chest_acc_thresh_z) && abs(chest_acc_z) > 250)
                {
                  chest_acc_thresh_z = abs(chest_acc_z);
                  printf("Updated chest_acc_thresh_z to %d \n", chest_acc_thresh_z);
                }


                chest_gyro_x_int16 = (int16_t) ( converted_received_bytes[21] | (converted_received_bytes[22] << 8));
                chest_gyro_x = (float) chest_gyro_x_int16 * G_GAIN;
                chest_gyro_x += chest_gyro_x * DT;

                if (chest_gyro_x > gyro_thresh_x_hi && chest_gyro_x > 20)
                {
                  gyro_thresh_x_hi = chest_gyro_x;
                  printf("Updated gyro_thresh_x_hi to %f \n", gyro_thresh_x_hi);
                }
                if (chest_gyro_x < gyro_thresh_x_lo && chest_gyro_x > 20)
                {
                  gyro_thresh_x_lo = chest_gyro_x;
                  printf("Updated gyro_thresh_x_lo to %f \n", gyro_thresh_x_lo);
                }

                break;

              /*case 1:
                if (first_measure == 0)
                {
                  chest_acc_thresh_z = (int16_t) ( converted_received_bytes[19] | (converted_received_bytes[20] << 8)) >> 4;
                //chest_gyro_x_int16 = (int16_t) ( converted_received_bytes[21] | (converted_received_bytes[22] << 8));
                //chest_gyro_x = (float) chest_gyro_x_int16 * G_GAIN;
                //chest_gyro_x += chest_gyro_x * DT;
                  first_measure = 1;
                }
                else if (-1 * (int16_t) ( converted_received_bytes[19] | (converted_received_bytes[20] << 8)) >> 4 < chest_acc_thresh_z)
                {
                  chest_acc_thresh_z = (int16_t) ( converted_received_bytes[19] | (converted_received_bytes[20] << 8)) >> 4;
                //chest_gyro_x_int16 = (int16_t) ( converted_received_bytes[21] | (converted_received_bytes[22] << 8));
                //chest_gyro_x = (float) chest_gyro_x_int16 * G_GAIN;
                //chest_gyro_x += chest_gyro_x * DT;
                }
                break;

              case 2:
                if (first_measure == 0)
                {
                  chest_acc_thresh_x = (int16_t) ( converted_received_bytes[15] | (converted_received_bytes[16] << 8)) >> 4;
                  //chest_gyro_z_int16 = (int16_t) ( converted_received_bytes[25] | (converted_received_bytes[26] << 8));
                  //chest_gyro_z = (float) chest_gyro_z_int16 * G_GAIN;
                  //chest_gyro_z += chest_gyro_z * DT;
                  first_measure = 1;
                }
                else if (-1 * (int16_t) ( converted_received_bytes[15] | (converted_received_bytes[16] << 8)) >> 4 > chest_acc_thresh_x)
                {
                  chest_acc_thresh_x = (int16_t) ( converted_received_bytes[15] | (converted_received_bytes[16] << 8)) >> 4;
                  //chest_gyro_z_int16 = (int16_t) ( converted_received_bytes[25] | (converted_received_bytes[26] << 8));
                  //chest_gyro_z = (float) chest_gyro_z_int16 * G_GAIN;
                  //chest_gyro_z += chest_gyro_z * DT;
                }
                break;*/

              case 1: //Calibrate left and right
              chest_acc_x = (int16_t) ( converted_received_bytes[15] | (converted_received_bytes[16] << 8)) >> 4;
              if (abs(chest_acc_x) < abs(chest_acc_thresh_x) && abs(chest_acc_x) > 250)
              {
                chest_acc_thresh_x = abs(chest_acc_x);
                printf("Updated chest_acc_thresh_x to %d \n", chest_acc_thresh_x);
              }


              chest_gyro_z_int16 = (int16_t) ( converted_received_bytes[25] | (converted_received_bytes[26] << 8));
              chest_gyro_z = (float) chest_gyro_z_int16 * G_GAIN;
              chest_gyro_z += chest_gyro_z * DT;

              if (chest_gyro_z > gyro_thresh_z_hi && chest_gyro_z > 20)
              {
                gyro_thresh_z_hi = chest_gyro_z;
                printf("Updated gyro_thresh_z_hi to %f \n", gyro_thresh_z_hi);
              }
              if (chest_gyro_z < gyro_thresh_z_lo && chest_gyro_z > 20)
              {
                gyro_thresh_z_lo = chest_gyro_z;
                printf("Updated gyro_thresh_z_lo to %f \n", gyro_thresh_z_lo);
              }

              break;

              default:
		            printf("\n");

            }




          }


        }
      }


    }


  }
  current_cnt = 0;

}






//Just a place to store bluetooth code.
void do_bluetooth()
{
  /*struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char dest[18] = MAC_ADDRESS;

    char buf[1024] = { 0 };
    char str[1024] = { 0 };
    int bluetooth_status;
	int blueooth_socket;
    uint8_t rfcomm_channel = 0;
    socklen_t opt = sizeof(rem_addr);

    printf("connection to Bluetooth device %s\n",dest);


    //Bluetooth starts here
    rfcomm_channel = register_service();

    printf("rfcomm_channel in main is %d\n",rfcomm_channel);
    blueooth_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_channel = rfcomm_channel;
    str2ba(dest, &loc_addr.rc_bdaddr);

    // connect to Android, using Rfcomm secure protocol
    bluetooth_status = connect(blueooth_socket, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

	// see if we connected successfully to the smartphone
	if (bluetooth_status == 0)
		printf("successfully connected\n");
	else
	{
		printf("cannot connect to %s. return value : %d\n",dest, bluetooth_status);
		close(blueooth_socket);
		return 0;
	}

	*/

}

// for Bluetooth service
uint8_t register_service();



int main()
{
	// variables
	struct  timeval tvBegin, tvEnd,tvDiff;
	uint8_t buff;
	int fd;
	uint8_t received_bytes[RECEIVED_BYTES_SIZE];
	uint8_t data_to_send[SENDING_DATA_SIZE];
	float_byte_converter myTemperatureConverter, myECGConverter, myAltitudeConverter;

  //Variables for Program management
  int mode = 0; //0 for Calibration mode, 1 for normal operation mode, 2 for Manual Settings
  //Accelerometer Threshholds
  int chest_acc_thresh_x = 400;
  int chest_acc_thresh_z = 600;
  int thigh_acc_thresh_x = 200;
  int thigh_acc_thresh_z = 200;

  //Gyroscope Thresholds
  float gyro_thresh_x_hi= 1100;
  float gyro_thresh_x_lo= 300;
  float gyro_thresh_z_hi = 1100;
  float gyro_thresh_z_lo = 300;


	// variables for fall detection and posture recognition
	int thigh_position, chest_acc_position, chest_gyro_position;
	int time_to_up_sit, time_to_fall;
	bool fall_occured = false;
	int current_cnt = 0;

	int current_state = 0;
	int previous_state=99;

	float current_temperature;
	float previous_temperature = -100;

	int current_bpm = 0;
	int previous_bpm = 1992269212;

	int current_altitude = 0;
	int previous_altitude = 0;

	// wyi3 : open serial for ZigBee Explorer
   if ((fd = serialOpen ("/dev/ttyUSB0", 57600)) < 0)
    {
  	fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
  	return 1 ;
    }
    else
  	 printf("successfully opened serial connection\n");

  //MODE SELECTION
  while (mode != 3)
  {
     mode = mode_select();

     if (mode == 0)
     {
       printf("Now calibrating fall detectors...\n");
       cin.ignore();

       calibrate_data_chest(fd, received_bytes, current_cnt, chest_acc_thresh_x, chest_acc_thresh_z,
                           gyro_thresh_z_hi, gyro_thresh_z_lo, gyro_thresh_x_hi, gyro_thresh_x_lo,
                           thigh_position, thigh_acc_thresh_x, thigh_acc_thresh_z);

       /*receive_data_thigh(fd, received_bytes, current_cnt, time_to_fall, time_to_up_sit,
                           chest_acc_position, chest_acc_thresh_x, chest_acc_thresh_z, chest_gyro_position,
                           gyro_thresh_z_hi, gyro_thresh_z_lo, gyro_thresh_x_hi, gyro_thresh_x_lo, data_to_send,
                           myTemperatureConverter, myECGConverter, myAltitudeConverter, current_temperature,
                           thigh_position, thigh_acc_thresh_x, thigh_acc_thresh_z, current_state, previous_state, current_altitude, previous_altitude, current_bpm, previous_bpm,
                           previous_temperature);*/


     }

     if (mode == 2)
     {
       calibrate_thresholds(chest_acc_thresh_x, chest_acc_thresh_z, thigh_acc_thresh_x,
         thigh_acc_thresh_z, gyro_thresh_x_hi, gyro_thresh_x_lo, gyro_thresh_z_hi, gyro_thresh_z_lo);
     }

     if (mode == 4)
     {
       display_thresholds(chest_acc_thresh_x, chest_acc_thresh_z, thigh_acc_thresh_x,
         thigh_acc_thresh_z, gyro_thresh_x_hi, gyro_thresh_x_lo, gyro_thresh_z_hi, gyro_thresh_z_lo);
     }
  /// Part for Bluetooth
    do_bluetooth();  //Currently just a stub. See function definition.

	// wyi3 : data retrieval and communication starts in this loop
    while(mode == 1)
    {
       /**********Read the data from ttyUSB0*********/
       receive_data(fd, received_bytes, current_cnt, time_to_fall, time_to_up_sit,
                         chest_acc_position, chest_acc_thresh_x, chest_acc_thresh_z, chest_gyro_position,
                         gyro_thresh_z_hi, gyro_thresh_z_lo, gyro_thresh_x_hi, gyro_thresh_x_lo, data_to_send,
                         myTemperatureConverter, myECGConverter, myAltitudeConverter, current_temperature,
                         thigh_position, thigh_acc_thresh_x, thigh_acc_thresh_z, current_state, previous_state, current_altitude, previous_altitude, current_bpm, previous_bpm,
                         previous_temperature);
    }
 }

  return 0;
}

// for Bluetooth service
uint8_t register_service()
{

	bdaddr_t tmp = { };

    uint8_t svc_uuid_int[] = { 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
    //uint32_t svc_uuid_int[] = { 0x00001101,0x00001000,0x80000080,0x5F9B34FB };
    uuid_t svc_uuid;
    int err;
    bdaddr_t target;
    sdp_list_t *response_list = NULL, *search_list, *attrid_list;
    sdp_session_t *session = 0;

	uint8_t rfcomm_channel = 0;

    str2ba( MAC_ADDRESS, &target );

    // connect to the SDP server running on the remote machine
    session = sdp_connect( &tmp, &target, SDP_RETRY_IF_BUSY );

    // specify the UUID of the application we're searching for
    sdp_uuid128_create( &svc_uuid, &svc_uuid_int );
    search_list = sdp_list_append( NULL, &svc_uuid );

    // specify that we want a list of all the matching applications' attributes
    uint32_t range = 0x0000ffff;
    attrid_list = sdp_list_append( NULL, &range );

    // get a list of service records that have UUID 0xabcd
    err = sdp_service_search_attr_req( session, search_list,
            SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
    sdp_list_t *r = response_list;

    // go through each of the service records
    for (; r; r = r->next ) {
        sdp_record_t *rec = (sdp_record_t*) r->data;
        sdp_list_t *proto_list;

        // get a list of the protocol sequences
        if( sdp_get_access_protos( rec, &proto_list ) == 0 ) {
        sdp_list_t *p = proto_list;

        // go through each protocol sequence
        for( ; p ; p = p->next ) {
            sdp_list_t *pds = (sdp_list_t*)p->data;

            // go through each protocol list of the protocol sequence
            for( ; pds ; pds = pds->next ) {

                // check the protocol attributes
                sdp_data_t *d = (sdp_data_t*)pds->data;
                int proto = 0;
                for( ; d; d = d->next ) {
                    switch( d->dtd ) {
                        case SDP_UUID16:
                        case SDP_UUID32:
                        case SDP_UUID128:
                            proto = sdp_uuid_to_proto( &d->val.uuid );
                            break;
                        case SDP_UINT8:
                            if( proto == RFCOMM_UUID ) {
                                printf("rfcomm channel: %d\n",d->val.int8);
				rfcomm_channel = d->val.int8;
                            }
                            break;
                    }
                }
            }
            sdp_list_free( (sdp_list_t*)p->data, 0 );
        }
        sdp_list_free( proto_list, 0 );

        }

        //printf("found service record 0x%x\n", rec->handle);
        sdp_record_free( rec );
    }

    return rfcomm_channel;
}
