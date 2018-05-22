#pragma once

extern "C" void g726_Encode(unsigned char *speech, char *bitstream);
extern "C" void g726_Decode(char *bitstream, unsigned char *speech);

extern  "C" void G729aCoder_Init();
extern  "C" void G729aCoder(short SpeechBuf[], short serial[]);
extern  "C" void G729aDecoder_Init();
extern  "C" void G729aDecoder(short serial[], short speech[]);


/* va_g729a API functions prototypes and constants */

#define  L_FRAME_COMPRESSED 10
#define  L_FRAME            80

extern  "C" void va_g729a_init_encoder();
extern  "C" void va_g729a_encoder(short *speech, unsigned char *bitstream);
extern  "C" void va_g729a_init_decoder();
extern  "C" void va_g729a_decoder(unsigned char *bitstream, short *synth_short, int bfi);
