# MPC
---

**Model Predictive Control Project**

The goals / steps of this project are the following:

* Description of the model
* Discussion about the chosen *N* (timestep length) and *dt* (elapsed duration between timesteps) values
* Polynomial is fitted to waypoints
* 100-millisecond latency is implemented
* The vehicle must successfully drive a lap around the track

[//]: # (References)
[simulator]: https://github.com/udacity/self-driving-car-sim/releases
[win 10 update]: https://support.microsoft.com/de-de/help/4028685/windows-get-the-windows-10-creators-update
[uWebSocketIO]: https://github.com/uWebSockets/uWebSockets
[linux on win 10]: https://www.howtogeek.com/249966/how-to-install-and-use-the-linux-bash-shell-on-windows-10/
[MinGW]: http://www.mingw.org/
[CMake]: https://cmake.org/install/
[ipopt and cppad]: https://github.com/udacity/CarND-MPC-Project/blob/master/install_Ipopt_CppAD.md
[eigen lib]: http://eigen.tuxfamily.org/index.php?title=Main_Page
[udacity code]: https://github.com/udacity/CarND-MPC-Project
[data md]: ./DATA.md
[equations]: ./imgs/mpc-equations.jpg "Model Equations"
[output video]: ./imgs/mpc.gif "MPC Project Video"

---

## Files Submitted & Code Quality

### 1. Submission includes all required files and every TODO task has been accomplished 

For this project, I have used the [MPC Project Starter Code][udacity code] from Udacity and I have modified the following three files:
```cpp
main.cpp
MPC.cpp
MPC.h
```

### 2. Code must compile without errors

This project was done on Windows 10. In order to set up this project I had to:
* update my Windows 10 Version with the [Windows 10 Creators Update][win 10 update]
* install the [Linux Bash Shell][linux on win 10] (with Ubuntu 16.04) for Windows 10
* set up and install [uWebSocketIO][uWebSocketIO] through the Linux Bash Shell for Windows 10
* [download the simulator from Udacity][simulator]
* [install Ipopt and CppAD][ipopt and cppad]
* [download the Eigen library][eigen lib]  which is already part of the repo in the `src` folder

For a description of the sent back data from the simulator, please refer to the [DATA.md][data md] file.
**To update the Linux Bash Shell to Ubuntu 16.04 the Windows 10 Creators Update has to be installed!**

Also, [CMake][CMake] and a gcc/g++ compiler like [MinGW][MinGW] is required in order to compile and build the project.

Once the install for uWebSocketIO is complete, the main program can be built and run by doing the following from the project top directory in the Linux Bash Shell.

1. `mkdir build`
2. `cd build`
3. `cmake .. -G "Unix Makefiles" && make` on Windows 10 or `cmake .. && make` on Linux or Mac
4. `./mpc`

Then the simulator has to be started and *Project 5: MPC Controller* has to be selected. When everything is set up the Linux Bash Shell should print: 
```bash 
Listening to Port 4567
Connected
```

### 3. Description of the model
The model is based on the following equations to predict the `x`, `y`, `psi`, `v`, `cte` (cross-track error) & `epsi` (heading error) values:

![model equations][equations]

The 100 millisecond latency is initialized as a constant `dt = 0.1` on line 111 in `main.cpp`.

### 4. Discussion about the chosen *N* and *dt* values
At first, values of `N = 20` and `dt = 0.05` were chosen. Those (high) initial values had a negative impact on the vehicle's steering because the car nearly left the road. After fine-tuning these parameters, the final values are now `N = 12` and `dt = 0.04` which keep the vehicle very close to the center of the road and therefore ensure the vehicle drives around the track safely. A high `dt` value also had the effect that the car was in an acceleration-brake-loop and did not maintain a constant velocity.

### 5. Final Output:

![final output][output video]

Note: This gif is accelerated and does not match the actual speed during this recording.

---

## Discussion

### 1. Briefly discuss any problems / issues you faced in your implementation of this project.
Every time I started the simulator and ran the code, the vehicle instantly steered 25 degrees to the right for a few seconds and then to the left. This kept repeating the whole time. The problem was that my steering value (line 119 in `main.cpp`) already had a value between -1 and 1 so there was no necessity to divide the steering value by the `deg2rad()` function on line 18 in `main.cpp`.
