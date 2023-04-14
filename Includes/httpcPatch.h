/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: httpcPatch.h
Open source lines: 320/320 (100.00%)
*****************************************************/

/**
 * @file httpcPatch.h
 * @brief HTTP service patched to not allocate memory.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "3ds/services/sslc.h"

	/// HTTP context.
	typedef struct {
		Handle servhandle; ///< Service handle.
		u32 httphandle;    ///< HTTP handle.
	} httpcPatchContext;

	/// HTTP request method.
	typedef enum {
		HTTPCPATCH_METHOD_GET = 0x1,
		HTTPCPATCH_METHOD_POST = 0x2,
		HTTPCPATCH_METHOD_HEAD = 0x3,
		HTTPCPATCH_METHOD_PUT = 0x4,
		HTTPCPATCH_METHOD_DELETE = 0x5
	} HTTPCPATCH_RequestMethod;

	/// HTTP request status.
	typedef enum {
		HTTPCPATCH_STATUS_REQUEST_IN_PROGRESS = 0x5, ///< Request in progress.
		HTTPCPATCH_STATUS_DOWNLOAD_READY = 0x7       ///< Download ready.
	} HTTPCPATCH_RequestStatus;

	/// HTTP KeepAlive option.
	typedef enum {
		HTTPCPATCH_KEEPALIVE_DISABLED = 0x0,
		HTTPCPATCH_KEEPALIVE_ENABLED = 0x1
	} HTTPCPATCH_KeepAlive;

	/// Result code returned when a download is pending.
#define HTTPCPATCH_RESULTCODE_DOWNLOADPENDING 0xd840a02b

// Result code returned when asked about a non-existing header.
#define HTTPCPATCH_RESULTCODE_NOTFOUND 0xd840a028

// Result code returned when any timeout function times out.
#define HTTPCPATCH_RESULTCODE_TIMEDOUT 0xd820a069

/// Initializes HTTPCPATCH. For HTTP GET the sharedmem_size can be zero. The sharedmem contains data which will be later uploaded for HTTP POST. sharedmem_size should be aligned to 0x1000-bytes.
	Result httpcPatchInit(u32 sharedmem_addr, u32 sharedmem_size);

	/// Exits HTTPCPATCH.
	void httpcPatchExit(void);

	/**
	 * @brief Opens a HTTP context.
	 * @param context Context to open.
	 * @param url URL to connect to.
	 * @param use_defaultproxy Whether the default proxy should be used (0 for default)
	 */
	Result httpcPatchOpenContext(httpcPatchContext* context, HTTPCPATCH_RequestMethod method, const char* url, u32 use_defaultproxy);

	/**
	 * @brief Closes a HTTP context.
	 * @param context Context to close.
	 */
	Result httpcPatchCloseContext(httpcPatchContext* context);

	/**
	 * @brief Cancels a HTTP connection.
	 * @param context Context to close.
	 */
	Result httpcPatchCancelConnection(httpcPatchContext* context);

	/**
	 * @brief Adds a request header field to a HTTP context.
	 * @param context Context to use.
	 * @param name Name of the field.
	 * @param value Value of the field.
	 */
	Result httpcPatchAddRequestHeaderField(httpcPatchContext* context, const char* name, const char* value);

	/**
	 * @brief Adds a POST form field to a HTTP context.
	 * @param context Context to use.
	 * @param name Name of the field.
	 * @param value Value of the field.
	 */
	Result httpcPatchAddPostDataAscii(httpcPatchContext* context, const char* name, const char* value);

	/**
	 * @brief Adds a POST form field with binary data to a HTTP context.
	 * @param context Context to use.
	 * @param name Name of the field.
	 * @param value The binary data to pass as a value.
	 * @param len Length of the binary data which has been passed.
	 */
	Result httpcPatchAddPostDataBinary(httpcPatchContext* context, const char* name, const u8* value, u32 len);

	/**
	 * @brief Adds a POST body to a HTTP context.
	 * @param context Context to use.
	 * @param data The data to be passed as raw into the body of the post request.
	 * @param len Length of data passed by data param.
	 */
	Result httpcPatchAddPostDataRaw(httpcPatchContext* context, const u32* data, u32 len);

	/**
	 * @brief Begins a HTTP request.
	 * @param context Context to use.
	 */
	Result httpcPatchBeginRequest(httpcPatchContext* context);

	/**
	 * @brief Receives data from a HTTP context.
	 * @param context Context to use.
	 * @param buffer Buffer to receive data to.
	 * @param size Size of the buffer.
	 */
	Result httpcPatchReceiveData(httpcPatchContext* context, u8* buffer, u32 size);

	/**
	 * @brief Receives data from a HTTP context with a timeout value.
	 * @param context Context to use.
	 * @param buffer Buffer to receive data to.
	 * @param size Size of the buffer.
	 * @param timeout Maximum time in nanoseconds to wait for a reply.
	 */
	Result httpcPatchReceiveDataTimeout(httpcPatchContext* context, u8* buffer, u32 size, u64 timeout);

	/**
	 * @brief Gets the request state of a HTTP context.
	 * @param context Context to use.
	 * @param out Pointer to output the HTTP request state to.
	 */
	Result httpcPatchGetRequestState(httpcPatchContext* context, HTTPCPATCH_RequestStatus* out);

	/**
	 * @brief Gets the download size state of a HTTP context.
	 * @param context Context to use.
	 * @param downloadedsize Pointer to output the downloaded size to.
	 * @param contentsize Pointer to output the total content size to.
	 */
	Result httpcPatchGetDownloadSizeState(httpcPatchContext* context, u32* downloadedsize, u32* contentsize);

	/**
	 * @brief Gets the response code of the HTTP context.
	 * @param context Context to get the response code of.
	 * @param out Pointer to write the response code to.
	 */
	Result httpcPatchGetResponseStatusCode(httpcPatchContext* context, u32* out);

	/**
	 * @brief Gets the response code of the HTTP context with a timeout value.
	 * @param context Context to get the response code of.
	 * @param out Pointer to write the response code to.
	 * @param timeout Maximum time in nanoseconds to wait for a reply.
	 */
	Result httpcPatchGetResponseStatusCodeTimeout(httpcPatchContext* context, u32* out, u64 timeout);

	/**
	 * @brief Gets a response header field from a HTTP context.
	 * @param context Context to use.
	 * @param name Name of the field.
	 * @param value Pointer to output the value of the field to.
	 * @param valuebuf_maxsize Maximum size of the value buffer.
	 */
	Result httpcPatchGetResponseHeader(httpcPatchContext* context, const char* name, char* value, u32 valuebuf_maxsize);

	/**
	 * @brief Adds a trusted RootCA cert to a HTTP context.
	 * @param context Context to use.
	 * @param cert Pointer to DER cert.
	 * @param certsize Size of the DER cert.
	 */
	Result httpcPatchAddTrustedRootCA(httpcPatchContext* context, const u8* cert, u32 certsize);

	/**
	 * @brief Adds a default RootCA cert to a HTTP context.
	 * @param context Context to use.
	 * @param certID ID of the cert to add, see sslc.h.
	 */
	Result httpcPatchAddDefaultCert(httpcPatchContext* context, SSLC_DefaultRootCert certID);

	/**
	 * @brief Sets the RootCertChain for a HTTP context.
	 * @param context Context to use.
	 * @param RootCertChain_contexthandle Contexthandle for the RootCertChain.
	 */
	Result httpcPatchSelectRootCertChain(httpcPatchContext* context, u32 RootCertChain_contexthandle);

	/**
	 * @brief Sets the ClientCert for a HTTP context.
	 * @param context Context to use.
	 * @param cert Pointer to DER cert.
	 * @param certsize Size of the DER cert.
	 * @param privk Pointer to the DER private key.
	 * @param privk_size Size of the privk.
	 */
	Result httpcPatchSetClientCert(httpcPatchContext* context, const u8* cert, u32 certsize, const u8* privk, u32 privk_size);

	/**
	 * @brief Sets the default clientcert for a HTTP context.
	 * @param context Context to use.
	 * @param certID ID of the cert to add, see sslc.h.
	 */
	Result httpcPatchSetClientCertDefault(httpcPatchContext* context, SSLC_DefaultClientCert certID);

	/**
	 * @brief Sets the ClientCert contexthandle for a HTTP context.
	 * @param context Context to use.
	 * @param ClientCert_contexthandle Contexthandle for the ClientCert.
	 */
	Result httpcPatchSetClientCertContext(httpcPatchContext* context, u32 ClientCert_contexthandle);

	/**
	 * @brief Sets SSL options for the context.
	 * The HTTPCPATCH SSL option bits are the same as those defined in sslc.h
	 * @param context Context to set flags on.
	 * @param options SSL option flags.
	 */
	Result httpcPatchSetSSLOpt(httpcPatchContext* context, u32 options);

	/**
	 * @brief Sets the SSL options which will be cleared for the context.
	 * The HTTPCPATCH SSL option bits are the same as those defined in sslc.h
	 * @param context Context to clear flags on.
	 * @param options SSL option flags.
	 */
	Result httpcPatchSetSSLClearOpt(httpcPatchContext* context, u32 options);

	/**
	 * @brief Creates a RootCertChain. Up to 2 RootCertChains can be created under this user-process.
	 * @param RootCertChain_contexthandle Output RootCertChain contexthandle.
	 */
	Result httpcPatchCreateRootCertChain(u32* RootCertChain_contexthandle);

	/**
	 * @brief Destroy a RootCertChain.
	 * @param RootCertChain_contexthandle RootCertChain to use.
	 */
	Result httpcPatchDestroyRootCertChain(u32 RootCertChain_contexthandle);

	/**
	 * @brief Adds a RootCA cert to a RootCertChain.
	 * @param RootCertChain_contexthandle RootCertChain to use.
	 * @param cert Pointer to DER cert.
	 * @param certsize Size of the DER cert.
	 * @param cert_contexthandle Optional output ptr for the cert contexthandle(this can be NULL).
	 */
	Result httpcPatchRootCertChainAddCert(u32 RootCertChain_contexthandle, const u8* cert, u32 certsize, u32* cert_contexthandle);

	/**
	 * @brief Adds a default RootCA cert to a RootCertChain.
	 * @param RootCertChain_contexthandle RootCertChain to use.
	 * @param certID ID of the cert to add, see sslc.h.
	 * @param cert_contexthandle Optional output ptr for the cert contexthandle(this can be NULL).
	 */
	Result httpcPatchRootCertChainAddDefaultCert(u32 RootCertChain_contexthandle, SSLC_DefaultRootCert certID, u32* cert_contexthandle);

	/**
	 * @brief Removes a cert from a RootCertChain.
	 * @param RootCertChain_contexthandle RootCertChain to use.
	 * @param cert_contexthandle Contexthandle of the cert to remove.
	 */
	Result httpcPatchRootCertChainRemoveCert(u32 RootCertChain_contexthandle, u32 cert_contexthandle);

	/**
	 * @brief Opens a ClientCert-context. Up to 2 ClientCert-contexts can be open under this user-process.
	 * @param cert Pointer to DER cert.
	 * @param certsize Size of the DER cert.
	 * @param privk Pointer to the DER private key.
	 * @param privk_size Size of the privk.
	 * @param ClientCert_contexthandle Output ClientCert context handle.
	 */
	Result httpcPatchOpenClientCertContext(const u8* cert, u32 certsize, const u8* privk, u32 privk_size, u32* ClientCert_contexthandle);

	/**
	 * @brief Opens a ClientCert-context with a default clientclient. Up to 2 ClientCert-contexts can be open under this user-process.
	 * @param certID ID of the cert to add, see sslc.h.
	 * @param ClientCert_contexthandle Output ClientCert context handle.
	 */
	Result httpcPatchOpenDefaultClientCertContext(SSLC_DefaultClientCert certID, u32* ClientCert_contexthandle);

	/**
	 * @brief Closes a ClientCert context.
	 * @param ClientCert_contexthandle ClientCert context to use.
	 */
	Result httpcPatchCloseClientCertContext(u32 ClientCert_contexthandle);

	/**
	 * @brief Downloads data from the HTTP context into a buffer.
	 * The *entire* content must be downloaded before using httpcPatchCloseContext(), otherwise httpcPatchCloseContext() will hang.
	 * @param context Context to download data from.
	 * @param buffer Buffer to write data to.
	 * @param size Size of the buffer.
	 * @param downloadedsize Pointer to write the size of the downloaded data to.
	 */
	Result httpcPatchDownloadData(httpcPatchContext* context, u8* buffer, u32 size, u32* downloadedsize);

	/**
	 * @brief Sets Keep-Alive for the context.
	 * @param context Context to set the KeepAlive flag on.
	 * @param option HTTPCPATCH_KeepAlive option.
	 */
	Result httpcPatchSetKeepAlive(httpcPatchContext* context, HTTPCPATCH_KeepAlive option);

#ifdef __cplusplus
}
#endif