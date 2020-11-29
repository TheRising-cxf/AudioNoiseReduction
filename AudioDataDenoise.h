#pragma once
//WebRtc��Ҫ��ͷ�ļ�
#include "signal_processing_library.h"
#include "noise_suppression_x.h"
#include "noise_suppression.h"
#include "gain_control.h"
//rnnoise��Ҫ��ͷ�ļ�
#include "rnnoise.h"
#include "common.h"
//speex��Ҫ��ͷ�ļ�
#include <speex/speex_preprocess.h>
class AudioDataDenoise
{
public:
	AudioDataDenoise();
	~AudioDataDenoise();
	void Init();
	void InitWebRtc();
	void InitSpeex();
	void InitRnnoise();
	void DealWithWebRtc(short * audioData);
	void DealWithSpeex(short * audioData);
	void DealWithRnnoise(short * audioData);
private:
	DenoiseState* m_pRnnoise;
	NsHandle * m_pNS_inst;
	SpeexPreprocessState *st;
};

