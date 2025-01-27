# BachelorThesis
Development of a Gravity-Based Tangible Interface as a Multifunctional, Interactive Learning Platform for Children

**Project Structure Overview:**

**ReceiverSketchESP32**
This folder contains the code for the primary hub of the prototype, corresponding to **Chapter 4.3.1** and **4.3.3** in the thesis.
(Note: The code for the webserver is located in the "data" folder.)

    To access the code: navigate to ReceiverSketchESP32/src/main.cpp.

    Within main.cpp, the regions are marked using #pragma region "RegionName". The current lines for each region are as follows:
        dataprocessing: Lines 7–159
        applications: Lines 161–370
        webserver: Lines 372–434
        setup: Lines 436–492
        loop: Lines 494–514

    The code for the webpage can be found under ReceiverSketchESP32/data.
    The "data" folder contains the following files:
        index.html
        script.js
        style.css

**ESP32Transmitter**
This folder contains the code for the physical objects in the prototype, corresponding to **Chapter 4.3.2** in the thesis.

    To access the code: navigate to ESP32Transmitter/src/main.cpp.

**Additional Code**
The repository also includes additional code for sensor calibration and microcontroller setup:

    MPU6050 Calibration: This folder contains code used for calibrating the sensor for the physical objects.
    Retrieve MAC Address: This folder contains code for retrieving the MAC address of the microcontrollers.
