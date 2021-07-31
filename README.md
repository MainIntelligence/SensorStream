# SensorStream
Utilities to help with collecting common sensor data in Linux

C++ code working with low-level interfaces for collecting data from the environment on a linux machine.
Currently this library interfaces against:
  -Sound devices - speakers and microphone.

Utilities are provided for modifying collected signals in the Utils directory of the project.
Some examples of utilities provided by MI:
  - Blurrer structure - for removing higher frequencies in data (tested against microphone data, good for 
    removing the "hiss" present in raw audio), you get control over the degree of blurring (removed frequencies)
