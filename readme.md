# 1. Embedded System Coursework 2 - Group Hex Future


- [1. Embedded System Coursework 2 - Group Hex Future](#1-embedded-system-coursework-2---group-hex-future)
  - [## 1.1. Core Functionality and Specifications](#-11-core-functionality-and-specifications)
  - [## 1.2. Identification of all tasks that are performed by the system with   their method of implementation, thread or interrupt](#-12-identification-of-all-tasks-that-are-performed-by-the-system-with---their-method-of-implementation-thread-or-interrupt)

## 1.1. Core Functionality and Specifications
--------------------------------------

The following section will descript the core functionality requested by the specification document.

<br />

## 1.2. Identification of all tasks that are performed by the system with   their method of implementation, thread or interrupt
--------------------------------------

1 interrupt and 5 threads are used for the music synthesiser. 

<br />
**SampleISR**:  Produce notes in different waveforms in response to different key press
<br />
PlayMusic: 
<br />
CAN_DecodeTask:
<br />
CAN_TxTask: 
<br />
DisplayTask: 
<br />
ScanKeysTask: <b>word</b>

<center>

| Task           |   Type    | Priority |
| :------------- | :-------: | :------: |
| SampleISR      | Interrupt | Highest  |
| PlayMusic      |  Thread   |    1     |
| CAN_DecodeTask |  Thread   |    2     |
| CAN_TxTask     |  Thread   |    3     |
| DisplayTask    |  Thread   |    4     |
| ScanKeysTask   |  Thread   |    5     |

Table 1: Summary of Tasks with Types and Priority
</center>
