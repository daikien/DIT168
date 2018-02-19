# DIT168

## Introduction
The goal with this project is to create an autonomous car, it is suppose to be able to drive along a “road” and stay within the lane while driving autonomously. Furthermore we will try to implement a “following” feature where the car will be able to follow another car. We will try to write a general protocol for this.

## GitHub Layout

The development team picked the github layout as master, developer, release branches, hotfixes and feature branches. 
For each user story or task, a new branch will be created designated for implementing the new feature. When the feature has been implemented, a pull request for merging the feature branch into the develop branch will be created. 
The pull request has to be reviewed by at least two members of the development team and by giving their approval accepting the pull request.
When the develop branch has sufficient features for a release, it is merged into the release branch where testing is executed and bug-fixes are applied until the branch works as intended. When the branch is deemed done it is merged into the master and given a version number. To be mentioned, bug-fixing and testing are also executing during the reviewing of pull request and developing phase.
When the hotfix has been tested, it will be merged into both the master, where it will be given a version number and into the develop branch for backup.

## How to clone, build and test
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
5. Start the Recieve binary
```
./Recieve
```
6. Open a new terminal inside the build folder and start the Send binary
```
./Send
```
Now you can type a number in the Send terminal and it will appear in the Recieve terminal. 
