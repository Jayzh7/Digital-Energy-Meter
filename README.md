# DEM
Embedded Software project
Overview
------
The DEM is a real-time system. It needs to retrieve waveform amplitude
information (“samples”) from an analog-to-digital converter at precise
intervals, do calculations with those samples, and perform timing operations
Features
------
I'm using ten threads to carry out different tasks with different priorities to achieve hard real-time.
And the calculations are all fixed point calculation. Not a single float type is used.
