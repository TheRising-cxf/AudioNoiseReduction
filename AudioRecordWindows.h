#ifndef _AUDIO_RECORD_WINDOWS_H_
#define _AUDIO_RECORD_WINDOWS_H_
#include <Windows.h>
#include <iostream>
#include <array>
#include <vector>
#pragma comment(lib,"winmm.lib")
#define CHANNEL_NUM  1                                           // ������
#define SAMPLE_RATE 48000                                      // ÿ�������
#define SAMPLE_BITS 16                                           // ����λ��
#define AUDIO_DATA_BLOCK_SIZE (480 * SAMPLE_BITS / 8 * 1)  // �������ݿ��С = ������*λ��/2*�루�ֽڣ�                                
#define BUFFER_NUM 10         
// ����������
//!
//! @brief ¼��״̬ö�� 
//! 
enum RecordStatus
{
	OpenDevice,
	RecordStart,
	RecordWriteData,
	RecordStop,
	CloseDevice
};
//!
//! @brief �ص�����
//! 
typedef void(*AudioRecordCallback)(std::array <char, AUDIO_DATA_BLOCK_SIZE> audioDataBlock, RecordStatus recordStatus);
namespace AudioRecordSpace
{
	//!
	//! @brief wav�ļ�ͷ
	//! 
	typedef struct WavHeader
	{
		char            riff[4];                    // = "RIFF"
		UINT32          size_8;                     // = FileSize - 8
		char            wave[4];                    // = "WAVE"
		char            fmt[4];                     // = "fmt "
		UINT32          fmt_size;                   // = PCMWAVEFORMAT�Ĵ�С : 
		//PCMWAVEFORMAT
		UINT16          format_tag;                 // = PCM : 1
		UINT16          channels;                   // = ͨ���� : 1
		UINT32          samples_per_sec;            // = ������ : 8000 | 6000 | 11025 | 16000
		UINT32          avg_bytes_per_sec;          // = ÿ��ƽ���ֽ��� : samples_per_sec * bits_per_sample / 8
		UINT16          block_align;                // = ÿ�������ֽ��� : wBitsPerSample / 8
		UINT16          bits_per_sample;            // = ��������: 8 | 16
		char            data[4];                    // = "data";
		//DATA
		UINT32          data_size;                  // = �����ݳ��� 
	};
	class AudioRecordWindows
	{
	public:
		AudioRecordWindows();
		virtual~AudioRecordWindows();
		//!
		//! @brief ��¼���豸
		//!
		bool OpenAudioDevice();
		//!
		//! @brief �ر�¼���豸
		//!
		void CloseAudioDevice();
		//!
		//! @brief ��ʼ��¼������
		//!
		void InitWaveFormat(LPWAVEFORMATEX WaveFormat, WORD ChannelNum, DWORD SampleRate, WORD BitsPerSample);
		//!
		//! @brief ��ʼ¼��
		//!
		void StartRecord();
		//!
		//! @brief ֹͣ¼��
		//!
		void StopRecord();
		//!
		//! @brief ����¼��
		//!
		void ResetRecord();
		//!
		//! @brief ������Ҫ¼��wav�ļ�����
		//!
		void SetWavFileName(const char* filePath);
		//!
		//! @brief ������Ҫ¼��wav�ļ�����
		//!
		void SetPcmFileName(const char* filePath);
		//!
		//! @brief д¼��wav�ļ�
		//!
		void WriteWavFile();
		//!
		//! @brief д¼��pcm�ļ�
		//!
		void WritePcmFile();
		//!
		//! @brief ע��ص�����
		//!
		void RegisterCallback(AudioRecordCallback audioCallback);
		//!
		//! @brief ϵͳ¼���ص�����
		//!
		static DWORD(CALLBACK WaveAPI_Callback)(        // WaveAPI�ص�����
			HWAVEIN hWaveIn,                            //  �����豸
			UINT uMsg,                                  //  ��Ϣ
			DWORD_PTR dwInstance,                       //  ����
			DWORD_PTR dwParam1,                         //  �����õĻ�����ָ��
			DWORD_PTR dwParam2                          //  ����
			);
	private:
		HWAVEIN m_AudioDevice;                                                   // ��Ƶ�����豸
		WAVEHDR m_WaveHdrBuffer[BUFFER_NUM];                                     // ����������
		static std::array<char, AUDIO_DATA_BLOCK_SIZE> m_AudioDataBlock;       // ��ǰ¼��һ֡��Ƶ����
		static std::vector<std::array<char, AUDIO_DATA_BLOCK_SIZE>> m_AudioData; // �洢����¼�Ƶ���Ƶ����
		static bool m_bStopRecord;                                               // �Ƿ�ֹͣ¼��
		static bool m_bPushData;                                                 // �Ƿ���m_AudioDataBlock��push����
		std::string m_WavFilePath;                                               // Wav¼���ļ�����
		std::string m_PcmFilePath;                                               // Pcm¼���ļ�����
		bool m_bSaveWavFile;                                                     // �Ƿ񱣴�wav�ļ�
		FILE* m_WavFileOpen;                                                     // ¼��wav�ļ�ָ��
		bool m_bSavePcmFile;                                                     // �Ƿ񱣴�Pcm�ļ�
		FILE* m_PcmFileOpen;                                                     // ¼��Pcm�ļ�ָ��
		WavHeader m_WavHeader;                                                   // wav�ļ�ͷ
		static AudioRecordCallback m_Callback;                                   // �ⲿ�ص�����
		static bool m_bCallback;                                                 // �Ƿ�ص��ⲿ�ص�����
	};
}
#endif // !_AUDIO_RECORD_WINDOWS_H_