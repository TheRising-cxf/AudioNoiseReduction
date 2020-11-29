#include "AudioDataDenoise.h"
AudioDataDenoise::AudioDataDenoise()
{
}


AudioDataDenoise::~AudioDataDenoise()
{
	rnnoise_destroy(m_pRnnoise);
	WebRtcNs_Free(m_pNS_inst);
	speex_preprocess_state_destroy(st);
}

void AudioDataDenoise::Init()
{
	InitWebRtc();
	InitSpeex();
	InitRnnoise();
}

void AudioDataDenoise::InitWebRtc()
{
	m_pNS_inst = NULL;
	WebRtcNs_Create(&m_pNS_inst);
	WebRtcNs_Init(m_pNS_inst, 32000);
	WebRtcNs_set_policy(m_pNS_inst, 1);

}

void AudioDataDenoise::InitSpeex()
{
	st = speex_preprocess_state_init(FRAME_SIZE, SAMPLE_RATE);
	int i = 1;
	speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DENOISE, &i);
	i = 0;
	speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC, &i);
	i = 8000;
	speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC_LEVEL, &i);
	i = 0;
	speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB, &i);
	float f = .0;
	speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
	f = .0;
	speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);
}


void AudioDataDenoise::InitRnnoise()
{
	m_pRnnoise = rnnoise_create(NULL);
}

void AudioDataDenoise::DealWithWebRtc(short * audioData)
{
	short shBufferOut[FRAME_SIZE] = { 0 };
	short data[FRAME_SIZE] = { 0 };
	short shInL[160] = { 0 }, shInH[160] = { 0 };
	short shOutL[160] = { 0 }, shOutH[160] = { 0 };
	static int  filter_state1[6] = { 0 }, filter_state12[6] = { 0 };
	static int  Synthesis_state1[6] = { 0 }, Synthesis_state12[6] = { 0 };

	short * tmp = (short*)audioData;
	for (int n = 0; n < CHANNEL_NUM; n++) {
		for (int j = 0; j < FRAME_SIZE; j++) data[j] = tmp[j*CHANNEL_NUM + n];
		//������Ҫʹ���˲���������Ƶ���ݷָߵ�Ƶ���Ը�Ƶ�͵�Ƶ�ķ�ʽ���뽵�뺯���ڲ�
		WebRtcSpl_AnalysisQMF(data, FRAME_SIZE, shInL, shInH, filter_state1, filter_state12);

		//����Ҫ����������Ը�Ƶ�͵�Ƶ�����Ӧ�ӿڣ�ͬʱ��Ҫע�ⷵ������Ҳ�Ƿָ�Ƶ�͵�Ƶ
		if (0 == WebRtcNs_Process(m_pNS_inst, shInL, shInH, shOutL, shOutH))
		{
			//�������ɹ�������ݽ�����Ƶ�͵�Ƶ���ݴ����˲��ӿڣ�Ȼ���ý����ص�����д���ļ�
			WebRtcSpl_SynthesisQMF(shOutL, shOutH, FRAME_SIZE / 2, shBufferOut, Synthesis_state1, Synthesis_state12);
		}
		for (int j = 0; j < FRAME_SIZE; j++)  tmp[j*CHANNEL_NUM + n] = shBufferOut[j];
	}
}

void AudioDataDenoise::DealWithSpeex(short * audioData)
{
	speex_preprocess_run(st, (spx_int16_t*)(audioData));
}

void AudioDataDenoise::DealWithRnnoise(short * audioData)
{
	float data[FRAME_SIZE];
	short * tmp = (short*)audioData;
	for (int i = 0; i < CHANNEL_NUM; i++) {
		for (int j = 0; j < FRAME_SIZE; j++) data[j] = tmp[j*CHANNEL_NUM + i];
		rnnoise_process_frame(m_pRnnoise, data, data,1);
		for (int j = 0; j < FRAME_SIZE; j++)  tmp[j*CHANNEL_NUM + i] = (short)data[j];
	}
}
