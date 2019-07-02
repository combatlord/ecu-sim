# ecu-sim
This is a project to build an OBD2 simulator. 

##Building
Simply connect to CAN board to and arduino pro micro and an OBD conenctor as shown in obd_sim_diagram.png

##Setting up
First, install the arduino sketch onto the simulator. Then, with the simulator plugged into your PC run the following command from the root directory of this project 

...
python ./ecu-sim.py <serial port name>
...

For example on linux, if your simulator shows up on port /dev/ttyACM0 you would run 
...
...
python ./ecu-sim.py /dev/ttyACM0
...

You will now be able to run commands to adjust the values output py the simualtor

##Commands
