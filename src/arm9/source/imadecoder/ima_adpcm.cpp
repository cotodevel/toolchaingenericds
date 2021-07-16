/***************************
 * ima-adpcm decoder by Discostew
 ***************************/
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "ima_adpcm.h"
#include "soundTGDS.h"
#include "consoleTGDS.h"

#define seek(x) fseek( fin, (x), SEEK_SET )
#define skip(x) fseek( fin, (x), SEEK_CUR )
#define tell() ftell(fin)

IMA_Adpcm_Player player;	//Actual PLAYER Instance. See ima_adpcm.cpp -> [PLAYER: section
int ADPCMchunksize = 0;
int ADPCMchannels = 0;

extern "C" {
	void decode_mono_ima_adpcm( IMA_Adpcm_Data *data, u8 *src, s16 *dest, int iterations );
	void decode_stereo_ima_adpcm( IMA_Adpcm_Data *data, u8 *src, s16 *dest, int iterations );
}

IMA_Adpcm_Stream::IMA_Adpcm_Stream()	__attribute__ ((optnone))
{
	
}

IMA_Adpcm_Stream::~IMA_Adpcm_Stream()	__attribute__ ((optnone))
{
	
}

int IMA_Adpcm_Stream::reset( FILE * audioFileHandle, bool loop )	__attribute__ ((optnone))
{
	fin = audioFileHandle;
	close();
	if( !fin )
		return IMA_ADPCM_ERROR_CANNOTOPEN;
	if( fget32() != 0x46464952 )		// "RIFF"
	{
		return IMA_ADPCM_ERROR_NOTRIFFWAVE;
	}
	int size = fget32();
	if( fget32() != 0x45564157 )		// "WAVE"
	{
		return IMA_ADPCM_ERROR_NOTRIFFWAVE;
	}
	
	// parse WAV structure
	while( tell() < size )
	{
		u32 code = fget32();
		u32 csize = fget32();
		switch( code )
		{
		case 0x20746D66: 	// format chunk
			
			// catch invalid format
			format = fget16();
			if(( format != WAV_FORMAT_PCM ) && ( format != WAV_FORMAT_IMA_ADPCM ))
			{
				return IMA_ADPCM_ERROR_UNSUPPORTED;
			}
			
			channels = fget16();
			
			// catch invalid channels
			if(( channels < 1 ) || ( channels > 2 ))
			{
				return IMA_ADPCM_ERROR_INVALIDCHANNELS;
			}
				
			sampling_rate = fget32();// sample rate
			skip( 4 );	// avg bytes/second
			block = fget16();
			
			sampBits = fget16();

			if((( format == WAV_FORMAT_PCM ) && (( sampBits != 8 ) && ( sampBits != 16 ))) ||
				(( format == WAV_FORMAT_IMA_ADPCM ) && ( sampBits != 4 )))
			{
				return IMA_ADPCM_ERROR_UNSUPPORTED;
			}
			
			skip( csize - 0x10 );
			break;
			
		case 0x61746164:	// data chunk
			wave_data = tell();
			loop1 = 0;
			skip( csize );
			wave_end = tell();
			if( format == WAV_FORMAT_PCM )
				loop2 = csize >> ( sampBits == 16 ? channels : ( channels - 1 ));
			else
				loop2 = csize << ( 2 - channels );
			break;
			
		case 0x6C706D73:	// sampler chunk
		{
			int s;
			skip( 28 );
			int nl = fget32();
			skip(4);
			s = 36;
			if( nl && loop) 
			{
				skip( 8 );
				loop1 = fget32() >> ( 2 - channels );
				loop2 = fget32() >> ( 2 - channels );
				s += 8+4+4;
			}
			skip( csize - s );
		}
			break;
		default:
			skip( csize );
		}
	}
	wave_loop = loop;
	oddnibble = 0;
	data.curSamps = 0;
	position = 0;
	seek( wave_data );
	currentblock = tell();
	state = state_beginblock;
	return IMA_ADPCM_OKAY;
}

int IMA_Adpcm_Stream::stream( s16 *target, int length )	__attribute__ ((optnone))
{
	/*
	if( format == WAV_FORMAT_PCM )
		return stream_pcm( target, length );
	else
	*/
	return decode_ima( target, length );
}

int IMA_Adpcm_Stream::stream_pcm( s16 *target, int length )	__attribute__ ((optnone))
{
	if( fin )
	{
		if( !wave_loop && ( currentblock >= wave_end ))
			return 1;
		while( length )
		{
			if( !wave_loop ) {
				if( position >= loop2 )
				{
					int i = length * channels;
					if( sampBits == 8 ) i >>= 1;
					for( ; i; i-- )
						*target++ = 0;
					length = 0;
					break;
				}
			}
			if( position == loop1 ) loop_cblock = currentblock;

			int iterations, cpysize;

			iterations = length;
			if( position < loop1 )
			{
				if( position + iterations >= loop1 )
					iterations = loop1-position;
			}
			else if( position < loop2 )
			{
				if( position + iterations >= loop2 )
					iterations = loop2-position;
			}
			cpysize = iterations << ( sampBits == 16 ? channels : ( channels - 1 ));
			fread( target, 1, cpysize, fin );
			length -= iterations;
			position += iterations;
			currentblock += cpysize;
			if( sampBits == 8 )
				target += iterations >> ( 2 - channels );
			else
				target += iterations * channels;

			
			if(( position == loop2 ) && wave_loop ) {
				seek( loop_cblock );
				currentblock = loop_cblock;
				position = loop1;
			}	
		}
	}
	return 0;
}

int IMA_Adpcm_Stream::decode_ima( s16 *target, int length )	__attribute__ ((optnone))
{
	if( fin )
	{
		if( !wave_loop && ( currentblock >= wave_end ))
			return 1;
		while(length)
		{
			switch( state )
			{
				case state_beginblock:
				{
					getblock();
					if( !wave_loop ) {
						if( position >= loop2 )
						{
							int i = length * channels;
							for( ; i; i-- ) {
								*target++ = 0;
							}
							length = 0;
							break;
						}
					}
					else
					{
						if( position == loop1 ) capture_frame();
						if( position >= loop2 ) restore_frame();
					}

					data.samp1 = (short int)get16();
					data.step1 = (short int)get16();

					*target++ = data.samp1;
					if( channels == 2 )
					{
						data.samp2 = (short int)get16();
						data.step2 = (short int)get16();
						*target++ = data.samp2;
					}
										
					blockremain -= 8;
					state = state_decode;
					
					length--;
					position++;
					
					if( position == loop1 ) capture_frame();
					if( position == loop2 ) restore_frame();
					
					break;
				}
				case state_decode:
				{
					int iterations;
					
					iterations = (length > blockremain ? blockremain : length);
					
					if( position < loop1 )
					{
						if( position + iterations >= loop1 )
							iterations = loop1-position;
					}
					else if( position < loop2 )
					{
						if( position + iterations >= loop2 )
							iterations = loop2-position;
					}
					
					if( channels == 2 )
					{
						src = srcb + data.curSamps;
						decode_stereo_ima_adpcm( &data, src, target, iterations );
						srcb += iterations;
						position += iterations;
					}
					else
					{
						src = srcb + ( data.curSamps >> 1 );
						decode_mono_ima_adpcm( &data, src, target, iterations );
						srcb += ( iterations >> 1 );
						position += ( iterations >> 1 );

						if( iterations & 0x1 )
						{
								oddnibble = !oddnibble;
								if( oddnibble )
								{
									srcb++;
									position++;
								}
						}
					}
					
					length -= iterations;
					blockremain -= iterations;
					target += ( iterations * channels );
					
					if( position == loop1 ) capture_frame();
					if( position == loop2 )
					{
						restore_frame();
						break;
					}
					
					if( blockremain == 0 )
						state = state_beginblock;
					break;
				}
			}
		}
	}
	return 0;
}

void IMA_Adpcm_Stream::close() __attribute__ ((optnone))	{
	if( fin ){
		fseek(fin, 0, SEEK_SET);
	}
	//Audio stop here....
	/*
	if(closeCb != NULL){
		closeCb();
	}
	*/
}

void IMA_Adpcm_Stream::capture_frame()	__attribute__ ((optnone))
{
	loop_data = data;
	loop_src = srcb;
	loop_oddnibble = oddnibble;
	loop_state = state;
	loop_br = blockremain;
	loop_cblock = currentblock;
}

void IMA_Adpcm_Stream::restore_frame()	__attribute__ ((optnone))
{
	seek( loop_cblock );
	getblock();
	data = loop_data;
	srcb = loop_src;
	oddnibble = loop_oddnibble;
	state = loop_state;
	blockremain = loop_br;
	position = loop1;
}

int IMA_Adpcm_Stream::fget8() __attribute__ ((optnone))	{
	u8 a[1];
	fread( a, 1, 1, fin );
	return a[0];
}

int IMA_Adpcm_Stream::fget16() __attribute__ ((optnone))	{
	return fget8() + (fget8() << 8);
}

u32 IMA_Adpcm_Stream::fget32() __attribute__ ((optnone))	{
	return fget16() | (fget16() << 16);
}

int IMA_Adpcm_Stream::get8() __attribute__ ((optnone))	{
	return *srcb++;
}

int IMA_Adpcm_Stream::get16()	__attribute__ ((optnone))	{
	return get8() | (get8() << 8);
}

u32 IMA_Adpcm_Stream::get32()	__attribute__ ((optnone))	{
	return get16() | (get16() << 16);
}

void IMA_Adpcm_Stream::getblock()	__attribute__ ((optnone))
{
	currentblock = tell();
	blockremain = block << ( 2 - channels );
	fread( datacache, 1, block, fin );
	srcb = datacache;
}

int IMA_Adpcm_Stream::get_channels()	__attribute__ ((optnone))	{
	return channels;
}

int IMA_Adpcm_Stream::get_sampling_rate()	__attribute__ ((optnone))	{
	return sampling_rate;
}

int IMA_Adpcm_Stream::get_format()	__attribute__ ((optnone))	{
	return (( format == WAV_FORMAT_PCM ? (( sampBits >> 4 ) << 1 ) : WAV_FORMAT_PCM ) + ( channels - 1 ));
}

/*********************************************
 *
 * [PLAYER: These are the implementation, not the INSTANCE. It must be instanced first from PLAYER object implementation]
 *
 *********************************************/

int on_stream_request( int length, void* dest, int format )	__attribute__ ((optnone))	{
	return player.i_stream_request( length, dest, format );
}

IMA_Adpcm_Player::IMA_Adpcm_Player()	__attribute__ ((optnone))	{
	active=false;
}

int IMA_Adpcm_Player::play( FILE * ADFileHandle, bool loop_audio, bool automatic_updates, int buffer_length, closeSoundHandle closeHandle )	__attribute__ ((optnone))	{
	active = false;
	stop();
	autofill = automatic_updates;
	int result = stream.reset( ADFileHandle, loop_audio );
	if( result ){
		return result;
	}
	
	stream.closeCb = closeHandle;
	paused = false;
	active=true;
	setvolume( 4 );
	// open stream
	cutOff = false;
	sndPaused = false;
	
	DisableSoundSampleContext();	//Disable ARM7 TGDS Sound stuff because decoders require a lot of power.
	
	// IMA-ADPCM file
	int fsize = FS_getFileSizeFromOpenHandle(ADFileHandle);
	ADPCMchunksize = buffer_length;
	soundData.channels = headerChunk.wChannels = ADPCMchannels = stream.get_channels();
	
	headerChunk.dwSamplesPerSec = stream.get_sampling_rate();
	headerChunk.wFormatTag = 1;
	headerChunk.wBitsPerSample = 16;	//Always signed 16 bit PCM out
	
	soundData.len = fsize;
	bufCursor = 0;
	soundData.loc = 0;
	soundData.dataOffset = ftell(ADFileHandle);
	soundData.filePointer = ADFileHandle;
	
	setSoundInterpolation(1);
	setSoundFrequency(headerChunk.dwSamplesPerSec);
	setWavDecodeCallback(IMAADPCMDecode);
	setSoundLength(ADPCMchunksize);		
	mallocData(ADPCMchunksize*2);
	IMAADPCMDecode();
	soundData.sourceFmt = SRC_WAV;
	internalCodecType = SRC_WAVADPCM;
	startSound9();
	return 0;
}

void IMA_Adpcm_Player::pause()	__attribute__ ((optnone))	{
	if( active )
		paused = true;
}

void IMA_Adpcm_Player::resume()	__attribute__ ((optnone))	{
	if( active )
		paused = false;
}

void IMA_Adpcm_Player::stop()	__attribute__ ((optnone))	{
	stream.close();
	active=false;
	setvolume( 0 );
}

void IMA_Adpcm_Player::setvolume( int vol )	__attribute__ ((optnone))	{
	setVolume(vol);
}

int IMA_Adpcm_Player::getvolume()	__attribute__ ((optnone))	{
	return getVolume();
}
bool IMA_Adpcm_Player::isactive()	__attribute__ ((optnone))	{
	return active;
}

IMA_Adpcm_Stream * IMA_Adpcm_Player::getStream()	__attribute__ ((optnone))	{
	return &stream;
}

int IMA_Adpcm_Player::i_stream_request( int length, void * dest, int format )	__attribute__ ((optnone))	{
	if( !paused ) {
		if( !stream.stream( (s16*)dest, length ))
		{
			
		}
		else{
			stop();
		}
	} else {
		s16 *d = (s16*)dest;
		int i = length * 2;
		for( ; i; i-- ) {
			*d++ = 0;
		}
	}
	return length;
}

void IMA_Adpcm_Player::update()	__attribute__ ((optnone))	{
	
}

volatile static s16 tmpData[ADPCM_SIZE * 2 * 2];	//ADPCM uses 1 src as decoding frame, and 2nd src as scratchpad + 2 channels

__attribute__((section(".itcm")))
void IMAADPCMDecode()	__attribute__ ((optnone))	{
	player.i_stream_request(ADPCMchunksize, (void*)&tmpData[0], WAV_FORMAT_IMA_ADPCM);
	coherent_user_range((uint32)tmpData, (uint32)(ADPCMchunksize* 2 * ADPCMchannels));
	if(soundData.channels == 2)
	{
		uint i=0;
		for(i=0;i<(ADPCMchunksize * 2);++i)
		{					
			lBuffer[i] = tmpData[i << 1];
			rBuffer[i] = tmpData[(i << 1) | 1];
		}
	}
	else
	{
		uint i=0;
		for(i=0;i<(ADPCMchunksize * 2);++i)
		{
			lBuffer[i] = tmpData[i];
			rBuffer[i] = tmpData[i];
		}
	}
}