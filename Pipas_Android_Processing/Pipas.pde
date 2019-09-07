import android.content.Intent;
import android.os.Bundle;
import android.view.MotionEvent;
import android.os.Environment;
import java.text.DecimalFormat;

import ketai.net.bluetooth.*;
import ketai.ui.*;
import ketai.net.*;
import ketai.data.*;

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
int homestatus = 0; //using this as a flag to know which button i tapped on first screen

ArrayList<String> devicesDiscovered = new ArrayList();
ArrayList<String> bgImages = new ArrayList();


boolean isConfiguring = true;
String bluetoothdata;

String UIText;

String msgToDraw = "no file set";
String dataFile = "";

PImage bg;
int bgNum = 0;
int showNum = 0;
int ram = 0;


PImage D;
PImage C;
PImage I;
PImage V;
PImage S;
PImage Back;
PImage Camera;
PFont font;
PFont boldfont;
boolean bluetoothconnected = false;
int  page = 0;
int bgnum = 0;

PImage icon;
PImage background00;
PImage background01;
PImage background02;
PImage background03;
PImage background04;

int wallpaper = 0;

// change the screen size here;
float w = 768.00/1080.00;
float h = 1280.00/1920.00;

KetaiGesture gesture;

//PHOTO CYCLE ARRAY INFO
ArrayList<String> names;
int bgImageId = 0;
int strokeweight;


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
public boolean surfaceTouchEvent(MotionEvent event) {

  //call to keep mouseX, mouseY, etc updated
  super.surfaceTouchEvent(event);

  //forward event to class for processing
  return gesture.surfaceTouchEvent(event);
}

void setup()
{   
  fullScreen(P3D);
  orientation(PORTRAIT);
  backgroundHue = random(255);
  println("width * "+w);
  println("height * "+h);
  stroke(255);
  font = loadFont("Futura-Medium-48.vlw");
  boldfont = loadFont("Futura-Bold-48.vlw");
  //font = loadFont("LithosPro-Regular-24.vlw");
  Pipassetup();
  ram = int(random(1,15));
  gesture = new KetaiGesture(this);
  bt.start();
  
  bgImages.add("vidigal01.jpg");
  bgImages.add("vidigal02.jpg");
  bgImages.add("vidigal03.jpg");
  bgImages.add("vidigal04.jpg");
  bgImages.add("vidigal05.jpg");
  bgImages.add("vidigal06.jpg");
  bgImages.add("vidigal07.jpg");
                                                                                                                                                                                                                   
 
   UIText =  "1 • Descobrir" +
    "\n"+
    "2 • Conectar" +
    "\n"+
    "3 • Verificar";
    
    

}

void draw(){
  if (page == 0){
    background(#E1F3F8);
    
    // 4 buttons
    D = loadImage("Discover.png");
    image(D,width/2-390*w,250*h);
    C = loadImage("Connect.png");
    image(C,width/2-60*w,250*h);
    I = loadImage("Information.png");
    image(I,width/2+240*w,250*h);


    //DETECT TAPS AND SET STATUS
    if(mouseY <=390*h&&mouseY>=250*h&&mouseX<=width/2-260*w&&mouseX>=width/2-400*w){
      homestatus = 1;
    }
    else if(mouseY <=390*h&&mouseY>=250*h&&mouseX<=width/2+40*w&&mouseX>=width/2-100*w){
      homestatus = 2;
    }
    else if(mouseY <=390*h&&mouseY>=250*h&&mouseX<=width/2+380*w&&mouseX>=width/2+240*w){
      homestatus = 3;
    }
    

    if (homestatus == 1){
      discover();
    }
    else if (homestatus == 2){
      connect();
    }
    else if(homestatus == 3){
      info = getBluetoothInformation();
    }
    
      //WRITING TEXT TO SCREEN
      textFont(boldfont,40*w);
      fill(#5ca3fa);
      text("Etapas:\n\n" + UIText, width/2-350*w,650*h);
      textFont(font,40*w);
      fill(0);
      text(info, width/2-350*w,1100*h);
  }
  else if (page == 1){
      if(load){
        read ++;
        if(read ==1){        
          Pipasload(); 
          loadP = true;
        }
      }
      if(loadP){
        //changebackground();
        //colored background
        Pipasdraw();
        //SCREENSHOT HERE SO IT CAPTURES BEFORE THE UI ELEMENTS ARE DRAWN
        if(mousePressed){
           if(mouseY <=200*h&&mouseY>=0*h&&mouseX<=width&&mouseX>=width-100*w){
              println("SCREENSHOT");
              bgNum++;
              bg = get();
              bg.save(getSdWritableFilePathOrNull ("/img-0"+ram+bgNum+".jpg"));
              
              textFont(boldfont,40*w);
              fill(255);
              text("Snap!", width/2-40*w,400*h);
              //Camera = loadImage("cameraover.png");
              //image(Camera,width-135*w,60*h);

         
            }
        }
      } 
    }
         //fill(#91D6E2);
    //noStroke();
    //rect(width/2-400*w,550*h,800*w,800*h,50*w);
   drawUI();
}


 void discover(){
      info = "Buscando Pipas...";
     /*if(mouseY <=390*h&&mouseY>=250*h&&mouseX<=200){
        info = "Buscando Pipas...";
      }*/
      bluetoothconnected = false;
      names = bt.getDiscoveredDeviceNames();
      
      if (homestatus == 1){
        if (names.size()<1){
            info = "Procurando pipas..." + "\n\n";
        }
        else{
            info = "Pipas encontradas: "+ names.size() + "\n\n";
            println("number of names found = "+ names.size());
            for (int i=0; i < names.size(); i++){
              info += "["+i+"] "+names.get(i).toString() + "\n\n";
            }
        }
      }
     else if (homestatus != 1){
       info = "Aperte 1 para começar.";
     }
       
  }


void connect(){
     if(bt.getConnectedDeviceNames().size()>0){
       bluetoothconnected = true;
       if(currentlength>0){
         info = "Sincronizando os dados: "+ currentlength;
         if(load){
             V = loadImage("view.png");
             //V.resize(int(V.width*w),int(V.height*h));
             image(V,width/2-125*w,height-400*h);
           info = "Pronto!\n\n"+
                 "Aperte o butão em baixo pra ver o gráfico.";
           }
          }
         else{
            info = "Conectado!\n\n"+
               "Pode sincronizar agora.";
           }
     }
  }


//Call back method to manage data received
void onBluetoothDataEvent(String who, byte[] data)
{
  if (!bluetoothconnected)
    return;

  info2 += new String(data);
  if(info2.length() >0) {
  dataFile = getSdWritableFilePathOrNull("strings-0"+ram+showNum+".txt");
  if ( dataFile == null ){
        String errorMsg = "There was error getting SD card path. Maybe your device doesn't have SD card mounted at the moment";
        println(errorMsg);
        msgToDraw = errorMsg;
  }
  else{
    
      // now we can use save strings.
      String[] list = info2.split("\\n");
      saveStrings(dataFile, list);
      msgToDraw = "looks like we've managed to save strings to file: [" + dataFile + "]";
       currentlength = list.length;
        for(int i=0;i<list.length; i++){
          if(list[i].equals("<")) load =true;
         }
        println(currentlength);
        }
  }
}

String getBluetoothInformation()
{
  String btInfo = "";
  btInfo += "Procurando o kit: " + bt.isDiscovering() + "\n";
  btInfo += "Pronto pra procurar: "+bt.isDiscoverable() + "\n\n";

  ArrayList<String> devices = bt.getConnectedDeviceNames();
  if(bt.getConnectedDeviceNames().size()>0){
    for (String device: devices)
    {
      btInfo+= "Kits conectados: \n"+device+"\n\n"+
            "Pronto! Aperte e manta o butão pra sync";
    }
  }
  else {
  btInfo+= "Tente conectar de novo.";
  }
  return btInfo;
}


String getSdWritableFilePathOrNull(String relativeFilename){
   File externalDir = Environment.getExternalStorageDirectory();
   if ( externalDir == null ){
      return null;
   }
   String sketchName= this.getClass().getSimpleName();
   //println("simple class (sketch) name is : " + sketchName );
   File sketchSdDir = new File(externalDir, sketchName);
   
   File finalDir =  new File(sketchSdDir, relativeFilename);
   return finalDir.getAbsolutePath();
}
