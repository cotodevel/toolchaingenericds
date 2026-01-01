#ifdef ARM9
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
IMA_Adpcm_Stream::IMA_Adpcm_Stream()
{
	
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
IMA_Adpcm_Stream::~IMA_Adpcm_Stream()
{
	
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::reset( FILE * audioFileHandle, bool loop )
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::stream( s16 *target, int length )
{
	if( format == WAV_FORMAT_PCM )
		return stream_pcm( target, length );
	else
		return decode_ima( target, length );
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::stream_pcm( s16 *target, int length )
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::decode_ima( s16 *target, int length )	
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void IMA_Adpcm_Stream::close() {
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void IMA_Adpcm_Stream::capture_frame()	
{
	loop_data = data;
	loop_src = srcb;
	loop_oddnibble = oddnibble;
	loop_state = state;
	loop_br = blockremain;
	loop_cblock = currentblock;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void IMA_Adpcm_Stream::restore_frame()	
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::fget8() {
	u8 a[1];
	fread( a, 1, 1, fin );
	return a[0];
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::fget16() {
	return fget8() + (fget8() << 8);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
u32 IMA_Adpcm_Stream::fget32() {
	return fget16() | (fget16() << 16);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::get8() {
	return *srcb++;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::get16() {
	return get8() | (get8() << 8);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
u32 IMA_Adpcm_Stream::get32()	{
	return get16() | (get16() << 16);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void IMA_Adpcm_Stream::getblock()	
{
	currentblock = tell();
	blockremain = block << ( 2 - channels );
	fread( datacache, 1, block, fin );
	srcb = datacache;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::get_channels(){
	return channels;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::get_sampling_rate(){
	return sampling_rate;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Stream::get_format()	{
	return (( format == WAV_FORMAT_PCM ? (( sampBits >> 4 ) << 1 ) : WAV_FORMAT_PCM ) + ( channels - 1 ));
}

/*********************************************
 *
 * [PLAYER: These are the implementation, not the INSTANCE. It must be instanced first from PLAYER object implementation]
 *
 *********************************************/

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int on_stream_request( int length, void* dest )	{
	return player.i_stream_request( length, dest );
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
IMA_Adpcm_Player::IMA_Adpcm_Player(){
	active=false;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Player::play( FILE * ADFileHandle, bool loop_audio, bool automatic_updates, int buffer_length, closeSoundHandle closeHandle, uint32 sourceBuf)	{
	if(sourceBuf == 0){
		printf("IMA-ADPCM failure");
		while(1==1){}
		return -1;
	}
	
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
	
	// IMA-ADPCM file
	int fsize = FS_getFileSizeFromOpenHandle(ADFileHandle);
	ADPCMchunksize = buffer_length;
	soundData.channels = headerChunk.wChannels = ADPCMchannels = stream.get_channels();
	
	headerChunk.dwSamplesPerSec = stream.get_sampling_rate();
	headerChunk.wFormatTag = 1;
	headerChunk.wBitsPerSample = 16;	//Always signed 16 bit PCM out
	
	soundData.len = fsize;
	soundData.loc = 0;
	soundData.dataOffset = ftell(ADFileHandle);
	soundData.filePointer = ADFileHandle;
	
	setSoundInterpolation(1);
	setSoundFrequency(headerChunk.dwSamplesPerSec);
	setWavDecodeCallback(WAV_IMAADPCM_Decoder);
	setSoundLength(ADPCMchunksize);		
	mallocData(ADPCMchunksize*2);
	wavDecode();
	internalCodecType = soundData.sourceFmt = SRC_WAVADPCM;
	startSound9(sourceBuf);
	return 0;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void IMA_Adpcm_Player::pause()	{
	if( active )
		paused = true;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void IMA_Adpcm_Player::resume()	{
	if( active )
		paused = false;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void IMA_Adpcm_Player::stop()	{
	stream.close();
	active=false;
	setvolume( 0 );
	cutOff = true; //notify the TGDS sound stream context ADPCM playback ended
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void IMA_Adpcm_Player::setvolume( int vol )	{
	setVolume(vol);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Player::getvolume()	{
	return getVolume();
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool IMA_Adpcm_Player::isactive()	{
	return active;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
IMA_Adpcm_Stream * IMA_Adpcm_Player::getStream(){
	return &stream;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int IMA_Adpcm_Player::i_stream_request( int length, void * dest){
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

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void IMA_Adpcm_Player::update()	{
	
}

volatile static s16 tmpData[ADPCM_SIZE * 2 * 2];	//ADPCM uses 1 src as decoding frame, and 2nd src as scratchpad + 2 channels

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WAV_IMAADPCM_Decoder(){

	player.i_stream_request(ADPCMchunksize, (void*)&tmpData[0]);
	coherent_user_range((uint32)tmpData, (uint32)(ADPCMchunksize* 2 * ADPCMchannels));
	if(soundData.channels == 2)
	{
		int i=0;
		for(i=0; i<(ADPCMchunksize * 2);++i)
		{					
			lBuffer[i] = tmpData[i << 1];
			rBuffer[i] = tmpData[(i << 1) | 1];
		}
	}
	else
	{
		int i=0;
		for(i=0;i<(ADPCMchunksize * 2);++i)
		{
			lBuffer[i] = tmpData[i];
			rBuffer[i] = tmpData[i];
		}
	}
}
#endif