/**
 *  Early Reflection Class
 *
 *  Copyright (C) 2006-2018 Teru Kamogashira
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "freeverb/earlyref.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

const long FV3_(earlyref)::preset0_size = 18;
const fv3_float_t FV3_(earlyref)::preset0_delayL[] = { 0.0043f, 0.0215f, 0.0225f, 0.0268f, 0.0270f, 0.0298f, 0.0458f, 0.0485f, 0.0572f, 0.0587f, 0.0595f, 0.0612f, 0.0707f, 0.0708f, 0.0726f, 0.0741f, 0.0753f, 0.0797f, };
const fv3_float_t FV3_(earlyref)::preset0_delayR[] = { 0.0053f, 0.0225f, 0.0235f, 0.0278f, 0.0290f, 0.0288f, 0.0468f, 0.0475f, 0.0582f, 0.0577f, 0.0575f, 0.0622f, 0.0697f, 0.0718f, 0.0736f, 0.0751f, 0.0763f, 0.0817f, };
const fv3_float_t FV3_(earlyref)::preset0_gainL[] = { 0.841f, 0.504f, 0.491f, 0.379f, 0.380f, 0.346f, 0.289f, 0.272f, 0.192f, 0.193f, 0.217f, 0.181f, 0.180f, 0.181f, 0.176f, 0.142f, 0.167f, 0.134f, };
const fv3_float_t FV3_(earlyref)::preset0_gainR[] = { 0.842f, 0.506f, 0.489f, 0.382f, 0.300f, 0.346f, 0.290f, 0.271f, 0.193f, 0.192f, 0.217f, 0.195f, 0.192f, 0.166f, 0.186f, 0.131f, 0.168f, 0.133f, };

const long FV3_(earlyref)::preset1_size = 6;
const fv3_float_t FV3_(earlyref)::preset1_delayL[] = { 0.0199f, 0.0354f, 0.0389f, 0.0414f, 0.0699f, 0.0796f, };
const fv3_float_t FV3_(earlyref)::preset1_delayR[] = { 0.0209f, 0.0364f, 0.0399f, 0.0424f, 0.0709f, 0.0806f, };
const fv3_float_t FV3_(earlyref)::preset1_gainL[] = { 1.020f, 0.818f, 0.635f, 0.719f, 0.267f, 0.242f, };
const fv3_float_t FV3_(earlyref)::preset1_gainR[] = { 1.021f, 0.820f, 0.633f, 0.722f, 0.187f, 0.243f, };

const long FV3_(earlyref)::preset2_size = 4;
const fv3_float_t FV3_(earlyref)::preset2_delayL[] = { 0.0090f, 0.0118f, 0.0205f, 0.0213f, };
const fv3_float_t FV3_(earlyref)::preset2_delayR[] = { 0.0098f, 0.0145f, 0.0203f, 0.0230f, };
const fv3_float_t FV3_(earlyref)::preset2_gainL[] = { 1.35f, -1.15f, 1.15f, -1.14f, };
const fv3_float_t FV3_(earlyref)::preset2_gainR[] = { 1.36f, -1.16f, -1.00f, 1.14f, };

const long FV3_(earlyref)::preset11_sizeL = 11;
const long FV3_(earlyref)::preset11_sizeR = 11;
const fv3_float_t FV3_(earlyref)::preset11_delayL[] = { 0.003568f, 0.011703f, 0.019526f, 0.024870f, 0.037740f, 0.048089f, 0.053948f, 0.061333f, 0.061344f, 0.070073f, 0.077130f, };
const fv3_float_t FV3_(earlyref)::preset11_gainL[] = { 0.963333f, -0.806667f, 0.706667f, -0.656667f, -0.556667f, 0.526667f, -0.506667f, 0.503333f, 0.450000f, -0.470000f, 0.456667f, };
const fv3_float_t FV3_(earlyref)::preset11_delayR[] = { 0.002818f, 0.009115f, 0.017042f, 0.023885f, 0.033068f, 0.042307f, 0.051234f, 0.059896f, 0.067984f, 0.067995f, 0.076458f, };
const fv3_float_t FV3_(earlyref)::preset11_gainR[] = { 0.980000f,-0.850000f,0.726667f,-0.660000f,0.583333f,0.553333f,-0.503333f,0.486667f,-0.473333f,-0.466667f,0.463333f,};

const long FV3_(earlyref)::preset12_sizeL = 12;
const long FV3_(earlyref)::preset12_sizeR = 12;
const fv3_float_t FV3_(earlyref)::preset12_delayL[] = { 0.006344f, 0.012286f, 0.023385f, 0.028495f, 0.036385f, 0.045750f, 0.053427f, 0.059266f, 0.063281f, 0.063292f, 0.066437f, 0.066448f, };
const fv3_float_t FV3_(earlyref)::preset12_gainL[] = { 0.896667f, 0.796667f, -0.653333f, 0.623333f, -0.570000f, 0.530000f, -0.496667f, 0.483333f, 0.480000f, 0.406667f, -0.470000f, -0.470000f, };
const fv3_float_t FV3_(earlyref)::preset12_delayR[] = { 0.004568f, 0.011266f, 0.015687f, 0.030203f, 0.030214f, 0.038740f, 0.048172f, 0.054073f, 0.058870f, 0.062417f, 0.064510f, 0.064521f, };
const fv3_float_t FV3_(earlyref)::preset12_gainR[] = { 0.946667f,0.823333f,-0.753333f,0.593333f,0.600000f,0.556667f,0.530000f,-0.516667f,0.490000f,-0.480000f,0.476667f,0.410000f,};

const long FV3_(earlyref)::preset13_sizeL = 10;
const long FV3_(earlyref)::preset13_sizeR = 12;
const fv3_float_t FV3_(earlyref)::preset13_delayL[] = { 0.003568f, 0.011703f, 0.019526f, 0.024870f, 0.037740f, 0.048089f, 0.053948f, 0.061333f, 0.070073f, 0.077125f, };
const fv3_float_t FV3_(earlyref)::preset13_gainL[] = { 0.963333f, -0.813333f, 0.700000f, -0.650000f, -0.560000f, 0.526667f, -0.500000f, 0.486667f, -0.463333f, 0.463333f, };
const fv3_float_t FV3_(earlyref)::preset13_delayR[] = { 0.002818f, 0.009115f, 0.017042f, 0.023885f, 0.033062f, 0.033073f, 0.033120f, 0.042307f, 0.051234f, 0.059901f, 0.067990f, 0.076458f, };
const fv3_float_t FV3_(earlyref)::preset13_gainR[] = { 0.980000f,-0.856667f,0.723333f,-0.660000f,0.580000f,0.580000f,0.166667f,0.550000f,-0.506667f,0.476667f,-0.466667f,0.460000f,};

const long FV3_(earlyref)::preset14_sizeL = 11;
const long FV3_(earlyref)::preset14_sizeR = 13;
const fv3_float_t FV3_(earlyref)::preset14_delayL[] = { 0.002062f, 0.008031f, 0.023010f, 0.042016f, 0.055766f, 0.059432f, 0.064536f, 0.071240f, 0.076490f, 0.079250f, 0.079292f, };
const fv3_float_t FV3_(earlyref)::preset14_gainL[] = { 0.993333f, -0.873333f, 0.663333f, -0.543333f, 0.503333f, -0.486667f, 0.480000f, -0.466667f, 0.466667f, -0.463333f, -0.186667f, };
const fv3_float_t FV3_(earlyref)::preset14_delayR[] = { 0.001396f, 0.009427f, 0.023260f, 0.039740f, 0.039786f, 0.052427f, 0.052479f, 0.053552f, 0.062042f, 0.067380f, 0.067391f, 0.073630f, 0.076490f, };
const fv3_float_t FV3_(earlyref)::preset14_gainR[] = { 1.013333f,0.833333f,-0.660000f,0.546667f,0.190000f,0.506667f,0.150000f,-0.506667f,0.493333f,0.473333f,0.473333f,-0.463333f,0.473333f,};

const long FV3_(earlyref)::preset15_sizeL = 11;
const long FV3_(earlyref)::preset15_sizeR = 10;
const fv3_float_t FV3_(earlyref)::preset15_delayL[] = { 0.009531f, 0.014042f, 0.028557f, 0.037885f, 0.043745f, 0.051255f, 0.057661f, 0.066589f, 0.075375f, 0.084964f, 0.084974f, };
const fv3_float_t FV3_(earlyref)::preset15_gainL[] = { -0.636667f, 1.013333f, 0.770000f, 0.866667f, 0.983333f, 0.700000f, 0.786667f, 0.763333f, 0.796667f, 0.680000f, 0.690000f, };
const fv3_float_t FV3_(earlyref)::preset15_delayR[] = { 0.006198f, 0.016792f, 0.022510f, 0.033880f, 0.048922f, 0.056766f, 0.067510f, 0.074745f, 0.080651f, 0.083865f, };
const fv3_float_t FV3_(earlyref)::preset15_gainR[] = { 1.006667f,0.736667f,0.973333f,0.760000f,0.713333f,0.963333f,0.980000f,0.920000f,-0.670000f,0.870000f,};

const long FV3_(earlyref)::preset16_sizeL = 12;
const long FV3_(earlyref)::preset16_sizeR = 11;
const fv3_float_t FV3_(earlyref)::preset16_delayL[] = { 0.003021f, 0.008531f, 0.010703f, 0.012203f, 0.014682f, 0.018547f, 0.018604f, 0.025391f, 0.034026f, 0.038948f, 0.047807f, 0.047818f, };
const fv3_float_t FV3_(earlyref)::preset16_gainL[] = { 0.823333f, 0.773333f, -0.810000f, -0.873333f, 0.770000f, 0.723333f, 0.166667f, -0.766667f, -0.753333f, 0.656667f, 0.533333f, 0.533333f, };
const fv3_float_t FV3_(earlyref)::preset16_delayR[] = { 0.002526f, 0.005193f, 0.005255f, 0.006682f, 0.013500f, 0.016937f, 0.020109f, 0.022115f, 0.028370f, 0.036130f, 0.043328f, };
const fv3_float_t FV3_(earlyref)::preset16_gainR[] = { 0.963333f,0.883333f,0.170000f,0.736667f,-0.870000f,-0.790000f,-0.766667f,-0.763333f,0.750000f,0.653333f,0.520000f,};

const long FV3_(earlyref)::preset17_sizeL = 11;
const long FV3_(earlyref)::preset17_sizeR = 11;
const fv3_float_t FV3_(earlyref)::preset17_delayL[] = { 0.002964f, 0.003031f, 0.009786f, 0.010870f, 0.018380f, 0.019984f, 0.027745f, 0.043411f, 0.048651f, 0.053406f, 0.058391f, };
const fv3_float_t FV3_(earlyref)::preset17_gainL[] = { 0.913333f, 0.143333f, 0.626667f, 0.610000f, 0.800000f, 0.643333f, 0.773333f, -0.773333f, 0.630000f, -0.806667f, -0.683333f, };
const fv3_float_t FV3_(earlyref)::preset17_delayR[] = { 0.004193f, 0.009203f, 0.010198f, 0.010208f, 0.010266f, 0.015708f, 0.016917f, 0.030208f, 0.036193f, 0.042406f, 0.048135f, };
const fv3_float_t FV3_(earlyref)::preset17_gainR[] = { 0.976667f,0.726667f,0.716667f,0.716667f,0.143333f,-0.683333f,-0.730000f,-0.893333f,0.770000f,-0.856667f,-0.850000f,};

const long FV3_(earlyref)::preset18_sizeL = 11;
const long FV3_(earlyref)::preset18_sizeR = 13;
const fv3_float_t FV3_(earlyref)::preset18_delayL[] = { 0.004693f, 0.009786f, 0.012036f, 0.015625f, 0.019521f, 0.019531f, 0.024615f, 0.032896f, 0.040786f, 0.046208f, 0.050464f, };
const fv3_float_t FV3_(earlyref)::preset18_gainL[] = { 0.810000f, 0.853333f, -0.733333f, 1.023333f, 0.690000f, 0.683333f, 0.906667f, -0.903333f, 0.666667f, 0.726667f, 0.660000f, };
const fv3_float_t FV3_(earlyref)::preset18_delayR[] = { 0.004026f, 0.009510f, 0.014000f, 0.015312f, 0.019276f, 0.024344f, 0.024406f, 0.031292f, 0.031354f, 0.042391f, 0.045625f, 0.045693f, 0.050609f, };
const fv3_float_t FV3_(earlyref)::preset18_gainR[] = { 0.883333f,0.803333f,-0.876667f,0.823333f,0.836667f,0.853333f,0.166667f,-0.903333f,-0.173333f,0.783333f,0.880000f,0.140000f,0.790000f,};

const long FV3_(earlyref)::preset19_sizeL = 10;
const long FV3_(earlyref)::preset19_sizeR = 11;
const fv3_float_t FV3_(earlyref)::preset19_delayL[] = { 0.006031f, 0.019214f, 0.032687f, 0.041682f, 0.047917f, 0.055474f, 0.059266f, 0.064068f, 0.068786f, 0.075062f, };
const fv3_float_t FV3_(earlyref)::preset19_gainL[] = { 0.933333f, 0.833333f, 0.793333f, 0.783333f, 0.840000f, -0.793333f, 0.860000f, 0.716667f, -0.810000f, 0.733333f, };
const fv3_float_t FV3_(earlyref)::preset19_delayR[] = { 0.007031f, 0.012599f, 0.025599f, 0.034943f, 0.044250f, 0.050255f, 0.062771f, 0.066646f, 0.066656f, 0.071807f, 0.078354f, };
const fv3_float_t FV3_(earlyref)::preset19_gainR[] = { 0.706667f,0.873333f,0.846667f,0.846667f,-0.836667f,0.826667f,-0.860000f,0.780000f,0.776667f,-0.890000f,-0.836667f,};

const long FV3_(earlyref)::preset20_sizeL = 12;
const long FV3_(earlyref)::preset20_sizeR = 11;
const fv3_float_t FV3_(earlyref)::preset20_delayL[] = { 0.004292f, 0.013469f, 0.023667f, 0.029073f, 0.033005f, 0.044031f, 0.053677f, 0.060505f, 0.071667f, 0.071677f, 0.079833f, 0.079844f, };
const fv3_float_t FV3_(earlyref)::preset20_gainL[] = { 0.950000f, -0.786667f, 0.680000f, 0.620000f, -0.556667f, 0.520000f, 0.500000f, -0.476667f, 0.466667f, 0.470000f, -0.450000f, -0.456667f, };
const fv3_float_t FV3_(earlyref)::preset20_delayR[] = { 0.007917f, 0.014245f, 0.026464f, 0.030865f, 0.041781f, 0.052109f, 0.057672f, 0.065104f, 0.074745f, 0.082375f, 0.082385f, };
const fv3_float_t FV3_(earlyref)::preset20_gainR[] = { 0.863333f,0.736667f,-0.643333f,-0.593333f,0.536667f,0.523333f,-0.483333f,0.486667f,-0.463333f,0.450000f,0.453333f,};

const long FV3_(earlyref)::preset21_sizeL = 11;
const long FV3_(earlyref)::preset21_sizeR = 11;
const fv3_float_t FV3_(earlyref)::preset21_delayL[] = { 0.004474f, 0.014313f, 0.023130f, 0.030646f, 0.038906f, 0.048729f, 0.056156f, 0.064510f, 0.064521f, 0.072859f, 0.081219f, };
const fv3_float_t FV3_(earlyref)::preset21_gainL[] = { 0.950000f, 0.756667f, 0.680000f, 0.616667f, 0.563333f, 0.523333f, 0.493333f, 0.493333f, 0.446667f, 0.460000f, 0.456667f, };
const fv3_float_t FV3_(earlyref)::preset21_delayR[] = { 0.007729f, 0.007797f, 0.017010f, 0.025641f, 0.033901f, 0.040318f, 0.051432f, 0.058495f, 0.066859f, 0.075224f, 0.085417f, };
const fv3_float_t FV3_(earlyref)::preset21_gainR[] = { 0.893333f,0.146667f,0.733333f,0.656667f,0.576667f,0.543333f,0.510000f,0.503333f,0.466667f,0.470000f,0.480000f,};

const long FV3_(earlyref)::preset22_sizeL = 10;
const long FV3_(earlyref)::preset22_sizeR = 10;
const fv3_float_t FV3_(earlyref)::preset22_delayL[] = { 0.003276f, 0.010714f, 0.019526f, 0.024656f, 0.033474f, 0.037687f, 0.044661f, 0.052552f, 0.052599f, 0.069260f, };
const fv3_float_t FV3_(earlyref)::preset22_gainL[] = { 0.963333f, 0.810000f, 0.720000f, 0.640000f, 0.596667f, 0.540000f, 0.520000f, 0.493333f, 0.173333f, 0.473333f, };
const fv3_float_t FV3_(earlyref)::preset22_delayR[] =  { 0.003276f, 0.010714f, 0.019526f, 0.024656f, 0.033474f, 0.037682f, 0.044661f, 0.052552f, 0.058604f, 0.069266f, };
const fv3_float_t FV3_(earlyref)::preset22_gainR[] = { 0.970000f,0.803333f,0.720000f,0.646667f,0.596667f,0.543333f,0.523333f,0.503333f,0.483333f,0.470000f,};

FV3_(earlyref)::FV3_(earlyref)()
  
{
  tapLengthL = tapLengthR = 0;
  gainTableL = gainTableR = delayTableL = delayTableR = NULL;
  setdryr(0.8f); setwetr(0.5f); setwidth(0.2f);
  setLRDelay(0.3f);
  setLRCrossApFreq(750, 4);
  setDiffusionApFreq(150, 4);
  loadPresetReflection(FV3_EARLYREF_PRESET_DEFAULT);
  setoutputlpf(20000);
  setoutputhpf(4);
  mute();
}

FV3_(earlyref)::FV3_(~earlyref)()
{
  unloadReflection();
}

void FV3_(earlyref)::mute()
{
  FV3_(revbase)::mute();
  delayLineL.mute(); delayLineR.mute(); delayLtoR.mute(); delayRtoL.mute();
  allpassXL.mute(); allpassXR.mute(); allpassL2.mute(); allpassR2.mute();
  // DISC: reset the four output filter histories as well as delay memories.
  out1_lpf.mute(); out2_lpf.mute(); out1_hpf.mute(); out2_hpf.mute();
}

void FV3_(earlyref)::loadPresetReflection(long program)
{
  switch(program)
    {
    case FV3_EARLYREF_PRESET_11:
      currentPreset = FV3_EARLYREF_PRESET_11;
      loadReflection(preset11_delayL, preset11_gainL, preset11_delayR, preset11_gainR, preset11_sizeL, preset11_sizeR);
      break;
    case FV3_EARLYREF_PRESET_12:
      currentPreset = FV3_EARLYREF_PRESET_12;
      loadReflection(preset12_delayL, preset12_gainL, preset12_delayR, preset12_gainR, preset12_sizeL, preset12_sizeR);
      break;
    case FV3_EARLYREF_PRESET_13:
      currentPreset = FV3_EARLYREF_PRESET_13;
      loadReflection(preset13_delayL, preset13_gainL, preset13_delayR, preset13_gainR, preset13_sizeL, preset13_sizeR);
      break;
    case FV3_EARLYREF_PRESET_14:
      currentPreset = FV3_EARLYREF_PRESET_14;
      loadReflection(preset14_delayL, preset14_gainL, preset14_delayR, preset14_gainR, preset14_sizeL, preset14_sizeR);
      break;
    case FV3_EARLYREF_PRESET_15:
      currentPreset = FV3_EARLYREF_PRESET_15;
      loadReflection(preset15_delayL, preset15_gainL, preset15_delayR, preset15_gainR, preset15_sizeL, preset15_sizeR);
      break;
    case FV3_EARLYREF_PRESET_16:
      currentPreset = FV3_EARLYREF_PRESET_16;
      loadReflection(preset16_delayL, preset16_gainL, preset16_delayR, preset16_gainR, preset16_sizeL, preset16_sizeR);
      break;
    case FV3_EARLYREF_PRESET_17:
      currentPreset = FV3_EARLYREF_PRESET_17;
      loadReflection(preset17_delayL, preset17_gainL, preset17_delayR, preset17_gainR, preset17_sizeL, preset17_sizeR);
      break;
    case FV3_EARLYREF_PRESET_18:
      currentPreset = FV3_EARLYREF_PRESET_18;
      loadReflection(preset18_delayL, preset18_gainL, preset18_delayR, preset18_gainR, preset18_sizeL, preset18_sizeR);
      break;
    case FV3_EARLYREF_PRESET_19:
      currentPreset = FV3_EARLYREF_PRESET_19;
      loadReflection(preset19_delayL, preset19_gainL, preset19_delayR, preset19_gainR, preset19_sizeL, preset19_sizeR);
      break;
    case FV3_EARLYREF_PRESET_20:
      currentPreset = FV3_EARLYREF_PRESET_20;
      loadReflection(preset20_delayL, preset20_gainL, preset20_delayR, preset20_gainR, preset20_sizeL, preset20_sizeR);
      break;
    case FV3_EARLYREF_PRESET_21:
      currentPreset = FV3_EARLYREF_PRESET_21;
      loadReflection(preset21_delayL, preset21_gainL, preset21_delayR, preset21_gainR, preset21_sizeL, preset21_sizeR);
      break;
    case FV3_EARLYREF_PRESET_22:
      currentPreset = FV3_EARLYREF_PRESET_22;
      loadReflection(preset22_delayL, preset22_gainL, preset22_delayR, preset22_gainR, preset22_sizeL, preset22_sizeR);
      break;
      
    case FV3_EARLYREF_PRESET_1:
      currentPreset = FV3_EARLYREF_PRESET_1;
      loadReflection(preset1_delayL, preset1_gainL, preset1_delayR, preset1_gainR, preset1_size, preset1_size);
      break;
    case FV3_EARLYREF_PRESET_2:
      currentPreset = FV3_EARLYREF_PRESET_2;
      loadReflection(preset2_delayL, preset2_gainL, preset2_delayR, preset2_gainR, preset2_size, preset2_size);
      break;

    case FV3_EARLYREF_PRESET_0:
    default:
      currentPreset = FV3_EARLYREF_PRESET_0;
      loadReflection(preset0_delayL, preset0_gainL, preset0_delayR, preset0_gainR, preset0_size, preset0_size);
      break;
    }
}

long FV3_(earlyref)::getCurrentPreset()
{
  return currentPreset;
}

void FV3_(earlyref)::loadUserReflection(const fv3_float_t * delayL, const fv3_float_t * gainL, const fv3_float_t * delayR, const fv3_float_t * gainR, long sizeL, long sizeR)
  
{
  if(delayL == NULL||gainL == NULL||delayR == NULL||gainR == NULL||sizeL <= 0||sizeR <= 0) return;
  currentPreset = -1;
  loadReflection(delayL, gainL, delayR, gainR, sizeL, sizeR);
}

void FV3_(earlyref)::loadReflection(const fv3_float_t * delayL, const fv3_float_t * gainL, const fv3_float_t * delayR, const fv3_float_t * gainR, long sizeL, long sizeR)
  
{
  unloadReflection();
  if (true)
    {
      gainTableL = new fv3_float_t[sizeL];
      gainTableR = new fv3_float_t[sizeR];
      delayTableL = new fv3_float_t[sizeL];
      delayTableR = new fv3_float_t[sizeR];
    }
  else if (false)
    {
      std::fprintf(stderr, "earlyref::loadReflection() bad_alloc\n");
      delete[] gainTableL;
      delete[] gainTableR;
      delete[] delayTableL;
      delete[] delayTableR;
      disc_fv_oom();
    }
  tapLengthL = sizeL;
  tapLengthR = sizeR;
  for(long i = 0;i < sizeL;i ++)
    {
      gainTableL[i] = gainL[i];
      delayTableL[i] = getTotalFactorFs()*delayL[i];
    }
  for(long i = 0;i < sizeR;i ++)
    {
      gainTableR[i] = gainR[i];
      delayTableR[i] = getTotalFactorFs()*delayR[i];
    }
  long maxLengthL = (long)(maxDelay(delayTableL, tapLengthL)+10);
  long maxLengthR = (long)(maxDelay(delayTableR, tapLengthR)+10);
  delayLineL.setsize(maxLengthL);
  delayLineR.setsize(maxLengthR);
  mute();
}

fv3_float_t FV3_(earlyref)::maxDelay(const fv3_float_t * delaySet, long size)
{
  fv3_float_t max = 0;
  for(long i = 0;i < size;i ++)
    {
      if(max < delaySet[i]) max = delaySet[i];
    }
  return max;
}

void FV3_(earlyref)::unloadReflection()
{
  if(tapLengthL == 0||tapLengthR == 0) return;
  delete[] gainTableL;
  delete[] gainTableR;
  delete[] delayTableL;
  delete[] delayTableR;
  tapLengthL = tapLengthR = 0;
}

void FV3_(earlyref)::processreplace(fv3_float_t *inputL, fv3_float_t *inputR, fv3_float_t *outputL, fv3_float_t *outputR, long numsamples)
  
{
  if(numsamples <= 0) return;
  if(tapLengthL == 0||tapLengthR == 0) return;

  while(numsamples-- > 0)
    {
      *outputL = delayL(*inputL)*dry;
      *outputR = delayR(*inputR)*dry;
      fv3_float_t wetL = 0, wetR = 0;
      delayLineL.process(*inputL);
      delayLineR.process(*inputR);
      for(long i = 0;i < tapLengthL;i ++){ wetL += gainTableL[i]*delayLineL.at(delayTableL[i]); }
      for(long i = 0;i < tapLengthR;i ++){ wetR += gainTableR[i]*delayLineR.at(delayTableR[i]);	}
      // width = -1 ~ +1
      wetL = delayWL(wetL); wetR = delayWR(wetR);
      *outputL += out1_lpf(out1_hpf(allpassL2(wet1 * wetL + wet2 * allpassXL(delayRtoL(*inputR + wetR)))));
      *outputR += out2_lpf(out2_hpf(allpassR2(wet1 * wetR + wet2 * allpassXR(delayLtoR(*inputL + wetL)))));
      inputL ++; inputR ++; outputL ++; outputR ++;
    }
}

void FV3_(earlyref)::setLRDelay(fv3_float_t value_ms)
{
  lrDelay = (long)((fv3_float_t)currentfs*value_ms/1000.0f);
  delayRtoL.setsize(lrDelay);
  delayLtoR.setsize(lrDelay);
}

fv3_float_t FV3_(earlyref)::getLRDelay()
{
  return (fv3_float_t)lrDelay*1000.0f/(fv3_float_t)currentfs;
}

void FV3_(earlyref)::setLRCrossApFreq(fv3_float_t fc, fv3_float_t bw)
{
  lrCrossApFq = fc, lrCrossApBw = bw;
  allpassXL.setAPF_RBJ(fc, bw, currentfs, FV3_BIQUAD_RBJ_BW);
  allpassXR.setAPF_RBJ(fc, bw, currentfs, FV3_BIQUAD_RBJ_BW);
}

fv3_float_t FV3_(earlyref)::getLRCrossApFreq()
{
  return lrCrossApFq;
}

void FV3_(earlyref)::setDiffusionApFreq(fv3_float_t fc, fv3_float_t bw)
{
  diffApFq = fc, diffApBw = bw;
  allpassL2.setAPF_RBJ(fc, bw, currentfs, FV3_BIQUAD_RBJ_BW);
  allpassR2.setAPF_RBJ(fc, bw, currentfs, FV3_BIQUAD_RBJ_BW);
}

fv3_float_t FV3_(earlyref)::getDiffusionApFreq()
{
  return diffApFq;
}

void FV3_(earlyref)::setoutputlpf(fv3_float_t value)
{
  if(value < 0) value = 0;
  if(value > currentfs/2) value = currentfs/2;
  outputlpf = value;
  out1_lpf.setLPF_BW(outputlpf, currentfs);
  out2_lpf.setLPF_BW(outputlpf, currentfs);
}

fv3_float_t FV3_(earlyref)::getoutputlpf() const
{
  return outputlpf;
}

void FV3_(earlyref)::setoutputhpf(fv3_float_t value)
{
  if(value < 0) value = 0;
  if(value > currentfs/2) value = currentfs/2;
  outputhpf = value;
  out1_hpf.setHPF_BW(outputhpf, currentfs);
  out2_hpf.setHPF_BW(outputhpf, currentfs);
}

fv3_float_t FV3_(earlyref)::getoutputhpf() const
{
  return outputhpf;
}

void FV3_(earlyref)::setFsFactors()
{
  FV3_(revbase)::setFsFactors();
  setLRDelay(0.2f);
  setLRCrossApFreq(lrCrossApFq, lrCrossApBw);
  setDiffusionApFreq(diffApFq, diffApBw);
  loadPresetReflection(currentPreset);
}

#include "freeverb/fv3_ns_end.h"
