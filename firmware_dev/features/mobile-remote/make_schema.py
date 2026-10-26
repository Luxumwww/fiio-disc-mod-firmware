from pathlib import Path
import json
P=Path(__file__).resolve().parent
def param(key,label,lo,hi,step,default,unit=''):
 return dict(key=key,label=label,min=lo,max=hi,step=step,default=default,unit=unit)
effects=[
 (0,'hall','音乐厅氛围',[param('dry','原声',0,2,.01,.85),param('wet','混响',0,2,.01,.75),param('room','空间大小',.05,.98,.01,.9),param('damping','高频阻尼',0,1,.01,.4),param('width','立体宽度',0,1,.01,.75)]),
 (2,'limiter','压限器',[param('threshold','阈值',-36,-1,.5,-15,'dB'),param('ratio','压缩比',1,12,.1,3),param('attack','启动时间',1,100,1,10,'ms'),param('release','恢复时间',20,1000,10,200,'ms'),param('makeup','补偿增益',-12,12,.5,0,'dB'),param('mix','混合比例',0,1,.01,1)]),
 (3,'tube','仿电子管',[param('drive','饱和程度',1,6,.1,3.2),param('bias','谐波偏置',0,.8,.01,.4),param('tone','高频柔化',2000,18000,100,7000,'Hz'),param('color','染色比例',0,1,.01,.75),param('output','输出增益',.5,2,.01,1.4)]),
 (4,'v8','V8 混响',[param('decay','尾音时长',.25,5,.05,1.1,'s'),param('damping','高频截止',1000,16000,100,6000,'Hz'),param('diffusion','扩散程度',.1,.8,.01,.55),param('width','立体宽度',0,2,.01,1),param('dry','原声比例',0,1,.01,0),param('wet','混响强度',0,3,.01,2.04)]),
 (5,'warm','PurestWarm3 暖声',[param('color','暖声程度',0,1.5,.01,.5),param('mix','混合比例',0,1,.01,1),param('output','输出增益',.5,1.5,.01,1)]),
 (6,'dragonfly','Dragonfly 大厅',[param('room','大厅规模',.5,4,.1,3.2),param('decay','尾音时长',.3,5,.1,1.6,'s'),param('predelay','反射延时',0,100,1,12,'ms'),param('damping','高频截止',2000,16000,100,10000,'Hz'),param('width','立体宽度',0,1,.01,1),param('early','早期反射',0,1,.01,.30),param('late','后期混响',0,1.5,.01,.85),param('dry','原声比例',0,1,.01,.25)]),
 (7,'galactic','Galactic3 银河',[param('decay','尾音收敛',.05,1,.01,.65),param('brightness','明亮度',.2,1,.01,.7),param('modulation','空间流动',0,.6,.01,.15),param('density','细节密度',.5,1,.01,1),param('size','空间大小',.05,1,.01,.55),param('mix','混响比例',0,1,.01,.7)])]
schema={'version':1,'effects':[dict(index=i,key=k,name=n,params=params) for i,k,n,params in effects]}
P.joinpath('dsp-schema.json').write_text(json.dumps(schema,ensure_ascii=False,indent=2),encoding='utf-8');P.joinpath('server/schema.json').write_text(json.dumps(schema,ensure_ascii=False),encoding='utf-8')
rows=[];ranges=[];counts=[]
for i in range(8):
 values=next((params for idx,_,_,params in effects if idx==i),[]);counts.append(len(values))
 rows.append('{'+','.join([str(float(v['default']))+'f' for v in values]+['0.f']*(12-len(values)))+'}')
 ranges.append('{'+','.join(['{'+str(float(v['min']))+'f,'+str(float(v['max']))+'f}' for v in values]+['{0.f,0.f}']*(12-len(values)))+'}')
header='#ifndef DISC_DSP_PARAMETERS_H\n#define DISC_DSP_PARAMETERS_H\n#include <math.h>\nstatic const unsigned disc_param_count[8]={'+','.join(map(str,counts))+'};\nstatic const float disc_param_default[8][12]={'+','.join(rows)+'};\nstatic const float disc_param_range[8][12][2]={'+','.join(ranges)+'};\nstatic int disc_params_valid(const float p[8][12]){for(unsigned i=0;i<8;i++)for(unsigned j=0;j<12;j++)if(!isfinite(p[i][j])||p[i][j]<disc_param_range[i][j][0]||p[i][j]>disc_param_range[i][j][1])return 0;return 1;}\n#endif\n'
P.parent.joinpath('audio-dsp/parameters.h').write_text(header,newline='\n');print('schema parameters',sum(counts))
