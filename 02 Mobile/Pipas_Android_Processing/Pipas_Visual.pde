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
    strokeWeight(5);
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
