# ecu-sim
This is a project to build an OBD2 simulator. The full writeup [can be found here](https://blog.speedfox.co.uk/articles/1562110227-obd_simulator_for_carputer_development/).

## Building
Simply connect to CAN board to and arduino pro micro and an OBD conenctor as shown in obd_sim_diagram.png

## Setting up
First, install the arduino sketch onto the simulator. Then, with the simulator plugged into your PC run the following command from the root directory of this project 

```
python ./ecu-sim.py <serial port name>
```

For example on linux, if your simulator shows up on port /dev/ttyACM0 you would run 

```
python ./ecu-sim.py /dev/ttyACM0
```

You will now be able to run commands to adjust the values output py the simualtor

## Commands
### setval
Sets the value returned for a partiular property from the simulator. FOr example, to have the simulator return a speed of 80km/h run 
```
>setval 13  80
```
The values entered here will be the value retunred from the simualtor before any formulas are applied by the software that interprets these values. 
### setcode
Adds an error code to the list of error codes returned from teh simulator. For example, to have the simulator return error code P1234 you can run 
```
>setcode 1234
```
