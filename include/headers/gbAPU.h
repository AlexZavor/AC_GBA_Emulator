#ifndef GBAPU_H
#define GBAPU_H

#include "globals.h"
#include "gbMEM.h"
#include <math.h>

#define PLAYING_FREQ 440


class gbAPU {
private:
	static gbMEM* APU_MEM;
public:
	void APU_setMEM(gbMEM* mem){
		APU_MEM = mem;
	}

	void APU_clearMEM(){
		APU_MEM = nullptr;
	}

	float CH1() {
		static float envelope = 1;
		static float envelopetmr = 0;
		static float freq = 0;
		static float freqtmr = 0;
		static float timer;
		static bool  useLength;
		static bool useFreqShift = false;
		if (APU_MEM->MEM[0xFF14] & 0b10000000) { //new note start
			APU_MEM->MEM[0xFF14] &= 0b01111111;
			envelope = ((APU_MEM->MEM[0xFF12] & 0b11110000) >> 4) / 15.0f;
			envelopetmr = (1.0f / 64.0f) * (APU_MEM->MEM[0xFF12] & 0b00000111);
			timer = ((64.0f - (APU_MEM->MEM[0xFF11] & 0b00111111)) * (1.0f / 256.0f));
			useLength = APU_MEM->MEM[0xFF14] & 0b01000000;
			freq = 131072.0f / (2048.0f - (APU_MEM->MEM[0xFF13] + ((APU_MEM->MEM[0xFF14] & 0b00000111) << 8)));
			freqtmr = (0.0078) * ((APU_MEM->MEM[0xFF10] & 0b01110000) >> 4);
			if (freqtmr != 0) { useFreqShift = true; }
			else { useFreqShift = false; }
		}
		if (useLength) {	//note length
			if (timer > 0) {
				timer -= 1.0/AUDIO_FREQ;
			}
			else {
				return 0;
			}
		}
		if (envelope > 0 && envelope < 1) {	//envelope
			if (envelopetmr > 0) {
				envelopetmr -= 1.0/AUDIO_FREQ;
			}
			else if (envelopetmr <= 0) {
				envelope -= (-1 * (APU_MEM->MEM[0xFF12] & 0b00001000)) * (1.0f / 15.0f);
				envelopetmr = (1.0f / 64.0f) * (APU_MEM->MEM[0xFF12] & 0b00000111);
			}
			if (envelope > 1) {
				envelope = 1;
			}if (envelope < 0) {
				envelope = 0;
			}
		}
		if (useFreqShift) {
			if (freqtmr > 0) {
				freqtmr -= 1.0/AUDIO_FREQ;
			}
			else if (freqtmr <= 0) {
				freqtmr = (0.0078) * ((APU_MEM->MEM[0xFF10] & 0b01110000) >> 4);
				//equation here
				bool sub = (APU_MEM->MEM[0xFF10] & 0b00001000);
				if (sub) {
					freq -= (freq / (pow(2, APU_MEM->MEM[0xFF10] & 0b00000111)));
				}
				else {
					freq += (freq / (pow(2, APU_MEM->MEM[0xFF10] & 0b00000111)));
				}
			}
		}
		float dutyCycle;
		switch ((APU_MEM->MEM[0xFF11]&0b11000000)>>6) //Duty Cycle
		{
		case 0:
			dutyCycle = 0.125;
			break;
		case 1:
			dutyCycle = 0.25;
			break;
		case 2:
			dutyCycle = 0.50;
			break;
		case 3:
			dutyCycle = 0.75;
			break;
		default:
			dutyCycle = 0.5;
		}

		static int  squareCount = (AUDIO_FREQ/freq)*dutyCycle;
		static bool squareState = true;
		squareCount--;
		if(squareCount == 0){
			if(squareState){
				// invert duty cycle for low state
				dutyCycle = 1 - dutyCycle;
			}
			squareCount = (AUDIO_FREQ/freq)*dutyCycle;
			squareState = !squareState;
		}
		return squareState * envelope;
	}
	float CH2() {
		static float envelope = 1;
		static float envelopetmr = 0;
		static float timer;
		static bool  useLength;
		if (APU_MEM->MEM[0xFF19] & 0b10000000) { //new note start
			APU_MEM->MEM[0xFF19] &= 0b01111111;
			envelope = ((APU_MEM->MEM[0xFF17] & 0b11110000) >> 4) / 15.0f;
			envelopetmr = (1.0f / 64.0f) * (APU_MEM->MEM[0xFF17] & 0b00000111);
			timer = ((64.0f - (APU_MEM->MEM[0xFF16] & 0b00111111)) * (1.0f / 256.0f));
			useLength = APU_MEM->MEM[0xFF19] & 0b01000000;
		}
		if (useLength) {	//note length
			if (timer > 0) {
				timer -= 1.0/AUDIO_FREQ;
			}
			else {
				return 0;
			}
		}
		if (envelope > 0 && envelope < 1) {	//envelope
			if (envelopetmr > 0) {
				envelopetmr -= 1.0/AUDIO_FREQ;
			}
			else if (envelopetmr <= 0) {
				envelope -= (-1 * (APU_MEM->MEM[0xFF17] & 0b00001000)) * (1.0f / 15.0f);
				envelopetmr = (1.0f / 64.0f) * (APU_MEM->MEM[0xFF17] & 0b00000111);
			}
			if (envelope > 1) {
				envelope = 1;
			}if (envelope < 0) {
				envelope = 0;
			}
		}
		float dutyCycle;
		switch ((APU_MEM->MEM[0xFF16] & 0b11000000) >> 6) //Duty Cycle
		{
		case 0:
			dutyCycle = 0.125;
			break;
		case 1:
			dutyCycle = 0.25;
			break;
		case 2:
			dutyCycle = 0.50;
			break;
		case 3:
			dutyCycle = 0.75;
			break;
		default:
			dutyCycle = 0.5;
		}

		float freq = 131072.0f / (2048.0f - (APU_MEM->MEM[0xFF18] + ((APU_MEM->MEM[0xFF19] & 0b00000111) << 8)));

		static int  squareCount = (AUDIO_FREQ/freq)*dutyCycle;
		static bool squareState = true;
		squareCount--;
		if(squareCount == 0){
			if(squareState){
				// invert duty cycle for low state
				dutyCycle = 1 - dutyCycle;
			}
			squareCount = (AUDIO_FREQ/freq)*dutyCycle;
			squareState = !squareState;
		}
		return squareState * envelope;
	}
	float CH3() {
		return 0;
	}
	float CH4() {
		return 0;
	}


	void gbAudioCallback(Sint16* samples, int sample_count){
		memset(samples, 0, sample_count);
		if(APU_MEM == nullptr){return;}

		//loop through many samples
		for (int i = 0; i < sample_count; ++i){
			float left = 0;
			float right = 0;
			if (APU_MEM->MEM[0xFF26] & 0b10000000) {
				//CH1
				float ch1 = CH1();
				if (ch1) { APU_MEM->MEM[0xFF26] |= 0b00000001; }
				if (APU_MEM->MEM[0xFF25] & 0b00010000) {
					left += ch1;
				}
				if (APU_MEM->MEM[0xFF25] & 0b00000001) {
					right += ch1;
				}
				//CH2
				float ch2 = CH2();
				if (ch2) { APU_MEM->MEM[0xFF26] |= 0b00000010; }
				if (APU_MEM->MEM[0xFF25] & 0b00100000) {
					left += ch2;
				}
				if (APU_MEM->MEM[0xFF25] & 0b00000010) {
					right += ch2;
				}
				//CH3
				float ch3 = CH3();
				if (true) { APU_MEM->MEM[0xFF26] |= 0b00000100; }
				if (APU_MEM->MEM[0xFF25] & 0b01000000) {
					left += ch3;
				}
				if (APU_MEM->MEM[0xFF25] & 0b00000100) {
					right += ch3;
				}
				//CH4
				float ch4 = CH4();
				if (true) { APU_MEM->MEM[0xFF26] |= 0b00001000; }
				if (APU_MEM->MEM[0xFF25] & 0b10000000) {
					left += ch4;
				}
				if (APU_MEM->MEM[0xFF25] & 0b00001000) {
					right += ch4;
				}
			}
			//Volumes
			left *=  ((float)((APU_MEM->MEM[0xFF24] & 0b00000111) + 1) / 7.0f);
			right *= ((float)(((APU_MEM->MEM[0xFF24] & 0b01110000)>>4) + 1) / 7.0f);
			float audio = (left+right);//I'm tired. its mono now
			samples[i] = VOLUME*audio;
		}
		
		// Square tone test
		
		// static int count = (AUDIO_FREQ/PLAYING_FREQ)/2;
		// static bool squareState = 0;
		// for (int i = 0; i < sample_count; ++i)
		// {
		// 	samples[i] = VOLUME*squareState;
		// 	count--;
		// 	if(count == 0){
		// 		count = (AUDIO_FREQ/PLAYING_FREQ)/2;
		// 		squareState = !squareState;
		// 	}
		// }
	}
};


#endif //GBAPU_H