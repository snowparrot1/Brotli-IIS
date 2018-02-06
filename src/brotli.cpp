// brotli.cpp : Defines the exported functions for the HTTP Compression Scheme.  See httpcompression.h from the SDK

#include "stdafx.h"

// Startup code, called once as soon as compression scheme dll is loaded by IIS compression module
DLL_EXPORT HRESULT WINAPI InitCompression(	VOID)
{
	return S_OK;
}

// Shutdown code, called before compression scheme dll is unloaded by IIS compression module
DLL_EXPORT VOID WINAPI DeInitCompression(VOID)
{
}

// Create a new compression context, called once per request
DLL_EXPORT HRESULT WINAPI CreateCompression(OUT PVOID *context, IN ULONG /*reserved*/)
{
	*context = ::BrotliEncoderCreateInstance(NULL, NULL, NULL);
	return *context ? S_OK : E_FAIL;
}

// Destroy compression context, called at the end of each request
DLL_EXPORT VOID WINAPI DestroyCompression(IN PVOID context)
{
	::BrotliEncoderDestroyInstance((BrotliEncoderState*)context);
}

// Compress data, called in a loop until full response is processed.
DLL_EXPORT HRESULT WINAPI Compress(
	IN  OUT PVOID           context,            // compression context
	IN      CONST BYTE *    input_buffer,       // input buffer
	IN      LONG            input_buffer_size,  // size of input buffer
	IN      PBYTE           output_buffer,      // output buffer
	IN      LONG            output_buffer_size, // size of output buffer
	OUT     PLONG           input_used,         // amount of input buffer used
	OUT     PLONG           output_used,        // amount of output buffer used
	IN      INT             compression_level   // compression level (0..11)
)
{
	if (compression_level < BROTLI_MIN_QUALITY || compression_level > BROTLI_MAX_QUALITY)
		return E_INVALIDARG;

	auto encoderState = (BrotliEncoderState*)context;

	// This should succeed on the first call and fail once any data has been processed.
	// Since IIS won't change compression level between calls, we can ignore the return.
	::BrotliEncoderSetParameter(encoderState, BROTLI_PARAM_QUALITY, compression_level);

	*input_used = 0;
	*output_used = 0;

	size_t size_in = input_buffer_size;
	size_t size_out = output_buffer_size;
	CONST BYTE* next_in = input_buffer;
	PBYTE next_out = output_buffer;

	// Compress input data.  If input is empty, we're at the end of the stream, so finish up.
	auto operation = input_buffer_size ? BROTLI_OPERATION_PROCESS : BROTLI_OPERATION_FINISH;
	if (!::BrotliEncoderCompressStream(encoderState, operation, &size_in, &next_in, &size_out, &next_out, NULL))
		return E_FAIL;

	*input_used = input_buffer_size - (LONG)size_in;
	*output_used = output_buffer_size - (LONG)size_out;

	// Return S_OK to continue looping, S_FALSE to complete the response.
	return input_buffer_size || !::BrotliEncoderIsFinished(encoderState) ? S_OK : S_FALSE;
}
