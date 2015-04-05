# Run The following script to install on your desktop the lar\_ws repository #
```
#!/bin/bash 
sudo apt-get install python-webkit
sudo apt-get install python-imaging-tk
mkdir -p ~/Desktop/lar_ws/src&&
(cd ~/Desktop/lar_ws/src && catkin_init_workspace)&&
(cd ~/Desktop/lar_ws && catkin_make)&&
 git clone https://odedyec@code.google.com/p/lar-car-project/ ~/Desktop/lar_ws/src/lar_car_project &&
(cd ~/Desktop/lar_ws && catkin_make)
(cd ~/Desktop/lar_ws && catkin_make install)
echo "source ~/Desktop/lar_ws/devel/setup.bash" >> ~/.bashrc 
```