# DEM
## Overview
------
The DEM is a real-time system. It needs to retrieve waveform amplitude
information (“samples”) from an analog-to-digital converter at precise
intervals, do calculations with those samples, and perform timing operations

## Features
------
### 1. Ten threads to carry out different tasks with different priorities to achieve hard real-time.

|  File         | Thread            | Priority|
| ------------- |:--------------:| -----:|
|  main.c       | InitModulesThread | 0 |
|  meter.c      | VoltageThread     | 3 |
|  meter.c      | CurrentThread     | 4 |
|  meter.c      | CalcThread        | 8 |
|  UART.c       | TxThread          | 7 |
|  UART.c       | RxThread          | 1 |
|  DAC.c        | OutputThread      | 2 |
|  Protocol.c   | ProtocolThread    | 5 |
|  Interface.c  | PushButtonThread  | 6 |
|  Interface.c  | DisplayThread     | 9 |

### 2. And the calculations are all fixed point calculation. Not a single float type is used.
