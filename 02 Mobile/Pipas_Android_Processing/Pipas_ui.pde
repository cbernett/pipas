/*  UI-related functions */

boolean changedBg = false;
String flag = "color";

void mousePressed()
{
  //println(w);
  //keyboard button -- toggle virtual keyboard
  if(bluetoothconnected && load){
    if(mouseY <=height-150*h&&mouseY>=1028-400*h&&mouseX<=width/2+125*w&&mouseX>=width/2-125*w){
        //if we're entering draw mode then clear canvas
        if(page == 0){
          page = 1;
          showNum++;
        }
    }
  }
  
  if(page == 0){
     //button Discover
    if(mouseY <=390*h&&mouseY>=250*h&&mouseX<=width/2-260*w&&mouseX>=width/2-400*w){
      bt.discoverDevices();
      //homestatus = 1;
      //info = getBluetoothInformation();
    }
    //list Discover bluetooth
    else if(mouseY <=390*h&&mouseY>=250*h&&mouseX<=width/2+40*w&&mouseX>=width/2-100*w){
       if (bt.getDiscoveredDeviceNames().size() > 0)
        klist = new KetaiList(this, bt.getDiscoveredDeviceNames());
        //homestatus = 2;
    }
  }
  else if(page == 1){
     /*if(mouseY <=200*h&&mouseY>=0*h&&mouseX<=105&&mouseX>=0){
         //println("back");
          page = 0;
     }
     bgnum = int(random(4));
   */
    if(mouseY <=200*h&&mouseY>=0*h&&mouseX<=105&&mouseX>=0){
         //println("back");
          page = 0;
       }
    else if(mouseY <=200*h&&mouseY>=0&&mouseX<=width/2-20*w&&mouseX>=width/2-300*w){
         //change BG Color
         backgroundHue = random(255);
         flag = "color";
    }
    else if(mouseY <=200*h&&mouseY>=0&&mouseX<=width/2+300*w&&mouseX>=width/2+20*w){
         //change BG Image
      flag = "image";
      bgImageId++;
      if (bgImageId >= bgImages.size()){
        bgImageId = 0;
      }
         //setBg("image");
    }
    /*else if(mouseY <=200*h&&mouseY>=0*h&&mouseX<=width-5*w&&mouseX>=width-200*w){
         //SCREEENSHOT 
         println("screenshot");
         bgNum++;
         bg = get();
         bg.save(getSdWritableFilePathOrNull ("/img-0"+ram+bgNum+".jpg"));
    }*/
  }
}

PImage colorset;
PImage bg01;
PImage bg02;
PImage bg03;
PImage bg04;
void drawUI()
{
  //Draw top shelf UI buttons

  pushStyle();
  
  //CB ADDING CODE HERE

  if (page == 0){
    fill(#000000);
    noStroke();
    
    //TOP BAR
    rect(0,0,width,200*h);
    //icon.resize(100,100);
    
    //APP NAME / LOGO
    icon = loadImage("logowhite.png");
    //icon.resize(int(icon.width*w),int(icon.height*h));
    image(icon,width/2-100*w,(200*h)/2-60);
    //rect(0, 0, width/3, 100);
  }
  else if(page == 1){
      //VISUALIZATION PAGE UI
        Back = loadImage("back.png");
        //Back.resize(int(Back.width*w),int(Back.height*h));
        image(Back,5*w,60*h);
        Camera = loadImage("camera.png");
        image(Camera,width-135*w,60*h);
        fill(#E1F3F8);
         //rect(width/2-405,height-305,810,210,20);
        colorset = loadImage("color.png");
         //colorset.resize(int(colorset.width*w),int(colorset.height*h));
        textFont(font,30*w);
         //text("Cores:",width/2-200*w,height-320*h);
        image(colorset,width/2-400*w,height-300*h);
        //text("Cor 2:",width/2-400*w,height-200*h);
        image(colorset,width/2-400*w,height-200*h);
        //text("Cor 3:",width/2-400*w,height-100*h);
        image(colorset,width/2-400*w,height-100*h);
        noFill();
               
         //BG COLOR CIRCLE BUTTON
         textFont(boldfont,40*w);
         fill(255);
         text("Cor", width/2-190*w,130*h);
         
         //SELECT IMAGE BUTTON
         //colorMode(HSB, 255);
         //fill(0);
         //rect(width/2+20,60*h,200*w,100*h);
         textFont(boldfont,40*w);
         fill(255);
         text("Imagem", width/2+12*w,130*h);
         
         //DRAW BACKGROUND IMAGE BUTTONS
         colorMode(RGB);
  }
  popStyle();
}

void setBg(){
  if (flag == "color"){
    colorMode(HSB, 255); 
    background(backgroundHue, 200, 80);

  }
  else if (flag == "image"){
    pushMatrix();
    println("changing the bg, and the changedBg flag is " + changedBg);
    
    println("bgImageID is " + bgImageId);
    background01 = loadImage(bgImages.get(bgImageId));
    println("image is "+ background01);
    image(background01, 0, 0, width, height);
    popMatrix();
  }

}

/*void changebackground(){
       
       //println("mouseX is " + mouseX);
       //println("mouseY is " + mouseY);         
       //setBg();
}*/

float colorX = 0;
void changeGraphColor(){
    colorMode(RGB);
    if(mouseX<=width/2-100*w&mouseX>=width/2-400*w){
      colorX = map((mouseX-(width/2-400*w)),20*w,300*w,0,255);
      if(mouseY <=height-200*h&&mouseY>=height-300*h){
      //println(colorX);
      xCol = color(255,colorX,0, 255);
      noFill();
      stroke(#E1F3F8);
      rect(mouseX-20*w,height-290*h,55*w,55*h,25*w);
      }
      else if(mouseY <=height-100*h&&mouseY>=height-200*h){
      yCol = color(255,colorX,0);
      noFill();
      stroke(#E1F3F8);
      rect(mouseX-20*w,height-190*h,55*w,55*h,25*w);
      }
      else if(mouseY <=height&&mouseY>=height-100*h){
      zCol = color(255,colorX,0, 255);
      noFill();
      stroke(#E1F3F8);
      rect(mouseX-20*w,height-90*h,55*w,55*h,25*w);
      }
    }
    else if(mouseX<=width/2&&mouseX>=width/2-100*w){
      colorX = map((mouseX-(width/2-400*w)),300*w,400*h,255,0);
      if(mouseY <=height-200*h&&mouseY>=height-300*h){
      //println(colorX);
        xCol = color(colorX,255,0);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-290*h,55*w,55*h,25);
        }
        else if(mouseY <=height-100*h&&mouseY>=height-200*h){
        yCol = color(colorX,255,0, 255);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-190*h,55*w,55*h,25*w);
        }
        else if(mouseY <=height&&mouseY>=height-100*h){
        zCol = color(colorX,255,0);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-90*h,55*w,55*h,25*w);
        }  
      }
    else if(mouseX<=width/2+150*w&&mouseX>=width/2){
      colorX = map((mouseX-(width/2-400*w)),400*w,550*h,0,255);
      if(mouseY <=height-200*h&&mouseY>=height-300*h){
      //println(colorX);
        xCol = color(0,255,colorX);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-290*h,55*w,55*h,25*w);
        }
        else if(mouseY <=height-100*h&&mouseY>=height-200*h){
        yCol = color(0,255,colorX);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-190*h,55*w,55*h,25*w);
        }
        else if(mouseY <=height&&mouseY>=height-100*h){
        zCol = color(0,255,colorX);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-90*h,55*w,55*h,25*w);
        }  
      }
    else if(mouseX<=width/2+280*w&&mouseX>=width/2+150*w){
      colorX = map((mouseX-(width/2-400*w)),550*w,680*h,255,0);
      if(mouseY <=height-200*h&&mouseY>=height-300*h){
      //println(colorX);
        xCol = color(0,colorX,255);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-290*h,55*w,55*h,25*w);
        }
        else if(mouseY <=height-100*h&&mouseY>=height-200*h){
        yCol = color(0,colorX,255);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-190*h,55*w,55*h,25*w);
        }
        else if(mouseY <=height&&mouseY>=height-100*h){
        zCol = color(0,colorX,255);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-90*h,55*w,55*h,25*w);
        }  
      }
    else if(mouseX<=width/2+400*w&&mouseX>=width/2+280*w){
      colorX = map((mouseX-(width/2-400*w)),680*w,800*h,0,200);
      if(mouseY <=height-200*h&&mouseY>=height-300*h){
      //println(colorX);
        xCol = color(colorX,0,255);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-290*h,55*w,55*h,25*w);
        }
        else if(mouseY <=height-100*h&&mouseY>=height-200*h){
        yCol = color(colorX,0,255);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-190*h,55*w,55*h,25*w);
        }
        else if(mouseY <=height&&mouseY>=height-100*h){
        zCol = color(colorX,0,255);
        noFill();
        stroke(#E1F3F8);
        rect(mouseX-20*w,height-90*h,55*w,55*h,25*w);
        }  
      }    
}

void onKetaiListSelection(KetaiList klist)
{
  String selection = klist.getSelection();
  bt.connectToDeviceByName(selection);
  //dispose of list for now
  klist = null;
}
