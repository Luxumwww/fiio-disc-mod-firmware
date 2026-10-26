/* Adapted from Airwindows Galactic3 (MIT). Original source in third_party/open-effects.
 * FP32; deterministic silence; VST wrappers/dither removed; coefficients cached at create. */
enum {
		bez_AL,
		bez_AR,
		bez_BL,
		bez_BR,
		bez_CL,
		bez_CR,	
		bez_InL,
		bez_InR,
		bez_UnInL,
		bez_UnInR,
		bez_SampL,
		bez_SampR,
		bez_cycle,
		bez_total
	};
typedef struct {
	float iirAL;
	float iirBL;
	
	float aIL[6480];
	float aJL[3660];
	float aKL[1720];
	float aLL[680];
	
	float aAL[9700];
	float aBL[6000];
	float aCL[2320];
	float aDL[940];
	
	float aEL[15220];
	float aFL[8460];
	float aGL[4540];
	float aHL[3200];
	
	float aML[3111];
	float aMR[3111];
	float vibML, vibMR, depthM, oldfpd;
	
	float feedbackAL;
	float feedbackBL;
	float feedbackCL;
	float feedbackDL;
		
	float iirAR;
	float iirBR;
	
	float aIR[6480];
	float aJR[3660];
	float aKR[1720];
	float aLR[680];
	
	float aAR[9700];
	float aBR[6000];
	float aCR[2320];
	float aDR[940];
	
	float aER[15220];
	float aFR[8460];
	float aGR[4540];
	float aHR[3200];
	
	float feedbackAR;
	float feedbackBR;
	float feedbackCR;
	float feedbackDR;
		
	int countA, delayA;
	int countB, delayB;
	int countC, delayC;
	int countD, delayD;
	int countE, delayE;
	int countF, delayF;
	int countG, delayG;
	int countH, delayH;
	int countI, delayI;
	int countJ, delayJ;
	int countK, delayK;
	int countL, delayL;
	int countM, delayM;//all these ints are shared across channels, not duplicated
	
	float vibM;
	
	 //the new undersampling. bez signifies the bezier curve reconstruction
	float bez[bez_total];	
	
	uint32_t fpdL;
	uint32_t fpdR;
	//default stuff
	
    float A;
    float B;
    float C;
    float D;
    float E;
	float F;//parameters. Always 0-1, and we scale/alter them elsewhere.

float overallscale,regen,attenuate,lowpass,drift,derez,size,wet;
} Galactic3State;
static void galactic_recalculate(Galactic3State *s,unsigned rate){
	s->overallscale = 1.0f;
	s->overallscale /= 44100.0f;
	s->overallscale *= rate;
		
	s->regen = 0.0625f+((1.0f-s->A)*0.0625f);
	s->attenuate = (1.0f - (s->regen / 0.125f))*1.333f;
	s->lowpass = powf(1.00001f-(1.0f-s->B),2.0f)/sqrtf(s->overallscale);
	s->drift = powf(s->C,3)*0.001f;
	s->derez = s->D/s->overallscale;
	if (s->derez < 0.0005f) s->derez = 0.0005f; if (s->derez > 1.0f) s->derez = 1.0f;
	s->derez = 1.0f / ((int)(1.0f/s->derez));
	//this hard-locks s->derez to exact subdivisions of 1.0f
	s->size = (s->E*1.77f)+0.1f;
	s->wet = 1.0f-(powf(1.0f-s->F,3));
	
	s->delayI = 3407.0f*s->size;
	s->delayJ = 1823.0f*s->size;
	s->delayK = 859.0f*s->size;
	s->delayL = 331.0f*s->size;
	s->delayA = 4801.0f*s->size;
	s->delayB = 2909.0f*s->size;
	s->delayC = 1153.0f*s->size;
	s->delayD = 461.0f*s->size;
	s->delayE = 7607.0f*s->size;
	s->delayF = 4217.0f*s->size;
	s->delayG = 2269.0f*s->size;
	s->delayH = 1597.0f*s->size;
	s->delayM = 256;
	
}
static void galactic_reset(Galactic3State *s,unsigned rate){memset(s,0,sizeof(*s));s->A=0.65f;s->B=0.7f;s->C=0.15f;s->D=1.0f;s->E=0.55f;s->F=0.7f;s->vibM=3;s->oldfpd=.5f;s->bez[bez_cycle]=1;s->fpdL=0x12345678;s->fpdR=0x87654321;s->countA=1;s->countB=1;s->countC=1;s->countD=1;s->countE=1;s->countF=1;s->countG=1;s->countH=1;s->countI=1;s->countJ=1;s->countK=1;s->countL=1;s->countM=1;
 galactic_recalculate(s,rate);
}
static Galactic3State *galactic_create(unsigned rate){Galactic3State *s=calloc(1,sizeof(*s));if(s)galactic_reset(s,rate);return s;}
static void galactic_sample(Galactic3State *s,float io[2]){float *in1=io,*in2=io+1,*out1=io,*out2=io+1;
		float inputSampleL = *in1;
		float inputSampleR = *in2;
		float drySampleL = inputSampleL;
		float drySampleR = inputSampleR;
		
		s->vibM += (s->oldfpd*s->drift);
		if (s->vibM > (3.141592653589793238f*2.0f)) {
			s->vibM = 0.0f;
			s->oldfpd = 0.4294967295f+(s->fpdL*0.0000000000618f);
		}
		
		s->aML[s->countM] = inputSampleL * s->attenuate;
		s->aMR[s->countM] = inputSampleR * s->attenuate;
		s->countM++; if (s->countM < 0 || s->countM > s->delayM) s->countM = 0;
		
		float offsetML = (sinf(s->vibM)+1.0f)*127;
		float offsetMR = (sinf(s->vibM+(3.141592653589793238f/2.0f))+1.0f)*127;
		int workingML = s->countM + offsetML;
		int workingMR = s->countM + offsetMR;
		float interpolML = (s->aML[workingML-((workingML > s->delayM)?s->delayM+1:0)] * (1-(offsetML-floorf(offsetML))));
		interpolML += (s->aML[workingML+1-((workingML+1 > s->delayM)?s->delayM+1:0)] * ((offsetML-floorf(offsetML))) );
		float interpolMR = (s->aMR[workingMR-((workingMR > s->delayM)?s->delayM+1:0)] * (1-(offsetMR-floorf(offsetMR))));
		interpolMR += (s->aMR[workingMR+1-((workingMR+1 > s->delayM)?s->delayM+1:0)] * ((offsetMR-floorf(offsetMR))) );
		inputSampleL = interpolML;
		inputSampleR = interpolMR;
		//predelay that applies vibrato
		//want vibrato speed AND depth like in MatrixVerb
		
		s->iirAL = (s->iirAL*(1.0f-s->lowpass))+(inputSampleL*s->lowpass); inputSampleL = s->iirAL;
		s->iirAR = (s->iirAR*(1.0f-s->lowpass))+(inputSampleR*s->lowpass); inputSampleR = s->iirAR;
		//initial filter
		
		s->bez[bez_cycle] += s->derez;
		s->bez[bez_SampL] += ((inputSampleL+s->bez[bez_InL]) * s->derez);
		s->bez[bez_SampR] += ((inputSampleR+s->bez[bez_InR]) * s->derez);
		s->bez[bez_InL] = inputSampleL; s->bez[bez_InR] = inputSampleR;
		if (s->bez[bez_cycle] > 1.0f) { //hit the end point and we do a reverb sample
			s->bez[bez_cycle] = 0.0f;
			
			s->aIL[s->countI] = (s->bez[bez_SampL]+s->bez[bez_UnInL]) + (s->feedbackAR * s->regen);
			s->aJL[s->countJ] = (s->bez[bez_SampL]+s->bez[bez_UnInL]) + (s->feedbackBR * s->regen);
			s->aKL[s->countK] = (s->bez[bez_SampL]+s->bez[bez_UnInL]) + (s->feedbackCR * s->regen);
			s->aLL[s->countL] = (s->bez[bez_SampL]+s->bez[bez_UnInL]) + (s->feedbackDR * s->regen);
			s->bez[bez_UnInL] = s->bez[bez_SampL];
			
			s->aIR[s->countI] = (s->bez[bez_SampR]+s->bez[bez_UnInR]) + (s->feedbackAL * s->regen);
			s->aJR[s->countJ] = (s->bez[bez_SampR]+s->bez[bez_UnInR]) + (s->feedbackBL * s->regen);
			s->aKR[s->countK] = (s->bez[bez_SampR]+s->bez[bez_UnInR]) + (s->feedbackCL * s->regen);
			s->aLR[s->countL] = (s->bez[bez_SampR]+s->bez[bez_UnInR]) + (s->feedbackDL * s->regen);
			s->bez[bez_UnInR] = s->bez[bez_SampR];
			
			s->countI++; if (s->countI < 0 || s->countI > s->delayI) s->countI = 0;
			s->countJ++; if (s->countJ < 0 || s->countJ > s->delayJ) s->countJ = 0;
			s->countK++; if (s->countK < 0 || s->countK > s->delayK) s->countK = 0;
			s->countL++; if (s->countL < 0 || s->countL > s->delayL) s->countL = 0;
			
			float outIL = s->aIL[s->countI-((s->countI > s->delayI)?s->delayI+1:0)];
			float outJL = s->aJL[s->countJ-((s->countJ > s->delayJ)?s->delayJ+1:0)];
			float outKL = s->aKL[s->countK-((s->countK > s->delayK)?s->delayK+1:0)];
			float outLL = s->aLL[s->countL-((s->countL > s->delayL)?s->delayL+1:0)];
			float outIR = s->aIR[s->countI-((s->countI > s->delayI)?s->delayI+1:0)];
			float outJR = s->aJR[s->countJ-((s->countJ > s->delayJ)?s->delayJ+1:0)];
			float outKR = s->aKR[s->countK-((s->countK > s->delayK)?s->delayK+1:0)];
			float outLR = s->aLR[s->countL-((s->countL > s->delayL)?s->delayL+1:0)];
			//first block: now we have four outputs
			
			s->aAL[s->countA] = (outIL - (outJL + outKL + outLL));
			s->aBL[s->countB] = (outJL - (outIL + outKL + outLL));
			s->aCL[s->countC] = (outKL - (outIL + outJL + outLL));
			s->aDL[s->countD] = (outLL - (outIL + outJL + outKL));
			s->aAR[s->countA] = (outIR - (outJR + outKR + outLR));
			s->aBR[s->countB] = (outJR - (outIR + outKR + outLR));
			s->aCR[s->countC] = (outKR - (outIR + outJR + outLR));
			s->aDR[s->countD] = (outLR - (outIR + outJR + outKR));
			
			s->countA++; if (s->countA < 0 || s->countA > s->delayA) s->countA = 0;
			s->countB++; if (s->countB < 0 || s->countB > s->delayB) s->countB = 0;
			s->countC++; if (s->countC < 0 || s->countC > s->delayC) s->countC = 0;
			s->countD++; if (s->countD < 0 || s->countD > s->delayD) s->countD = 0;
			
			float outAL = s->aAL[s->countA-((s->countA > s->delayA)?s->delayA+1:0)];
			float outBL = s->aBL[s->countB-((s->countB > s->delayB)?s->delayB+1:0)];
			float outCL = s->aCL[s->countC-((s->countC > s->delayC)?s->delayC+1:0)];
			float outDL = s->aDL[s->countD-((s->countD > s->delayD)?s->delayD+1:0)];
			float outAR = s->aAR[s->countA-((s->countA > s->delayA)?s->delayA+1:0)];
			float outBR = s->aBR[s->countB-((s->countB > s->delayB)?s->delayB+1:0)];
			float outCR = s->aCR[s->countC-((s->countC > s->delayC)?s->delayC+1:0)];
			float outDR = s->aDR[s->countD-((s->countD > s->delayD)?s->delayD+1:0)];
			//second block: four more outputs
			
			s->aEL[s->countE] = (outAL - (outBL + outCL + outDL));
			s->aFL[s->countF] = (outBL - (outAL + outCL + outDL));
			s->aGL[s->countG] = (outCL - (outAL + outBL + outDL));
			s->aHL[s->countH] = (outDL - (outAL + outBL + outCL));
			s->aER[s->countE] = (outAR - (outBR + outCR + outDR));
			s->aFR[s->countF] = (outBR - (outAR + outCR + outDR));
			s->aGR[s->countG] = (outCR - (outAR + outBR + outDR));
			s->aHR[s->countH] = (outDR - (outAR + outBR + outCR));
			
			s->countE++; if (s->countE < 0 || s->countE > s->delayE) s->countE = 0;
			s->countF++; if (s->countF < 0 || s->countF > s->delayF) s->countF = 0;
			s->countG++; if (s->countG < 0 || s->countG > s->delayG) s->countG = 0;
			s->countH++; if (s->countH < 0 || s->countH > s->delayH) s->countH = 0;
			
			float outEL = s->aEL[s->countE-((s->countE > s->delayE)?s->delayE+1:0)];
			float outFL = s->aFL[s->countF-((s->countF > s->delayF)?s->delayF+1:0)];
			float outGL = s->aGL[s->countG-((s->countG > s->delayG)?s->delayG+1:0)];
			float outHL = s->aHL[s->countH-((s->countH > s->delayH)?s->delayH+1:0)];
			float outER = s->aER[s->countE-((s->countE > s->delayE)?s->delayE+1:0)];
			float outFR = s->aFR[s->countF-((s->countF > s->delayF)?s->delayF+1:0)];
			float outGR = s->aGR[s->countG-((s->countG > s->delayG)?s->delayG+1:0)];
			float outHR = s->aHR[s->countH-((s->countH > s->delayH)?s->delayH+1:0)];
			//third block: final outputs
			
			s->feedbackAL = (outEL - (outFL + outGL + outHL));
			s->feedbackBL = (outFL - (outEL + outGL + outHL));
			s->feedbackCL = (outGL - (outEL + outFL + outHL));
			s->feedbackDL = (outHL - (outEL + outFL + outGL));
			s->feedbackAR = (outER - (outFR + outGR + outHR));
			s->feedbackBR = (outFR - (outER + outGR + outHR));
			s->feedbackCR = (outGR - (outER + outFR + outHR));
			s->feedbackDR = (outHR - (outER + outFR + outGR));
			//which we need to feed back into the input again, a bit
			
			inputSampleL = (outEL + outFL + outGL + outHL)/8.0f;
			inputSampleR = (outER + outFR + outGR + outHR)/8.0f;
			//and take the final combined sum of outputs
			
			s->bez[bez_CL] = s->bez[bez_BL];
			s->bez[bez_BL] = s->bez[bez_AL];
			s->bez[bez_AL] = inputSampleL;
			s->bez[bez_SampL] = 0.0f;
			
			s->bez[bez_CR] = s->bez[bez_BR];
			s->bez[bez_BR] = s->bez[bez_AR];
			s->bez[bez_AR] = inputSampleR;
			s->bez[bez_SampR] = 0.0f;
		}
		float CBL = (s->bez[bez_CL]*(1.0f-s->bez[bez_cycle]))+(s->bez[bez_BL]*s->bez[bez_cycle]);
		float CBR = (s->bez[bez_CR]*(1.0f-s->bez[bez_cycle]))+(s->bez[bez_BR]*s->bez[bez_cycle]);
		float BAL = (s->bez[bez_BL]*(1.0f-s->bez[bez_cycle]))+(s->bez[bez_AL]*s->bez[bez_cycle]);
		float BAR = (s->bez[bez_BR]*(1.0f-s->bez[bez_cycle]))+(s->bez[bez_AR]*s->bez[bez_cycle]);
		float CBAL = (s->bez[bez_BL]+(CBL*(1.0f-s->bez[bez_cycle]))+(BAL*s->bez[bez_cycle]))*0.125f;
		float CBAR = (s->bez[bez_BR]+(CBR*(1.0f-s->bez[bez_cycle]))+(BAR*s->bez[bez_cycle]))*0.125f;
		inputSampleL = CBAL;
		inputSampleR = CBAR;		
		
		s->iirBL = (s->iirBL*(1.0f-s->lowpass))+(inputSampleL*s->lowpass); inputSampleL = s->iirBL;
		s->iirBR = (s->iirBR*(1.0f-s->lowpass))+(inputSampleR*s->lowpass); inputSampleR = s->iirBR;
		//end filter
		
		if (s->wet < 1.0f) {
			inputSampleL = (inputSampleL * s->wet) + (drySampleL * (1.0f-s->wet));
			inputSampleR = (inputSampleR * s->wet) + (drySampleR * (1.0f-s->wet));
		}
		
		s->fpdL ^= s->fpdL<<13; s->fpdL ^= s->fpdL>>17; s->fpdL ^= s->fpdL<<5;
		*out1 = inputSampleL;
		*out2 = inputSampleR;

		
		
		
		
    }
