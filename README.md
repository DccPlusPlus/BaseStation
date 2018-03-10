What’s DCC++
------------

DCC++ is an open-source hardware and software system for the operation of DCC-equipped model railroads.

The system consists of two parts, the DCC++ Base Station and the DCC++ Controller.

The DCC++ Base Station consists of an Arduino micro controller fitted with an Arduino Motor Shield that can be connected directly to the tracks of a model railroad.

The DCC++ Controller provides operators with a customizable GUI to control their model railroad.  It is written in Java using the Processing graphics library and IDE and communicates with the DCC++ Base Station via a standard serial connection over a USB cable or wireless over BlueTooth.

What’s in this Repository
-------------------------

This repository, BaseStation-NodeMCU, contains a stripped-down DCC++ Base Station sketch designed for compiling and uploading into a NodeMCU.  All sketch files are in the folder named DCCpp_NodeMCU. More information about the sketch can be found in the included PDF file.

To utilize this sketch, simply download a zip file of this repository and open the file DCCpp_NodeMCU.ino within the DCCpp_NodeMCU folder using your Arduino IDE.  Please do not rename the folder containing the sketch code, nor add any files to that folder.  The Arduino IDE relies on the structure and name of the folder to properly display and compile the code.

This particular version only supports sensor inputs.

For more information on the overall DCC++ system, please follow the links in the PDF file.

-March 10, 2017
