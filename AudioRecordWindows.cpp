#include "AudioRecordWindows.h"
#include "rnnoise.h"
#include <string.h>
#include <speex/speex_preprocess.h>
namespace AudioRecordSpace
{
	// ��̬������ʼ��
	int nRet = 0;
	FILE *fpIn = NULL;
	FILE *fpOut = NULL;

	char *pInBuffer = NULL;
	char *pOutBuffer = NULL;

	
	AudioDataDenoise m_audioDataDenoise;
	//SpeexPreprocessState *state = speex_preprocess_state_init(480, SAMPLE_RATE);

	std::array <char, AUDIO_DATA_BLOCK_SIZE> AudioRecordWindows::m_AudioDataBlock = {};
	std::vector<std::array<char, AUDIO_DATA_BLOCK_SIZE>> AudioRecordWindows::m_AudioData = { {} };
	bool AudioRecordWindows::m_bStopRecord = false;
	bool AudioRecordWindows::m_bPushData = true;
	bool AudioRecordWindows::m_bCallback = false;;
	AudioRecordCallback AudioRecordWindows::m_Callback = NULL;
	AudioRecordWindows::AudioRecordWindows()
	{
		/*int denoise = 1;

		int noiseSuppress = -25;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_DENOISE, &denoise);

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noiseSuppress);



		int i;

		i = 0;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_AGC, &i);

		i = 80000;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_AGC_LEVEL, &i);

		i = 0;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_DEREVERB, &i);

		float f = 0;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);

		f = 0;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);
*/
		m_audioDataDenoise.Init();
		m_WavFileOpen = NULL;
		m_PcmFileOpen = NULL;
		m_bSaveWavFile = false;
		m_bSavePcmFile = false;
		m_WavFilePath = "";
		m_PcmFilePath = "";
		m_WavHeader =
		{
		{ 'R', 'I', 'F', 'F' },

		0,
		{ 'W', 'A', 'V', 'E' },
		{ 'f', 'm', 't', ' ' },
		sizeof(PCMWAVEFORMAT) ,
		WAVE_FORMAT_PCM,
		1,
		SAMPLE_RATE,
		SAMPLE_RATE*(SAMPLE_BITS / 8),
		SAMPLE_BITS / 8,
		SAMPLE_BITS,
		{ 'd', 'a', 't', 'a' },
		0
		};
	}
	AudioRecordWindows::~AudioRecordWindows()
	{
	}
	bool AudioRecordWindows::OpenAudioDevice()
	{
		int audioDeviceNum = waveInGetNumDevs();
		if (audioDeviceNum <= 0)
		{
			std::cout << "Windowsû���ҵ�¼���豸����ȷ��Windows�ҵ���¼���豸" << std::endl;
			return false;
		}
		else
		{
			for (unsigned int i = 0; i < audioDeviceNum; ++i)
			{
				WAVEINCAPS waveInCaps;
				MMRESULT mmResult = waveInGetDevCaps(i, &waveInCaps, sizeof(WAVEINCAPS));
				if (i == 0)
				{
					std::cout << "��ǰĬ�ϵ�¼���豸��Ϣ������" << waveInCaps.szPname << std::endl;
				}
				else
				{
					std::cout << "����¼���豸��Ϣ����" << waveInCaps.szPname << std::endl;
				}
			}
		}
		WAVEFORMATEX waveFormate;
		InitWaveFormat(&waveFormate, CHANNEL_NUM, SAMPLE_RATE, SAMPLE_BITS);
		waveInOpen(&m_AudioDevice, WAVE_MAPPER, &waveFormate, (DWORD_PTR)WaveAPI_Callback, DWORD_PTR(this), CALLBACK_FUNCTION);
		if (m_bCallback)
		{
			m_Callback(m_AudioDataBlock, RecordStatus::OpenDevice);
		}
		return true;
	}
	void AudioRecordWindows::CloseAudioDevice()
	{
		if (m_bCallback)
		{
			m_Callback(m_AudioDataBlock, RecordStatus::CloseDevice);
		}
		waveInClose(m_AudioDevice);
	}
	void AudioRecordWindows::InitWaveFormat(LPWAVEFORMATEX WaveFormat, WORD ChannelNum, DWORD SampleRate, WORD BitsPerSample)
	{
		// ������Ƶ���β���
		WaveFormat->wFormatTag = WAVE_FORMAT_PCM;
		WaveFormat->nChannels = ChannelNum;
		WaveFormat->nSamplesPerSec = SampleRate;
		WaveFormat->nAvgBytesPerSec = SampleRate * ChannelNum * BitsPerSample / 8;
		WaveFormat->nBlockAlign = ChannelNum * BitsPerSample / 8;
		WaveFormat->wBitsPerSample = BitsPerSample;
		WaveFormat->cbSize = 0;
		std::cout << "����������" << std::endl;
		std::cout << "������" << ChannelNum << std::endl;
		std::cout << "������" << SampleRate << "Hz" << std::endl;
		std::cout << "λ��" << BitsPerSample << std::endl;
	}
	void AudioRecordWindows::StartRecord()
	{
		for (unsigned int i = 0; i < BUFFER_NUM; ++i)
		{
			m_WaveHdrBuffer[i].lpData = new char[AUDIO_DATA_BLOCK_SIZE];
			m_WaveHdrBuffer[i].dwBufferLength = AUDIO_DATA_BLOCK_SIZE;
			m_WaveHdrBuffer[i].dwBytesRecorded = 0;
			m_WaveHdrBuffer[i].dwUser = i;
			m_WaveHdrBuffer[i].dwFlags = 0;
			m_WaveHdrBuffer[i].dwLoops = 0;
			m_WaveHdrBuffer[i].lpNext = NULL;
			m_WaveHdrBuffer[i].reserved = 0;
			// �Ž�������
			waveInPrepareHeader(m_AudioDevice, &m_WaveHdrBuffer[i], sizeof(WAVEHDR));
			waveInAddBuffer(m_AudioDevice, &m_WaveHdrBuffer[i], sizeof(WAVEHDR));
		}
		// �����Ƶ����
		m_AudioData.clear();
		m_AudioData.shrink_to_fit();
		m_AudioData.resize(0);
		// ��ʼ¼��
		waveInStart(m_AudioDevice);
		if (m_bCallback)
		{
			m_Callback(m_AudioDataBlock, RecordStatus::RecordStart);
		}
	}
	void AudioRecordWindows::StopRecord()
	{
		m_bStopRecord = true;
		// ֹͣ¼���豸
		waveInStop(m_AudioDevice);
		waveInReset(m_AudioDevice);
		if (m_bCallback)
		{
			m_Callback(m_AudioDataBlock, RecordStatus::RecordStop);
		}
		// �ͷŻ�����
		for (unsigned int i = 0; i < BUFFER_NUM; ++i)
		{
			waveInUnprepareHeader(m_AudioDevice, &m_WaveHdrBuffer[i], sizeof(WAVEHDR));
			delete m_WaveHdrBuffer[i].lpData;
			m_WaveHdrBuffer[i].lpData = NULL;
		}
		// ����wav�ļ�
		if (m_bSaveWavFile)
		{
			WriteWavFile();
		}
		// ����pcm�ļ�
		if (m_bSavePcmFile)
		{
			WritePcmFile();
		}
	}
	void AudioRecordWindows::ResetRecord()
	{
		m_AudioData.clear();
		m_AudioData.shrink_to_fit();
		m_bSaveWavFile = false;
		m_bSavePcmFile = false;
		m_bPushData = true;
		m_bStopRecord = false;
		m_bCallback = false;
		m_Callback = NULL;
		m_WavFileOpen = NULL;
		m_PcmFileOpen = NULL;
		m_WavFilePath = "";
		m_PcmFilePath = "";
	}
	void AudioRecordWindows::SetWavFileName(const char * filePath)
	{
		m_bSaveWavFile = true;
		m_WavFilePath = filePath;
		// ���Դ��ļ��������ļ�
		errno_t err = fopen_s(&m_WavFileOpen, m_WavFilePath.c_str(), "wb");
		if (err > 0)
		{
			std::cout << "�ļ�����ʧ�ܣ�" << err << " ����ļ�����ռ��" << std::endl;
			m_bSaveWavFile = false;
		}
	}
	void AudioRecordWindows::SetPcmFileName(const char * filePath)
	{
		m_bSavePcmFile = true;
		m_PcmFilePath = filePath;
		// ���Դ��ļ��������ļ�
		errno_t err = fopen_s(&m_PcmFileOpen, m_PcmFilePath.c_str(), "wb");
		if (err > 0)
		{
			std::cout << "�ļ�����ʧ�ܣ�" << err << " ����ļ�����ռ��" << std::endl;
			m_bSavePcmFile = false;
		}
	}
	void AudioRecordWindows::WriteWavFile()
	{
		// �༭��д��Waveͷ��Ϣ
		m_WavHeader.data_size = AUDIO_DATA_BLOCK_SIZE * m_AudioData.size();
		m_WavHeader.size_8 = m_WavHeader.data_size + 32;
		fwrite(&m_WavHeader, sizeof(m_WavHeader), 1, m_WavFileOpen);
		// ׷��RawData
		fwrite(m_AudioData.data(), AUDIO_DATA_BLOCK_SIZE * m_AudioData.size(), 1, m_WavFileOpen);
		// д�����
		fclose(m_WavFileOpen);
	}
	void AudioRecordWindows::WritePcmFile()
	{
		// ׷��RawData
		fwrite(m_AudioData.data(), AUDIO_DATA_BLOCK_SIZE * m_AudioData.size(), 1, m_PcmFileOpen);
		// д�����
		fclose(m_PcmFileOpen);
	}
	void AudioRecordWindows::RegisterCallback(AudioRecordCallback audioCallback)
	{
		m_bCallback = true;
		m_Callback = audioCallback;
	}
	DWORD(AudioRecordWindows::WaveAPI_Callback)(HWAVEIN hWaveIn, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
	{
		// ��Ϣswitch
		switch (uMsg)
		{
		case WIM_OPEN:      // �豸�ɹ��Ѵ�
			std::cout << "�豸�ɹ���" << std::endl;
			break;
		case WIM_DATA:      // ����������������
		// ֹͣ���Ƶ������WIM_DATA,�Ѿ�������ת�����Բ�����������ݡ�����������������������ظ��ġ�
			if (m_bPushData)
			{
				//std::cout << "����������������" << std::endl;
				// �ѻ��������ݿ�������
				memcpy(m_AudioDataBlock.data(), ((LPWAVEHDR)dwParam1)->lpData, AUDIO_DATA_BLOCK_SIZE);
				// û��¼��ȥ�ı����Ϊ0xcd,�ĳ�0������ĩβ���ֱ�����ֻ�ڽ���¼��ʱ���У���Ӱ����ӻ���Ч�ʡ�
				if (((LPWAVEHDR)dwParam1)->dwBytesRecorded < AUDIO_DATA_BLOCK_SIZE)
				{
					for (size_t i = ((LPWAVEHDR)dwParam1)->dwBytesRecorded; i < AUDIO_DATA_BLOCK_SIZE; i++)
					{
						m_AudioDataBlock.at(i) = 0;
					}
				}


				//m_audioDataDenoise.DealWithWebRtc((short*)m_AudioDataBlock.data());
				m_audioDataDenoise.DealWithRnnoise((short*)m_AudioDataBlock.data());

				//speex_preprocess_run(state, (spx_int16_t*)(m_AudioDataBlock.data()));


				// �����һ֡
				m_AudioData.push_back(m_AudioDataBlock);
				// ����������˻ص�����
				if (m_bCallback)
				{
					m_Callback(m_AudioDataBlock, RecordStatus::RecordWriteData);
				}
			}
			// �����Ҫֹͣ¼���򲻼�����ӻ���
			if (!m_bStopRecord)
			{
				waveInAddBuffer(hWaveIn, (LPWAVEHDR)dwParam1, sizeof(WAVEHDR));//��ӵ�������
			}
			else
			{
				// ����Ѿ�ֹͣ��¼�ƣ��Ͳ�Ҫ��д������
				m_bPushData = false;
			}
			break;
		case WIM_CLOSE:
			// �����ɹ����
			std::cout << "¼���豸�Ѿ��ر�..." << std::endl;
			break;
		default:
			break;
		}
		return 0;
	}
}