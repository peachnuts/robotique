#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions 	   */ 
#include <errno.h>   /* ERROR Number Definitions           */
#include "std_msgs/String.h"

#include <sstream>
/*
double get_lidar(){


		char read_buffer[32];   //Buffer to store the data received           
		
		int  bytes_read = 0;    // Number of bytes read by the read() system call 
 		int i = 0;
		//int tempo;
		//while(1){
		bytes_read = read(fd,&read_buffer,32); // Read the data               

		for(i=0;i<bytes_read;i++)	 //printing only the received characters
		    printf("fonction get_lidar: %c\n",read_buffer[i]);
		return atoi(read_buffer);

}

*/

int main(int argc, char** argv){
  ros::init(argc, argv, "fake_laser_scan_publisher");// ros::init(argc, argv, "talker");

  ros::NodeHandle n;//ok
  ros::Publisher Scan_pub = n.advertise<sensor_msgs::LaserScan>("fakeScan", 50);
  ros::Publisher chatter_pub = n.advertise<std_msgs::String>("chatter", 1000);


  unsigned int num_readings = 200;
  double laser_frequency = 400;
  double ranges[num_readings];
  double intensities[num_readings];

  int count = 0;
//------------------------------------------------------------------------------------------------------

		int fd;/*File Descriptor*/

        fd = open("/dev/ttyUSB0",O_RDWR | O_NOCTTY);	/* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
		struct termios SerialPortSettings;	/* Create the structure                          */

		tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */

		/* Setting the Baud rate */
		cfsetispeed(&SerialPortSettings,B115200); /* Set Read  Speed as 9600                       */
		cfsetospeed(&SerialPortSettings,B115200); /* Set Write Speed as 9600                       */

		/* 8N1 Mode */
		SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
		SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
		SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
		SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */
		
		SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
		SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */ 
		
		
		SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
		SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */

		SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/
		
		/* Setting Time outs */
		SerialPortSettings.c_cc[VMIN] = 10; /* Read at least 10 characters */
		SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */



	        /*------------------------------- Read data from serial port -----------------------------*/

		tcflush(fd, TCIFLUSH);   /* Discards old data in the rx buffer            */


//---------------------------------------------------------------------------------------------------
		char read_buffer[32];   /* Buffer to store the data received              */	
		int  bytes_read = 0;    /* Number of bytes read by the read() system call */
 		int i = 0;
		//int tempo;
		//while(1){
		

		
//-------------------------------------------------------------------------------------------------
  ros::Rate r(400);//  ros::Rate loop_rate(10);
  while(n.ok()){
//-----------------------------------------------------------------------
 std_msgs::String msg;
 std::stringstream ss;
//-------------------------------------------------------------------
bytes_read = read(fd,&read_buffer,32); /* Read the data  
                 */
//for(i=0;i<bytes_read;i++)	 /*printing only the received characters*/
	//	    printf("fonction get_lidar: %c\n",read_buffer[i]);

		double distance=atoi(read_buffer);
    //generate some fake data for our laser scan
    for(unsigned int i = 0; i < num_readings; ++i){
      
	  ranges[i] =distance/100.0;
      intensities[i] = 100 + count;
    }
    ros::Time scan_time = ros::Time::now();

    //populate the LaserScan message
    sensor_msgs::LaserScan scan;
    scan.header.stamp = scan_time;
    scan.header.frame_id = "fake_laser_frame";
    scan.angle_min = -3.14;
    scan.angle_max = 3.14;
    scan.angle_increment = 6.28 / num_readings;
   // scan.time_increment = (1 / laser_frequency) / (num_readings);
	scan.time_increment = 1 / laser_frequency;
    scan.range_min = 0.0;
    scan.range_max = 100.0;

    scan.scan_time = (1/2);

    scan.ranges.resize(num_readings);
    scan.intensities.resize(num_readings);
    for(unsigned int i = 0; i < num_readings; ++i){
      scan.ranges[i] = ranges[i];
      scan.intensities[i] = intensities[i];
    }
    ss <<count<< "distance mesure " << distance;
    msg.data = ss.str();
    ROS_INFO("%s", msg.data.c_str());
    chatter_pub.publish(msg);

    ros::spinOnce();

    Scan_pub.publish(scan);
    ++count;
    r.sleep();//


  }
}
