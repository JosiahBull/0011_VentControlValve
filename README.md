# Vent Control Valve
Basic airflow vents to control an AC system using butterfly valves.

Makes use of a 3D printed valve structure (CAD files in repo). Acuation is carried out by a small brushed DC motor, controlled by a relay & esp32. A circuit diagram is not included in this repo.

Exposes a basic REST api to the connected network, allowing for easy control of the vent through POST and GET commands. The system is designed to be powered off a 20v laptop power supply.
