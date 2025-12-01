# Metal Detection and Automated Sorting System
### Course : Embedded Systems & Internet of Things Laboratory — 3rd Year, 1st Semester
This project is an IoT-enabled automated sorting system that detects metallic and non-metallic objects using sensors and sorts them using a robotic arm. Features are:
- Automated Sorting Conveyor Belt
- Detect Metal vs Non-Metal
- Robotic Arm Sorting
- Cloud Logging
- Blynk Remote Control
- ESP32-Based Smart IoT System


## Working process
The Metal Detection and Automated Sorting System begins by running a conveyor belt that carries objects toward a sensing and sorting area. As an object approaches, the HC-SR04 ultrasonic sensor detects its presence and signals the ESP32 to temporarily keep the motor running so the object reaches the next stage. When the object reaches the E2E-X5MF2 inductive proximity sensor, the system checks whether the object is metallic or non-metallic. If the proximity sensor detects metal, the ESP32 triggers the servo-driven robotic hand to rotate to 0° and drop the object to the left side; if no metal is detected, the servo rotates to 180° and drops the object to the right side. Every metal detection event is logged in the Firebase Realtime Database with a timestamp, and the system updates the daily count, last detection time, and total count. At the same time, the entire system—motor status, counts, and live logs—is remotely monitored and controlled through the Blynk IoT app, where users can turn the system on or off and view real-time data. This integrates embedded hardware, cloud connectivity, and mobile IoT control into a fully automated smart sorting solution.

## Components Used
- ESP32 Development Board
- HC-SR04 Ultrasonic Distance Sensor
- Inductive Proximity Sensor Switch (E2E-X5MF2)
- MG995 High-Torque Servo Motor
- SRD-05VDC-SL-C Relay Module
- LM2596 DC-DC Step Down Buck Converter (with Digital Display)
- BC547 NPN Transistor
- 1 kΩ Resistors (for voltage divider)
- Breadboard
- Jumper Wires (Male–Male, Male–Female)
- 3 × 3.7V Rechargeable Lithium Batteries


<p float="left">
  <img src="https://github.com/user-attachments/assets/33ca0f4b-b8d1-4bb4-8971-55d37e5338c7" width="320" />
  <img src="https://github.com/user-attachments/assets/b07cb302-7a9a-4b71-9c33-a322c35fa232" width="320" />

  <a href="https://github.com/user-attachments/assets/ed9a5f79-bd0a-4779-85e9-5b4c1fb402fb">
    <img src="https://github.com/user-attachments/assets/b07cb302-7a9a-4b71-9c33-a322c35fa232" width="320" />
  </a>
</p>


https://github.com/user-attachments/assets/ed9a5f79-bd0a-4779-85e9-5b4c1fb402fb





