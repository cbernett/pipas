class DataPoint{
  
  float sampleTime;
  PVector euler;
  PVector linAcc;

  
  public
  DataPoint(String[] pieces){
    
    //multiply by 0.001 to convert from milliseconds to seconds
    sampleTime = float(pieces[0]) * 0.001;
    
    //Sensor outputs ZYX, so re-order pieces in the Euler pvector for clarity 
    euler = new PVector(Float.parseFloat(pieces[3]), Float.parseFloat(pieces[2]), Float.parseFloat(pieces[1]));
    linAcc = new PVector(Float.parseFloat(pieces[4]), Float.parseFloat(pieces[5]), Float.parseFloat(pieces[6]));
    
  }
  
  
}


class Particle{
  
  PVector pos, birthPlace;
  PVector vel;

  float trans;
  float size;
  float damping;
      
  public
  Particle(PVector _pos){
  
    birthPlace = new PVector(_pos.x, _pos.y, _pos.z);
    
    pos = new PVector(_pos.x, _pos.y, _pos.z);
    
    float v = 1.0;
    vel = new PVector(random(-v, v), random(-v, v), random(-v, v));

    
    damping = 0.91;
    trans = 255;
    size = random(0.5, 2);


  }  
  
  void update(){
    
    pos.add(vel);
    vel.mult(damping);
    
    //fade out
    if(trans > 0){
      trans -= 4.5;
    }
    
  }
  
  void display(){
    
    pushMatrix();


    strokeWeight(1);
    stroke(255, trans);
    
    translate(pos.x, pos.y, pos.z);
    quad(-size, 0, 0, -size, size, 0, 0, size);
    
    rotateY(PI/2);
    quad(-size, 0, 0, -size, size, 0, 0, size);

    rotateX(PI/2);
    quad(-size, 0, 0, -size, size, 0, 0, size);
    
    
    popMatrix();

    line(birthPlace.x, birthPlace.y, birthPlace.z, pos.x, pos.y, pos.z);
    
  }
 
}

/*---------------------------------------------------------------------*/

String[] lines;

//array for all the data points parsed from file
DataPoint[] dataList;

//Arrays to hold the riemann integrated velocity and position data
PVector[] velocities;

//arrays for paths that will be drawn
PVector[] path, xpath, ypath, zpath;

ArrayList<Particle> particles = new ArrayList<Particle>();

//current point being processed/drawn
int currentDataPoint = 0;

//Current point for animation
PVector pos;

//what we'll use to scale the visualization
float scaleVisuals = 1.0; 

//how big the visualization should be in coordinate space after scaling
float maxVisDimension = 600;  

//the header on the data file takes the 
//first 6 lines so data starts on line 7
int dataStartingLine = 6;

//How long to wait before drawing the next point, 
//i.e. how fast we're drawing the animation
float animationDelay = 20; 

//variable to keep track of time
long lastTime = 0;

//Object orientation
float yaw = 0;  //Z
float pitch = 0;  //Y
float roll = 0;  //X


//Visual/UI stuff
PImage backgroundGradient;
float backgroundHue;
PVector worldCenter;

//colors
color xCol, yCol, zCol;

//camera stuff
PVector easedCameraPos;
PVector rawCameraPos;
float easingSpeed = 0.08;

//timer to wait after finished drawing
int waitAfterComplete = 5000;

//helper variable
float pctComplete = 0;

//switch to start and stop playback
boolean playing = true;
float value = 1;

/**
 * <p>Ketai Library for Android: http://KetaiProject.org</p>
 *
 * <p>KetaiBluetooth wraps the Android Bluetooth RFCOMM Features:
 * <ul>
 * <li>Enables Bluetooth for sketch through android</li>
 * <li>Provides list of available Devices</li>
 * <li>Enables Discovery</li>
 * <li>Allows writing data to device</li>
 * </ul>
 * <p>Updated: 2012-05-18 Daniel Sauter/j.duran</p>
 */

//required for BT enabling on startup
import android.content.Intent;
import android.os.Bundle;
import android.view.MotionEvent;

import ketai.net.bluetooth.*;
import ketai.ui.*;
import ketai.net.*;

import ketai.data.*;

import oscP5.*;

KetaiBluetooth bt;
KetaiSQLite db;

String info = "";
String info2 = "";
String[] lines2;
KetaiList klist;
PVector remoteMouse = new PVector();
int prelength = -1;
int currentlength = 0;
boolean load = false;
int read = 0;
boolean loadP = false;

ArrayList<String> devicesDiscovered = new ArrayList();
boolean isConfiguring = true;
String bluetoothdata;

String UIText;
String CREATE_DB_SQL = "CREATE TABLE data ( time INTEGER PRIMARY KEY, x FLOAT NOT NULL, y FLOAT NOT NULL, z FLOAT NOT NULL);";

//********************************************************************
// The following code is required to enable bluetooth at startup.
//********************************************************************
void onCreate(Bundle savedInstanceState) {
  super.onCreate(savedInstanceState);
  bt = new KetaiBluetooth(this);
}

void onActivityResult(int requestCode, int resultCode, Intent data) {
  bt.onActivityResult(requestCode, resultCode, data);
}
//********************************************************************

void setup()
{   
  fullScreen(P3D);
  orientation(PORTRAIT);
  background(2, 34, 115);
  stroke(255);
  textSize(50);
  
  Pipassetup();
  
  db = new KetaiSQLite(this);

  //start listening for BT connections
   if ( db.connect() )
  {
    // for initial app launch there are no tables so we make one
    if (!db.tableExists("data"))
      db.execute(CREATE_DB_SQL);
  }
  bt.start();

  UIText =  "1 - discover devices\n" +
    "2 - make this device discoverable\n" +
    "3 - connect to device from discovered list.\n" +
    "4 - list paired devices\n" +
    "5 - Bluetooth info\n";
}

void draw()
{
  if (isConfiguring)
  {
    ArrayList<String> names;
    background(0);
    //based on last key pressed lets display
    //  appropriately
    if (key == '5')
      info = getBluetoothInformation();
    else 
    {
      if (key == '4')
      {
        info = "Paired Devices:\n";
        names = bt.getPairedDeviceNames();
      }
      else
      {
        info = "Discovered Devices:\n";
        names = bt.getDiscoveredDeviceNames();
      }

      for (int i=0; i < names.size(); i++)
      {
        info += "["+i+"] "+names.get(i).toString() + "\n";
      }
    }
    text(UIText + "\n\n" + info, 5, 150);}
  else
  {
    background(0);
    if(load){
    text("load success!",5, 150);
    read ++;
    if(read ==1){
    Pipasload(); 
    loadP = true;
    }
}
    //text("press 'a' to load file",5,200);
    //text("press 'g' to draw the path!",5,250);
    if(loadP){
    background(0);
    Pipasdraw();
    }
  }
  drawUI();
}


//Call back method to manage data received
void onBluetoothDataEvent(String who, byte[] data)
{
  if (isConfiguring)
    return;

  info2 += new String(data);
  if(info2.length() >0) {
  //println(info2);
  String[] list = info2.split("\\n");
 saveStrings("2.txt", list);
 currentlength = list.length;
  for(int i=0;i<list.length; i++){
  if(list[i].equals("<")) load =true;
  }
  println(currentlength);
  }
}

String getBluetoothInformation()
{
  String btInfo = "Server Running: ";
  btInfo += bt.isStarted() + "\n";
  btInfo += "Discovering: " + bt.isDiscovering() + "\n";
  btInfo += "Device Discoverable: "+bt.isDiscoverable() + "\n";
  btInfo += "\nConnected Devices: \n";

  ArrayList<String> devices = bt.getConnectedDeviceNames();
  for (String device: devices)
  {
    btInfo+= device+"\n";
  }

  return btInfo;
}