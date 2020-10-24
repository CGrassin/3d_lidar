// LidarViewer.pde Processing sketch
// http://www.charleslabs.fr/en/project-3D+Lidar+Scanner+MK2

// Load sketch into Processing, available from:
// https://processing.org/
// You also need to install the PeasyCam library: Sketch > Import Library... > Add Library... > Search for "peasycam"

// This software read coordinates of the Arduino Lidar 
// (format: "X Y Z") and displays it as a 3D point cloud that you
// can move around using the mouse.
// Press 's' to save the cloud as a text file.
// Press 'x' to clear the points.

import processing.serial.*;
import peasy.*;
import java.io.FileWriter;
import javax.swing.JOptionPane;

Serial serial;
PeasyCam cam;
final float angleIncrement=0.1f;

ArrayList<PVector> pointList;

final int SERIAL_SPEED = 115200;

void setup() {
  size(1760 , 990, P3D);
  //surface.setResizable(true);
  
  surface.setTitle("3D Lidar Viewer");
  
  colorMode(RGB, 255, 255, 255);
  pointList = new ArrayList<PVector>();
  
  // PeasyCam
  cam = new PeasyCam(this, 500);
  cam.rotateZ(-3.1415/4);
  cam.rotateX(-3.1415/4);
  
  // Serial Port (added dialog)
  try {
    if (Serial.list().length == 0) {
      println("No serial device connected");
      exit();
    }
    else if (Serial.list().length == 1) {
      // only one device, select it (or try...)
      serial = new Serial(this, Serial.list()[0], SERIAL_SPEED);
    }
    else {
      // more than 1, show dialog
      String[] ports = Serial.list();
      
      String selection = (String) JOptionPane.showInputDialog(null,
          "Select the serial port that corresponds to the Lidar:",
          "Select serial port",
          JOptionPane.PLAIN_MESSAGE,
          null,
          ports,
          ports[ports.length-1]);
      
      if (selection == null || selection.isEmpty()) 
        exit();

      serial = new Serial(this, selection, SERIAL_SPEED); // change baud rate to your liking
    }
  }
  catch (Exception e) { 
    println("Not able to connect to serialPort (error:"+e.getClass().getName()+" " + e.getMessage() + ")");
    exit();
  }
}

void draw() {
  // Prepare window & drawing
  perspective();
  background(33);
  stroke(255,255,255);
  sphere(0.5f);
  fill(50);
  ellipse(0, 0, 10, 10);

  //Read Serial Port (if we can)
  String serialString = serial.readStringUntil('\n');
  if (serialString != null) {
    String[] coordinates = split(serialString, '\t');
    if (coordinates.length == 4) {
      pointList.add(new PVector(float(coordinates[0]), float(coordinates[1]), float(coordinates[2])));
    }
  }
 
  // Draw the actual point cloud
  for (int index = 0; index < pointList.size(); index++) {
    PVector v = pointList.get(index);
    if (index == pointList.size() - 1) {
      // Draw a line between Lidar and last point
      stroke(255, 15, 15);
      line(0, 0, 0, v.x,v.y,v.z);
    }
    // Draw point with a variable color
    stroke(255-v.z, 255-v.y, 255-v.x);
    point(v.x, v.y, v.z);
  }
}

// Handle keyboard events : movement, save & clear all points.
void keyReleased() {
  if (key =='x') {
    // erase all points
    pointList.clear();
  } else if(key == 's'){
    selectOutput("Select a file to write to:", "saveToFile");
  }
  else if (key == CODED) {
    if (keyCode == UP) {
      cam.rotateX(angleIncrement);
    } else if (keyCode == DOWN) {
      cam.rotateX(-angleIncrement);
    }else if (keyCode == LEFT) {
      cam.rotateY(angleIncrement);
    }else if (keyCode == RIGHT) {
      cam.rotateY(-angleIncrement);
    }
  }
}

// Function to save the point cloud in the Processing install directory
void saveToFile(final File selection){
  final String fileName = selection.getAbsolutePath();
  PrintWriter pw = null;
 
  try{
    pw = new PrintWriter(new FileWriter(fileName,true));
    for(int i=0;i<pointList.size();i++)
      pw.println((int)pointList.get(i).x + " " + 
                  (int)pointList.get(i).y + " " +
                  (int)pointList.get(i).z);
  }
  catch(Exception e){
  }
  finally {
    if(pw != null) pw.close();
  }
}
