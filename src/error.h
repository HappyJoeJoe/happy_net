#ifndef __ERROR_H__
#define __ERROR_H__

enum buffer_errcode
{
	kBUFFER_EOF		= -10001,
	kBUFFER_EAGAIN  = -10002,
	kBUFFER_ERROR   = -10003,
};

enum decoder_errcode
{
	kDECODER_AGAIN  = -20001,
	kDECODER_ERROR  = -20002,
};

#endif