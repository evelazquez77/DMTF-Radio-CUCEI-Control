# DMTF-Radio-CUCEI-Control
This program controls the operation of a radio receiver by DMTF commands sent through the same receive radio channel.
The system is performed by an Arduino Nano that controls a set of ICs as an MT8870 DTMF Decoder, an FM radio receiver module based on the Si4703 and a MC4066 as a base of digital volume control.
The main functions are:
  1.- The selective and general power on and off of the audio power system
        Command: #MMSSP#
            MM=[ Corresponding classroom module number (00~99 where 00 is for General School) ]
            SS=[ Corresponding classroom number (00~99 where 00 is for General Module) ]
            P=[ A: General Power ON, B: General Power OFF, C: Selective Power ON, D: Selective Power OFF ]
  2.- 4-level audio volume control
        Command: #MMSSV#
            MMSS= Same as above.
            V=[ 0: Mute, 1~2:Intermediate Levels of Sound Volume 3: Full Sound Volume ]
  3.- General power on in emergency mode with and w/o alarm sound.
            Command: #**#
