%For this project, we will be developing a communication system that will
%take in 3 signals; a voice recording, a sine wave and a linear
%combination of a sine wave and cosine wave. These signals will go through
%sampling, PAM and PCM. 

%Block 1:This section of the code is to capture the voice recording and 
% store it in a array. The code displays to start speaking and end speaking 
% while recording for 2 seconds.The code also saves this data to a file as 
% well as the sampling frequency that was used to capture it. 
%


%Block 2: This secion of the code plots the voice recording as an amplitude
%for each sample. There are two plots, one plot is the whole voice
%recording while the other is a windowed version of it. 

%Block 3: This section of the code generates and plots with time two
%sinusiouds. One sinusioud is a sine wave with a frequency of 10Hz and the
%other is a linear combination of a sine and cosine which also has a
%fundamental freqeuncy of 10. 

%Block 4:The goal here was to see aliasing. We can view this by resampling the
%sinusiouds. Here, the frequency to sample is a slider which can be
%adjusted. Its visible that as we increase skip_val which is the period,
%then the freq  decreases which could violate nyquist theory; therefore
%causing aliasing. 

%Block 5: This portion of the code takes all thre signals, the voice, sine,
% and linear combination of sinusoids and plots them in the frequency domain.
%From these plots, we can tell what are the frequency components of each
%signal. 

%Blcok 6:This portion of the code takes the zoomed in voice data and cleans it up.
%This is done by making the data into an array and averaging it. This helps
%clean the image up and make clearer observations.

%Block 7: This portion of the code is to view the power that each singal
%has with respect to freqeuncy. This is done by taking the autocorrelation of the signal in
%time domain and then computing the fourier transform of that data and
%plotting it. 

%Block 8: We saw last time how aliasing affects a signal in time domain. Now we will
%observe how aliasing is affected in frequency domain. We do this by
%resampling the original signal in time, then computing the fft of it and
%plotting it.

%Block 9-11: The three portions of the code are attempting to reconsturct the
%original signal. This is done by multipling each data point with a sinc
%pulse. This reconstructs the signal and you get back the original.
%Although in this portion of the code, false readings are being given. For
%example, we know that the sine wave has a frequency of 10 hz, although
%when recontructing it, in time it looks right. But in frequency, it
%doesn't show that it is a 10hz signal but a signal <5hz. 

%Block 12: The code is taking what was already done (PAM) and turning it
%into PCM. It does this for the three signals, each signal goes through it
%three times with an increasing amount of bits each time. This is to show
%that as you increase the amount of bits to be used for quantizing, that
%the better the results/fidelity. 

%Block 13: In this section of the code, we ultimatly just create a RC
%pulse in frequency domain which corelates to a windowed sinc in time
%domain. There is a slider once again which controls the rolloff factor. 

%Block 14: This is an attempt to convolve the RC pulse with the bit stream
%but it doesn't work for somereason. 

%Block 15: In order to plot it, we have to create the RC pulse another way.
%Luckily in matlab there is a function to do this. 

%Block 16: From the transmitter signal created by the previous code, we add
%white gaussian noise to simulate a real-life environment. 

%Block 17: This block of code is to convert the bit stream already
%developed into a polar NRZ. To do this, the amplitudes of the bits can be
%+A -A where A is not 0. In order to accomplish this, I just shifted
%everyhthing down 0.5 so that it doesn't go to 0 and that |+A| =|-A|

%Block 18: This block plots the convolution of the stream bits with the
%pulse shaping of the RC pulse with a R=0.5. The code plots it in frequency
%domain and time domain. 