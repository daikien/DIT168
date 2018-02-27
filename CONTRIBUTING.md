How to contribute
---
Trello board will be used for clarifying contribution. The development team will generate new user stories at the beginning of each development phase(sprint). User stories will be posted on trello board and during the development team meeting. Vertical user stories are recommended but not mandatory for team members to take. When it comes to a big or unfamiliar user story, it is also possible for team members to work on the user story in pair or divide it into several tasks to implement. 
 
First install the libcluon dependencies:  
```
sudo add-apt-repository ppa:chrberger/libcluon
```
```
sudo apt-get update
```
```
sudo apt-get install libcluon
```

How to clone, build and test natively
---
1. Clone the project  
```
git clone https://github.com/daikien/DIT168.git
```

2. Inside the Demo folder, create a build folder and move into it  
```
mkdir build && cd build
```

3. Run CMake  
```
cmake ..
```

4. Run make  
```
make
```

5. Start the Receive binary  
```
./Receive
```

6. Open a new terminal inside the build folder and start the Send binary  
```
./Send
```

Now you can type a number in the Send terminal and it will appear in the Receive terminal.

How to clone, build and test using Docker
---
1. Clone the project 
```
git clone https://github.com/daikien/DIT168.git
```

2. Inside the Demo folder, build the test  
```
sudo docker build -t test -f Dockerfile .
```

3. Run the test  
```
sudo docker run --rm -ti --net=host test
```

4. Start the Receive binary  
```
./Receive
```

5. Open a new terminal and run the test again then start the Send binary  
```
./Send
```

Now you can type a number in the Send terminal and it will appear in the Receive terminal.

Coding guidelines
---
The guidelines for coding will be focused on the following 3 parts. Quality, reduction of complexity, and refactoring. For quality assurance, peer review and pull request are the recommended way for our development team to implement. Reduction of complexity will always be a guideline for team members to keep in mind during the developing phase. The more complex the code is the more likely it is to be buggy, the more difficult the bugs are to find and the more likely there are to be hidden bugs. Refactoring will be helpful when it comes to readability and structure.

How to package and release the binaries of your software using Docker to upload binaries to the car
---
1. First we save the binaries using: docker save DockerImages > dit168.tar
2. Then we transfer the .tar file to the car via usb.
3. Lastly we upload the binaries to the car using: cat dit168.tar | docker load
