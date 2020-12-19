#ifndef _DS_IMA_ADPCM_H
#define _DS_IMA_ADPCM_H

#include <stdio.h>
#include "cstream_fs.h"
#include "soundTGDS.h"
#include "fatfslayerTGDS.h"

#define ADPCM_SIZE (int)(2048)		//TGDS IMA-ADPCM buffer size
typedef bool (*closeSoundHandle)();	//ret: true = closed sound stream. false = couldn't close sound stream

enum
{
	state_beginblock,
	state_decode
};

enum WAV_FORMAT{	
	WAV_FORMAT_PCM			= 0x1,
	WAV_FORMAT_IMA_ADPCM	= 0x11
};

enum {
	IMA_ADPCM_OKAY,
	IMA_ADPCM_ERROR_CANNOTOPEN,
	IMA_ADPCM_ERROR_NOTRIFFWAVE,
	IMA_ADPCM_ERROR_UNSUPPORTED,
	IMA_ADPCM_ERROR_INVALIDCHANNELS
};

//**********************************************************************************************
//**********************************************************************************************
//**********************************************************************************************

typedef struct t_IMA_Adpcm_Data {

	int curSamps;
	int data1, step1, samp1;
	int data2, step2, samp2;
} IMA_Adpcm_Data;

//**********************************************************************************************
//**********************************************************************************************
//**********************************************************************************************

#ifdef __cplusplus

class IMA_Adpcm_Stream
{
private:
	IMA_Adpcm_Data		data;
	IMA_Adpcm_Data		loop_data;
	
	u8*		datacache;
	u8*		loop_src;
	int		loop_cblock;
	int		loop_state;
	int		loop_br;
	int		loop_oddnibble;
	int		wave_data;
	int		wave_end;
	u8		*srcb;
	u8		*src;
	int		oddnibble;
	int		block;
	int		blockremain;
	int		loop1, loop2;
	int		channels;
	int		position;
	int		currentblock;
	int		state;
	int		format;
	int		sampBits;
	int		sampling_rate;
	int		wave_loop;
	int		is_processing;
	
	void	open( const void* );
	void	skip( int );
	void	seek( int );
	int		tell();
	int		fget8();
	int		fget16();
	u32		fget32();
	void	getblock();
	int		get8();
	int		get16();
	u32		get32();
	
	void	capture_frame();
	void	restore_frame();
public:
	
	IMA_Adpcm_Stream();
	~IMA_Adpcm_Stream();
	CStreamFS *pCStreamFS;	//audio mp2 stream	//FILE	*fin;
	int reset(bool loop );
	int stream( s16 *target, int length, int frmt );
	void close();
	int		stream_pcm( s16 *target, int length );
	int		decode_ima( s16 *target, int length );
	int get_channels();
	int get_sampling_rate();
	int get_mm_format();
	closeSoundHandle closeCb;
};

//**********************************************************************************************
//**********************************************************************************************
//**********************************************************************************************

class IMA_Adpcm_Player {

	IMA_Adpcm_Stream stream;
	bool autofill;
	bool paused;
	bool active;
	int targetBufferDecodeSize;
public:
	wavFormatChunk headerChunk;
	IMA_Adpcm_Player();
	int play( struct fd *fdInst, bool loop_audio, bool automatic_updates, int buffer_length = ADPCM_SIZE, closeSoundHandle = NULL);
	void pause();
	void resume();
	void stop();
	bool isactive();
	bool ispaused();
	int i_stream_request( int length, void * dest, int frmt );
	void settargetBufferDecodeSize( int bufSize );
	int gettargetBufferDecodeSize();
	IMA_Adpcm_Stream * getStream();
	// update function for manual filling
	void update();
};
#endif

#endif

#ifdef __cplusplus
extern "C" {
extern IMA_Adpcm_Player player;
extern IMA_Adpcm_Player *active_player;
extern IMA_Adpcm_Stream * strm;
#endif

extern int ADPCMchunksize;
extern int ADPCMchannels;


#ifdef __cplusplus
}
#endif
