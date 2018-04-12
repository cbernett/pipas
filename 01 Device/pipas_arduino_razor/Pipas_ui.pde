/*  UI-related functions */


void mousePressed()
{
    
  //keyboard button -- toggle virtual keyboard
  if (mouseY <= 100 && mouseX > 0 && mouseX < width/3)
    KetaiKeyboard.toggle(this);
  else if (mouseY <= 100 && mouseX > width/3 && mouseX < 2*(width/3)) //config button
  {
    isConfiguring=true;
  }
  else if (mouseY <= 100 && mouseX >  2*(width/3) && mouseX < width) // draw button
  {
    if (isConfiguring)
    {
      //if we're entering draw mode then clear canvas
      background(0);
      isConfiguring=false;
    }
  }
  
}

void mouseDragged()
{
  if (isConfiguring)
    return;
   value += mouseX*0.01;
  //send data to everyone
  //  we could send to a specific device through
  //   the writeToDevice(String _devName, byte[] data)
  //  method.
  OscMessage m = new OscMessage("/remoteMouse/");
  m.add(mouseX);
  m.add(mouseY);

  bt.broadcast(m.getBytes());
 
}

public void keyPressed() {
   //playing = !playing;
  if (key =='3')
  {
    //If we have not discovered any devices, try prior paired devices
    if (bt.getDiscoveredDeviceNames().size() > 0)
      klist = new KetaiList(this, bt.getDiscoveredDeviceNames());
    else if (bt.getPairedDeviceNames().size() > 0){
      klist = new KetaiList(this, bt.getPairedDeviceNames());    
    }
  }
  else if (key == '1')
  {
    bt.discoverDevices();
  }
  else if (key == '8')
    bt.stop();
  else if (key == '2')
  {
    bt.makeDiscoverable();
  }
  else if (key == '7')
  {
    bt.start();
  }
  
  else if(key == 'a'){
     Pipasload();
     println("yes");
    }
}


void drawUI()
{
  //Draw top shelf UI buttons

  pushStyle();
  fill(131,126,1);
  stroke(255);
  rect(0, 0, width/3, 100);

  if (isConfiguring)
  {
    noStroke();
    fill(57,55,1);
  }
  else
    fill(131,126,1);

  rect(width/3, 0, width/3, 100);

  if (!isConfiguring)
  {  
    noStroke();
    fill(57,55,1);
  }
  else
  {
    fill(131,126,1);
    stroke(255);
  }
  rect((width/3)*2, 0, width/3, 100);

  fill(255);
  text("Keyboard", 5, 50); 
  text("Bluetooth", width/3+5, 50); 
  text("Interact", width/3*2+5, 50); 

  popStyle();
}

void onKetaiListSelection(KetaiList klist)
{
  String selection = klist.getSelection();
  bt.connectToDeviceByName(selection);

  //dispose of list for now
  klist = null;
}