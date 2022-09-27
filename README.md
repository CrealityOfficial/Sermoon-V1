# Sermoon-V1/V1 PRO Firmware

Creality, dedicating to bringing benefits to human beings via technology innovations, has received support from both users and 3D printing enthusiasts. With gratefulness, Creality wants to continue the pace of making the world a better place with you all. This time, Creality will open the source code and we believe GitHub is the way to go. 

This is the repository that contains the source code and the development versions of the firmware running on the Creality , It's based on the well-known Marlin but with modifications.

The firmware for the Creality Sermoon-v1/v1 pro is proudly based on Marlin2.0 byScott Lahteine (@thinkyhead), Roxanne Neufeld (@Roxy-3D), Chris Pepper (@p3p), Bob Kuhn (@Bob-the-Kuhn), João Brazio (@jbrazio), Erik van der Zalm (@ErikZalm) and is distributed under the terms of the GNU GPL 3 license.

If you want to download the latest firmware version, go to Releases page and download the needed files. In the [releases](https://github.com/CrealityOfficial/Sermoon-v1) page you will find the source code and the SD Files needed for the LCD Display. After that, normally you need to update the SD files of the display and gradually complete the updates of menus, graphics and functionalities. 

Please refer to: [YouTube](https://www.youtube.com/watch?v=NmoRRFW4zTc)
In order to get instructions on how to flash the firmware and load new LCD SD files to the display. 


# New Features
1. Repair the bug of slow printing speed with Cloud APP.
2. Repair the bug of losing gcode file while resume printing.
3. Repair the bug of slow speed when printing threading.
4. Repair the bug that the parameter setting of "Control-Motion-Jerk " cannot be saved.
5. Repair the bug that Z-axis acceleration cannot be set to 100-200m/s2.
7. Repair the bug that the interface of auto leveling function doesn’t exit when CR-touch is abnormal.
8. Repair the bug that the computer port sends the “Pause” (M25) command but the display doesn’t synchronize show the “Continue printing” page when online printing.

# Special Point
A special structure on Sermoon-V1/V1 Pro we must introduce is expansion port, which can fulfill more possibility.

## How to install CrealityPrint software
1、Please download “Creality Print” to install in [Creality official website](https://www.creality.com/download)

2、Double click to open the software installation package, click "Next-"Accept".

3、Confirm the installation location of the software, click "Browse"-"Next"- "Install"-"Finish".


# Issues and Suggestions
Your feedback is very important to us, as it helps us improve even faster. Please test this firmware and let us know if it misbehaves in any way. We are standing by!

In order to get responses in an efficient way, we recommend you to follow some guidelines:

1、First of all, search for related issues.

2、Detail the firmware version you're running.

3、Explain to us the error or bug, so that we can test it properly.

4、In the title, indicate the label of the issue. (For example: #issue)

# Development Process
The code is currently in development, trying to improve functionalities.
Since it’s possible for the **advanced users** to contribute in firmware development, we suppose you know the points even if they have not been clearly illustrated by Creality.

The master branch is stable and it's currently of the version 2.0.x. The master branch stores code created by Creality. Once a release is done, the users, get to upgrade the version and give feedback to us. We get to know the bugs as well as optimization based on the feedback from you and Creality will make a decision on what to be included into the master branch. 

By integrating suggested improvements, we will make a branch from the version.

This is a classic code development process and we want more, so we really want you to participate from the very beginning.
