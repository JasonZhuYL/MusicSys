# 1. Embedded System Coursework 2 - Group Hex Future


- [1. Embedded System Coursework 2 - Group Hex Future](#1-embedded-system-coursework-2---group-hex-future)
  - [## 1.1. Core Functionality and Specifications](#-11-core-functionality-and-specifications)
  - [## 1.2. Identification of all tasks that are performed by the system with   their method of implementation, thread or interrupt](#-12-identification-of-all-tasks-that-are-performed-by-the-system-with---their-method-of-implementation-thread-or-interrupt)
- [Advanced Features](#advanced-features)
  - [## 2.1 Low & High Pass Filter](#-21-low--high-pass-filter)
  - [## 2.2 Reverb](#-22-reverb)
  - [## 2.3 Polyphony with Stereo Sound](#-23-polyphony-with-stereo-sound)
  - [## 2.4 Keyboard Auto-Detect through Handshake Signals](#-24-keyboard-auto-detect-through-handshake-signals)
  - [## 2.5 Intuitive Distributed User Interface](#-25-intuitive-distributed-user-interface)
  - [## 2.6 Waveform Selection](#-26-waveform-selection)

## 1.1. Core Functionality and Specifications
--------------------------------------

The following section will descript the core functionality requested by the specification document.

<br />

## 1.2. Identification of all tasks that are performed by the system with   their method of implementation, thread or interrupt
--------------------------------------

<b>SampleISR</b>:  Produce notes in different waveforms in response to different key press
<br />
<b>PlayMusic</b>: Play pre-programmed music note when designated knob is pressed
<br />
<b>CAN_DecodeTask</b>: Decode messages related CAN communication for transimission purpose
<br />
<b>CAN_TxTask</b>: Transmit CAN bus messages for playing and ending a note
<br />
<b>DisplayTask</b>: Display information about music synthesiser setting in real-time
<br />
<b>ScanKeysTask</b>: Read any changes in inputs and update the variables in the software

<br />
Overall, <b>1 interrupt and 5 threads</b> are used for the music synthesiser. Table 1 below summarise the tasks with type and priority.

<br />
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

# Advanced Features


## 2.1 Low & High Pass Filter
--------------------------------------
To enable music note manipulation, Low Pass and High Pass Filtering of music signal are implemented. 

**Pressing** **of** **Knob** **0**: To switch between **No Filter (NUL), Low Pass Filter (LPF), High Pass Filter (HPF)**.
<br />
**Turning of Knob 0**: To adjust the intensity of different filtering ranging from 0-9

```C++
const uint8_t lpf[10][12] = {
{10,10,10,10,10,10,10,10,10,9,8,7}
,{10,10,10,10,10,10,10,10,9,8,7,6}
,{10,10,10,10,10,10,10,9,8,7,6,5}
,{10,10,10,10,10,10,9,8,7,6,5,4}
,{10,10,10,10,10,9,8,7,6,5,4,3}
,{10,10,10,10,9,8,7,6,5,4,3,2}
,{10,10,10,9,8,7,6,5,4,3,2,1}
,{10,10,9,8,7,6,5,4,3,2,1,1}
,{10,9,8,7,6,5,4,3,2,1,1,1}
,{9,8,7,6,5,4,3,2,1,1,1,1}
};

const uint8_t hpf[10][12] = {
{7,8,9,10,10,10,10,10,10,10,10,10}
,{6,7,8,9,10,10,10,10,10,10,10,10}
,{5,6,7,8,9,10,10,10,10,10,10,10}
,{4,5,6,7,8,9,10,10,10,10,10,10}
,{3,4,5,6,7,8,9,10,10,10,10,10}
,{2,3,4,5,6,7,8,9,10,10,10,10}
,{1,2,3,4,5,6,7,8,9,10,10,10}
,{1,1,2,3,4,5,6,7,8,9,10,10}
,{1,1,1,2,3,4,5,6,7,8,9,10}
,{1,1,1,1,2,3,4,5,6,7,8,9}
};
```


## 2.2 Reverb
--------------------------------------

Reverb is the persistence of sound after a sound is produced. Reverb is created using array of size 51 to store previous Vout values and replaying 3 elements of the array at different volume to emulate sound reflection in a room.

```C++
//*******   Reverb  ***********
if(reverb_switch == 1){
    VoutReverb[0] = Vout;
    for(uint8_t i=50; i>0; i--){
        VoutReverb[i] = VoutReverb[i-1];
    }
    Vout = Vout+VoutReverb[45]*0.7+VoutReverb[35]*0.5+VoutReverb[20]*0.2;
}
//*******   Reverb  ***********
```

The part of code is implemented within **SampleISR()** function and is adapted for different waveforms such as SawTooth, Triangular and Square.


## 2.3 Polyphony with Stereo Sound
--------------------------------------
Polyphony, in music, is the simultaneous combination of two or more tones or melodic lines. In our music synthesiser, the sound of music notes will be produced respectively by the speakers of corresponding board where the keys are pressed. This enables polyphony with Stereo Sound where combination of music notes are much more similar to real piano.

Clipping of speaker is avoided using Stereo Sound system. Having notes played from different speakers allow a larger number of notes to be played at the same time instead of going through one speaker only. 

## 2.4 Keyboard Auto-Detect through Handshake Signals
--------------------------------------

## 2.5 Intuitive Distributed User Interface
--------------------------------------

## 2.6 Waveform Selection
--------------------------------------

Waveforms such as Square and Triangle waves are implemented in addition to Sawtooth wave. All of these waves are defined by mathematical function that is written in code.

**Turning of Knob 1**: To select different waveforms

```C++
void sampleISR(){...}
```
<details>
<summary>Click to view "Waveform Implementation in sampleISR()" </summary>

```C++
void sampleISR()
{
    int32_t Vout = 0;
    if(waveform_mode == 0){ //generate sawtooth wave
        if(filter_mode == 2){
            for (uint8_t i=0;i<keypressed_pointer;i++){
                phaseAcc[i] += (currentStepSize[i]*195225/10)*hpf[var0][keypressed[i]];
            }
        }else if(filter_mode == 1){
            for (uint8_t i=0;i<keypressed_pointer;i++){
                phaseAcc[i] += (currentStepSize[i]*195225/10)*lpf[var0][keypressed[i]];
            }
        }
        else{
            for (uint8_t i=0;i<keypressed_pointer;i++){
                phaseAcc[i] += (currentStepSize[i]*195225);
            }
        }
        
        for (uint8_t i=0;i<keypressed_pointer;i++){
            Vout += (phaseAcc[i] >> 24);
        }
        Vout = Vout >> (8 - int(volume) / 2);  

        //*******   Reverb  ***********
        if(reverb_switch == 1){
            VoutReverb[0] = Vout;
            for(uint8_t i=50; i>0; i--){
                VoutReverb[i] = VoutReverb[i-1];
            }
            Vout = Vout+VoutReverb[45]*0.7+VoutReverb[35]*0.5+VoutReverb[20]*0.2;
        }
        //*******   Reverb  ***********
        analogWrite(OUTR_PIN, Vout + 128);

    }else if(waveform_mode == 1){ //generate triangular wave

        if (filter_mode == 2){
            for (uint8_t i = 0; i < keypressed_pointer; i++){
                phaseAcc[i] += (currentStepSize[i] * 195225 / 10) * hpf[var0][keypressed[i]];
            }
        }
        else if (filter_mode == 1){
            for (uint8_t i = 0; i < keypressed_pointer; i++){
                phaseAcc[i] += (currentStepSize[i] * 195225 / 10) * lpf[var0][keypressed[i]];
            }
        }
        else{
            for (uint8_t i = 0; i < keypressed_pointer; i++){
                phaseAcc[i] += (currentStepSize[i] * 195225);
            }
        }
        for (uint8_t i = 0; i < keypressed_pointer; i++){
            Vout += (phaseAcc[i] >> 24);
        }

        if (Vout < 0){
            Vout = 128 + (Vout >> (8 - int(volume) / 2)) *4;
        }
        else{
            Vout = 127 - (Vout >> (8 - int(volume) / 2)) *4;
        }

        //*******   Reverb  ***********
        if(reverb_switch == 1){
            VoutReverb[0] = Vout;
            for(uint8_t i=50; i>0; i--){
                VoutReverb[i] = VoutReverb[i-1];
            }
            Vout = Vout+VoutReverb[45]*0.7+VoutReverb[35]*0.5+VoutReverb[20]*0.2;
        }
        //*******   Reverb  ***********
        analogWrite(OUTR_PIN, Vout);

    }else if(waveform_mode == 2){ //generate square wave
        if (filter_mode == 2){
            for (uint8_t i = 0; i < keypressed_pointer; i++){
                phaseAcc[i] += (currentStepSize[i] * 195225 / 10) * hpf[var0][keypressed[i]];
            }
        }
        else if (filter_mode == 1){
            for (uint8_t i = 0; i < keypressed_pointer; i++){
                phaseAcc[i] += (currentStepSize[i] * 195225 / 10) * lpf[var0][keypressed[i]];
            }
        }
        else{
            for (uint8_t i = 0; i < keypressed_pointer; i++){
                phaseAcc[i] += (currentStepSize[i] * 195225);
            }
        }
        int32_t Vout_intermediate = 0;
        for (uint8_t i = 0; i < keypressed_pointer; i++){
            Vout_intermediate += (phaseAcc[i] >> 24);
        }
        Vout_intermediate = Vout_intermediate >> (8 - int(volume) / 2);
        if (Vout_intermediate > 0){
            Vout = INT32_MAX >> 24;
            Vout = Vout >> (8 - int(volume) / 2);
        }
        else{
            Vout = INT32_MIN >> 24;
            Vout = Vout >> (8 - int(volume) / 2);
        }

        //*******   Reverb  ***********
        if(reverb_switch == 1){
            VoutReverb[0] = Vout;
            for(uint8_t i=50; i>0; i--){
                VoutReverb[i] = VoutReverb[i-1];
            }
            Vout = Vout+VoutReverb[45]*0.7+VoutReverb[35]*0.5+VoutReverb[20]*0.2;
        }
        //*******   Reverb  ***********
        analogWrite(OUTR_PIN, Vout + 128);
    }

}
```
</details>


