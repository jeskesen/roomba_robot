/*********************************************************************
*
* Software License Agreement (BSD License)
*
*  Copyright (c) 2013, ISR University of Coimbra.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the ISR University of Coimbra nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
* Author: Gonçalo Cabrita on 06/06/2013
* Copied from the Turtlebot interactive markers package.
*********************************************************************/

#include <ros/ros.h>
#include <interactive_markers/interactive_marker_server.h>
#include <string.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose.h>
#include <tf/tf.h>

using namespace visualization_msgs;

class RoombaMarkerServer
{
  public:
    RoombaMarkerServer()
      : pnh("~"), server("roomba_marker_server")
    {

      pnh.param<std::string>("link_name", link_name, "/base_link");
      pnh.param<double>("linear_scale", linear_scale, 1.0);
      pnh.param<double>("angular_scale", angular_scale, 2.2);

      vel_pub = nh.advertise<geometry_msgs::Twist>("teleop/cmd_vel", 1);
      createInteractiveMarkers();

      ROS_INFO("InteractiveMarkers Teleop - %s - Initialized.", __FUNCTION__);
    }

    void processFeedback(
        const InteractiveMarkerFeedbackConstPtr &feedback );
  
  private:
    void createInteractiveMarkers();
  
    ros::NodeHandle nh;
    ros::NodeHandle pnh;
    ros::Publisher vel_pub;
    interactive_markers::InteractiveMarkerServer server;
    
    double linear_scale;
    double angular_scale;
    
    std::string link_name;
};

void RoombaMarkerServer::processFeedback(
    const InteractiveMarkerFeedbackConstPtr &feedback )
{
  // Handle angular change (yaw is the only direction in which you can rotate)
  double yaw = tf::getYaw(feedback->pose.orientation);
  
  geometry_msgs::Twist vel;
  vel.angular.z = angular_scale*yaw;
  vel.linear.x = linear_scale*feedback->pose.position.x;

  vel_pub.publish(vel);    
  
  // Make the marker snap back to turtlebot
  server.setPose("roomba_marker", geometry_msgs::Pose());
  
  server.applyChanges();
}

void RoombaMarkerServer::createInteractiveMarkers()
{ 
  // create an interactive marker for our server
  InteractiveMarker int_marker;
  int_marker.header.frame_id = link_name;
  int_marker.name = "roomba_marker";
  
  InteractiveMarkerControl control;

  control.orientation_mode = InteractiveMarkerControl::FIXED;
  control.orientation.w = 1;
  control.orientation.x = 1;
  control.orientation.y = 0;
  control.orientation.z = 0;
  control.name = "move_x";
  control.interaction_mode = InteractiveMarkerControl::MOVE_AXIS;
  int_marker.controls.push_back(control);

  control.orientation.w = 1;
  control.orientation.x = 0;
  control.orientation.y = 1;
  control.orientation.z = 0;
  control.name = "rotate_z";
  
  control.interaction_mode = InteractiveMarkerControl::MOVE_ROTATE;
  int_marker.controls.push_back(control);
  
  server.insert(int_marker, boost::bind( &RoombaMarkerServer::processFeedback, this, _1 ));
  
  server.applyChanges();
}


int main(int argc, char** argv)
{
  ros::init(argc, argv, "roomba_interactive_markers_teleop");
  RoombaMarkerServer roomba_teleop;

  puts("Reading from rviz");
  puts("---------------------------");
  puts("Use rviz to move the Roomba.");
  
  ros::spin();
}
