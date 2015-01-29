# Radiation Detector

Initially, I've written the DSO application to record and visualize the signals of a very simple radiation detector, I've build. 

<a href="http://tinypic.com?ref=a49c7m" target="_blank">
 <img src="http://i57.tinypic.com/a49c7m.png" border="0" alt="Image and video hosting by TinyPic">
</a>

The detector uses the photo-electric effect of a (gamma) photon on a photo diode. The detector [circuit](circuit.pdf) consists of 4, cheap BPW32 photo diodes followed by an amplifier with an ridiculous huge gain. The photo diodes are operated in complete darkness and in reverse direction. Therefore they are basically non-conducting. Whenever a photon enters one of these photo diodes, the photo electric effect will cause, for a very short time, a tiny current to flow in reverse direction. This leads to a tiny voltage drop over the diodes which gets amplified (and inverted) by the first OpAmp. 

The second OpAmp acts as an (inverting) differentiator and amplifier. This stage is necessary as there is always a very small (thermal) current that flows in the reverse direction, resulting into a small (constant) bias voltage that would then be amplified further. The output of the differentiator can then be captured directly using a sound card and the DSO application. 

Additionally, I added an integrating comparator that drives (via the buffer) a LED and a piezo beeper. The sensitivity of the detector will be set by the 5k potentiometer at the comparator. 

