/**
 * @file offb_node.cpp
 * @brief offboard example node, written with mavros version 0.14.2, px4 flight
 * stack and tested in Gazebo SITL
 */

#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <gazebo_msgs/ContactsState.h>
#include <gazebo_msgs/ContactState.h>
#include <stdlib.h>
#include <iarc_create/remote.h>

# define M_PI           3.14159265358979323846

// Global Variables
bool runSigState = true;
float robotspeed = 0.33;
float sec2reserve = 2.456;
float sec2autoreserve = 20.0;
float sec2noise = 5.0;
float noiseduration = 0.85;


// Time between trajectory noise injections
ros::Timer noisetimer;
// Time between auto-reverse
ros::Timer autoreversetimer;


ros::Publisher  * p_cmd_vel_pub;

// Global Fonctions
void bumperCallBack(const gazebo_msgs::ContactsState msg);
void remoteCallBack(const iarc_create::remote msg);
void autoreversetimerCallback(const ros::TimerEvent&);
void noisetimerCallback(const ros::TimerEvent&);

void sendvel(float x_rate, float yaw_rate);

int main(int argc, char **argv)
{
    ros::init(argc, argv, "sheep");

    ros::NodeHandle nh;

    ros::Subscriber bumber_sub = nh.subscribe<gazebo_msgs::ContactsState>("create2_bumper", 10, bumperCallBack);
    ros::Subscriber remote_sub = nh.subscribe<iarc_create::remote>("remote", 10, remoteCallBack);
    ros::Publisher cmd_vel_pub = nh.advertise<geometry_msgs::Twist>("create2/cmd_vel", 5);

    p_cmd_vel_pub = &cmd_vel_pub;

    //Sets up the random number generator
     srand(time(0));


    // Time between auto-reverse
    autoreversetimer = nh.createTimer(ros::Duration(sec2autoreserve), autoreversetimerCallback, false, false);
    // Time between trajectory noise injections
    noisetimer = nh.createTimer(ros::Duration(sec2noise), noisetimerCallback, false, false);

    // wait for FCU connection
    if(ros::ok()){

      ROS_INFO("Sheep Started!");

    for(int i = 10; i>0; i--){
      sendvel(0.0, 0.0);
     }
    }

    ros::spin();
    
    return 0;
}


void sendvel(float x_rate, float yaw_rate){

  geometry_msgs::Twist vel;
  vel.linear.x = x_rate;
  vel.linear.y = 0.0;
  vel.linear.z = 0.0;
  vel.angular.x = 0.0;
  vel.angular.y = 0.0;
  vel.angular.z = yaw_rate;

  p_cmd_vel_pub->publish(vel);

  ros::Duration(1/200).sleep();

}

void bumperCallBack(const gazebo_msgs::ContactsState msg)
{

  if(msg.states.size() >= 1)// && reverse_flag == false)
  {
     ROS_INFO("We bumped something; Revert!");

     sendvel(0.0, M_PI/sec2reserve);

     ros::Duration(sec2reserve).sleep();

     sendvel(0.0, 0.0);
     ros::Duration(0.015).sleep();
     sendvel(robotspeed, 0.0);


     ROS_INFO("Bumped; Full turn!");
     //reversetimer = p_nh->createTimer(ros::Duration(sec2reserve), reversetimerCallback, true);
     //reverse_flag = 1;
  }
}

void remoteCallBack(const iarc_create::remote msg)
{

  ROS_INFO("Remote Call!");
  if(msg.runSigState){

    ROS_INFO("Got the green light!");

    sendvel(robotspeed, 0.0);

    noisetimer.setPeriod(ros::Duration(sec2noise),true);
    autoreversetimer.setPeriod(ros::Duration(sec2autoreserve),true);
    autoreversetimer.start();
    noisetimer.start();

  }else{
    ROS_INFO("Stop all!");

    autoreversetimer.stop();
    noisetimer.stop();

    sendvel(0.0, 0.0);
  }

};

void autoreversetimerCallback(const ros::TimerEvent&)
{
     autoreversetimer.stop();
     ROS_INFO("Time to go back");

     sendvel(0.0, M_PI/sec2reserve);

     ros::Duration(sec2reserve).sleep();

     sendvel(0.0, 0.0);
     ros::Duration(0.015).sleep();
     sendvel(robotspeed, 0.0);

     ROS_INFO("Autoreverse; Full turn!");
     autoreversetimer.start();
}

void noisetimerCallback(const ros::TimerEvent&)
{
     noisetimer.stop();
     ROS_INFO("Attached your seatbelt");
     float noise = ((rand() % 64) - 32) * 0.066 / 9;

     sendvel(robotspeed, noise);

     ros::Duration(noiseduration).sleep();

     sendvel(robotspeed, 0.0);

     ROS_INFO("Noise all good");
     noisetimer.start();
}
