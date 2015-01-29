# Radiation Detector

Initially, I have written the DSO application to record and visualize the signals of a very simple radiation detector: 

<a href="http://tinypic.com?ref=a49c7m" target="_blank">
 <img src="http://i57.tinypic.com/a49c7m.png" border="0" alt="Image and video hosting by TinyPic">
</a>

The detector uses the photo-electric effect of a (gamma) photon on a photo diode. The detector [circuit](circuit.pdf) (based on the [*Geigerle*](www.opengeiger.de)) consists of 4, cheap BPW32 photo diodes followed by an (inverting) amplifier with an ridiculous huge gain. The photo diodes are operated in complete darkness and in reverse direction. Therefore they are basically non-conducting. Whenever a photon enters one of these photo diodes, the photo electric effect will cause, for a very short time, a tiny current in the reverse direction. This leads to a tiny voltage drop over the diodes which gets amplified (and inverted) by the first OpAmp. 

The second OpAmp acts as an (inverting) differentiator and amplifier. This stage is necessary as there is always a very small (thermal) current that flows in the reverse direction, resulting into a small (constant) bias voltage that would then be amplified further. The output of the differentiator can then be captured directly using a sound card and the DSO application as shown above. 

Additionally, I added an integrating comparator that drives (via the buffer) a LED and a piezo beeper. The sensitivity of the detector will be set by the 5k potentiometer at the comparator. 

Whenever you are rebuilding the detector, do not expect any spectacular from this very simple circuit. Its performance, precisely its sensitivity and conversion rate depends highly on the photo diodes used. I have chosen these particular photo diodes because they are cheep, not because the are good radiation detectors. They are very thin, means the silicon layers that form the photo diode are only a few microns thick. Hence only a very small portion of the gamma photons that enter the photo diodes will actually be detected.

Further, the sensitivity of the whole circuit depends largely on the noise level and the capacity of the photo diodes. This implies, that increasing the number of photo diodes will not necessarily increase the sensitivity as the capacity of the diodes will also increase. The higher the capacity, the higher the energy of the gamma photon needs to be, to be detectable. All OpAmps used here are TL07X types, actually I used the TL074. In my opinion, this OpAmp has a good noise-to-price ratio. 

In principle it is possible to do some simple gamma radiation spectroscopy with this circuit as the reverse current through the diodes and therefore the voltage-peak height is (approximately) proportional to the energy loss of the gamma photon in the diode. As mentioned above, the silicon layers of the photo diodes are very thin. Hence it is very unlikely that the complete energy of the gamma photon is absorbed within the diode, turning the energy measurement very unreliable.